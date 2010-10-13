// Methods for CGPXParser

#include "stdafx.h"
#include "CGPXParser.h"

/**************************************************************************/
/**************************** CGPXParser **********************************/
/**************************************************************************/

CGPXParser::CGPXParser(TCHAR *fileNameIn) :
nItem(0),
szFileName(fileNameIn),
lat(0),
lon(0),
ele(0),
name(NULL),
desc(NULL),
sym(NULL),
time(NULL)
{
	clear();
}

CGPXParser::~CGPXParser(void)
{
	clear();
}

void CGPXParser::clear(void)
{
	lat=lon=ele=0;
	if(name) {
		delete [] name;
		name=NULL;
	}
	if(desc) {
		delete [] desc;
		desc=NULL;
	}
	if(sym) {
		delete [] sym;
		sym=NULL;
	}
}

BOOL CGPXParser::validate(TCHAR *type, unsigned flag)
{
	if(flag & GPX_CHECK_NAME) {
		if(!name) {
			errMsg(_T("%s[%d]: name not found:\n%s"),
				type,nItem,szFileName);
			return FALSE;
		}
	}
	if(flag & GPX_CHECK_DESC) {
		if(!desc) {
			errMsg(_T("%s[%d]: desc not found:\n%s"),
				type,nItem,szFileName);
			return FALSE;
		}
	}
	if(flag & GPX_CHECK_SYM) {
		if(!sym) {
			errMsg(_T("%s[%d]: sym not found:\n%s"),
				type,nItem,szFileName);
			return FALSE;
		}
	}
	if(flag & GPX_CHECK_TIME) {
		if(!time) {
			errMsg(_T("%s[%d]: time not found:\n%s"),
				type,nItem,szFileName);
			return FALSE;
		}
	}
	return TRUE;
}

CWaypoint *CGPXParser::createWaypoint(BOOL checkedIn) {
	if(!validate(_T("wpt"), GPX_CHECK_NAME|GPX_CHECK_SYM|
		GPX_CHECK_LAT|GPX_CHECK_LON|GPX_CHECK_ELE)) {
			return NULL;
	}
	CWaypoint *waypoint=new CWaypoint(checkedIn,name,lat,lon,ele,sym);
	return waypoint;
}

CRoute *CGPXParser::createRoute(BOOL checkedIn) {
	if(!validate(_T("rte"), GPX_CHECK_NAME)) {
		return NULL;
	}
	CRoute *route=new CRoute(FALSE,name);
	return route;
}

CTrack *CGPXParser::createTrack(BOOL checkedIn) {
	if(!validate(_T("trk"), GPX_CHECK_NAME)) {
		return NULL;
	}
	CTrack *track=new CTrack(checkedIn,name);
	return track;
}

CTrackpoint *CGPXParser::createTrackpoint(BOOL checkedIn, BYTE startTrack) {
	if(!validate(_T("trkpt"), GPX_CHECK_LAT|GPX_CHECK_LON|GPX_CHECK_ELE)) {
		return NULL;
	}
	if(time) {
		time_t unixTime = parseTime(time, NULL);
		if(unixTime) {
			CTrackpoint *trackpoint=new CTrackpoint(checkedIn,startTrack,
				lat,lon,ele,unixTime);
			return trackpoint;
		}
	}
	// Handle failure to get the time
	char *timeString="GarminTime0";
	CTrackpoint *trackpoint=new CTrackpoint(checkedIn,startTrack,
		lat,lon,ele,timeString);
	return trackpoint;
}


