// Methods for CData

#include "stdafx.h"

#define DEBUG_UPLOAD 0

#include "GPSLink.h"

#define DATA_BUFSIZE 512
#define DLE 16
#define ETX 3

#define LAT_TOL (.00001/SEMI2DEG)
#define LON_TOL (.00001/SEMI2DEG)
#define ALT_TOL 5.

// Function prototypes
static BYTE *storeVariableArray(BYTE *next, char **array);
static BYTE *retrieveVariableArray(BYTE *next, char **array);
static BOOL copyVariableArray(char **newArray, char *array);

// Global variables
static const TCHAR fixName[6][9]={
	_T("Unusable"),_T("Invalid"),
	_T("2D"),_T("3D"),
	_T("2D Diff"),_T("3D Diff")
};
static const BYTE default_class[18]={
	0x00,0x00,
	0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,
};			       
static const char typeName[5][2]={
	"W",
	"R",
	"H",
	"T",
	"C",
};

// Utilities
static BYTE *storeVariableArray(BYTE *next, char **array)
{
	char *ptr=(char *)next;
	int len;
	len=strlen(ptr)+1;
	*array=new char[len];
	if(!*array) return NULL;
	strcpy(*array,ptr);
	return (BYTE *)(ptr+len);
}

static BYTE *retrieveVariableArray(BYTE *next, char **array)
{
	char *ptr=(char *)next;
	int len;

	if(*array) {
		// String is allocated
		len=strlen(*array)+1;
		strcpy(ptr,*array);
	} else {
		// String has not been allocated
		len=1;
		*ptr='\0';
	}

	return (BYTE *)(ptr+len);
}

static BOOL copyVariableArray(char **newArray, char *array)
{
	int len;

	if(array) {
		// Allocate a new string
		len=strlen(array)+1;
		*newArray=new char[len];
		if(!*newArray) {
			lbprintf(_T("copyVariableArray: Cannot create array"));
			return FALSE;
		} else {
			strncpy(*newArray,array,len);
		}
	} else {
		// String has not been allocated
		*newArray=NULL;
	}

	return TRUE;
}

/**************************************************************************/
/**************************** CBlockList ***************************************/
/**************************************************************************/

CBlockList::CBlockList(void)
{
}

CBlockList::CBlockList(const CBlockList &old)
{
	// Copy the data from the old one
	copy(old);
}

CBlockList::~CBlockList(void)
{
	// Clear the blockList and delete the blocks
	empty(TRUE);
}

