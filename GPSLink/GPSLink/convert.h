// Header for conversion routines

#ifndef _INC_CONVERT_H
#define _INC_CONVERT_H

#define SEMI2DEG (180.0/0x80000000)
#define RAD2DEG 57.29577951
#define M2FT 3.280839895
#define MPS2MPH 2.236936292
#define DAY2SEC 86400.0

#define TIME_CHARS_A_AMPM 23
#define TIME_CHARS_A_24   20
#define TIME_CHARS_A_GPX  21
#define TIME_CHARS_U_AMPM TIME_CHARS_A_AMPM*sizeof(TCHAR)
#define TIME_CHARS_U_24   TIME_CHARS_A_24*sizeof(TCHAR)
#define TIME_CHARS_U_GPX  TIME_CHARS_A_GPX*sizeof(TCHAR)
#define CURTIME_CHARS_U   23*sizeof(TCHAR)

#include "CData.h"

typedef enum {
    Format_AMPM,
    Format_24,
    Format_GPX,
} TimeFormat;

// Function prototypes
void timeInit(void);
void printGarminTime(LPTSTR szTime, long gTime, TimeFormat fmt);
void printCurrentTime(LPTSTR szTime);
void printCurrentTimeGPX(LPTSTR szTime);
void printElapsedTime(LPTSTR szTime, long gTime);
long convertTimeStringToLong(char *timeString);
long convertSystemTimeToLong(SYSTEMTIME sTime);
long convertUnixTimeToLong(time_t unixTime);
void convertUnixTimeToFileTime(time_t t, LPFILETIME pft);
void convertUnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst);
long getTimeOffset(void);
double greatCircleDistance(double lat1, double lat2, double lon1, double lon2);
double getDistance(CBlock *block1, CBlock *block2);
char *getSymbolName(int index);
char *getSymbolKeyName(int index);
char *getEtrexSymbolKeyName(int index);
int getSymbolNumber(char *symbol);
double getElapsedTime(SYSTEMTIME sPrev);
void hsort(char *array[], int indx[], int n, int foldCase);

// Global variables
extern int nSymbols;
extern int nSymbolsEtrex;

#endif // _INC_CONVERT_H