void CGPXParser::parse(DOMNode *node)
{
	clear();
	nItem++;
	if(node->hasAttributes()) {
		DOMNamedNodeMap *pAttributes = node->getAttributes();
		int nSize = pAttributes->getLength();
		for(int i=0; i < nSize; i++) {
			DOMAttr *pAttributeNode=(DOMAttr*)pAttributes->item(i);
			char *attrName = XMLString::transcode(pAttributeNode->getName());
			char *attrValue = XMLString::transcode(pAttributeNode->getValue());
			if(!strcmp(attrName,"lat")) {
				lat=atof(attrValue);
			} else if(!strcmp(attrName,"lon")) {
				lon=atof(attrValue);
			}
			XMLString::release(&attrName);
			XMLString::release(&attrValue);
		}
	}
	for(DOMNode *node1 = node->getFirstChild(); node1 != 0; node1=node1->getNextSibling()) {
		if (node1->getNodeType() != DOMNode::ELEMENT_NODE) continue;
		char *name1 = XMLString::transcode(node1->getNodeName());
		if(!strcmp(name1,"ele")) {
			char *value = XMLString::transcode(node1->getTextContent());
			if(value) ele=atof(value)*M2FT;
			XMLString::release(&value);
		} else if(!strcmp(name1,"name")) {
			char *value = XMLString::transcode(node1->getTextContent());
			if(value) {
				int len=strlen(value);
				name=new char[len+1];
				if(name) {
					strcpy(name,value);
				}
			}
			XMLString::release(&value);
		} else if(!strcmp(name1,"desc")) {
			char *value = XMLString::transcode(node1->getTextContent());
			if(value) {
				int len=strlen(value);
				desc=new char[len+1];
				if(desc) {
					strcpy(desc,value);
				}
			}
			XMLString::release(&value);
		} else if(!strcmp(name1,"sym")) {
			char *value = XMLString::transcode(node1->getTextContent());
			if(value) {
				// Form is symbol, color
				// Just use the part up to the first comma
				// TODO: Handle the color too
				char *offsetstr = strchr(value, ',' );
				if ( offsetstr ) {
					*offsetstr = '\0';
				}
				int len=strlen(value);
				sym=new char[len+1];
				if(sym) {
					strcpy(sym,value);
				}
			}
			XMLString::release(&value);
		} else if(!strcmp(name1,"time")) {
			char *value = XMLString::transcode(node1->getTextContent());
			if(value) {
				int len=strlen(value);
				time=new char[len+1];
				if(time) {
					strcpy(time,value);
				}
			}
			XMLString::release(&value);
		}
		XMLString::release(&name1);
	}
}

time_t CGPXParser::parseTime(const char *cdatastr, int *microsecs) 
{
	int off_hr = 0;
	int off_min = 0;
	int off_sign = 1;
	char *offsetstr = NULL;
	char *pointstr = NULL;
	struct tm tm;
	time_t rv = 0;
	char *timestr = strdup(cdatastr);

	memset(&tm, 0, sizeof(tm));

	offsetstr = strchr( timestr, 'Z' );
	if ( offsetstr ) {
		/* zulu time; offsets stay at defaults */
		*offsetstr = '\0';
	} else {
		offsetstr = strchr( timestr, '+' );
		if ( offsetstr ) {
			/* positive offset; parse it */
			*offsetstr = '\0';
			sscanf( offsetstr+1, "%d:%d", &off_hr, &off_min );
		} else {
			offsetstr = strchr( timestr, 'T' );
			if ( offsetstr ) {
				offsetstr = strchr( offsetstr, '-' );
				if ( offsetstr ) {
					/* negative offset; parse it */
					*offsetstr = '\0';
					sscanf( offsetstr+1, "%d:%d", 
						&off_hr, &off_min );
					off_sign = -1;
				}
			}
		}
	}

	pointstr = strchr( timestr, '.' );
	if ( pointstr ) {
		if (microsecs) {
			double fsec;
			sscanf(pointstr, "%le", &fsec);
			/* Round to avoid FP jitter */
			*microsecs = (int)(.5 + (fsec * 1000000.0));
		}
		*pointstr = '\0';
	}

	sscanf(timestr, "%d-%d-%dT%d:%d:%d", 
		&tm.tm_year,
		&tm.tm_mon,
		&tm.tm_mday,
		&tm.tm_hour,
		&tm.tm_min,
		&tm.tm_sec);
	tm.tm_mon -= 1;
	tm.tm_year -= 1900;
	tm.tm_isdst = 0;

	rv = mkgmtime(&tm) - off_sign*off_hr*3600 - off_sign*off_min*60;

	free(timestr);

	return rv;
}

time_t CGPXParser::mkgmtime(struct tm *t)
{
	short  month, year;
	time_t result;
	static int      m_to_d[12] =
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	month = t->tm_mon;
	year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	result = (year - 1970) * 365 + m_to_d[month];
	if (month <= 1)
		year -= 1;
	result += (year - 1968) / 4;
	result -= (year - 1900) / 100;
	result += (year - 1600) / 400;
	result += t->tm_mday;
	result -= 1;
	result *= 24;
	result += t->tm_hour;
	result *= 60;
	result += t->tm_min;
	result *= 60;
	result += t->tm_sec;
	return(result);
}
