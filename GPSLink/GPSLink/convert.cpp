// Conversion routines for GPSLink

#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include "garmin.h"
#include "convert.h"
#include "utils.h"

#define REARTH 3956. // mi Based on def. of nautical mi. (Varies 3937 to 3976 mi)
#define MI2NMI 1.852 // Exact
#define DEG2RAD .01745329252     // pi/180
// Use to convert time
#define _SECOND ((__int64) 10000000)
#define _MINUTE (60 * _SECOND)
#define _HOUR   (60 * _MINUTE)
#define _DAY    (24 * _HOUR) 

// Global variables
BOOL timeInitialized=FALSE;
int nSymbols=sizeof(symKey)/sizeof(SymbolKey);
int nSymbolsEtrex=sizeof(symKeyEtrex)/sizeof(SymbolKey);
static long gmtTimeOffset=0;
static SYSTEMTIME garminSystemTime0={1989,12,0,31,0,0,0,0};
static FILETIME garminFileTime0;  // Sun Dec 31 1989 00:00:00

void timeInit(void)
{
	if(timeInitialized) return;
	SYSTEMTIME sSystemTime,sLocalTime;
	FILETIME fLocalTime,fSystemTime;
	ULONGLONG qwLocalTime,qwSystemTime,qwDiff;

	// Set file time for Garmin  time zero
	SystemTimeToFileTime(&garminSystemTime0,&garminFileTime0);

	// Determine the local offset from GMT in seconds
	GetSystemTime(&sSystemTime);
	SystemTimeToFileTime(&sSystemTime,&fSystemTime);
	FileTimeToLocalFileTime(&fSystemTime,&fLocalTime);
	FileTimeToSystemTime(&fLocalTime,&sLocalTime);
	qwSystemTime=(((ULONGLONG)fSystemTime.dwHighDateTime)<<32) +
		fSystemTime.dwLowDateTime;
	qwLocalTime=(((ULONGLONG)fLocalTime.dwHighDateTime)<<32) +
		fLocalTime.dwLowDateTime;
	// These are unsigned values so we have to be careful to subtract
	// the smaller from the larger
	if(qwLocalTime > qwSystemTime) {
		gmtTimeOffset=(long)((qwLocalTime-qwSystemTime)/_SECOND);
	} else {
		gmtTimeOffset=-(long)((qwSystemTime-qwLocalTime)/_SECOND);
	}
	qwDiff=qwSystemTime-qwLocalTime;
#if 0
	lbprintf(_T("1234567891123456789212345678931234567894")
		_T("12345678951234567896123456789712345678981234567899"));
#endif
}

long getTimeOffset(void)
{
	// Initialize the time variables
	if(!timeInitialized) timeInit();

	return(gmtTimeOffset);
}

void printCurrentTime(LPTSTR szTime)
// Prints the current time in the given buffer
{
	SYSTEMTIME sTime;
	WORD hour;
	static TCHAR monthName[12][4]={_T("Jan"),_T("Feb"),_T("Mar"),_T("Apr"),
		_T("May"),_T("Jun"),_T("Jul"),_T("Aug"),
		_T("Sep"),_T("Oct"),_T("Nov"),_T("Dec")
	};

	// Get StyatemTime is local, not UTC
	GetLocalTime(&sTime);
	// Print the result
	if(sTime.wHour < 1) {
		hour = sTime.wHour + 12;
	} else if(sTime.wHour < 13) {
		hour = sTime.wHour;
	} else {
		hour = sTime.wHour - 12;
	}
	// Size should be 23 chars including null
	_stprintf(szTime,_T("%s %02d, %04d %02d:%02d:%02d%s"),
		monthName[sTime.wMonth-1],sTime.wDay,sTime.wYear,
		hour,
		sTime.wMinute,
		sTime.wSecond,
		sTime.wHour<12?_T("a"):_T("p"));
}

