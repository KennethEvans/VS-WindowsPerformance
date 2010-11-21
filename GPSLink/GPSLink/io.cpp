// I/O routines for GPSLink

#include "stdafx.h"

#define READ_LINESIZE 512
#define DELIMITER "Delimiter"
#define GMTOFFSET "GMTOffset"

#define G7TOWIN_ID "Version 2:CSV"
#define G7TOWIN_SIGNIFICANT_CHARS 9
#define G7TOWIN_ZONE_OFFSET "ZoneOffset:"

#define XML_ID "xml"

#include "GPSLink.h"
#include "version.h"
#include "CGPXParser.h"

// Function prototypes
static void lbPrintSeparatedFileName(TCHAR *fileName);
static void resetViews();

// Global variables

int listWaypoint(CWaypoint *pWaypoint)
{
	TCHAR *unicodeString;

	if(!pWaypoint) {
		lbprintf(_T(" NULL"));
		return 0;
	}
	if(pWaypoint->getIdent()) {
		unicodeString=NULL;
		ansiToUnicode(pWaypoint->getIdent(),&unicodeString);
		if(unicodeString) {
			lbprintf(_T(" %s"),unicodeString);
			free(unicodeString);
		} else {
			lbprintf(_T(" Cannot convert ANSI string"));
			return 0;
		}
	} else {
		lbprintf(_T(" Invalid identifier"));
		return 0;
	}

	return 1;
}

int listPosition(CPosition *pPosition)
{
	double speed,vnorth,veast,vup;
	TCHAR ns,ew;

	if(!pPosition) return 0;

	if(!pPosition->isUseful()) {
		lbprintf(_T(" Position is %s"),pPosition->getFixName());
		return 0;
	}

	double epe=pPosition->getEPE();
	double eph=pPosition->getEPH();
	double epv=pPosition->getEPV();
	lbprintf(_T(" %s EPE=%2.0f EPEH=%2.0f EPEV=%2.0f"),
		pPosition->getFixName(),epe,eph,epv);
	veast=pPosition->getVEast();
	vnorth=pPosition->getVNorth();
	vup=pPosition->getVUp();
	speed=pow(veast*veast+vnorth*vnorth,.5);
	lbprintf(_T(" speed: %2.1f (%2.1f,%2.1f,%2.1f)"),
		speed,veast,vnorth,vup);
	long gTime=pPosition->getGTime();
	if(gTime >= 0) {
		TCHAR timeString[TIME_CHARS_U_AMPM];
		printGarminTime(timeString,gTime,Format_AMPM);
		lbprintf(_T(" %s"),timeString);
	} else {
		lbprintf(_T(" GarminTime0"));
	}
	double latitude=pPosition->getLatitude();
	if(latitude >= 0) {
		ns='N';
	} else {
		latitude=-latitude;
		ns='S';
	}
	double longitude=pPosition->getLongitude();
	if(longitude >= 0) {
		ew='E';
	} else {
		longitude=-longitude;
		ew='W';
	}
	double height=pPosition->getAltitude();
	lbprintf(_T(" %.6f%c %.6f%c %.0f ft"),
		latitude,ns,longitude,ew,height);
	return 1;
}

void listWaypoints(void)
{
	tsDLIterBD<CBlock> iter(waypointList.first());
	tsDLIterBD<CBlock> eol;

	// Do the header
	lbprintf(_T("Waypoints [%d]"),waypointList.getCount());
	while(iter != eol) {
		CBlock *pBlock=iter;
		double latitude,longitude,height;
		TCHAR ns,ew;
		TCHAR *unicodeString;
		int status=1;

		if(pBlock->getIdent()) {
			unicodeString=NULL;
			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T(" %s Sym=%d"),unicodeString,
					((CWaypoint *)pBlock)->getSymbol());
				free(unicodeString);
			} else {
				lbprintf(_T("Cannot convert ANSI string"));
				status=0;
			}
		} else {
			lbprintf(_T(" Invalid identifier"));
			status=0;
		}
		if(!status) {
			++iter;
			continue;
		}
		latitude=pBlock->getLatitude();
		if(latitude >= 0) {
			ns='N';
		} else {
			latitude=-latitude;
			ns='S';
		}
		longitude=pBlock->getLongitude();
		if(longitude >= 0) {
			ew='E';
		} else {
			longitude=-longitude;
			ew='W';
		}
		height=pBlock->getAltitude();
		lbprintf(_T(" %.6f%c %.6f%c %.0f ft"),
			latitude,ns,longitude,ew,height);

		++iter;
	}
}

void listRoutes(void)
{
	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;
	int status;

	// Do the header
	lbprintf(_T("Routes [%d]"),routeList.getCount());
	while(iter != eol) {
		CBlock *pBlock=iter;
		const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
		// Do the method for the route
		lbprintf(_T(""));
		status=1;
		if(pBlock->getIdent()) {
			TCHAR *unicodeString;

			unicodeString=NULL;
			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T(" %s"),unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T(" Cannot convert ANSI string"));
				status=0;
			}
		} else {
			lbprintf(_T(" Invalid ID"));
			status=0;
		}
		if(!status) {
			++iter;
			continue;
		}
		lbprintf(_T("  Waypoints [%d]"),blockList->getCount());
		// Do the waypoints
		tsDLIterBD<CBlock> iter1(blockList->first());
		while(iter1 != eol) {
			CBlock *pBlock1=iter1;
			// Do the method for the waypoint
			listWaypoint((CWaypoint *)pBlock1);
			++iter1;
		}
		++iter;
	}
}

void listTracks(void)
{
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	BOOL status;

	// Do the header
	lbprintf(_T("Tracks [%d]"),trackList.getCount());
	while(iter != eol) {
		CBlock *pBlock=iter;
		const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
		// Do the method for the track
		lbprintf(_T(""));
		status=1;
		if(pBlock->getIdent()) {
			TCHAR *unicodeString;

			unicodeString=NULL;
			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T(" %s"),unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T(" Cannot convert ANSI string"));
				status=0;
			}
		} else {
			lbprintf(_T(" InvalidID"));
			status=0;
		}
		if(!status) {
			++iter;
			continue;
		}
		lbprintf(_T("  Trackpoints [%d]"),blockList->getCount());
		// Do the trackpoints
		tsDLIterBD<CBlock> iter1(blockList->first());
		while(iter1 != eol) {
			CBlock *pBlock1=iter1;
			double latitude,longitude,height;
			TCHAR ns ,ew;
			long gTime;

			latitude=pBlock1->getLatitude();
			if(latitude >= 0) {
				ns='N';
			} else {
				latitude=-latitude;
				ns='S';
			}
			longitude=pBlock1->getLongitude();
			if(longitude >= 0) {
				ew='E';
			} else {
				longitude=-longitude;
				ew='W';
			}
			height=pBlock1->getAltitude();
			lbprintf(_T(" %.6f%c %.6f%c %.0f ft"),
				latitude,ns,longitude,ew,height);
			gTime=((CTrackpoint *)pBlock1)->getGTime();
			if(gTime >= 0) {
				TCHAR timeString[TIME_CHARS_U_AMPM];
				printGarminTime(timeString,gTime,Format_AMPM);
				lbprintf(_T(" %s New=%s"),timeString,
					((CTrackpoint *)pBlock1)->isNewTrack()?_T("Yes"):_T("No"));
			} else {
				lbprintf(_T(" GarminTime0 New=%s"),
					((CTrackpoint *)pBlock1)->isNewTrack()?_T("Yes"):_T("No"));
			}
			++iter1;
		}
		++iter;
	}
}

