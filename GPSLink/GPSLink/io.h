// Header for I/O routines

#include "CData.h"

#ifndef _INC_IO_H
#define _INC_IO_H

int listWaypoint(CWaypoint *pWaypoint);
int listPosition(CPosition *pPosition);
void listWaypoints(void);
void listRoutes(void);
void listTracks(void);
void readGPSLFile(TCHAR *szFileName);
void writeGPSLFile(TCHAR *szFileName, char delimiter);
void readG7ToWinFile(TCHAR *szFileName);
void readGPXFile(TCHAR *szFileName);
void readImageFile(TCHAR *szFileName);
void writeGPSUFile(TCHAR *szFileName);
void writeSA8File(TCHAR *szFileName);
void writeGPXFile(TCHAR *szFileName);

#endif // _INC_IO_H
