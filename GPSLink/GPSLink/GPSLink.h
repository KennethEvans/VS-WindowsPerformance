// Header file for GPSLink

#if !defined(_GPSLINK_H)
#define _GPSLINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define GPSLINK_ID "!GPSLINK"

#define WM_BEGINDRAWPOS  (WM_USER+1)
#define WM_DRAWPOS       (WM_USER+2)
#define WM_ENDDRAWPOS    (WM_USER+3)
#define WM_RESETCALPOINT (WM_USER+4)

#define MENU_HEIGHT 26

#include <stdio.h>
#include "console.h"
#include "utils.h"
#include "progress.h"
#include "convert.h"
#include "io.h"
#include "sa.h"
#include "garmin.h"
#include "CData.h"
#include "CMap.h"

typedef enum {
    EDIT_CUT,
    EDIT_COPY,
    EDIT_PASTE,
    EDIT_DELETE,
} EditType;

typedef enum {
    NEW_WPT,
    POS_WPT,
    EDIT_WPT,
    EDIT_RTE_WPT,
    LV_WPT,
    EDIT_TPT,
    LV_TPT,
} EditDialogType;

typedef enum {
    FORWARD,
    BACK,
} Direction;

// Function prototypes

// GPSLink.cpp
void startPVTData(void);
void stopPVTData(void);
void viewWindow(HWND hWnd);

// edit.cpp
void refreshLV(HWND hLV);
BOOL deleteCheckedBlocks(BOOL val);
BOOL pasteBlocks(WPARAM wParam);
BOOL checkSelectedBlocks(BOOL checkState);
BOOL checkBlocks(DataType type, BOOL checkState, BOOL doList, BOOL doParent=TRUE);
BOOL deleteBlocks(DataType type);
BOOL removeSelectedBlocks(EditType editType);
void trackListSetStart();

// comm.cpp
BOOL PortInitialize(LPTSTR);
BOOL PortClose(void);
DWORD WINAPI PortReadThread(LPVOID);
BOOL GetID(void);
void queueCommand(CommandType cmdType);
void abortCurrentTransfer(void);
BOOL sendPacket(BYTE *data, BYTE *buf, PacketType pktType,
  BYTE nData, DWORD *nBuf);

// wptLV.cpp
LRESULT notifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
CBlock *lvGetBlockFromItem(HWND hLV, int iItem);
CBlock *lvGetBlockFromPoint(HWND hLV, POINT point);
UINT lvGetHitFlagFromPoint(HWND hLV, POINT point);
LRESULT wptNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
void wptCreateLV(HWND hWndParent);
void wptGetSelections(void);
 
// rteLV.cpp
LRESULT rteNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
void rteCreateLV(HWND hWndParent);
void rteGetSelections(void);
 
// trkLV.cpp
LRESULT trkNotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
  LPARAM lParam);
void trkCreateLV(HWND hWndParent);
void trkGetSelections(void);
void trkConvertS(void);
void trkConvertAllS(int type);
void trkNextS(Direction direction);

// map.cpp
void createMapWindow(HWND hWndParent);
void changeMapImage(CMap *pNewMap);
void doEditWaypoint(EditDialogType type);
void doEditTrackpoint(EditDialogType type);
void positionToLatLon(double latitude, double longitude);
CBlock *getTrackFromTrackpoint(CBlock *pBlock);

// Global variables
extern CBlockList waypointList;
extern CBlockList routeList;
extern CBlockList trackList;
extern CBlockList clipboardList;
extern HINSTANCE hInst;        // The current instance
extern HWND hWndMain;          // The main window
extern HWND hLB;               // ListBox handle
extern HWND hWptLV;            // Waypoint ListView handle
extern HWND hRteLV;            // Route ListView handle
extern HWND hTrkLV;            // Track ListView handle
extern HWND hMap;              // Map window ListView handle
extern HWND hCurWnd;           // Window that is currently in view
extern HANDLE hPort;           // Serial port handle
extern HANDLE hReadThread;     // Handle to the read thread
extern FILE *fp;               // Handle to FILE
extern char delimiter;         // Delimiter for writing files
extern BOOL pvtDataInProgress; // Flag for PVT data
extern CPosition position;     // CPosition for PVT data
extern CBlock *pMenuBlock;     // Block corresponding to popup menu
extern CMap pMapDefault;
extern CMap *pMap;
extern CMap *pMapFile;
extern BOOL saveWpt;
extern BOOL saveRte;
extern BOOL saveTrk;
extern BOOL saveMap;
extern BOOL saveMapShortName;
extern BOOL saveCheckedOnly;
extern BOOL showTrackpoints;
extern BOOL showTrackpointsOld;
extern BOOL showRouteWaypoints;
extern BOOL showRouteWaypointsOld;
extern COLORREF mapWptColor;
extern COLORREF mapRteColor;
extern COLORREF mapTrkColor;
extern COLORREF mapCalColor;
extern COLORREF mapBgColor;
extern COLORREF mapPosColor;
extern HMENU hLvPopupMenu;
extern HCURSOR specialCursor;

#endif // !defined(_GPSLINK_H)