void resetViews()
{
	// Redo the map
	if(hMap) {
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
	}

	// Redisplay the lists if visible
	if(hWptLV) wptCreateLV(hWndMain);
	if(hRteLV) rteCreateLV(hWndMain);
	if(hTrkLV) trkCreateLV(hWndMain);
}

void readGPSLFile(TCHAR *szFileName)
{
	TCHAR szMapFileName[_MAX_PATH];
	char *ansiFileName=NULL;
	char *ansiTime=NULL;
	LPWSTR unicodeString=NULL;
	DWORD result;
	char line[READ_LINESIZE];
	char *bytes;
	int len;
	char strDelim[3];
	int lineNum;
	char *token;
	char *name=NULL,*symbol=NULL,*timeString=NULL;
	double latitude,longitude,altitude,xPix,yPix;
	BYTE startTrack;
	CWaypoint *waypoint=NULL;
	CRoute *route=NULL;
	CTrack *track=NULL;
	CTrackpoint *trackpoint=NULL;
	BOOL rteDataInProgress=FALSE;
	BOOL rteDataAborted=FALSE;
	BOOL trkDataInProgress=FALSE;
	BOOL trkDataAborted=FALSE;
	BOOL mapDataInProgress=FALSE;
	BOOL mapDataAborted=FALSE;
	BOOL mapFound=FALSE;
	double fileGMTOffsetHr=0.0;
	char delimiter='\t';

	// Check the filename
	if(!szFileName || szFileName[0] == '\0') {
		errMsg(_T("Received invalid file name"));
		return;
	}

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename string"));
		return;
	}

	// Open the file
	fp=fopen(ansiFileName,"r");
	free(ansiFileName);
	ansiFileName=NULL;

	if(!fp) {
		errMsg(_T("Cannot read file:\n%s"),szFileName);
		return;
	}

	// Read ID
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Error reading file:\n%s"),
			szFileName);
		goto CLEANUP;
	}
	token=strtok(line,"\n");
	if(!strcmp(token,GPSLINK_ID)) {
		// Is GPSL file, continue
	} else if (!strncmp(token,G7TOWIN_ID,G7TOWIN_SIGNIFICANT_CHARS)) {
		// Is G7ToWinFile
		fclose(fp);
		fp=NULL;
		readG7ToWinFile(szFileName);
		return;
	} else if (strstr(token,XML_ID)) {
		// Is an XML file, assume GPX
		fclose(fp);
		fp=NULL;
		readGPXFile(szFileName);
		return;
	} else {
		errMsg(_T("Invalid GPSLink file (Bad ID):\n%s"),
			szFileName);
		goto CLEANUP;
	}

	// Read timestamp
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Invalid GPSLink file (Unexpected EOF):\n%s"),
			szFileName);
		goto CLEANUP;
	}

	// Delimiter
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Invalid GPSLink file (Unexpected EOF):\n%s"),
			szFileName);
		goto CLEANUP;
	}
	token=strtok(line,"=");
	if(strcmp(token,DELIMITER)) {
		errMsg(_T("No delimiter found, assuming TAB:\n%s"),
			DELIMITER,
			szFileName);
		delimiter='\t';
	} else {
		token=strtok(NULL,"\n");
		delimiter=token[0];
	}
	if(delimiter != ',' && delimiter != '\t') {
		errMsg(_T("Invalid delimiter, assuming TAB:\n%s"),
			szFileName);
		delimiter='\t';
	}
	sprintf(strDelim,"%c%c",delimiter,'\n');

	// GMTOffset
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Invalid GPSLink file (Unexpected EOF):\n%s"),
			szFileName);
		goto CLEANUP;
	}
	token=strtok(line,"=");
	if(strcmp(token,GMTOFFSET)) {
		errMsg(_T("No %s found, assuming 0:\n%s"),
			_T(GMTOFFSET),
			szFileName);
		fileGMTOffsetHr=0.0;
	} else {
		token=strtok(NULL,"\n");
		fileGMTOffsetHr=atof(token);
	}

	// Read the rest of the file
	lineNum=4;
	while(1) {
		bytes=fgets(line,READ_LINESIZE,fp);
		lineNum++;
		// Check EOF
		if(!bytes) goto CLEANUP;
		len=strlen(line);
		// A blank line will terminate routes and tracks in progress
		if(!len) {
			if(rteDataInProgress) {
				rteDataInProgress=FALSE;
				route=NULL;
			}
			if(trkDataInProgress) {
				trkDataInProgress=FALSE;
				track=NULL;
			}
		}
		// Skip comments
		if(line[0] == '#') continue;
		// Insure there at least two characters
		if(len < 2) continue;
		// Only handle lines that have a type identifier
		if(line[1] != delimiter) continue;
		switch(line[0]) {
	case 'W':
		// Waypoint
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		// Latitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint latitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		latitude=atof(token);
		// Longitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint longitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		longitude=atof(token);
		// Altitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint altitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		altitude=atof(token);
		// Symbol
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint symbol not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		symbol=token;
		if(rteDataInProgress) {
			// Route waypoint
			if(route && !rteDataAborted) {
				waypoint=new CWaypoint(TRUE,name,latitude,
					longitude,altitude,symbol);
				if(!waypoint) {
					errMsg(_T("Line %d: Cannot create route waypoint [%s]:")
						_T("\n%s"),
						lineNum,name,szFileName);
					goto CLEANUP;
				}
				route->addBlock((CBlock *)waypoint);
			}
		} else {
			// Regular waypoint
			waypoint=new CWaypoint(FALSE,name,latitude,longitude,altitude,
				symbol);
			if(!waypoint) {
				errMsg(_T("Line %d: Cannot create waypoint [%s]:\n%s"),
					lineNum,name,szFileName);
				goto CLEANUP;
			}
			if(waypoint->isDuplicate(&waypointList)) {
				delete waypoint;
			} else {
				waypointList.addBlock((CBlock *)waypoint);
			}
		}
		break;
	case 'R':
		// Route
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Route name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		route=new CRoute(FALSE,name);
		if(!route) {
			errMsg(_T("Line %d: Cannot create route [%s]:\n%s"),
				lineNum,name,szFileName);
			goto CLEANUP;
		}

		// Always add the route, even if duplicate.  The route
		// waypoints are internal and independent of the regular
		// waypoints.
		routeList.addBlock((CBlock *)route);
		rteDataInProgress=TRUE;
		rteDataAborted=FALSE;
		break;
	case 'H':
		// Track
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Track name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		track=new CTrack(FALSE,name);
		if(!track) {
			errMsg(_T("Line %d: Cannot create track [%s]:\n%s"),
				lineNum,name,szFileName);
			goto CLEANUP;
		}

		// Always add the track, even if duplicate.
		trackList.addBlock((CBlock *)track);
		trkDataInProgress=TRUE;
		trkDataAborted=FALSE;
		break;
	case 'T':
		// Trackpoint
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		// Decide if it a new track based on the first character
		if(name[0] == 'C' || name[0] == 'c') startTrack=0;
		else startTrack=1;
		// Latitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint latitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		latitude=atof(token);
		// Longitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint longitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		longitude=atof(token);
		// Altitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint altitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		altitude=atof(token);
		// Time
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint time not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		timeString=token;

		if(trkDataInProgress) {
			if(track && !trkDataAborted) {
				trackpoint=new CTrackpoint(TRUE,startTrack,
					latitude,longitude,altitude,timeString);
				if(!trackpoint) {
					errMsg(_T("Line %d: Cannot create trackpoint [%s]:")
						_T("\n%s"),
						lineNum,name,szFileName);
					goto CLEANUP;
				}
				track->addBlock((CBlock *)trackpoint);
			}
		} else {
			errMsg(_T("Line %d: Found trackpoint without track:\n%s"),
				lineNum,szFileName);
		}
		break;
	case 'M':
		// Map
		// Type
		token=strtok(line,strDelim);
		// Name (Be careful if delimiter is space)
		token=strtok(NULL,(delimiter == ' ')?"\n":strDelim);
		if(!token) {
			errMsg(_T("Line %d: Map name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// Convert to unicode
		unicodeString=NULL;
		ansiToUnicode(token,&unicodeString);
		if(!unicodeString) {
			errMsg(_T("Line %d: Could not convert map name to unicode:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		_tcsncpy(szMapFileName,unicodeString,_MAX_PATH);
		free(unicodeString);
		szMapFileName[_MAX_PATH-1]='\0';

		if(pMapFile) delete pMapFile;
		pMapFile=new CMap(hWndMain,szMapFileName,szFileName);
		if(!pMapFile) {
			errMsg(_T("Could not create CMap:\n%s"),szFileName);
			goto CLEANUP;
		}
		mapDataInProgress=TRUE;
		mapDataAborted=FALSE;
		mapFound=TRUE;
		break;
	case 'C':
		// Calibration point
		if(!pMapFile) break;
		if(!mapDataInProgress) {
			errMsg(_T("Line %d: Ignored stray calibration point:\n%s"),
				lineNum,szFileName);
			break;
		}
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Calibration Point name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		// Latitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Calibration Point latitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		latitude=atof(token);
		// Longitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Calibration Point longitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		longitude=atof(token);
		// XPix
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Calibration Point xPix not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		xPix=atof(token);
		// YPix
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Calibration Point yPix not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		yPix=atof(token);
		// Convert name to unicode
		unicodeString=NULL;
		ansiToUnicode(name,&unicodeString);
		if(!unicodeString) {
			errMsg(_T("Lin1e %d: Could not convert calibration point name ")
				_T("to unicode:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// Store the data
		pMapFile->addPoint(latitude,longitude,xPix,yPix,unicodeString);
		free(unicodeString);
		break;
	default:
		errMsg(_T("Invalid GPSLink file (Invalid type [%c]):\n%s"),
			line[0],szFileName);
		goto CLEANUP;
		}
	}

CLEANUP:
	fclose(fp);
	fp=NULL;
	lbprintf(_T("Read GPSL File:"));
	lbPrintSeparatedFileName(szFileName);
	// Handle a map if one was found
	if(mapFound && pMapFile) {
		// Set the GMT offset to the real offset
		pMapFile->setGMTOffsetHr(getTimeOffset()/3600);
		// Reset the map
		changeMapImage(pMapFile);
	}
	resetViews();
}

// G7ToWin CSV file
void readG7ToWinFile(TCHAR *szFileName)
{
	char *ansiFileName=NULL;
	char *ansiTime=NULL;
	LPWSTR unicodeString=NULL;
	DWORD result;
	char line[READ_LINESIZE];
	char *bytes;
	int len;
	char strDelim[3];
	int lineNum;
	char *token;
	char *name=NULL,*symbol=NULL;
	double latitude,longitude,altitude;
	BYTE startTrack;
	CWaypoint *waypoint=NULL;
	CRoute *route=NULL;
	CTrack *track=NULL;
	CTrackpoint *trackpoint=NULL;
	BOOL rteDataInProgress=FALSE;
	BOOL rteDataAborted=FALSE;
	BOOL trkDataInProgress=FALSE;
	BOOL trkDataAborted=FALSE;
	BOOL mapDataInProgress=FALSE;
	BOOL mapDataAborted=FALSE;
	BOOL mapFound=FALSE;
	double fileGMTOffsetHr=0.0;
	char delimiter=',';
	SYSTEMTIME sTime0={0,0,0,0,0,0,0,0};
	SYSTEMTIME sTime=sTime0;
	long gTime;
	char *ansiString=NULL;
	TCHAR timeString[TIME_CHARS_U_24];
	char ansiTimeString[TIME_CHARS_A_24];


	// Only accept comma as delimiter
	sprintf(strDelim,"%c%c",delimiter,'\n');

	// Check the filename
	if(!szFileName || szFileName[0] == '\0') {
		errMsg(_T("Received invalid filename"));
		return;
	}

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename string"));
		return;
	}

	// Open the file
	fp=fopen(ansiFileName,"r");
	free(ansiFileName);
	ansiFileName=NULL;
	if(!fp) {
		errMsg(_T("Cannot read file:\n%s"),szFileName);
		goto CLEANUP;
	}

	// Read ID
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Error reading file:\n%s"),
			szFileName);
		goto CLEANUP;
	}
	token=strtok(line,"\n");
	if (!strncmp(token,G7TOWIN_ID,G7TOWIN_SIGNIFICANT_CHARS)) {
		// Is G7ToWinFile, continue
	} else {
		errMsg(_T("Invalid G7ToWin file (Bad ID):\nExpected: \"%s\"\n%s"),
			_T(G7TOWIN_ID),szFileName);
		goto CLEANUP;
	}

	// Datum
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Invalid G7ToWin file (Unexpected EOF):\n%s"),
			szFileName);
		goto CLEANUP;
	}

	// Zone offset
	bytes=fgets(line,READ_LINESIZE,fp);
	if(!bytes) {
		errMsg(_T("Invalid G7ToWin file (Unexpected EOF):\n%s"),
			szFileName);
		goto CLEANUP;
	}
	token=strtok(line,strDelim);
	if(strcmp(token,G7TOWIN_ZONE_OFFSET)) {
		errMsg(_T("No %s found, assuming 0:\n%s"),
			_T(G7TOWIN_ZONE_OFFSET),
			szFileName);
		fileGMTOffsetHr=0.0;
	} else {
		token=strtok(NULL,"\n");
		fileGMTOffsetHr=atof(token);
	}

	// Read the rest of the file
	lineNum=3;
	while(1) {
		bytes=fgets(line,READ_LINESIZE,fp);
		lineNum++;
		// Check EOF
		if(!bytes) goto CLEANUP;
		len=strlen(line);
		// Check for a blank line
		if(!len) continue;
		// A line starting with "Type" will terminate routes and tracks in progress
		if(line[0] == '"') {
			if(rteDataInProgress) {
				rteDataInProgress=FALSE;
				route=NULL;
			}
			if(trkDataInProgress) {
				trkDataInProgress=FALSE;
				track=NULL;
			}
		}
		// Skip comments
		if(line[0] == '#') continue;
		// Insure there at least two characters
		if(len < 2) continue;
		// Only handle lines that have a type identifier
		if(line[1] != delimiter) continue;
		switch(line[0]) {
	case 'W':
		// Waypoint
		// Type
		token=strtok(line,strDelim);
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		// Latitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Lat not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		latitude=atof(token);
		// Longitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Long not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		longitude=atof(token);
		sTime=sTime0;
		// Month#
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Month not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wMonth=(WORD)atoi(token);
		// Day#
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Day not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wDay=(WORD)atoi(token);
		// Year
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Year not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wYear=(WORD)atoi(token);
		// Hour
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Hour not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wHour=(WORD)atoi(token);
		// Minute
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Min not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wMinute=(WORD)atoi(token);
		// Second
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Sec not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wSecond=(WORD)atoi(token);
		// Comment
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Comment not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// Symbol
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint Symbol# not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		symbol=token;
		// SymbolColor
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint SymbolColor not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// SymbolDisplay
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint SymbolDisplay not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// Altitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint altitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		altitude=atof(token);
		if(rteDataInProgress) {
			// Route waypoint
			if(route && !rteDataAborted) {
				waypoint=new CWaypoint(TRUE,name,latitude,
					longitude,altitude,symbol);
				if(!waypoint) {
					errMsg(_T("Line %d: Cannot create route waypoint [%s]:")
						_T("\n%s"),
						lineNum,name,szFileName);
					goto CLEANUP;
				}
				route->addBlock((CBlock *)waypoint);
			}
		} else {
			// Regular waypoint
			waypoint=new CWaypoint(FALSE,name,latitude,longitude,altitude,
				symbol);
			if(!waypoint) {
				errMsg(_T("Line %d: Cannot create waypoint [%s]:\n%s"),
					lineNum,name,szFileName);
				goto CLEANUP;
			}
			if(waypoint->isDuplicate(&waypointList)) {
				delete waypoint;
			} else {
				waypointList.addBlock((CBlock *)waypoint);
			}
		}
		break;
	case 'R':
		// Route
		// Type
		token=strtok(line,strDelim);
		// Index?
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Route Index not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		// Name
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Route name not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		name=token;
		route=new CRoute(FALSE,name);
		if(!route) {
			errMsg(_T("Line %d: Cannot create route [%s]:\n%s"),
				lineNum,name,szFileName);
			goto CLEANUP;
		}

		// Always add the route, even if duplicate.  The route
		// waypoints are internal and independent of the regular
		// waypoints.
		routeList.addBlock((CBlock *)route);
		rteDataInProgress=TRUE;
		rteDataAborted=FALSE;
		break;
	case 'E':
		// Route end?
		if(rteDataInProgress) {
			rteDataInProgress=FALSE;
			route=NULL;
		}
		break;
	case 'N':
		// Track or new segment
		// Type
		token=strtok(line,strDelim);
		// Name (look for ,, or ,name,)
		if(line[2] != delimiter) {
			// There is a name between the commas
			// The part before | is the name
			token=strtok(NULL,"|");
			if(!token) {
				errMsg(_T("Line %d: Track name not found:\n%s"),
					lineNum,szFileName);
				goto CLEANUP;
			}
			name=token;
			// Is a new track since it has a name
			track=new CTrack(FALSE,name);
			if(!track) {
				errMsg(_T("Line %d: Cannot create track [%s]:\n%s"),
					lineNum,name,szFileName);
				goto CLEANUP;
			}

			// Always add the track, even if duplicate.
			trackList.addBlock((CBlock *)track);
			trkDataInProgress=TRUE;
			trkDataAborted=FALSE;
			break;
		}
		// No name, is a trackpoint new segment, fall through
	case 'T':
		// Trackpoint
		// Decide if it a new track based on the type
		if(line[0] == 'N') {
			// Type is already read
			startTrack=1;
		} else {
			// Type
			token=strtok(line,strDelim);
			startTrack=0;
		}
		// Name strtok will treat the ,, as one token, so the next
		// token is the latitude
		// Latitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Lat not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		latitude=atof(token);
		// Longitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Long not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		longitude=atof(token);
		sTime=sTime0;
		// Month#
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Month not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wMonth=(WORD)atoi(token);
		// Day#
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Day not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wDay=(WORD)atoi(token);
		// Year
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Year not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wYear=(WORD)atoi(token);
		// Hour
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Hour not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wHour=(WORD)atoi(token);
		// Minute
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Min not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wMinute=(WORD)atoi(token);
		// Second
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Trackpoint Sec not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		sTime.wSecond=(WORD)atoi(token);
		// Altitude
		token=strtok(NULL,strDelim);
		if(!token) {
			errMsg(_T("Line %d: Waypoint altitude not found:\n%s"),
				lineNum,szFileName);
			goto CLEANUP;
		}
		altitude=atof(token);

		// Convert the time
		gTime=convertSystemTimeToLong(sTime);
		if(gTime >= 0) {
			ansiString=NULL;
			printGarminTime(timeString,gTime,Format_24);
			unicodeToAnsi(timeString,&ansiString);
			if(ansiString) {
				strncpy(ansiTimeString,ansiString,TIME_CHARS_A_24);
				ansiTimeString[TIME_CHARS_A_24-1]='\0';
				free(ansiString);
				ansiString=NULL;
			} else {
				strncpy(ansiTimeString,"Error",TIME_CHARS_A_24);
				ansiTimeString[TIME_CHARS_A_24-1]='\0';
			}
		} else {
			strncpy(ansiTimeString,"GarminTime0",TIME_CHARS_A_24);
			ansiTimeString[TIME_CHARS_A_24-1]='\0';
		}

		if(trkDataInProgress) {
			if(track && !trkDataAborted) {
				trackpoint=new CTrackpoint(TRUE,startTrack,
					latitude,longitude,altitude,ansiTimeString);
				if(!trackpoint) {
					errMsg(_T("Line %d: Cannot create trackpoint [%s]:")
						_T("\n%s"),
						lineNum,name,szFileName);
					goto CLEANUP;
				}
				track->addBlock((CBlock *)trackpoint);
			}
		} else {
			errMsg(_T("Line %d: Found trackpoint without track:\n%s"),
				lineNum,szFileName);
		}
		break;
	default:
		errMsg(_T("Invalid G7ToWin file (Invalid type [%c]):\n%s"),
			line[0],szFileName);
		goto CLEANUP;
		}
	}

CLEANUP:
	fclose(fp);
	fp=NULL;
	lbprintf(_T("Read G7ToWin File:"));
	lbPrintSeparatedFileName(szFileName);
	resetViews();
}

// GPX File
void readGPXFile(TCHAR *szFileName)
{
	char *ansiFileName=NULL;
	char *ansiTime=NULL;
	DWORD result;
	CWaypoint *waypoint=NULL;
	CRoute *route=NULL;
	CTrack *track=NULL;
	CTrackpoint *trackpoint=NULL;

	// Check the filename
	if(!szFileName || szFileName[0] == '\0') {
		errMsg(_T("Received invalid filename"));
		return;
	}

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename string"));
		return;
	}

	// Initialize the XML4C system
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException&) {
		errMsg(_T("Error initializing XML Parser"));
		return;
	}

	// Instantiate the DOM parser
	static const XMLCh gLS[]={ chLatin_L, chLatin_S, chNull };
	DOMImplementation *impl=DOMImplementationRegistry::getDOMImplementation(gLS);
	DOMBuilder *parser=((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,0);

	parser->setFeature(XMLUni::fgDOMNamespaces,false);
	parser->setFeature(XMLUni::fgXercesSchema,false);
	parser->setFeature(XMLUni::fgXercesSchemaFullChecking,false);
	parser->setFeature(XMLUni::fgDOMValidation, false);
	parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true);

	// Setup the error handler
#if 0
	DOMCountErrorHandler errorHandler;
	parser->setErrorHandler(&errorHandler);
	errorHandler.resetErrors();
#endif

	// Parse
	bool errorOccurred = false;
	XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc = 0;
	try {
		parser->resetDocumentPool();
		doc = parser->parseURI(ansiFileName);
		if(!doc) {
			errMsg(_T("Cannot open:\n%s"),szFileName);
		}
	} catch (const XMLException&) {
		// TODO: Fix it to print the exception message
		errMsg(_T("XML error during parsing:\n%s"),szFileName);
		goto CLEANUP;;
	}  catch (const DOMException&) {
		errMsg(_T("DOM error during parsing:\n%s"),szFileName);
		goto CLEANUP;;
	} catch (...) {
		errMsg(_T("Unexpected error during parsing:\n%s"),szFileName);
		goto CLEANUP;;
	}

	// Create the GPX parser
	CGPXParser *gpxParser=new CGPXParser(szFileName);

	// Process
	DOMNode *topNode=(DOMNode*)doc->getDocumentElement();
	if(!topNode) {
		errMsg(_T("Top node is null:\n%s"),szFileName);
		goto CLEANUP;;
	}
	for(DOMNode *node = topNode->getFirstChild(); node != 0; node=node->getNextSibling()) {
		if (node->getNodeType() != DOMNode::ELEMENT_NODE) continue;
		char *topName = XMLString::transcode(node->getNodeName());
#if 0
		// Debug to console
		unicodeString=NULL;
		ansiToUnicode(topName,&unicodeString);
		if(unicodeString) {
			lbprintf(_T(" %s"),unicodeString);
			free(unicodeString);
		} else {
			lbprintf(_T(" Cannot convert ANSI string"));
		}
#endif
		if(!strcmp(topName,"wpt")) {
			gpxParser->parse(node);
			waypoint=gpxParser->createWaypoint(FALSE);
			if(!waypoint) {
				XMLString::release(&topName);
				goto CLEANUP;
			}
			if(waypoint->isDuplicate(&waypointList)) {
				delete waypoint;
			} else {
				waypointList.addBlock((CBlock *)waypoint);
			}
		} else if(!strcmp(topName,"rte")) {
			gpxParser->parse(node);
			route=gpxParser->createRoute(FALSE);
			if(!route) {
				XMLString::release(&topName);
				goto CLEANUP;
			}
			routeList.addBlock((CBlock *)route);

			// Loop over routepoints (waypoints)
			for(DOMNode *node1 = node->getFirstChild(); node1 != 0; node1=node1->getNextSibling()) {
				if (node1->getNodeType() != DOMNode::ELEMENT_NODE) continue;
				char *name1 = XMLString::transcode(node1->getNodeName());
				int checkName=strcmp(name1,"rtept");
				XMLString::release(&name1);
				if(checkName) continue;

				gpxParser->parse(node1);
				waypoint=gpxParser->createWaypoint(TRUE);
				if(!waypoint) {
					XMLString::release(&topName);
					goto CLEANUP;
				}
				route->addBlock((CBlock *)waypoint);
			}
		} else if(!strcmp(topName,"trk")) {
			gpxParser->parse(node);
			track=gpxParser->createTrack(FALSE);
			if(!track) {
				XMLString::release(&topName);
				goto CLEANUP;
			}
			trackList.addBlock((CBlock *)track);

			// Loop over track segments
			for(DOMNode *node1 = node->getFirstChild(); node1 != 0; node1=node1->getNextSibling()) {
				if (node1->getNodeType() != DOMNode::ELEMENT_NODE) continue;
				char *name1 = XMLString::transcode(node1->getNodeName());
				int checkName=strcmp(name1,"trkseg");
				XMLString::release(&name1);
				if(checkName) continue;

				// Loop over trackpoints
				BYTE startTrack=1;
				for(DOMNode *node2 = node1->getFirstChild(); node2 != 0; node2=node2->getNextSibling()) {
					if (node2->getNodeType() != DOMNode::ELEMENT_NODE) continue;
					char *name2 = XMLString::transcode(node2->getNodeName());
					checkName=strcmp(name2,"trkpt");
					XMLString::release(&name2);
					if(checkName)  continue;
					gpxParser->parse(node2);
					trackpoint=gpxParser->createTrackpoint(TRUE,startTrack);
					if(!trackpoint) {
						XMLString::release(&topName);
						goto CLEANUP;
					}
					track->addBlock((CBlock *)trackpoint);
					startTrack=0;
				}
			}
		}

		XMLString::release(&topName);
	}

CLEANUP:
	if(parser) parser->release();
	XMLPlatformUtils::Terminate();
	if(gpxParser) delete gpxParser;
	if(ansiFileName) {
		free(ansiFileName);
		ansiFileName=NULL;
	}
	lbprintf(_T("Read GPX File:"));
	lbPrintSeparatedFileName(szFileName);
	resetViews();
}

void readImageFile(TCHAR *szFileName)
{
	// Check the filename
	if(!szFileName || szFileName[0] == '\0') {
		errMsg(_T("Received invalid filename"));
		return;
	}

	if(pMapFile) delete pMapFile;
	pMapFile=new CMap(hWndMain,szFileName,NULL);
	lbprintf(_T("Read Image File:"));
	lbPrintSeparatedFileName(szFileName);
	if(!pMapFile) {
		errMsg(_T("Could not create CMap:\n%s"),szFileName);
		return;
	}

	// Set the GMT offset to the real offset
	pMapFile->setGMTOffsetHr(getTimeOffset()/3600.);

	// Reset the map
	changeMapImage(pMapFile);
}

void writeGPSLFile(TCHAR *szFileName, char delimiter)
{
	char *ansiFileName=NULL;
	char *ansiTime=NULL;
	TCHAR curTime[CURTIME_CHARS_U];
	DWORD result;

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename"));
		return;
	}

	fp=fopen(ansiFileName,"w");
	free(ansiFileName);
	ansiFileName=NULL;
	if(!fp) {
		errMsg(_T("Cannot write GPSL file:\n  %s"),szFileName);
		return;
	}

	// Print ID
	fprintf(fp,"%s\n",GPSLINK_ID);

	// Timestamp
	printCurrentTime(curTime);
	result=unicodeToAnsi(curTime,&ansiTime);
	fprintf(fp,"Saved %s\n",ansiTime?ansiTime:"");
	if(ansiTime) {
		free(ansiTime);
		ansiTime=NULL;
	}

	// Delimiter
	fprintf(fp,"%s=%c\n",DELIMITER,delimiter);

	// GMTOffset
	fprintf(fp,"%s=%g\n",GMTOFFSET,getTimeOffset()/3600.);

	// Waypoints
	if(saveWpt) {
		fprintf(fp,"\nWaypoints\n");
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;
		// Do the header
		fprintf(fp,"%s%c%s%c%s%c%s%c%s%c%s\n",
			"Type",delimiter,"Name",delimiter,
			"Latitude",delimiter,"Longitude",delimiter,
			"Alt",delimiter,"Symbol");
		// Do the waypoints
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the method for the waypoint
			double latitude,longitude,height;
			latitude=pBlock->getLatitude();
			longitude=pBlock->getLongitude();
			height=pBlock->getAltitude();
			int smbl=((CWaypoint *)pBlock)->getSymbol();
			fprintf(fp,"W%c%s%c%.6f%c%.6f%c%.0f%c%s\n",
				delimiter,pBlock->getIdent(),delimiter,
				latitude,delimiter,longitude,delimiter,height,delimiter,
				getSymbolName(smbl));
			status=1;
			++iter;
		}
	}

	// Routes
	if(saveRte) {
		fprintf(fp,"\nRoutes\n");
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;

		// Do the header
		fprintf(fp,"%s%c%s%c%s%c%s%c%s%c%s\n",
			"Type",delimiter,"Name",delimiter,
			"Latitude",delimiter,"Longitude",delimiter,
			"Alt",delimiter,"Symbol");
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			fprintf(fp,"R%c%s%c%c%c%c\n",
				delimiter,pBlock->getIdent(),delimiter,
				delimiter,delimiter,delimiter);
			status=1;
			// Do the waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Do the method for the waypoint
				double latitude,longitude,height;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				height=pBlock1->getAltitude();
				int smbl=((CWaypoint *)pBlock1)->getSymbol();
				fprintf(fp,"W%c%s%c%.6f%c%.6f%c%.0f%c%s\n",
					delimiter,pBlock1->getIdent(),delimiter,
					latitude,delimiter,longitude,delimiter,height,delimiter,
					getSymbolName(smbl));
				status=1;
				++iter1;
			}
			++iter;
		}
	}

	// Tracks
	if(saveTrk) {
		fprintf(fp,"\nTracks\n");
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;

		// Do the header
		fprintf(fp,"%s%c%s%c%s%c%s%c%s%c%s\n",
			"Type",delimiter,"Name",delimiter,
			"Latitude",delimiter,"Longitude",delimiter,
			"Alt",delimiter,"Time");
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			// Do the method for the track
			fprintf(fp,"H%c%s%c%c%c%c\n",
				delimiter,pBlock->getIdent(),delimiter,
				delimiter,delimiter,delimiter);
			status=1;
			// Do the trackpoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Do the method for the trackpoint
				long gTime;
				double latitude,longitude,height;
				char ansiTimeString[TIME_CHARS_A_24];

				gTime=((CTrackpoint *)pBlock1)->getGTime();
				if(gTime >= 0) {
					TCHAR timeString[TIME_CHARS_U_24];
					char *ansiString=NULL;

					printGarminTime(timeString,gTime,Format_24);
					unicodeToAnsi(timeString,&ansiString);
					if(ansiString) {
						strncpy(ansiTimeString,ansiString,TIME_CHARS_A_24);
						ansiTimeString[TIME_CHARS_A_24-1]='\0';
						free(ansiString);
						ansiString=NULL;
					} else {
						strncpy(ansiTimeString,"Error",TIME_CHARS_A_24);
						ansiTimeString[TIME_CHARS_A_24-1]='\0';
					}
				} else {
					strncpy(ansiTimeString,"GarminTime0",TIME_CHARS_A_24);
					ansiTimeString[TIME_CHARS_A_24-1]='\0';
				}
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				height=pBlock1->getAltitude();
				fprintf(fp,"T%c%s%c%.6f%c%.6f%c%.0f%c%s\n",
					delimiter,
					((CTrackpoint *)pBlock1)->isNewTrack()?"Start":"Cont",delimiter,
					latitude,delimiter,longitude,delimiter,
					height,delimiter,ansiTimeString);
				status=1;
				++iter1;
			}
			++iter;
		}
	}

	// Map
	if(saveMap && pMap && pMap != &pMapDefault) {
		double latitude,longitude,xPix,yPix;
		TCHAR *mapName=saveMapShortName?
			pMap->getShortFileName():pMap->getFileName();

		if(mapName) {
			char *ansiName=NULL;

			fprintf(fp,"\nMap\n");

			unicodeToAnsi(mapName,&ansiName);
			if(ansiName) {
				fprintf(fp,"%s%c%s\n","M",delimiter,ansiName);
				free(ansiName);
				ansiName=NULL;
			} else {
				fprintf(fp,"%s%c%s\n",
					"M",delimiter,"Error converting filename to ansi");
			}
			const tsDLList<CMapPoint> *pointList=pMap->getList();
			tsDLIterBD<CMapPoint> iter(pointList->first());
			tsDLIterBD<CMapPoint> eol;
			while(iter != eol) {
				CMapPoint *pPoint=(CMapPoint *)iter;
				char *ansiName=NULL;

				unicodeToAnsi(pPoint->getIdent(),&ansiName);
				if(ansiName) {
					fprintf(fp,"%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
						"C",delimiter,ansiName,delimiter,
						pPoint->getLatitude(),delimiter,
						pPoint->getLongitude(),delimiter,
						pPoint->getXPix(),delimiter,
						pPoint->getYPix());
					free(ansiName);
					ansiName=NULL;
				} else {
					fprintf(fp,"%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
						"C",delimiter,"Error",delimiter,
						pPoint->getLatitude(),delimiter,
						pPoint->getLongitude(),delimiter,
						pPoint->getXPix(),delimiter,
						pPoint->getYPix());
				}
				iter++;
			}
			// Print the corners
			BITMAP bmp=pMap->getBitmap();

			fprintf(fp,"#Image corners\n");

			xPix=0.; yPix=0.;
			pMap->calculateLatLon(xPix,yPix,&latitude,&longitude);
			fprintf(fp,"#%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
				"C",delimiter,"UL",delimiter,
				latitude,delimiter,longitude,delimiter,xPix,delimiter,yPix);

			xPix=bmp.bmWidth-1; yPix=0.;
			pMap->calculateLatLon(xPix,yPix,&latitude,&longitude);
			fprintf(fp,"#%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
				"C",delimiter,"UR",delimiter,
				latitude,delimiter,longitude,delimiter,xPix,delimiter,yPix);

			xPix=bmp.bmWidth-1; yPix=bmp.bmHeight-1;
			pMap->calculateLatLon(xPix,yPix,&latitude,&longitude);
			fprintf(fp,"#%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
				"C",delimiter,"LR",delimiter,
				latitude,delimiter,longitude,delimiter,xPix,delimiter,yPix);

			xPix=0.; yPix=bmp.bmHeight-1;
			pMap->calculateLatLon(xPix,yPix,&latitude,&longitude);
			fprintf(fp,"#%s%c%s%c%.6f%c%.6f%c%.2f%c%.2f\n",
				"C",delimiter,"LL",delimiter,
				latitude,delimiter,longitude,delimiter,xPix,delimiter,yPix);
		}
	}

	// Cleanup
	lbprintf(_T("Wrote GPSL File:"));
	lbPrintSeparatedFileName(szFileName);
	fclose(fp);
	fp=NULL;
}