void printCurrentTimeGPX(LPTSTR szTime)
// Prints the current time in GPX format to the given buffer
{
	SYSTEMTIME sTime;
	static TCHAR monthName[12][4]={_T("Jan"),_T("Feb"),_T("Mar"),_T("Apr"),
		_T("May"),_T("Jun"),_T("Jul"),_T("Aug"),
		_T("Sep"),_T("Oct"),_T("Nov"),_T("Dec")
	};

	// Get StyatemTime is UTC
	GetSystemTime(&sTime);

	// Print the result
	// Size should be 21 chars incl null
	_stprintf(szTime,_T("%04d-%02d-%02dT%02d:%02d:%02dZ"),
		sTime.wYear,sTime.wMonth,sTime.wDay,
		sTime.wHour,
		sTime.wMinute,
		sTime.wSecond);
}

void printGarminTime(LPTSTR szTime, long gTime, TimeFormat fmt)
// time is the Garmin time in a long
// Prints the Garmin time in the given buffer
{
	SYSTEMTIME sTime0, sTime;
	FILETIME fTime;
	ULONGLONG qwTime;
	WORD hour;
	BOOL status;
	static TCHAR monthName[12][4]={_T("Jan"),_T("Feb"),_T("Mar"),_T("Apr"),
		_T("May"),_T("Jun"),_T("Jul"),_T("Aug"),
		_T("Sep"),_T("Oct"),_T("Nov"),_T("Dec")
	};

	// Initialize the time variables
	if(!timeInitialized) timeInit();

	// Do the 64-bit arithmetic (fTime is then UTC time for the gTime)
	qwTime=(((ULONGLONG)garminFileTime0.dwHighDateTime)<<32)+
		garminFileTime0.dwLowDateTime+
		gTime*_SECOND;
	fTime.dwLowDateTime=(DWORD)(qwTime&0xFFFFFFFF);
	fTime.dwHighDateTime=(DWORD)(qwTime>>32);

	// Convert to system time (sTime0 is then UTC time for the gTime)
	status=FileTimeToSystemTime(&fTime,&sTime0);
	if(!status) {
		_stprintf(szTime,_T("FileTimeToSystemTime failed"));
		return;
	}

	// Convert to local time (sTime is then local time for the gTime)
	// (NULL for the LPTIME_ZONE_INFORMATION means current time zone)
	// Not done for Format_GPX
	if(fmt != Format_GPX) {
		SystemTimeToTzSpecificLocalTime(NULL, &sTime0, &sTime);
	}

	// Print the result
	switch(fmt) {
	case Format_AMPM:
		if(sTime.wHour < 1) {
			hour = sTime.wHour + 12;
		} else if(sTime.wHour < 13) {
			hour = sTime.wHour;
		} else {
			hour = sTime.wHour - 12;
		}
		// Size should be 23 chars incl null
		_stprintf(szTime,_T("%s %2d, %04d %2d:%02d:%02d%s"),
			monthName[sTime.wMonth-1],sTime.wDay,sTime.wYear,
			hour,
			sTime.wMinute,
			sTime.wSecond,
			sTime.wHour<12?_T("a"):_T("p"));
		break;
	case Format_24:
		// Size should be 20 chars incl null
		_stprintf(szTime,_T("%02d/%02d/%04d %02d:%02d:%02d"),
			sTime.wMonth,sTime.wDay,sTime.wYear,
			sTime.wHour,
			sTime.wMinute,
			sTime.wSecond);
		break;
	case Format_GPX:
		// Size should be 21 chars incl null
		// For this case we want UTC time
		_stprintf(szTime,_T("%04d-%02d-%02dT%02d:%02d:%02dZ"),
			sTime0.wYear,sTime0.wMonth,sTime0.wDay,
			sTime0.wHour,
			sTime0.wMinute,
			sTime0.wSecond);
		break;
	}
}