// Note this is designed to handle equal to either a tsDLLlist<CBLock>
// or a CBlockList
CBlockList &CBlockList::operator=(const tsDLList<CBlock> &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there
	this->empty(TRUE);

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

// Note this is designed to handle copy from either a
// tsDLLlist<CBLock> or a CBlockList
BOOL CBlockList::copy(const tsDLList<CBlock> &old)
{
	// Add the blocks into the new blocklist
	tsDLIterBD<CBlock> iter(old.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		pBlock=pBlock->clone();

		// Insert the new copy
		if(!pBlock) {
			lbprintf(_T("CBlocklist: Cannot create block"));
			return FALSE;
		} else {
			this->addBlock(pBlock);
		}
		++iter;
	}

	return TRUE;
}

#if 0
// Returns a pointer to the block at position pos.  If remove is true,
// removes it from the list.
CBlock *CBlockList::getBlock(const int pos, BOOL removeBlock)
{
	int i=1;     // First position is 1

	// Find the block by parsing the list (inefficient)
	tsDLIterBD<CBlock> iter(blockList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		if(i == pos) {
			CBlock *pBlock=iter;
			if(removeBlock) {
				blockList.remove(*pBlock);
				pBlock->setParentList(NULL);
			}
			return pBlock;
		}
		++i;
		++iter;
	}
	return NULL;
}
#endif

// Removes a block from the list and optionally deletes it
BOOL CBlockList::removeBlock(CBlock *pBlock, BOOL deleteBlock)
{
	int found=0;

	// Find the block by parsing the list (inefficient)
	CBlock *tmp=tsDLList<CBlock>::first();
	tsDLIterBD<CBlock> iter(tsDLList<CBlock>::first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		if((CBlock *)iter == pBlock) {
			found=1;
			break;
		}
		tmp=iter;
		++iter;
	}

	if(found) {
		tsDLList<CBlock>::remove(*pBlock);
		pBlock->setParentList(NULL);
		if(deleteBlock) {
			delete pBlock;
			pBlock=NULL;
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CBlockList::addBlock(CBlock *pBlock)
{
	tsDLList<CBlock>::add(*pBlock);
	pBlock->setParentList(this);

	return TRUE;
}

BOOL CBlockList::insertBlockBefore(CBlock *pBlock, CBlock *pTarget)
{
	tsDLIterBD<CBlock> eol;

	if(!pBlock) return FALSE;

	if(!count()) {
		tsDLList<CBlock>::add(*pBlock);
	} else {
		if(!pTarget) return FALSE;
		tsDLList<CBlock>::insertBefore(*pBlock,*pTarget);
	}
	pBlock->setParentList(this);

	return TRUE;
}

BOOL CBlockList::insertBlockAfter(CBlock *pBlock, CBlock *pTarget)
{
	tsDLIterBD<CBlock> eol;

	if(!pBlock) return FALSE;

	if(!count()) {
		tsDLList<CBlock>::add(*pBlock);
	} else {
		if(!pTarget) return FALSE;
		tsDLList<CBlock>::insertAfter(*pBlock,*pTarget);
	}
	pBlock->setParentList(this);

	return TRUE;
}

void CBlockList::empty(BOOL removeBlocks)
{
	// Clear the blockList and delete the blocks if specified
	while(getCount() > 0) {
		CBlock *pLast=tsDLList<CBlock>::last();
		tsDLList<CBlock>::remove(*pLast);
		pLast->setParentList(NULL);
		if(removeBlocks) delete pLast;
	}
}

int CBlockList::getPos(const CBlock *pBlock) const
{
	int i=1;     // First position is 1

	// Find the block by parsing the list (inefficient)
	tsDLIterBD<CBlock> iter(tsDLList<CBlock>::first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		if(iter == pBlock) {
			return i;
		}
		++i;
		++iter;
	}
	return 0;
}

// Prints out the items in the block list.  Intended for debugging.
void CBlockList::dump(void) const
{
	int pos=1;

	lbprintf(_T("Dumping blockList with count=%d\n"),
		tsDLList<CBlock>::count());
	tsDLIterBD<CBlock> iter(tsDLList<CBlock>::first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=(CBlock *)iter;
		lbprintf(_T("  Block %d: Type=%d Address=%x"),
			pos,iter->getType(),pBlock);
		++iter;
		++pos;
	}
}

/**************************************************************************/
/**************************** CBlock ***************************************/
/**************************************************************************/

CBlock::CBlock(DataType typeIn) :
parentList(NULL),
type(typeIn),
index(0),
checked(FALSE),
distance(0.0)
{
}

CBlock::CBlock(const CBlock &old)
{
	// Copy the data from the old one
	copy(old);
}

CBlock::~CBlock(void)
{
	if(parentList) {
		BOOL status;

		// Remove the block from the blocklist without deleting it
		if(parentList) {
			status=parentList->removeBlock(this,FALSE);
			parentList=NULL;
			if(!status) errMsg(_T("Failed to remove block"));
		}
	}
}

CBlock &CBlock::operator=(const CBlock &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CBlock::copy(const CBlock &old)
{
	parentList=NULL;  // Notice that it is not a copy
	type=old.type;
	index=old.index;
	distance=old.distance;
	checked=old.checked;

	return TRUE;
}

/**************************************************************************/
/**************************** CWaypoint ***********************************/
/**************************************************************************/

CWaypoint::CWaypoint(BOOL checkedIn) :
CBlock(TYPE_WPT),
wpt_class(0x00),
color(0xFF),
dspl(0),
attr(0x60),
smbl(178),  // Flag
lat(0),
lon(0),
alt(0.0),
dpth(0.0),
dist(0.0),
ident(NULL),
comment(NULL),
facility(NULL),
city(NULL),
addr(NULL),
cross_road(NULL),
link(FALSE),
link_class(3),
link_ident(NULL)
{
	// Set checked
	checked=checkedIn;

	// Initialize subclass and link subclass to default subclass
	for(int i=0; i<18; i++) subclass[i]=link_subclass[i]=default_class[i];

	// Initialize arrays
	state[0]=state[1]=' ';
	cc[0]=cc[1]=' ';
}


CWaypoint::CWaypoint(BOOL checkedIn, char *name, double latitude,
					 double longitude, double altitude, char *symbol) :
CBlock(TYPE_WPT),
wpt_class(0x00),
color(0xFF),
dspl(0),
attr(0x60),
smbl(178),  // Flag
lat(0),
lon(0),
alt(0.0),
dpth(0.0),
dist(0.0),
ident(NULL),
comment(NULL),
facility(NULL),
city(NULL),
addr(NULL),
cross_road(NULL),
link(FALSE),
link_class(3),
link_ident(NULL)
{
	// Set checked
	checked=checkedIn;

	// Initialize subclass and link subclass to default subclass
	for(int i=0; i<18; i++) subclass[i]=link_subclass[i]=default_class[i];

	// Initialize arrays
	state[0]=state[1]=' ';
	cc[0]=cc[1]=' ';

	if(name) {
		int len=strlen(name)+1;
		ident=new char[len];
		if(ident) {
			strcpy(ident,name);
		}
	} else {
		ident=new char[1];
		*ident='\0';
	}
	lat=(long)(latitude/SEMI2DEG+.5);
	lon=(long)(longitude/SEMI2DEG+.5);
	alt=(float)(altitude/M2FT);
	smbl=getSymbolNumber(symbol);    
}

CWaypoint::CWaypoint(const CWaypoint &old) :
CBlock(old)
{
	// Copy the data from the old one
	copy(old);
}

CWaypoint::~CWaypoint(void)
{
	if(ident) delete [] ident;
	if(comment) delete [] comment;
	if(facility) delete [] facility;
	if(city) delete [] city;
	if(addr) delete [] addr;
	if(cross_road) delete [] cross_road;
	if(link_ident) delete [] link_ident;
}

CWaypoint &CWaypoint::operator=(const CWaypoint &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there
	delete [] ident;
	delete [] comment;
	delete [] facility;
	delete [] city;
	delete [] addr;
	delete [] cross_road;
	delete [] link_ident;

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CWaypoint::copy(const CWaypoint &old)
{
	int i;
	BOOL status;

	CBlock::copy(old);

	wpt_class=old.wpt_class;
	color=old.color; 
	dspl=old.dspl; 
	attr=old.attr; 
	smbl=old.smbl; 
	for(i=0; i < 18; i++) subclass[i]=old.subclass[i];
	lat=old.lat;
	lon=old.lon;
	alt=old.alt;
	dpth=old.dpth; 
	dist=old.dist; 
	for(i=0; i < 2; i++) state[i]=old.state[i];
	for(i=0; i < 2; i++) cc[i]=old.cc[i];
	status=copyVariableArray(&ident,old.ident);
	if(!status) return FALSE;
	status=copyVariableArray(&comment,old.comment);
	if(!status) return FALSE;
	status=copyVariableArray(&facility,old.facility);
	if(!status) return FALSE;
	status=copyVariableArray(&city,old.city);
	if(!status) return FALSE;
	status=copyVariableArray(&addr,old.addr);
	if(!status) return FALSE;
	status=copyVariableArray(&cross_road,old.cross_road);
	if(!status) return FALSE;

	link=old.link;
	link_class=old.link_class;
	for(i=0; i < 2; i++) link_subclass[i]=old.link_subclass[i];
	status=copyVariableArray(&link_ident,old.link_ident);
	if(!status) return FALSE;

	return TRUE;
}

CBlock *CWaypoint::clone(void) const
{
	CWaypoint *pWaypoint=new CWaypoint(*this);
	return pWaypoint;
}

void CWaypoint::setSymbol(char *symbolName)
{
	smbl=getSymbolNumber(symbolName);
}

BOOL CWaypoint::sendData(PacketType type)
{
	BYTE data[DATA_BUFSIZE];
	BYTE buf[DATA_BUFSIZE];
	BYTE *next;
	BYTE nData=0;
	DWORD nBuf=0;
	BOOL status;

	// Make the data
	memcpy(data,&wpt_class,sizeof(wpt_class));
	memcpy(data+1,&color,sizeof(color));
	memcpy(data+2,&dspl,sizeof(dspl));
	memcpy(data+3,&attr,sizeof(attr));
	memcpy(data+4,&smbl,sizeof(smbl));
	memcpy(data+6,&subclass,sizeof(subclass));
	memcpy(data+24,&lat,sizeof(lat));
	memcpy(data+28,&lon,sizeof(lon));
	memcpy(data+32,&alt,sizeof(alt));
	memcpy(data+36,&dpth,sizeof(dpth));
	memcpy(data+40,&dist,sizeof(dist));
	memcpy(data+44,&state,sizeof(state));
	memcpy(data+46,&cc,sizeof(cc));
	next=data+48;
	next=retrieveVariableArray(next,&ident);
	next=retrieveVariableArray(next,&comment);
	next=retrieveVariableArray(next,&facility);
	next=retrieveVariableArray(next,&city);
	next=retrieveVariableArray(next,&addr);
	next=retrieveVariableArray(next,&cross_road);

	nData=next-data;

	// Send the packet
	status=sendPacket(data,buf,type,nData,&nBuf);

#if DEBUG_UPLOAD
	lbprintf(_T("Waypoint packet: nData=%d"),nData);
	dumpBytes(buf,nBuf);
#endif

	return status;
}

BOOL CWaypoint::sendLinkData(void)
{
	BYTE data[DATA_BUFSIZE];
	BYTE buf[DATA_BUFSIZE];
	BYTE *next;
	BYTE nData=0;
	DWORD nBuf=0;
	BOOL status;

	// Make the data
	memcpy(data,&link_class,sizeof(link_class));
	memcpy(data+2,&subclass,sizeof(subclass));
	next=data+20;
	next=retrieveVariableArray(next,&link_ident);

	nData=next-data;

	// Send the packet
	status=sendPacket(data,buf,Pid_Rte_Link_Data,nData,&nBuf);

#if DEBUG_UPLOAD
	lbprintf(_T("Waypoint packet: nData=%d"),nData);
	dumpBytes(buf,nBuf);
#endif

	return status;
}


BOOL CWaypoint::isDuplicate(CBlockList *list) const
{
	tsDLIterBD<CBlock> iter(list->first());
	tsDLIterBD<CBlock> eol;
	BOOL nameEqual;

	while(iter != eol) {
		CBlock *pBlock=iter;
		CWaypoint *waypoint=(CWaypoint *)pBlock;
		if(waypoint != this) {
			nameEqual=(this->ident == NULL && waypoint->ident == NULL ||
				(this->ident != NULL && waypoint->ident != NULL &&
				!strcmp(this->ident,waypoint->ident)));
			long dlat=labs(this->lat-waypoint->lat);
			long dlon=labs(this->lon-waypoint->lon);
			double dalt=fabs(this->alt-waypoint->alt);
			if(nameEqual && dlat < LAT_TOL && dlon < LON_TOL &&
				dalt < ALT_TOL && this->smbl == waypoint->smbl) {
					return TRUE;
			}
		}
		++iter;
	}
	return FALSE;
}

BOOL CWaypoint::setData(BYTE *data, BYTE nData)
{
	BYTE *next;

	ident=comment=facility=city=addr=cross_road=NULL;

	memcpy(&wpt_class,data,sizeof(wpt_class));
	memcpy(&color,data+1,sizeof(color));
	memcpy(&dspl,data+2,sizeof(dspl));
	memcpy(&attr,data+3,sizeof(attr));
	memcpy(&smbl,data+4,sizeof(smbl));
	memcpy(&subclass,data+6,sizeof(subclass));
	memcpy(&lat,data+24,sizeof(lat));
	memcpy(&lon,data+28,sizeof(lon));
	memcpy(&alt,data+32,sizeof(alt));
	memcpy(&dpth,data+36,sizeof(dpth));
	memcpy(&dist,data+40,sizeof(dist));
	memcpy(&state,data+44,sizeof(state));
	memcpy(&cc,data+46,sizeof(cc));
	next=data+48;
	next=storeVariableArray(next,&ident);
	if(!next) goto WPT_ERROR;
	next=storeVariableArray(next,&comment);
	if(!next) goto WPT_ERROR;
	next=storeVariableArray(next,&facility);
	if(!next) goto WPT_ERROR;
	next=storeVariableArray(next,&city);
	if(!next) goto WPT_ERROR;
	next=storeVariableArray(next,&addr);
	if(!next) goto WPT_ERROR;
	next=storeVariableArray(next,&cross_road);
	if(!next) goto WPT_ERROR;

	if((BYTE)(next-data) != nData) {
		lbprintf(_T("Data is wrong length %d vs. %d"),
			(BYTE)(next-data),nData);
		goto WPT_ERROR;
	}
	return TRUE;

WPT_ERROR:
	return FALSE;
}

BOOL CWaypoint::setLinkData(BYTE *data, BYTE nData)
{
	BYTE *next;

	link_ident=NULL;

	memcpy(&link_class,data,sizeof(link_class));
	memcpy(&subclass,data+2,sizeof(subclass));
	next=data+20;
	next=storeVariableArray(next,&link_ident);
	if(!next) goto WPT_LINK_ERROR;

	if((BYTE)(next-data) != nData) {
		lbprintf(_T("Link data is wrong length %d vs. %d"),
			(BYTE)(next-data),nData);
		goto WPT_LINK_ERROR;
	}
	return TRUE;

WPT_LINK_ERROR:
	return FALSE;
}

BOOL CWaypoint::setIdent(char *newIdent)
{
	int len;

	// Delete the old one
	if(ident) delete ident;

	// If the new one is NULL, use NULL
	if(!newIdent) {
		ident=NULL;
		return TRUE;
	}

	// Make the new one
	len=strlen(newIdent)+1;
	ident=new char[len];
	if(!ident) return FALSE;
	strcpy(ident,newIdent);

	return TRUE;
}

/**************************************************************************/
/**************************** CRoute **************************************/
/**************************************************************************/

CRoute::CRoute(BOOL checkedIn) :
CBlock(TYPE_RTE),
maxAlt(0.0)
{
	// Set checked
	checked=checkedIn;
}

CRoute::CRoute(BOOL checkedIn, char *name) :
CBlock(TYPE_RTE),
maxAlt(0.0)
{
	// Set checked
	checked=checkedIn;

	if(name) {
		int len=strlen(name)+1;
		rte_ident=new char[len];
		if(rte_ident) {
			strcpy(rte_ident,name);
		}
	} else {
		rte_ident=new char[1];
		*rte_ident='\0';
	}
}

CRoute::CRoute(const CRoute &old) :
CBlock(old)
{
	// Copy the data from the old one
	copy(old);
}

CRoute::~CRoute(void)
{
	if(rte_ident) delete [] rte_ident;
}

CRoute &CRoute::operator=(const CRoute &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there
	CBlockList::empty(TRUE);
	if(rte_ident) delete [] rte_ident;

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CRoute::copy(const CRoute &old)
{
	BOOL status;

	CBlock::copy(old);
	CBlockList::copy(old);

	status=copyVariableArray(&rte_ident,old.rte_ident);
	if(!status) return FALSE;
	maxAlt=old.maxAlt;

	return TRUE;
}

CBlock *CRoute::clone(void) const
{
	CRoute *pRoute=new CRoute(*this);
	return pRoute;
}

BOOL CRoute::sendData(PacketType type)
{
	BYTE data[DATA_BUFSIZE];
	BYTE buf[DATA_BUFSIZE];
	BYTE *next;
	BYTE nData=0;
	DWORD nBuf=0;
	BOOL status;

	// Make the data
	next=data;
	next=retrieveVariableArray(next,&rte_ident);

	nData=next-data;

	// Send the packet
	status=sendPacket(data,buf,type,nData,&nBuf);

#if DEBUG_UPLOAD
	lbprintf(_T("Route header packet: nData=%d"),nData);
	dumpBytes(buf,nBuf);
#endif

	return status;
}

BOOL CRoute::setData(BYTE *data, BYTE nData)
{
	BYTE *next;

	rte_ident=NULL;

	next=data;
	next=storeVariableArray(next,&rte_ident);
	if(!next) goto RTE_ERROR;

	if((BYTE)(next-data) != nData) {
		lbprintf(_T("Data is wrong length %d vs. %d"),
			(BYTE)(next-data),nData);
		goto RTE_ERROR;
	}
	return TRUE;

RTE_ERROR:
	return FALSE;
}

BOOL CRoute::setIdent(char *newIdent)
{
	int len;

	// Delete the old one
	if(rte_ident) delete rte_ident;

	// If the new one is NULL, use NULL
	if(!newIdent) {
		rte_ident=NULL;
		return TRUE;
	}

	// Make the new one
	len=strlen(newIdent)+1;
	rte_ident=new char[len];
	if(!rte_ident) return FALSE;
	strcpy(rte_ident,newIdent);

	return TRUE;
}

double CRoute::getLatitude(void) const
{
	if(getCount() > 0) {
		CBlock *pBlock=tsDLList<CBlock>::first();
		if(pBlock) return pBlock->getLatitude();
		else return 0.0;
	} else {
		return 0.0;
	}
}

double CRoute::getLongitude(void) const
{
	if(getCount() > 0) {
		CBlock *pBlock=tsDLList<CBlock>::first();
		if(pBlock) return pBlock->getLongitude();
		else return 0.0;
	} else {
		return 0.0;
	}
}

/**************************************************************************/
/**************************** CTrack **************************************/
/**************************************************************************/

CTrack::CTrack(BOOL checkedIn) :
CBlock(TYPE_TRK),
maxAlt(0.0)
{
	// Set checked
	checked=checkedIn;
}

CTrack::CTrack(BOOL checkedIn, char *name) :
CBlock(TYPE_TRK),
maxAlt(0.0)
{
	// Set checked
	checked=checkedIn;

	if(name) {
		int len=strlen(name)+1;
		trk_ident=new char[len];
		if(trk_ident) {
			strcpy(trk_ident,name);
		}
	} else {
		trk_ident=new char[1];
		*trk_ident='\0';
	}
}

CTrack::CTrack(const CTrack &old) :
CBlock(old)

{
	// Copy the data from the old one
	copy(old);
}

CTrack::~CTrack(void)
{
	if(trk_ident) delete [] trk_ident;
}


CTrack &CTrack::operator=(const CTrack &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there
	CBlockList::empty(TRUE);
	if(trk_ident) delete [] trk_ident;

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CTrack::copy(const CTrack &old)
{
	BOOL status;

	CBlock::copy(old);
	CBlockList::copy(old);

	dspl=old.dspl;
	color=old.color;
	status=copyVariableArray(&trk_ident,old.trk_ident);
	if(!status) return FALSE;
	maxAlt=old.maxAlt;;

	return TRUE;
}

CBlock *CTrack::clone(void) const
{
	CTrack *pTrack=new CTrack(*this);
	return pTrack;
}

BOOL CTrack::sendData(PacketType type)
{
	BYTE data[DATA_BUFSIZE];
	BYTE buf[DATA_BUFSIZE];
	BYTE *next;
	BYTE nData=0;
	DWORD nBuf=0;
	BOOL status;

	// Make the data
	memcpy(data,&dspl,sizeof(dspl));
	memcpy(data+1,&color,sizeof(color));
	next=data+2;
	next=retrieveVariableArray(next,&trk_ident);

	nData=next-data;

	// Send the packet
	status=sendPacket(data,buf,type,nData,&nBuf);

#if DEBUG_UPLOAD
	lbprintf(_T("Track packet: nData=%d"),nData);
	dumpBytes(buf,nBuf);
#endif

	return status;
}

BOOL CTrack::setData(BYTE *data, BYTE nData)
{
	BYTE *next;

	trk_ident=NULL;

	memcpy(&dspl,data,sizeof(dspl));
	memcpy(&color,data+1,sizeof(color));
	next=data+2;
	next=storeVariableArray(next,&trk_ident);
	if(!next) goto TRK_ERROR;

	if((BYTE)(next-data) != nData) {
		lbprintf(_T("Data is wrong length %d vs. %d"),
			(BYTE)(next-data),nData);
		goto TRK_ERROR;
	}
	return TRUE;

TRK_ERROR:
	return FALSE;
}

BOOL CTrack::setIdent(char *newIdent)
{
	int len;

	// Delete the old one
	if(trk_ident) delete trk_ident;

	// If the new one is NULL, use NULL
	if(!newIdent) {
		trk_ident=NULL;
		return TRUE;
	}

	// Make the new one
	len=strlen(newIdent)+1;
	trk_ident=new char[len];
	if(!trk_ident) return FALSE;
	strcpy(trk_ident,newIdent);

	return TRUE;
}

double CTrack::getLatitude(void) const
{
	if(getCount() > 0) {
		CBlock *pBlock=tsDLList<CBlock>::first();
		if(pBlock) return pBlock->getLatitude();
		else return 0.0;
	} else {
		return 0.0;
	}
}

double CTrack::getLongitude(void) const
{
	if(getCount() > 0) {
		CBlock *pBlock=tsDLList<CBlock>::first();
		if(pBlock) return pBlock->getLongitude();
		else return 0.0;
	} else {
		return 0.0;
	}
}

/**************************************************************************/
/**************************** CTrackpoint *********************************/
/**************************************************************************/

CTrackpoint::CTrackpoint(BOOL checkedIn) :
CBlock(TYPE_TPT),
lat(0),
lon(0),
alt(0),
dpth(0),
new_trk(0)
{
	// Set checked
	checked=checkedIn;
}

CTrackpoint::CTrackpoint(BOOL checkedIn, BYTE newTrack, double latitude,
						 double longitude, double altitude, char *timeString) :
CBlock(TYPE_TPT),
lat(0),
lon(0),
alt(0),
dpth(0),
new_trk(0)
{
	// Set checked
	checked=checkedIn;

	new_trk=newTrack;
	lat=(long)(latitude/SEMI2DEG+.5);
	lon=(long)(longitude/SEMI2DEG+.5);
	alt=(float)(altitude/M2FT);
	time=(DWORD)convertTimeStringToLong(timeString);
}

CTrackpoint::CTrackpoint(BOOL checkedIn, BYTE newTrack, double latitude,
						 double longitude, double altitude, time_t unixTime) :
CBlock(TYPE_TPT),
lat(0),
lon(0),
alt(0),
dpth(0),
new_trk(0)
{
	// Set checked
	checked=checkedIn;

	new_trk=newTrack;
	lat=(long)(latitude/SEMI2DEG+.5);
	lon=(long)(longitude/SEMI2DEG+.5);
	alt=(float)(altitude/M2FT);
	time=(DWORD)convertUnixTimeToLong(unixTime);
}

CTrackpoint::CTrackpoint(const CTrackpoint &old) :
CBlock(old)
{
	// Copy the data from the old one
	copy(old);
}

CTrackpoint::~CTrackpoint(void)
{
}

CTrackpoint &CTrackpoint::operator=(const CTrackpoint &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CTrackpoint::copy(const CTrackpoint &old)
{
	CBlock::copy(old);

	lat=old.lat;
	lon=old.lon;
	time=old.time;
	alt=old.alt;
	dpth=old.dpth;
	new_trk=old.new_trk;

	return TRUE;
}

CBlock *CTrackpoint::clone(void) const
{
	CTrackpoint *pTrackpoint=new CTrackpoint(*this);
	return pTrackpoint;
}

BOOL CTrackpoint::sendData(PacketType type)
{
	BYTE data[DATA_BUFSIZE];
	BYTE buf[DATA_BUFSIZE];
	BYTE *next;
	BYTE nData=0;
	DWORD nBuf=0;
	BOOL status;

	// Make the data
	memcpy(data,&lat,sizeof(lat));
	memcpy(data+4,&lon,sizeof(lon));
	memcpy(data+8,&time,sizeof(time));
	memcpy(data+12,&alt,sizeof(alt));
	memcpy(data+16,&dpth,sizeof(dpth));
	memcpy(data+20,&new_trk,sizeof(new_trk));
	next=data+21;

	nData=next-data;

	// Send the packet
	status=sendPacket(data,buf,type,nData,&nBuf);

#if DEBUG_UPLOAD
	lbprintf(_T("Trackpoint packet: nData=%d"),nData);
	dumpBytes(buf,nBuf);
#endif

	return status;
}

BOOL CTrackpoint::setData(BYTE *data, BYTE nData)
{
	// Seems to be 24, size of boolean is ambiguous, but doesn't account
	// for last 2 or 3 bytes
	if(nData < 21 || !data) return FALSE;

	memcpy(&lat,data,sizeof(lat));
	memcpy(&lon,data+4,sizeof(lon));
	memcpy(&time,data+8,sizeof(time));
	memcpy(&alt,data+12,sizeof(alt));
	memcpy(&dpth,data+16,sizeof(dpth));
	memcpy(&new_trk,data+20,sizeof(new_trk));

	return TRUE;
}

// Change new_trk if the first characters match Continue or Start
BOOL CTrackpoint::setIdent(char *newIdent)
{
	int lenS=5;  // strlen("Start") (not including \0)
	int lenC=8;  // strlen("Continue")
	int lenN;
	BOOL val;

	if(!newIdent) return FALSE;

	// Check if it is the first track
	if(parentList) {
		CBlockList *blockList=parentList;
		tsDLIterBD<CBlock> iter(blockList->first());
		CBlock *pBlock=iter;
		if(this == (CTrackpoint *)pBlock) {
			// Always set it to TRUE
			new_trk=TRUE;
			return FALSE;
		}
	}

	lenN=strlen(newIdent);

	// See if it matches Start (WCE doesn't support toupper, etc.)
	if(lenN > lenS) goto TRYCONTINUE;
	val=TRUE;
	if(newIdent[0] != 's' && newIdent[0] != 'S') goto TRYCONTINUE;
	if(lenN == 1) goto FOUND;
	if(newIdent[1] != 't' && newIdent[1] != 'T') goto TRYCONTINUE;
	if(lenN == 2) goto FOUND;
	if(newIdent[2] != 'a' && newIdent[2] != 'A') goto TRYCONTINUE;
	if(lenN == 3) goto FOUND;
	if(newIdent[3] != 'r' && newIdent[3] != 'R') goto TRYCONTINUE;
	if(lenN == 4) goto FOUND;
	if(newIdent[4] != 't' && newIdent[4] != 'T') goto TRYCONTINUE;
	if(lenN == 5) goto FOUND;

TRYCONTINUE:
	// See if it matches Continue;
	if(lenN > lenC) return FALSE;
	val=FALSE;
	if(newIdent[0] != 'c' && newIdent[0] != 'C') return FALSE;
	if(lenN == 1) goto FOUND;
	if(newIdent[1] != 'o' && newIdent[1] != 'O') return FALSE;
	if(lenN == 2) goto FOUND;
	if(newIdent[2] != 'n' && newIdent[2] != 'N') return FALSE;
	if(lenN == 3) goto FOUND;
	if(newIdent[3] != 't' && newIdent[3] != 'T') return FALSE;
	if(lenN == 4) goto FOUND;
	if(newIdent[4] != 'i' && newIdent[4] != 'I') return FALSE;
	if(lenN == 5) goto FOUND;
	if(newIdent[5] != 'n' && newIdent[5] != 'N') return FALSE;
	if(lenN == 6) goto FOUND;
	if(newIdent[6] != 'u' && newIdent[6] != 'U') return FALSE;
	if(lenN == 7) goto FOUND;
	if(newIdent[7] != 'e' && newIdent[7] != 'E') return FALSE;
	if(lenN == 8) goto FOUND;
	return FALSE;

FOUND:
	new_trk=val;
	return TRUE;
}

/**************************************************************************/
/**************************** CPosition ***********************************/
/**************************************************************************/

CPosition::CPosition(void) :
CBlock(TYPE_WPT),
lat(DBL_MAX)
{
}

CPosition::CPosition(const CPosition &old) :
CBlock(old)
{
	// Copy the data from the old one
	copy(old);
}

CPosition::~CPosition(void)
{
}

CPosition &CPosition::operator=(const CPosition &old)
{
	// Handle copying to itself
	if(this == &old) return *this;

	// Clear out what is there

	// Copy the data from the old one
	copy(old);

	// Return a value to allow sequential assignment
	return *this;
}

BOOL CPosition::copy(const CPosition &old)
{
	CBlock::copy(old);

	alt=old.alt;
	epe=old.epe;
	eph=old.eph;
	epv=old.epv;
	fix=old.fix;
	tow=old.tow;
	lat=old.lat;
	lon=old.lon;
	east=old.east;
	north=old.north;
	up=old.up;
	msl_hght=old.msl_hght;
	leap_scnds=old.leap_scnds;
	wn_days=old.wn_days;

	return TRUE;
}

CBlock *CPosition::clone(void) const
{
	CPosition *pPosition=new CPosition(*this);
	return pPosition;
}

BOOL CPosition::setData(BYTE *data, BYTE nData)
{
	if(nData != 64 || !data) return FALSE;

	memcpy(&alt,data,sizeof(alt));
	memcpy(&epe,data+4,sizeof(epe));
	memcpy(&eph,data+8,sizeof(eph));
	memcpy(&epv,data+12,sizeof(epv));
	memcpy(&fix,data+16,sizeof(fix));
	memcpy(&tow,data+18,sizeof(tow));
	memcpy(&lat,data+26,sizeof(lat));
	memcpy(&lon,data+34,sizeof(lon));
	memcpy(&east,data+42,sizeof(east));
	memcpy(&north,data+46,sizeof(north));
	memcpy(&up,data+50,sizeof(up));
	memcpy(&msl_hght,data+54,sizeof(msl_hght));
	memcpy(&leap_scnds,data+58,sizeof(leap_scnds));
	memcpy(&wn_days,data+60,sizeof(wn_days));

	return TRUE;
}

BOOL CPosition::setIdent(char *newIdent)
{
	// Don't allow changing name
	return FALSE;
}

const TCHAR *CPosition::getFixName(void) const
{
	if(fix >=0 && fix < 6) {
		return fixName[fix];
	} else {
		return fixName[1];
	}
}

/**************************************************************************/
/**************************** CCommand *****************************************/
/**************************************************************************/

CCommand::CCommand(CommandType typeIn) :
type(typeIn)
{
}

CCommand::~CCommand(void)
{
}