void writeGPSUFile(TCHAR *szFileName)
{
	char *ansiFileName=NULL;
	DWORD result;

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename string"));
		return;
	}

	fp=fopen(ansiFileName,"w");
	free(ansiFileName);
	ansiFileName=NULL;
	if(!fp) {
		errMsg(_T("Cannot write GPSU file:\n  %s"),szFileName);
		return;
	}

	// Header
	fprintf(fp,"H  SOFTWARE NAME & VERSION\n");
	fprintf(fp,"I  GPSU 4.01  FREEWARE VERSION\n");
	fprintf(fp,"S DateFormat=dd/mmm/yyyy\n");
	fprintf(fp,"S Timezone=-6\n");
	fprintf(fp,"S Units=S,F\n");
	fprintf(fp,"S SymbolSet=2\n");
	fprintf(fp,"\n");
	fprintf(fp,"H R DATUM\n");
	fprintf(fp,"M E               WGS 84 100  0.0000000E+00  0.0000000E+00  000  000  000\n");
	fprintf(fp,"\n");
	fprintf(fp,"H  COORDINATE SYSTEM\n");
	fprintf(fp,"U  LAT LON DEG\n");
	fprintf(fp,"\n");

	// Waypoints
	if(saveWpt) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;

		// Do the header
		fprintf(fp,"F ID---- Latitude   Longitude   Symbol-------"
			" T Alt(f) Comment\n");
		// Do the waypoints
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			double latitude,longitude,height;
			latitude=pBlock->getLatitude();
			longitude=pBlock->getLongitude();
			height=pBlock->getAltitude();
			// Fix later to use symbol names !!!!!
			fprintf(fp,"W %-6s %+010.6f %+011.6f %-13s I %6.0f\n",
				pBlock->getIdent(),latitude,longitude,"Flag",height);
			status=1;
			++iter;
		}
	}

	// Routes
	if(saveRte) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;

		// Do the header
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			// Do the method for the route
			fprintf(fp,"\nR 00  %s\n",pBlock->getIdent());
			fprintf(fp,"F ID----\n");
			status=1;
			// Do the waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Do the method for the waypoint
				fprintf(fp,"W %s\n",pBlock1->getIdent());
				status=1;
				++iter1;
			}
			++iter;
		}
	}

	// Tracks
	if(saveTrk) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;

		// Do the summary
		fprintf(fp,"\nH TRACK SUMMARY\n");
		fprintf(fp,"H Track            Pnts Date        Time     StopTime"
			" Duration    miles    mph Alt(f)\n");
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the method for the track
			unsigned pnts=((CTrack *)pBlock)->getCount();
			fprintf(fp,"H %-16s %4u 30/DEC/1989 17:59:59 17:59:59"
				" 00:00:00     0.00 ####.#      0\n",
				pBlock->getIdent(),pnts>0?pnts-1:0);
			status=1;
			++iter;
		}

		// Do the tracks
		fprintf(fp,"\nF Latitude   Longitude   Alt(f) Date        Time"
			"     S Duration    miles    mph\n");
		iter=trackList.first();
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			// Do the trackpoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Do the method for the trackpoint
				double latitude,longitude,height;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				height=pBlock1->getAltitude();
				int new_trk=((CTrackpoint *)pBlock1)->isNewTrack();
				if(new_trk) {
					CTrack *track=(CTrack *)pBlock1->getParentList();

					fprintf(fp,"T %+010.6f %+011.6f %6.0f 30/DEC/1989 17:59:59 %d %-s\n",
						latitude,longitude,height,new_trk,track->getIdent());
				} else {
					fprintf(fp,"T %+010.6f %+011.6f %6.0f 30/DEC/1989 17:59:59 %d 00:00:00"
						"    0.000 ####.#\n",
						latitude,longitude,height,new_trk);
				}
				status=1;
				++iter1;
			}
			++iter;
		}
	}

	// Cleanup
	lbprintf(_T("Wrote GPSU File:"));
	lbPrintSeparatedFileName(szFileName);
	fclose(fp);
	fp=NULL;
}