void printElapsedTime(LPTSTR szTime, long gTime)
{
	if(gTime < 0) {
		_stprintf(szTime,_T("Invalid"));
		return;
	}
	long days=gTime/(3600*24);
	long rem=gTime-days*3600*24;
	long hours=rem/3600;
	rem-=hours*3600;
	long minutes=rem/60;
	rem-=minutes*60;
	long seconds=rem;
	if(days) {
		_stprintf(szTime,_T("%d %s %02d:%02d:%02d"),
			days,days==1?_T("Day"):_T("Days"),hours,minutes,seconds);
#if 1
	} else {
		_stprintf(szTime,_T("%02d:%02d:%02d"),
			hours,minutes,seconds);
#else
	} else if(hours) {
		_stprintf(szTime,_T("%02d:%02d:%02d"),
			hours,minutes,seconds);
	} else if(minutes) {
		_stprintf(szTime,_T("   %02d:%02d"),
			minutes,seconds);
	} else {
		_stprintf(szTime,_T("      %02d"),
			seconds);
#endif
	}
}

// Converts a Format_24 string to a long
long convertTimeStringToLong(char *timeString)
{
	SYSTEMTIME sTime;
	long gTime;
	char *token;

	// Initialize the time variables
	if(!timeInitialized) timeInit();

	// Check if the time is GarminTime0
	if(!strcmp(timeString,"GarminTime0")) return(-1);

	// Parse string (sTime is the local time corresponding to the string)
	sTime.wDayOfWeek=0;
	sTime.wMilliseconds=0;

	// Month
	token=strtok(timeString,"/\n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wMonth=(WORD)atoi(token);
	}

	// Day
	token=strtok(NULL,"/\n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wDay=(WORD)atoi(token);
	}

	// Year
	token=strtok(NULL," \n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wYear=(WORD)atoi(token);
	}

	// Hour
	token=strtok(NULL,":\n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wHour=(WORD)atoi(token);
	}

	// Minute
	token=strtok(NULL,":\n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wMinute=(WORD)atoi(token);
	}

	// Second
	token=strtok(NULL,"\n");
	if(!token) {
		errMsg(_T("Error converting time string:\n%s"),timeString);
		return(-1);
	} else {
		sTime.wSecond=(WORD)atoi(token);
	}

	gTime=convertSystemTimeToLong(sTime);

	return(gTime);
}

// Converts a SYSTEMTIME to a long
long convertSystemTimeToLong(SYSTEMTIME sTime)
{
	FILETIME fTime;
	long gTime;

	// Initialize the time variables
	if(!timeInitialized) timeInit();

	// Convert to UTC time
	// (sTime is then the UTC time corresponding to the string)
	// (NULL for the LPTIME_ZONE_INFORMATION means current time zone)
	SYSTEMTIME sTime0;
	TzSpecificLocalTimeToSystemTime(NULL, &sTime, &sTime0);

	// Convert to local file time
	BOOL status=SystemTimeToFileTime(&sTime0,&fTime);
	if(!status) {
		errMsg(_T("SystemTimeToFileTime failed"));
		return(-1);
	}

	// Convert to 64-bit integer
	ULONGLONG qwTime=(((ULONGLONG)fTime.dwHighDateTime)<<32)+
		fTime.dwLowDateTime;
	ULONGLONG qwGTime0=(((ULONGLONG)garminFileTime0.dwHighDateTime)<<32)+
		garminFileTime0.dwLowDateTime;
	if(qwTime > qwGTime0) gTime=(long)((qwTime-qwGTime0)/_SECOND);
	else gTime=-(long)((qwGTime0-qwTime)/_SECOND);

	return(gTime);
}

// Converts a UTC time_t to a long
long convertUnixTimeToLong(time_t unixTime)
{
	// Initialize the time variables
	if(!timeInitialized) timeInit();

	// Convert to a FILETIME
	FILETIME fTime;
	convertUnixTimeToFileTime(unixTime, &fTime);

	// Convert to 64-bit integer
	long gTime;
	ULONGLONG qwTime=(((ULONGLONG)fTime.dwHighDateTime)<<32)+
		fTime.dwLowDateTime;
	ULONGLONG qwGTime0=(((ULONGLONG)garminFileTime0.dwHighDateTime)<<32)+
		garminFileTime0.dwLowDateTime;
	if(qwTime > qwGTime0) gTime=(long)((qwTime-qwGTime0)/_SECOND);
	else gTime=-(long)((qwGTime0-qwTime)/_SECOND);

	return(gTime);
}