void writeSA8File(TCHAR *szFileName)
{
	char *ansiFileName=NULL;
	DWORD result;

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		errMsg(_T("Cannot convert unicode filename string"));
		return;
	}

	// Open the file for binary write
	fp=fopen(ansiFileName,"wb");
	if(ansiFileName) {
		free(ansiFileName);
		ansiFileName=NULL;
	}
	if(!fp) {
		errMsg(_T("Cannot write SA8 file:\n  %s"),szFileName);
		return;
	}

	// Determine the lat,lon envelope
	double latMin,lonMin,latMax,lonMax;
	double latMin0,lonMin0,latMax0,lonMax0;
	int numObjs=0;

	latMin=lonMin=DBL_MAX;
	latMax=lonMax=-DBL_MAX;
	latMin0=latMin;
	lonMin0=lonMin;
	latMax0=latMax;
	lonMax0=lonMax;

	// Waypoint limits
	if(saveWpt) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			numObjs++;  // A waypoint symbol for each waypoint
			double lon=pBlock->getLongitude();
			double lat=pBlock->getLatitude();
			if(lat < latMin) latMin=lat;
			if(lon < lonMin) lonMin=lon;
			if(lat > latMax) latMax=lat;
			if(lon > lonMax) lonMax=lon;
			++iter;
		}
	}

	// Route limits
	if(saveRte) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			numObjs++;  // A line object for the route
			// Do the waypoints
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				numObjs++;  // A symbol object for the route waypoint
				double lon=pBlock1->getLongitude();
				double lat=pBlock1->getLatitude();
				if(lat < latMin) latMin=lat;
				if(lon < lonMin) lonMin=lon;
				if(lat > latMax) latMax=lat;
				if(lon > lonMax) lonMax=lon;
				++iter1;
			}
			++iter;
		}
	}

	// Track limits
	if(saveTrk) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the trackpoints
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Each track segment is a new object
				if(((CTrackpoint *)pBlock1)->isNewTrack()) numObjs++;
				double lon=pBlock1->getLongitude();
				double lat=pBlock1->getLatitude();
				if(lat < latMin) latMin=lat;
				if(lon < lonMin) lonMin=lon;
				if(lat > latMax) latMax=lat;
				if(lon > lonMax) lonMax=lon;
				++iter1;
			}
			++iter;
		}
	}

	// Return if there are no objects
	if(!numObjs) {
		errMsg(_T("Nothing to write"));
		fclose(fp);
		return;
	}

	// Set the values to 0 if they are different from the (unphysical)
	// starting values and let the SA routine handle it
	if(latMin == latMin0) latMin=0.0;
	if(lonMin == lonMin0) lonMin=0.0;
	if(latMax == latMax0) latMax=0.0;
	if(lonMax == lonMax0) lonMax=0.0;

	// Header
	setSAVer(SA_8);
	setSALimits(latMin,lonMin,latMax,lonMax,numObjs);
	writeSAHeader(fp);

	// Waypoints
	if(saveWpt) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			double latitude,longitude;
			latitude=pBlock->getLatitude();
			longitude=pBlock->getLongitude();
			// Write the waypoint
			writeSAWaypoint(fp,latitude,longitude,pBlock->getIdent());
			++iter;
		}
	}

	// Routes
	if(saveRte) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Count the waypoints
			int nPoints=0;
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				nPoints++;
				++iter1;
			}
			if(nPoints > 32767) {
				errMsg(_T("More than 32767 points in track segment.  ")
					_T("Street Atlas file will be corrupt"));
				nPoints=32767;
			}
			// Write the route header
			double latitude,longitude;
			latitude=pBlock->getLatitude();
			longitude=pBlock->getLongitude();
			writeSARouteHeader(fp,latitude,longitude,(short)nPoints);
			// Do the waypoint lines
			iter1=blockList->first();
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Write the waypoint (same as trackpoint) and symbol
				double latitude,longitude;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				writeSATrackpoint(fp,latitude,longitude);
				++iter1;
			}
			// Do the waypoint symbols
			iter1=blockList->first();
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				// Write the waypoint symbol
				double latitude,longitude;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				writeSARouteWaypoint(fp,latitude,longitude,pBlock1->getIdent());
				++iter1;
			}
			++iter;
		}
	}

	// Tracks
	if(saveTrk) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			// Do the trackpoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				double latitude,longitude;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				int new_trk=((CTrackpoint *)pBlock1)->isNewTrack();
				// Each track segment is a new object
				if(new_trk) {
					// Count the points in this segment
					tsDLIterBD<CBlock> iter2(pBlock1);
					tsDLIterBD<CBlock> eol;
					int nPoints=1;
					iter2++;
					while(iter2 != eol) {
						CBlock *pBlock2=iter2;
						if(saveCheckedOnly && !pBlock2->isChecked()) {
							iter2++;
							continue;
						}
						int new_trk=((CTrackpoint *)pBlock2)->isNewTrack();
						if(new_trk) {
							break;
						} else {
							nPoints++;
						}
						iter2++;
					}
					if(nPoints > 32767) {
						errMsg(_T("More than 32767 points in track segment.  ")
							_T("Street Atlas file will be corrupt"));
						nPoints=32767;
					}
					// Write a track header
					writeSATrackHeader(fp,latitude,longitude,(short)nPoints);
				}
				// Write the trackpoint
				writeSATrackpoint(fp,latitude,longitude);
				++iter1;
			}
			++iter;
		}
	}

	// Trailer
	writeSATrailer(fp);

	// Cleanup
	lbprintf(_T("Wrote SA8 File:"));
	lbPrintSeparatedFileName(szFileName);
	fclose(fp);
	fp=NULL;
}

void writeGPXFile(TCHAR *szFileName)
{
	char *ansiString=NULL;
	DWORD result;

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiString);
	if(!ansiString) {
		errMsg(_T("Cannot convert unicode filename"));
		return;
	}

	fp=fopen(ansiString,"w");
	free(ansiString);
	ansiString=NULL;
	if(!fp) {
		errMsg(_T("Cannot write GPX file:\n  %s"),szFileName);
		return;
	}

	// Print header
	fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n");
	fprintf(fp,"<gpx\n");
	TCHAR fileVersion[80];
	BOOL status=getModuleFileVersion(fileVersion,80);
	if(status) {
		result=unicodeToAnsi(fileVersion,&ansiString);
		if(ansiString) {
			fprintf(fp," creator=\"GPSLink %s\"\n",ansiString);
			free(ansiString);
			ansiString=NULL;
		} else {
			fprintf(fp," creator=\"GPSLink\"\n");
		}
	} else {
		fprintf(fp," creator=\"GPSLink\"\n");
	}
	fprintf(fp," version=\"1.1\"\n");
	fprintf(fp," xmlns=\"http://www.topografix.com/GPX/1/1\"\n");
	fprintf(fp," xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
	fprintf(fp," xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 \n");
	fprintf(fp,"   http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

	// Print metadata
	fprintf(fp,"<metadata>\n");
	TCHAR timeString[TIME_CHARS_U_GPX];
	printCurrentTimeGPX(timeString);
	result=unicodeToAnsi(timeString,&ansiString);
	if(ansiString) {
		fprintf(fp,"  <time>%s</time>\n", ansiString);
		free(ansiString);
		ansiString=NULL;
	}
	fprintf(fp,"</metadata>\n");

	// Waypoints
	if(saveWpt) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;
		// Do the waypoints
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the method for the waypoint
			double latitude,longitude,height;
			latitude=pBlock->getLatitude();
			longitude=pBlock->getLongitude();
			height=pBlock->getAltitude()/M2FT;
			int smbl=((CWaypoint *)pBlock)->getSymbol();
			fprintf(fp,"<wpt lat=\"%.6f\" lon=\"%.6f\">\n",latitude,longitude);
			fprintf(fp," <ele>%.6f</ele>\n",height);
			fprintf(fp," <name>%s</name>\n",pBlock->getIdent());
			fprintf(fp," <desc>%s</desc>\n",pBlock->getIdent());
			fprintf(fp," <sym>%s</sym>\n",getSymbolName(smbl));
			fprintf(fp,"</wpt>\n");
			status=1;
			++iter;
		}
	}

	// Routes
	if(saveRte) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			// Do the route
			fprintf(fp,"<rte>\n");
			fprintf(fp," <name>%s</name>\n",pBlock->getIdent());
			fprintf(fp," <desc>%s</desc>\n",pBlock->getIdent());
			status=1;
			// Do the waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				double latitude,longitude,height;
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				height=pBlock1->getAltitude()/M2FT;
				int smbl=((CWaypoint *)pBlock1)->getSymbol();
				fprintf(fp," <rtept lat=\"%.6f\" lon=\"%.6f\">\n",latitude,longitude);
				fprintf(fp,"  <ele>%.6f</ele>\n",height);
				fprintf(fp,"  <name>%s</name>\n",pBlock1->getIdent());
				fprintf(fp,"  <desc>%s</desc>\n",pBlock1->getIdent());
				fprintf(fp,"  <sym>%s</sym>\n",getSymbolName(smbl));
				fprintf(fp," </rtept>\n");
				status=1;
				++iter1;
			}
			fprintf(fp,"</rte>\n");
			++iter;
		}
	}

	// Tracks
	if(saveTrk) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		BOOL status;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(saveCheckedOnly && !pBlock->isChecked()) {
				iter++;
				continue;
			}
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			// Do the track
			fprintf(fp,"<trk>\n");
			fprintf(fp," <name>%s</name>\n",pBlock->getIdent());
			fprintf(fp," <desc>%s</desc>\n",pBlock->getIdent());
			status=1;
			// Do the trackpoints
			BOOL started=FALSE;
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				if(saveCheckedOnly && !pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				long gTime;
				double latitude,longitude,height;
				char ansiTimeString[TIME_CHARS_A_GPX];

				// We want the GPX time to be GMT time, which is what is in the CTrackpoint
				gTime=((CTrackpoint *)pBlock1)->getGTime();
				if(gTime >= 0) {
					TCHAR timeString[TIME_CHARS_U_GPX];
					printGarminTime(timeString,gTime,Format_GPX);
					unicodeToAnsi(timeString,&ansiString);
					if(ansiString) {
						strncpy(ansiTimeString,ansiString,TIME_CHARS_A_GPX);
						ansiTimeString[TIME_CHARS_A_GPX-1]='\0';
						free(ansiString);
						ansiString=NULL;
					} else {
						strncpy(ansiTimeString,"Error",TIME_CHARS_A_GPX);
						ansiTimeString[TIME_CHARS_A_GPX-1]='\0';
					}
				} else {
					// If the time is GarminTime0 then we don't want to write the time element
					// It will cause validators to fail if we do
#if 0
					strncpy(ansiTimeString,"GarminTime0",TIME_CHARS_A_GPX);
					ansiTimeString[TIME_CHARS_A_GPX-1]='\0';
#else
					ansiTimeString[0]='\0';
#endif
				}
				latitude=pBlock1->getLatitude();
				longitude=pBlock1->getLongitude();
				height=pBlock1->getAltitude()/M2FT;
				if(((CTrackpoint *)pBlock1)->isNewTrack()) {
					if(started) fprintf(fp," </trkseg>\n");
					fprintf(fp," <trkseg>\n");
					started=TRUE;
				}
				fprintf(fp,"  <trkpt lat=\"%.6f\" lon=\"%.6f\">\n",latitude,longitude);
				fprintf(fp,"    <ele>%.6f</ele>\n",height);
				if(ansiTimeString[0] != '\0') {
					// Don't print the time element unless the time is valid
					fprintf(fp,"    <time>%s</time>\n",ansiTimeString);
				}
				fprintf(fp,"  </trkpt>\n");
				status=1;
				++iter1;
			}
			if(started) fprintf(fp," </trkseg>\n");
			fprintf(fp,"</trk>\n");
			++iter;
		}
	}

	// Cleanup
	fprintf(fp,"</gpx>\n");
	lbprintf(_T("Wrote GPX File:"));
	lbPrintSeparatedFileName(szFileName);
	fclose(fp);
	fp=NULL;
}

static void lbPrintSeparatedFileName(TCHAR *fileName)
{
	TCHAR tempName[_MAX_PATH];
	_tcscpy(tempName,fileName);
	TCHAR *ptr=_tcsrchr(tempName,'\\');
	if(ptr) {
		*ptr='\0';
		lbprintf(_T(" %s"),tempName);
		lbprintf(_T(" %s"),ptr+1);
	} else {
		lbprintf(_T(" %s"),tempName);
	}
}