// Converts a time_t to a FILETIME
void convertUnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

// Converts a time_t to a SYSTEMTIME
void convertUnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)
{
	FILETIME ft;

	convertUnixTimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, pst);
}



// Returns great circle distance in mi assuming a spherical earth
double greatCircleDistance(double lat1, double lon1, double lat2, double lon2)
{
	double slon,slat,a,c,d;

	// Convert to radians
	lat1*=DEG2RAD;
	lon1*=DEG2RAD;
	lat2*=DEG2RAD;
	lon2*=DEG2RAD;

	// Haversine formula
	slon=sin((lon2-lon1)/2.);
	slat=sin((lat2-lat1)/2.);
	a=slat*slat+cos(lat1)*cos(lat2)*slon*slon;
	c=2*atan2(sqrt(a),sqrt(1-a));
	d=REARTH*c;

	return(d);
}

double getDistance(CBlock *block1, CBlock *block2)
{
	double d;

	if(!block1 || !block2) return 0.0;

	d=greatCircleDistance(block1->getLatitude(),block1->getLongitude(),
		block2->getLatitude(),block2->getLongitude());

	return d;
}

int getSymbolNumber(char *symbol)
{
	int i;
	int found=0;

	for(i=0; i < nSymbols; i++) {
		SymbolKey key=symKey[i];
		if(!strcmp(symbol,key.name)) return key.index;
	}
	return 0;
}

char *getSymbolName(int index)
{
	int i;
	int found=0;

	for(i=0; i < nSymbols; i++) {
		SymbolKey key=symKey[i];
		if(index == key.index) return key.name;
	}
	return "Not Found";
}

char *getSymbolKeyName(int index)
{
	SymbolKey key=symKey[index];
	return key.name;
}

char *getEtrexSymbolKeyName(int index)
{
	SymbolKey key=symKeyEtrex[index];
	return key.name;
}

// Return elapsed time in sec
double getElapsedTime(SYSTEMTIME sPrev)
{
	SYSTEMTIME sCur;
	FILETIME fPrev,fCur;
	LONGLONG qwPrev,qwCur,qwDiff;

	GetSystemTime(&sCur);
	SystemTimeToFileTime(&sPrev,&fPrev);
	SystemTimeToFileTime(&sCur,&fCur);
	qwPrev=(((LONGLONG)fPrev.dwHighDateTime)<<32) +
		fPrev.dwLowDateTime;
	qwCur=(((LONGLONG)fCur.dwHighDateTime)<<32) +
		fCur.dwLowDateTime;
	qwDiff=qwCur-qwPrev;

	// FILETIME is number of 100 ns intervals
	return (double)qwDiff*1.e-7;
}

//********************* hsort ****************************************/
void hsort(char *array[], int indx[], int n, int foldCase)
// ASCII sort
{
	int l,j,ir,indxt,i;
	char *q;
	int (*cmp)(const char *string1, const char *string2);

	// All done if none or one element
	if(n == 0) return;
	if(n == 1) {
		indx[0]=0;
		return;
	}

	// Determine the compare function
#ifdef UNDER_CE
	if(foldCase) cmp = _stricmp;
	else cmp = strcmp;
#else
	if(foldCase) cmp = stricmp;
	else cmp = strcmp;
#endif

	// Initialize indx array
	for(j=0; j < n; j++) indx[j]=j;
	// Loop over elements
	l=(n>>1);
	ir=n-1;
	for(;;) {
		if(l > 0) q=array[(indxt=indx[--l])];
		else {
			q=array[(indxt=indx[ir])];
			indx[ir]=indx[0];
			if(--ir == 0) {
				indx[0]=indxt;
				return;
			}
		}
		i=l;
		j=(l<<1)+1;
		while(j <= ir) {
			if(j < ir && (*cmp)(array[indx[j]],array[indx[j+1]]) < 0) j++;
			if((*cmp)(q,array[indx[j]]) < 0) {
				indx[i]=indx[j];
				j+=((i=j)+1);

			}
			else break;
		}
		indx[i]=indxt;
	}
}
