// Map window implementation

#include "stdafx.h"

#define TEST 1

#define DEBUG_DRAG 0
#define DEBUG_CLICK 0
#define DEBUG_DRAW 0
#define DEBUG_COMPASS 0
#define DEBUG_GAMMA 0

#define MAPWINDOWCLASS _T("GPSLINKMAP")
#define MAPCOMPASSCLASS _T("GPSLINKMAPCOMPASS")
#define MAPCURPOSCLASS _T("GPSLINKMAPCURPOS")

#define MAX_ENTRY_SIZE 25
#define DEFAULT_SYMBOL 178 // Flag
#define DEFAULT_POS_SYMBOL 18 // Waypoint Dot
#define USE_TAHOMA 1  // Use tahomna for the font

#define BGCOLOR RGB(156,207,156)

#ifdef UNDER_CE
// Is not supported, set this to one to test mechanisms, though
#define DO_GAMMA 0           // Set to implement gamma calibration
#else
#define DO_GAMMA 1           // Set to implement gamma calibration
#endif
#define GAMMA_IMAGE_ONLY 0   // Set to only draw image while adjusting
#define GAMMA_MULT 10        // Multiplier for gamma slider (Program defined)
#define GAMMA_MIN  2500      // Minimum value for gamma (Windows defined)
#define GAMMA_MAX 65000      // Maximum value for gamma (Windows defined)
#define GAMMA_DEFAULT 10000  // Gamma for no correction
#define GAMMA_LIGHT 5000     // Light convenience value
#define GAMMA_MEDIUM 7500    // Medium convenience value

#define MAPMESSAGE _T("Map Window")
#define MAPLIMITSSCALEFAC .2
#define LEFT_OFFSET 20
#define TOP_OFFSET 20
#define COMPASS_WIDTH 38  // 2 More than icon width, height
#define COMPASS_HEIGHT 38
#define COMPASS_MARGIN 5
#define SYMBSIZE 4
#define SQUARESIZE 1
#define POSITIONSIZE 9  // An odd number
#define POSITIONOFFSET (POSITIONSIZE/2) // Offset from point for MoveWindow
#define HOLDTIME 0.5

#define LONMIN -88.5
#define LONMAX -87.5
#define LATMIN 41.0
#define LATMAX 42.0
// Corresponds to about 364 ft or .69 mi
#define LATLON_EXTEND .001

#define ZOOMFAC 1.5
#define MOVEFAC_LARGE .5
#define MOVEFAC_SMALL .02

#include <math.h>
#include <float.h>
#include "GPSLink.h"
#include "resource.h"
#include "console.h"
#include "matrix.h"
#include "CMap.h"

typedef enum {
	CUR_POS_START,
	CUR_POS_CONTINUE,
	CUR_POS_END,
} CurPosMode;

typedef enum {
	CENTER_TOP,
	CENTER_CENTER,
	CENTER_BOTTOM,
	TOP_LEFT,
} PositionType;

typedef enum {
	CAL_NUMBER,
	WPT_NUMBER,
	POS_NUMBER,
} IncrementalNumberType;

typedef enum {
	SET_ALL,
	SET_ALL_EXC_INDEX,
	SET_LATLON,
} TrackSetValuesType;

// Function ptototypes

static ATOM registerMapClass(void);
static LRESULT CALLBACK mapWndProc(HWND hWnd, UINT message,
								   WPARAM wParam, LPARAM lParam);
static void paint(HWND hWnd);
static void createMapCompass(HWND hWndParent);
static ATOM registerMapCompassClass(void);
static LRESULT CALLBACK mapCompassWndProc(HWND hWnd, UINT message,
										  WPARAM wParam, LPARAM lParam);
static void paintCompass(HWND hWnd);
static void createMapCurPos(HWND hWndParent);
static ATOM registerMapCurPosClass(void);
static LRESULT CALLBACK mapCurPosWndProc(HWND hWnd, UINT message,
										 WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK calibrateDlgProc(HWND hDlg, UINT message,
										 WPARAM wParam, LPARAM lParam);
#if DO_GAMMA
static LRESULT CALLBACK gammaDlgProc(HWND hDlg, UINT message,
									 WPARAM wParam, LPARAM lParam);
static void resetGamma(void);
#endif
LRESULT CALLBACK editWaypointDlgProc(HWND hDlg, UINT message,
									 WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK editTrackpointDlgProc(HWND hDlg, UINT message,
									   WPARAM wParam, LPARAM lParam);
static void editWaypointSetValues(HWND hDlg, double lat, double lon, double alt,
								  char *name, int sym);
static void editTrackpointSetValues(HWND hDlg, double lat, double lon, double alt,
									int index, BOOL isNewTrack, TrackSetValuesType type);
static void paintCurPos(HWND hWnd);
static void positionToPoint(PositionType type);
static void zoomin(void);
static void zoomout(void);
static void zoomSpecified(double factor);
static void zoomNormal(void);
static void zoom(double x0, double y0, double x1, double y1);
static void move(int direction, double movefac);
static void moveup(double movefac);
static void movedown(double movefac);
static void moveright(double movefac);
static void moveleft(double movefac);
static void drawCross(HDC hdc, long X, long Y);
static void drawLine(HDC hdc, long X1, long Y1, long X2, long Y2);
static void drawSquare(HDC hdc, long X, long Y);
static void drawBoxOutline(HWND hwnd, POINT beg, POINT end);
static void getLatLonEnvelope(double *pLatMin, double *pLonMin,
							  double *pLatMax, double *pLonMax);
static BOOL adjustAspectRatio(double aspect, double *pLatMin, double *pLonMin,
							  double *pLatMax, double *pLonMax);
static BOOL restore(void);
static void drawCurrentPosition(CurPosMode mode);
static BOOL calculateLatLon(long xPix, long yPix,
							double *latitude, double *longitude);
static BOOL calculatePixel(double latitude, double longitude,
						   long *xPix, long *yPix);
static BOOL resetTransformation(void);
static int getNextNumber(IncrementalNumberType type);
static BOOL getBlockFromIndex(int index, CBlockList *pBlockList,
							  CBlock **ppBlock);
static BOOL findClosestWaypoint(int *pSel, CBlock **ppBlockSel,
								double *pDist);
static BOOL findClosestRouteWaypoint(int *pSel, CBlock **ppBlockSel,
									 double *pDist);
static BOOL findClosestTrackpoint(int *pSel, CBlock **ppBlockSel,
								  double *pDist);
static CBlock *getRouteFromWaypoint(CBlock *pBlock);
static void calculateTrackIndices(void);
static void doGoToLVWaypoint(void);
static void doGoToLVRouteWaypoint(void);
static void doGoToLVTrackpoint(void);

// Global variables

#if DO_GAMMA
HWND hGammaDlg=NULL;
BOOL adjustGamma=FALSE;
COLORADJUSTMENT colorAdjust={0};
WORD gamma = 10000;  // 2500 - 65,000, 10,000 is no adjustment
BOOL adjustGammaInProgress=FALSE;
HPALETTE hPalette = NULL;
#endif
HWND hMapCompass=NULL;
HDC hCompassDC=NULL;
HWND hMapCurPos=NULL;
HDC hCurPosDC=NULL;
HBITMAP hCompassBitmap = NULL;
HMENU hMapMenu=NULL;
HMENU hCurPosMenu=NULL;
HWND hCalDlg=NULL;
double pi=acos(-1.);
TCHAR szMapString[80];     // Used for debugging only ?
double xOff,yOff,scale,width,height;
BOOL plotWaypoints=TRUE;
BOOL plotRoutes=TRUE;
BOOL plotTracks=TRUE;
Matrix a(2,2),ainv(2,2),b(2,1);
BOOL isValid=FALSE;
COLORREF mapBgColor=BGCOLOR;
POINT menuPoint={0,0};
int nextCalNumber=0;
int nextWptNumber=0;
int nextPosNumber=0;
double xClickMap=0.0,yClickMap=0.0;
BOOL showCompass=TRUE;
BOOL drawTrackpoints=FALSE;
BOOL showCalibrationPoints=TRUE;
EditDialogType editDialogType=NEW_WPT;
static double measureStartLat=DBL_MIN;
static double measureStartLon=DBL_MIN;
static double measureEndLat=DBL_MIN;
static double measureEndLon=DBL_MIN;
static double measureDistance=0.0;

/////////////////////////////////////////////////////////////
//////////////////////// Map Window /////////////////////////
/////////////////////////////////////////////////////////////

void createMapWindow(HWND hWndParent)
{
	RECT rect;

	if(!hMap) {
		// Create the control
		// Ensure that the common control DLL is loaded.

		// Get the size and position of the parent window.
		GetClientRect(hWndParent,&rect);
		width=rect.right-rect.left;
		height=rect.bottom-rect.top;

		// Register the class
		registerMapClass();

		// Create the window
		hMap=CreateWindowEx(0L,
			MAPWINDOWCLASS,                 // Class
			_T("GPSLINK Map"),              // Title
			WS_VISIBLE|WS_CHILD|WS_BORDER|  // Style
			WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			rect.top,rect.left,rect.right-rect.left,rect.bottom-rect.top,
			hWndParent,
			(HMENU)ID_MAP_WINDOW,           // Child window ID
			hInst,
			NULL);

		if(hMap == NULL ) return;

		// Load the popupmenu
		hMapMenu=LoadMenu(hInst,MAKEINTRESOURCE(IDM_MAPMENU));
		hMapMenu=GetSubMenu(hMapMenu,0);

		// Create the compass
		if(!hMapCompass) createMapCompass(hMap);

		// Initialize the CMap
		if(!pMap) pMap=&pMapDefault;
		isValid=restore();
	} else {
		// Already created, set the window to no zooming, scaled to fit
		// points
		isValid=restore();

		// Make it repaint
		InvalidateRect(hMap,NULL,FALSE);
	}

	// Update the window 
	UpdateWindow(hMap);
}

static ATOM registerMapClass(void)
{
	WNDCLASS wc;

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)mapWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=sizeof(long);
	wc.hInstance=hInst;
	wc.hIcon=NULL;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=MAPWINDOWCLASS;

	return RegisterClass(&wc);
}

static LRESULT CALLBACK mapWndProc(HWND hWnd, UINT message,
								   WPARAM wParam, LPARAM lParam)
{
	int wmId,wmEvent;
#if 0
	static SYSTEMTIME btnDownTime;
#endif
	static RECT box;
	static POINT beg,end,point;
	static BOOL btnDown=FALSE;
	static BOOL boxSelected=FALSE;
	static BOOL drag=FALSE;
#ifdef UNDER_CE
	SHRGINFO shrg;
	DWORD gesture;
#endif

	switch(message) {
	case WM_BEGINDRAWPOS:
		drawCurrentPosition(CUR_POS_START);	
		break;
	case WM_DRAWPOS:
		drawCurrentPosition(CUR_POS_CONTINUE);	
		break;
	case WM_ENDDRAWPOS:
		drawCurrentPosition(CUR_POS_END);	
		break;
	case WM_CREATE:
		break;
#ifndef UNDER_CE
	case WM_RBUTTONDOWN:
		// Popup menu
		menuPoint.x=LOWORD(lParam);  // (Relative to client area)
		menuPoint.y=HIWORD(lParam);
		point=menuPoint;
		ClientToScreen(hWnd,&point);
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_SHOWCOMPASS,
			showCompass?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_TRACKPOINTS,
			drawTrackpoints?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_CALIBRATIONPOINTS,
			showCalibrationPoints?MF_CHECKED:MF_UNCHECKED);
		TrackPopupMenu(hMapMenu,
			TPM_LEFTALIGN|TPM_VCENTERALIGN,
			point.x,point.y,0,hWnd,NULL);
		break;
#endif
	case WM_LBUTTONDOWN:
		if(boxSelected) {
			// Cancel box
			POINT cur;
			cur.x=LOWORD(lParam);  // (Relative to client area)
			cur.y=HIWORD(lParam);
			drawBoxOutline(hWnd,beg,end);
			boxSelected=FALSE;
			if(PtInRect(&box,cur)) {
				// Click is in box, zoom
				double x0,y0,x1,y1;
				x0=(double)box.left;
				x1=(double)box.right;
				y0=(double)box.top;
				y1=(double)box.bottom;
				zoom(x0,y0,x1,y1);
				InvalidateRect(hMap,NULL,FALSE);
				UpdateWindow(hMap);
			}
		} else {
#ifdef UNDER_CE
			// Check for tap-hold
			menuPoint.x=LOWORD(lParam);  // (Relative to client area)
			menuPoint.y=HIWORD(lParam);
			shrg.cbSize=sizeof(SHRGINFO);
			shrg.hwndClient=hWnd;
			shrg.ptDown=menuPoint;
			shrg.dwFlags=SHRG_RETURNCMD;
			gesture=SHRecognizeGesture(&shrg);
			if(gesture == GN_CONTEXTMENU) {
				// Popup menu
				point=menuPoint;
				ClientToScreen(hWnd,&point);
				CheckMenuItem(hMapMenu,ID_MAPOPTIONS_SHOWCOMPASS,
					showCompass?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem(hMapMenu,ID_MAPOPTIONS_TRACKPOINTS,
					drawTrackpoints?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem(hMapMenu,ID_MAPOPTIONS_CALIBRATIONPOINTS,
					showCalibrationPoints?MF_CHECKED:MF_UNCHECKED);
				TrackPopupMenu(hMapMenu,
					TPM_LEFTALIGN|TPM_VCENTERALIGN,
					point.x,point.y,0,hWnd,NULL);
				break;
			}
#endif
			// Start drag
			beg.x=end.x=LOWORD(lParam);  // (Relative to client area)
			beg.y=end.y=HIWORD(lParam);
			btnDown=TRUE;
			drag=FALSE;
#if 0
			GetSystemTime(&btnDownTime);
#endif
			SetCursor(LoadCursor(NULL,IDC_CROSS));
			drawBoxOutline(hWnd,beg,end);
			// Capture the mouse
			SetCapture(hWnd);
		}
		break;
	case WM_MOUSEMOVE:
		if(btnDown) {
#if 0
			// Don't count it unless the mouse has been down for HOLDTIME
			if(getElapsedTime(btnDownTime) < HOLDTIME) break;
#endif
			drag=TRUE;
			SetCursor(LoadCursor(NULL,IDC_CROSS));
			drawBoxOutline(hWnd,beg,end);
			end.x=(short)LOWORD(lParam);
			end.y=(short)HIWORD(lParam);
			drawBoxOutline(hWnd,beg,end);
		}
		break;
	case WM_LBUTTONUP:
		if(btnDown) {
			end.x=(short)LOWORD(lParam);
			end.y=(short)HIWORD(lParam);
			if(beg.x == end.x || beg.y == end.y) {
				// Invalid box
				boxSelected=FALSE;
				drawBoxOutline(hWnd,beg,end);
			} else {
				// Valid box
				boxSelected=TRUE;
				// Save the rect for the box
				if(end.x > beg.x) {
					box.left=beg.x;
					box.right=end.x;
				} else {
					box.left=end.x;
					box.right=beg.x;
				}
				if(end.y > beg.y) {
					box.top=beg.y;
					box.bottom=end.y;
				} else {
					box.top=end.y;
					box.bottom=beg.y;
				}
			}
			btnDown=FALSE;
			drag=FALSE;
			ReleaseCapture();
			SetCursor(LoadCursor(NULL,IDC_ARROW));
		}
		break;
	case WM_PAINT:
		paint(hWnd);
		break;
	case WM_SIZE:
		// Reset the width and height
		width=LOWORD(lParam);
		height=HIWORD(lParam);

		// Reposition the compass to the standard place
		// (Figure out a better algorithm later)
		if(hMapCompass) {
			int newWidth=LOWORD(lParam);
			int newHeight=HIWORD(lParam);
			int newX=newWidth-COMPASS_WIDTH-COMPASS_MARGIN;
			int newY=newHeight-COMPASS_HEIGHT-COMPASS_MARGIN;
			if(newX < COMPASS_MARGIN) newX=COMPASS_MARGIN;
			if(newY < COMPASS_MARGIN) newY=COMPASS_MARGIN;

#if DEBUG_COMPASS
			{
				POINT p1={0,0},p2={0,0},p3={0,0};
				ClientToScreen(hWndMain,&p1);
				ClientToScreen(hMap,&p2);
				ClientToScreen(hMapCompass,&p3);
				RECT r1,r2,r3;
				GetClientRect(hWndMain,&r1);
				GetClientRect(hMap,&r2);
				GetClientRect(hMapCompass,&r3);
				POINT q1={r1.right,r1.bottom};
				POINT q2={r2.right,r2.bottom};
				POINT q3={r3.right,r3.bottom};
				ClientToScreen(hWndMain,&q1);
				ClientToScreen(hMap,&q2);
				ClientToScreen(hMapCompass,&q3);
				lbprintf(_T("Before"));
				lbprintf(_T("UL: hWndMain=(%d,%d)\n")
					_T(" hMap=(%d,%d) hMapC=(%d,%d)"),
					p1.x,p1.y,p2.x,p2.y,p3.x,p3.y);
				lbprintf(_T("LR: hWndMain=(%d,%d)\n")
					_T(" hMap=(%d,%d) hMapC=(%d,%d)"),
					q1.x,q1.y,q2.x,q2.y,q3.x,q3.y);
				lbprintf(_T("newX=%d newXOrigin=%d"),newX,p2.x+newX);
				lbprintf(_T("newY=%d newYOrigin=%d"),newY,p2.y+newY);
				lbprintf(_T(""));
			}
#endif
#if 0
			POINT mOrigin={0,0},cOrigin={0,0};
			ClientToScreen(hMap,&mOrigin);
			ClientToScreen(hMapCompass,&cOrigin);
#endif

			MoveWindow(hMapCompass,newX,newY,
				COMPASS_WIDTH,COMPASS_HEIGHT,TRUE);
#if DEBUG_COMPASS
			{
				POINT p1={0,0},p2={0,0},p3={0,0};
				ClientToScreen(hWndMain,&p1);
				ClientToScreen(hMap,&p2);
				ClientToScreen(hMapCompass,&p3);
				RECT r1,r2,r3;
				GetClientRect(hWndMain,&r1);
				GetClientRect(hMap,&r2);
				GetClientRect(hMapCompass,&r3);
				POINT q1={r1.right,r1.bottom};
				POINT q2={r2.right,r2.bottom};
				POINT q3={r3.right,r3.bottom};
				ClientToScreen(hWndMain,&q1);
				ClientToScreen(hMap,&q2);
				ClientToScreen(hMapCompass,&q3);
				lbprintf(_T("After"));
				lbprintf(_T("UL: hWndMain=(%d,%d)\n")
					_T(" hMap=(%d,%d) hMapC=(%d,%d)"),
					p1.x,p1.y,p2.x,p2.y,p3.x,p3.y);
				lbprintf(_T("LR: hWndMain=(%d,%d)\n")
					_T(" hMap=(%d,%d) hMapC=(%d,%d)"),
					q1.x,q1.y,q2.x,q2.y,q3.x,q3.y);
				lbprintf(_T(""));
			}
#endif
		}
		break;
	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		wmEvent=HIWORD(wParam); 
		switch(wmId) {       
	case ID_MAPOPTIONS_CALIBRATIONPOINTS:
		showCalibrationPoints=!showCalibrationPoints;
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_CALIBRATIONPOINTS,
			showCalibrationPoints?MF_CHECKED:MF_UNCHECKED);
		// Cause the map window to repaint with calibration points
		if(pMap && pMap != &pMapDefault && pMap->isValid()) {
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		break;
	case ID_MAPOPTIONS_SHOWCOMPASS:
		showCompass=!showCompass;
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_SHOWCOMPASS,
			showCompass?MF_CHECKED:MF_UNCHECKED);
		if(hMapCompass) {
			ShowWindow(hMapCompass,showCompass?SW_SHOW:SW_HIDE);
			UpdateWindow(hMapCompass);
		}
		break;
	case ID_MAPOPTIONS_TRACKPOINTS:
		drawTrackpoints=!drawTrackpoints;
		CheckMenuItem(hMapMenu,ID_MAPOPTIONS_TRACKPOINTS,
			drawTrackpoints?MF_CHECKED:MF_UNCHECKED);
		// If there is no tracklistview, the indices have not been calculated
		if(!hTrkLV) calculateTrackIndices();
		if(hMap && showTrackpoints) {
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);	
		}
		break;
	case ID_MAPOPTIONS_CENTER:
		positionToPoint(CENTER_CENTER);
		break;
	case ID_MAPOPTIONS_LEFTTOP:
		positionToPoint(TOP_LEFT);
		break;
	case ID_MAPOPTIONS_CENTERHIGH:
		positionToPoint(CENTER_TOP);
		break;
	case ID_MAPOPTIONS_CENTERLOW:
		positionToPoint(CENTER_BOTTOM);
		break;
	case ID_MAPOPTIONS_ZOOM400:
		zoomSpecified(4.0);
		break;
	case ID_MAPOPTIONS_ZOOM200:
		zoomSpecified(2.0);
		break;
	case ID_MAPOPTIONS_ZOOM50:
		zoomSpecified(.5);
		break;
	case ID_MAPOPTIONS_ZOOM25:
		zoomSpecified(.25);
		break;
	case ID_MAPOPTIONS_ZOOM_NORMAL:
		zoomNormal();
		break;
	case ID_MAPOPTIONS_RESTORE:
		createMapWindow(hWndMain);	    
		break;
	case ID_MAPOPTIONS_MEASURE_START:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		pMap->calculateLatLon(xClickMap,yClickMap,
			&measureStartLat,&measureStartLon);
		measureEndLat=measureStartLat;
		measureEndLon=measureStartLon;
		measureDistance=0.0;
		break;
	case ID_MAPOPTIONS_MEASURE_END:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		if(measureStartLat != DBL_MIN && measureStartLon != DBL_MIN) {
			double latitude,longitude;
			pMap->calculateLatLon(xClickMap,yClickMap,
				&latitude,&longitude);
			double distance=greatCircleDistance(latitude,longitude,
				measureStartLat,measureStartLon);
			measureEndLat=latitude;
			measureEndLon=longitude;
			measureDistance=distance;
			if(distance > 1) {
				infoMsg(_T("%.3f mi"),distance);
			} else {
				infoMsg(_T("%.3f mi\n%.0f ft"),distance,distance*5280.);
			}
		} else {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
		}
		break;
	case ID_MAPOPTIONS_MEASURE_INCREMENT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		if(measureStartLat != DBL_MIN && measureStartLon != DBL_MIN) {
			double latitude,longitude;
			pMap->calculateLatLon(xClickMap,yClickMap,
				&latitude,&longitude);
			double distance=greatCircleDistance(latitude,longitude,
				measureEndLat,measureEndLon);
			distance+=measureDistance;
			measureEndLat=latitude;
			measureEndLon=longitude;
			measureDistance=distance;
			if(distance > 1) {
				infoMsg(_T("%.3f mi"),distance);
			} else {
				infoMsg(_T("%.3f mi\n%.0f ft"),distance,distance*5280.);
			}
		} else {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
		}
		break;
	case ID_MAPOPTIONS_REMOVEIMAGE:
		if(!pMap || pMap == &pMapDefault) {
			errMsg(_T("There is no image to remove"));
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		if(pMap != pMapFile) {
			errMsg(_T("Image is not set to image from file"));
			break;
		}
		delete pMapFile;
		pMapFile=NULL;
		pMap=&pMapDefault;
		pMap->calibrate();
		break;
	case ID_MAPOPTIONS_CALIBRATE:
		// See if there is an image to calibrate
		if(!pMap || pMap == &pMapDefault) {
			errMsg(_T("No image to calibrate"));
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		if(!IsWindow(hCalDlg)) {
			hCalDlg=CreateDialog(hInst,(LPCTSTR)IDD_CALIBRATE,hWnd,
				(DLGPROC)calibrateDlgProc);
			ShowWindow(hCalDlg,SW_SHOW);
#ifdef UNDER_CE
			// Resize the map window so it will show
			if(hMap) {
				RECT rcMain,rcDlg;
				GetClientRect(hWndMain,&rcMain);
				GetClientRect(hCalDlg,&rcDlg);
				int newHeight=rcDlg.bottom-rcDlg.top;
				int newWidth=LOWORD(lParam);
				rcMain.top+=newHeight;
				MoveWindow(hMap,0,newHeight,rcMain.right-rcMain.left,
					rcMain.bottom-rcMain.top,FALSE);
				// Move the center down
				yOff+=.5*newHeight/scale;
				resetTransformation();
			} 
#endif
			// Cause the map window to repaint with calibration points
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		} else {
			// Dialog is up, reset it with current click point
			SendMessage(hCalDlg,WM_RESETCALPOINT,(WPARAM)0,(LPARAM)0);
		}
		break;
	case ID_MAPOPTIONS_ADJUSTGAMMA:
#if DEBUG_GAMMA
		if(hGammaDlg) {
			errMsg(_T("hGammaDlg=%d IsWindow=%s"),hGammaDlg,
				IsWindow(hGammaDlg)?_T("TRUE"):_T("FALSE"));
		} else {	
			errMsg(_T("hGammaDlg=NULL IsWindow=%s"),
				IsWindow(hGammaDlg)?_T("TRUE"):_T("FALSE"));
		}
#endif
#if DO_GAMMA
		if(!IsWindow(hGammaDlg)) {
			hGammaDlg=CreateDialog(hInst,(LPCTSTR)IDD_GAMMA,hWnd,
				(DLGPROC)gammaDlgProc);
			ShowWindow(hGammaDlg,SW_SHOW);
#ifdef UNDER_CE
			// Resize the map window so it will show
			if(hMap) {
				RECT rcMain,rcDlg;
				GetClientRect(hWndMain,&rcMain);
				GetClientRect(hGammaDlg,&rcDlg);
				int newHeight=rcDlg.bottom-rcDlg.top;
				int newWidth=LOWORD(lParam);
				rcMain.top+=newHeight;
				MoveWindow(hMap,0,newHeight,rcMain.right-rcMain.left,
					rcMain.bottom-rcMain.top,FALSE);
				// Move the center down
				yOff+=.5*newHeight/scale;
				resetTransformation();
			} 
#endif
			// Cause the map window to repaint
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		break;
#else  // #if DO_GAMMA
		errMsg(_T("Gamma adjustment is not available"));
		return DefWindowProc(hWnd,message,wParam,lParam);
#endif  // #if DO_GAMMA
	case ID_MAPOPTIONS_NEWWAYPOINT:
	case ID_MAPOPTIONS_EDIT_WAYPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doEditWaypoint((wmId == ID_MAPOPTIONS_NEWWAYPOINT)?NEW_WPT:EDIT_WPT);
		break;
	case ID_MAPOPTIONS_EDIT_ROUTEWAYPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doEditWaypoint(EDIT_RTE_WPT);
		break;
	case ID_MAPOPTIONS_EDIT_TRACKPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doEditTrackpoint(EDIT_TPT);
		break;
	case ID_MAPOPTIONS_GOTO_WAYPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doGoToLVWaypoint();
		break;
	case ID_MAPOPTIONS_GOTO_ROUTEWAYPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doGoToLVRouteWaypoint();
		break;
	case ID_MAPOPTIONS_GOTO_TRACKPOINT:
		// Convert menu point to map pixel coordinates
		xClickMap=menuPoint.x/scale+xOff;
		yClickMap=menuPoint.y/scale+yOff;
		doGoToLVTrackpoint();
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
		}

		// Cause the map window to repaint
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
	}

	return 0;
}

static void paint(HWND hWnd)
{
	HDC hdc;
	HBRUSH hOldBrush;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	RECT wRect;
	LONG vSpace;
	LONG hSpace;

#if 0
	// Get the this pointer
	probe=(CProbe *)GetWindowLong(hWnd,0);
	if(!probe) {
		errMsg(_T("CProbe::paint: Invalid this pointer"));
	}
#endif

	// Get graphics context
	hdc=BeginPaint(hWnd,&ps);

#ifndef UNDER_CE
#if USE_TAHOMA
	// Change font to Tahoma
	LOGFONT lf;
	HFONT hFont,oldFont;
	TCHAR *szString=MAPMESSAGE;

	// Initialize lf (Defaults correspond to 0's)
	memset(&lf,0,sizeof(LOGFONT));
	lf.lfHeight=14;
	_tcscpy(lf.lfFaceName,_T("Tahoma"));
	hFont=CreateFontIndirect(&lf);
	if(hFont) {
		oldFont=(HFONT)SelectObject(hdc,hFont);
	}
#endif
#endif

	// Paint the background with no border
	GetClientRect(hWnd,&wRect);
	hOldBrush=(HBRUSH)SelectObject(hdc,CreateSolidBrush(mapBgColor));
	HPEN hPen0=(HPEN)SelectObject(hdc,CreatePen(PS_SOLID,0,mapBgColor));
	Rectangle(hdc,0,0,wRect.right,wRect.bottom);
	DeleteObject(SelectObject(hdc,hPen0));

	// Paint the map if any
	if(pMap && pMap != &pMapDefault && pMap->isValid()) {
		double sWidth=(wRect.right-wRect.left)/scale;
		double sHeight=(wRect.bottom-wRect.top)/scale;
#ifndef UNDER_CE
		// SetStretchBltMode is not supported on WCE
#if DO_GAMMA
		if(adjustGamma) {
			SetStretchBltMode(hdc,HALFTONE);
			SetColorAdjustment(hdc,&colorAdjust);
#if 0
			// Create a halftone palette if not created
			if(!hPalette) {
				hPalette = CreateHalftonePalette(hdc);
				if(!hPalette) {
					errMsg(_T("Failed to create palette"));
				}
			}
			SelectPalette(hdc, hPalette, FALSE);
			RealizePalette(hdc);
#endif
		} else {
			// Set the stretch mode.  Default is BLACKONWHITE causing the
			// image to become dark when it is zooned in.  (Not supported and
			// apparently not needed on WCE)
			SetStretchBltMode(hdc,COLORONCOLOR);
		}
#else
		// Set the stretch mode.  Default is BLACKONWHITE causing the
		// image to become dark when it is zooned in.  (Not supported and
		// apparently not needed on WCE)
		SetStretchBltMode(hdc,COLORONCOLOR);
#endif  // #if DO_GAMMA
#endif  // #ifndef UNDER_CE
		StretchBlt(hdc,wRect.left,wRect.top,
			wRect.right-wRect.left,wRect.bottom-wRect.top,
			pMap->getHDCMem(),
			(int)(xOff+.5),(int)(yOff +.5),
			(int)(sWidth+.5),(int)(sHeight+.5),
			SRCCOPY);
	}

#if DO_GAMMA && GAMMA_IMAGE_ONLY
	// Quit here to avoid long redraws
	if(FALSE && adjustGammaInProgress) {
		// End paint
#ifndef UNDER_CE
		DeleteObject(SelectObject(hdc,oldFont));
#endif
		DeleteObject(SelectObject(hdc,hOldBrush));
		EndPaint(hWnd,&ps);
		return;
	}
#endif

	// Initialize metrics
	GetTextMetrics(hdc,&tm);
	vSpace=tm.tmHeight;
	hSpace=tm.tmAveCharWidth;

#if DEBUG_DRAW
	lbprintf(_T("latMax=%.1f lonMax=%.1f X=%ld Y=%ld"),latMax,lonMax,X(latMax),Y(lonMax));
	lbprintf(_T("latMin=%.1f lonMin=%.1f X=%ld Y=%ld"),latMin,lonMin,X(latMin),Y(lonMin));
#endif

	// Plot tracks
	if(plotTracks) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;

		SetBkColor(hdc,mapBgColor);
		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,mapTrkColor);
#if 0
		SetTextAlign(hdc,TA_TOP|TA_RIGHT);
#endif
		HPEN hPen0=(HPEN)SelectObject(hdc,CreatePen(PS_SOLID,0,mapTrkColor));
		while(iter != eol) {
			CBlock *pBlock=iter;
			CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			POINT prev,cur;
			BOOL first=TRUE;

			if(!pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the tracks
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				double longitude=pBlock1->getLongitude();
				double latitude=pBlock1->getLatitude();
				long x,y;
				calculatePixel(latitude,longitude,&x,&y);
				if(!pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				if(first) {
					first=FALSE;
					prev.x=x;
					prev.y=y;
					// Draw the name
					LPWSTR unicodeString=NULL;
					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						SIZE extent;
						GetTextExtentExPoint(hdc,
							unicodeString,_tcslen(unicodeString),0,NULL,
							NULL,&extent);
						ExtTextOut(hdc,x-2*SYMBSIZE-extent.cx,y,0,NULL,
							unicodeString,_tcslen(unicodeString),NULL);
						free(unicodeString);
					}
					if(drawTrackpoints) {
						drawSquare(hdc,x,y);
					}
				} else {
					cur.x=x;
					cur.y=y;
					CTrackpoint *trackpoint=(CTrackpoint *)pBlock1;
					if(!trackpoint->isNewTrack()) {
						drawLine(hdc,prev.x,prev.y,cur.x,cur.y);
					}
					if(drawTrackpoints) {
						drawSquare(hdc,cur.x,cur.y);
						// Draw the name
						TCHAR szText[80];
						_stprintf(szText,_T("%d%c%d"),
							pBlock->getIndex(),
							trackpoint->isNewTrack()?'S':'C',
							pBlock1->getIndex());
						ExtTextOut(hdc,x+2*SYMBSIZE,y,0,NULL,
							szText,_tcslen(szText),NULL);
					}
					prev=cur;
				}
				++iter1;
			}
			++iter;
		}
		DeleteObject(SelectObject(hdc,hPen0));
	}

	// Plot waypoints
	if(plotWaypoints) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;

		SetBkColor(hdc,mapBgColor);
		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,mapWptColor);
#if 0
		SetTextAlign(hdc,TA_TOP|TA_LEFT);
#endif
		HPEN hPen0=(HPEN)SelectObject(hdc,CreatePen(PS_SOLID,0,mapWptColor));
		while(iter != eol) {
			CBlock *pBlock=iter;
			double longitude=pBlock->getLongitude();
			double latitude=pBlock->getLatitude();
			long x,y;
			calculatePixel(latitude,longitude,&x,&y);
#if DEBUG_DRAW
			lbprintf(_T("lan=%.1f lon=%.1f x=%ld x=%ld"),
				latitude,longitude,x,y);
#endif
			if(!pBlock->isChecked()) {
				iter++;
				continue;
			}

			// Draw a cross
			drawCross(hdc,x,y);

			// Draw the name
			LPWSTR unicodeString=NULL;
			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				ExtTextOut(hdc,x+2*SYMBSIZE,y,0,NULL,
					unicodeString,_tcslen(unicodeString),NULL);
				free(unicodeString);
			}

			++iter;
		}
		DeleteObject(SelectObject(hdc,hPen0));
	}

	// Plot routes
	if(plotRoutes) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;

		SetBkColor(hdc,mapBgColor);
		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,mapRteColor);
#if 0
		SetTextAlign(hdc,TA_BOTTOM|TA_RIGHT);
#endif
		HPEN hPen0=(HPEN)SelectObject(hdc,CreatePen(PS_SOLID,0,mapRteColor));
		while(iter != eol) {
			CBlock *pBlock=iter;
			CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			POINT prev,cur;
			BOOL first=TRUE;

			if(!pBlock->isChecked()) {
				iter++;
				continue;
			}
			// Do the waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock1=iter1;
				double longitude=pBlock1->getLongitude();
				double latitude=pBlock1->getLatitude();
				long x,y;
				calculatePixel(latitude,longitude,&x,&y);
				if(!pBlock1->isChecked()) {
					iter1++;
					continue;
				}
				if(first) {
					first=FALSE;
					prev.x=x;
					prev.y=y;
					// Draw the name
					LPWSTR unicodeString=NULL;
					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						SIZE extent;
						GetTextExtentExPoint(hdc,
							unicodeString,_tcslen(unicodeString),0,NULL,
							NULL,&extent);
						ExtTextOut(hdc,x-2*SYMBSIZE-extent.cx,y-extent.cy,
							0,NULL,
							unicodeString,_tcslen(unicodeString),NULL);
						free(unicodeString);
					}
				} else {
					cur.x=x;
					cur.y=y;
					drawLine(hdc,prev.x,prev.y,cur.x,cur.y);
					prev=cur;
				}

				// Draw a cross
				drawCross(hdc,x,y);

				// Draw the waypoint name
				LPWSTR unicodeString=NULL;
				ansiToUnicode(pBlock1->getIdent(),&unicodeString);
				if(unicodeString) {
					ExtTextOut(hdc,x+2*SYMBSIZE,y,0,NULL,
						unicodeString,_tcslen(unicodeString),NULL);
					free(unicodeString);
				}

				++iter1;
			}
			++iter;
		}
		DeleteObject(SelectObject(hdc,hPen0));
	}

	// Plot calibration points
	if(showCalibrationPoints && pMap && pMap != &pMapDefault && pMap->isValid()) {
		const tsDLList<CMapPoint> *pointList=pMap->getList();
		tsDLIterBD<CMapPoint> iter(pointList->first());
		tsDLIterBD<CMapPoint> eol;

		SetBkColor(hdc,mapBgColor);
		SetBkMode(hdc,TRANSPARENT);
		SetTextColor(hdc,mapCalColor);
#if 0
		SetTextAlign(hdc,TA_TOP|TA_LEFT);
#endif
		HPEN hPen0=(HPEN)SelectObject(hdc,CreatePen(PS_SOLID,0,mapCalColor));
		while(iter != eol) {
			CMapPoint *pPoint=iter;
			long x=(long)(scale*(pPoint->getXPix()-xOff)+.5);
			long y=(long)(scale*(pPoint->getYPix()-yOff)+.5);

			// Draw a cross
			drawCross(hdc,x,y);

			// Draw the name
			ExtTextOut(hdc,x+2*SYMBSIZE,y,0,NULL,
				pPoint->getIdent(),_tcslen(pPoint->getIdent()),NULL);

			++iter;
		}
		DeleteObject(SelectObject(hdc,hPen0));
	}

	// Draw the current position using new values
	drawCurrentPosition(CUR_POS_START);

	// End paint
#ifndef UNDER_CE
#if USE_TAHOMA
	DeleteObject(SelectObject(hdc,oldFont));
#endif
#endif
	DeleteObject(SelectObject(hdc,hOldBrush));
	EndPaint(hWnd,&ps);
}

/////////////////////////////////////////////////////////////
//////////////////// Map Compass Window /////////////////////
/////////////////////////////////////////////////////////////

static void createMapCompass(HWND hWndParent)                                     
{
	RECT rect;

	if(hMapCompass) {
		// TBD
	} else {
		// Create the control
		// Ensure that the common control DLL is loaded.

		// Get the size and position of the parent window.
		GetClientRect(hWndParent,&rect);

		// Register the class
		registerMapCompassClass();

		// Create the window
		hMapCompass=CreateWindowEx(0L,
			MAPCOMPASSCLASS,                // Class
			TEXT("GPSLINK Map"),            // Title
			WS_VISIBLE|WS_CHILD|WS_BORDER|  // Style
			WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			rect.right-COMPASS_WIDTH-COMPASS_MARGIN,
			rect.bottom-COMPASS_HEIGHT-COMPASS_MARGIN,
			COMPASS_WIDTH,COMPASS_HEIGHT,
			hWndParent,
			(HMENU)ID_MAP_COMPASS,           // Child window ID
			hInst,
			NULL);

		// Update the window 
		ShowWindow(hMapCompass,showCompass?SW_SHOW:SW_HIDE);
		UpdateWindow(hMapCompass);
	}
}

static ATOM registerMapCompassClass(void)
{
	WNDCLASS wc;

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)mapCompassWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=sizeof(long);
	wc.hInstance=hInst;
	wc.hIcon=NULL;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=MAPCOMPASSCLASS;

	return RegisterClass(&wc);
}

static LRESULT CALLBACK mapCompassWndProc(HWND hWnd, UINT message,
										  WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	static BOOL btnDown=FALSE;
	static BOOL drag=FALSE;
	static SYSTEMTIME btnDownTime;
	// hit[0] is the former mouse position
	// hit[1] is the current mouse position
	// All are relative to hMap
	static POINT hit[2],delta;
	POINT origin;
#ifdef UNDER_CE
	POINT point;
	SHRGINFO shrg;
	DWORD gesture;
#endif

	switch(message) {
	case WM_CREATE:
		//Create memory DC to hold Bitmap
		hDC=GetDC(hWnd);
		hCompassDC=CreateCompatibleDC(hDC);
		if(!hCompassDC) {
			errMsg(_T("Could not create compass DC"));
		} else {
			// Load the compass bitmap
			hCompassBitmap=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_COMPASS));
			if(!hCompassBitmap) {
				errMsg(_T("Could not load compass bitmap"));
			} else {
				//Select the bitmap into the memory device context
				SelectObject(hCompassDC,hCompassBitmap);
			}
		}
		ReleaseDC(hWnd,hDC);
		break;
	case WM_PAINT:
		paintCompass(hWnd);
		break;
#ifndef UNDER_CE
	case WM_RBUTTONDOWN:
		hit[1].x=(short)LOWORD(lParam);
		hit[1].y=(short)HIWORD(lParam);
		// Switch depending on the region of the compass
		{
			double x=(double)(hit[1].x-(COMPASS_WIDTH-2)/2);
			double y=(double)(hit[1].y-(COMPASS_HEIGHT-2)/2);
			double r=sqrt(x*x+y*y);
			if(r < 8) {
				// Zoom to normal size
				zoomNormal();
			} else if(r < 16.5) {
				// Move
				double theta=atan2(-y,x);
				int direction=(int)floor(4.*theta/pi+4.5);
				move(direction,MOVEFAC_SMALL);
			} else {
				// Not implemented
			}
		}
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
		break;
#endif
	case WM_LBUTTONDOWN:
#ifdef UNDER_CE
		point.x=LOWORD(lParam);
		point.y=HIWORD(lParam);
		shrg.cbSize=sizeof(SHRGINFO);
		shrg.hwndClient=hWnd;
		shrg.ptDown=point;
		shrg.dwFlags=SHRG_RETURNCMD;
		gesture=SHRecognizeGesture(&shrg);
		if(gesture == GN_CONTEXTMENU) {
			hit[1].x=(short)LOWORD(lParam);
			hit[1].y=(short)HIWORD(lParam);
			// Switch depending on the region of the compass
			double x=(double)(hit[1].x-(COMPASS_WIDTH-2)/2);
			double y=(double)(hit[1].y-(COMPASS_HEIGHT-2)/2);
			double r=sqrt(x*x+y*y);
			if(r < 8) {
				// Zoom to normal size
				zoomNormal();
			} else if(r < 16.5) {
				// Move
				double theta=atan2(-y,x);
				int direction=(int)floor(4.*theta/pi+4.5);
				move(direction,MOVEFAC_SMALL);
			} else {
				// Not implemented
			}
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
			break;
		}
#endif
		btnDown=TRUE;
		drag=FALSE;
		hit[1].x=(short)LOWORD(lParam);
		hit[1].y=(short)HIWORD(lParam);
		GetSystemTime(&btnDownTime);
		MapWindowPoints(hWnd,hMap,&hit[1],1);
		hit[0]=hit[1];
		// Calculate delta
		origin.x=origin.y=0;
		MapWindowPoints(hWnd,hMap,&origin,1);
		delta.x=hit[1].x-origin.x;
		delta.y=hit[1].y-origin.y;
		// Capture the mouse
		SetCapture(hWnd);
		break;
	case WM_MOUSEMOVE:
		if(btnDown) {
			// Don't count it unless the mouse has been down for HOLDTIME
			if(getElapsedTime(btnDownTime) < HOLDTIME) break;
			drag=TRUE;
			hit[1].x=(short)LOWORD(lParam);
			hit[1].y=(short)HIWORD(lParam);
#if DEBUG_DRAG
			lbprintf(_T("compass: x0=%hd y0=%hd x=%d y=%d"),
				LOWORD(lParam),HIWORD(lParam),hit[1].x,hit[1].y);
#endif
			MapWindowPoints(hWnd,hMap,&hit[1],1);
			if(hit[1].x != hit[0].x || hit[1].y != hit[0].y) {
				// Get coordinates relative to parent
				origin.x=hit[1].x-delta.x;
				origin.y=hit[1].y-delta.y;
				// (Could adjust origin here to keep the window in bounds)
				// Move the window and repaint
				MoveWindow(hWnd,origin.x,origin.y,COMPASS_WIDTH,COMPASS_HEIGHT,
					TRUE);
				hit[0]=hit[1];
			}
		}
		break;
	case WM_LBUTTONUP:
		hit[1].x=(short)LOWORD(lParam);
		hit[1].y=(short)HIWORD(lParam);
		if(btnDown && !drag) {
			// Switch depending on the region of the compass
			double x=(double)(hit[1].x-(COMPASS_WIDTH-2)/2);
			double y=(double)(hit[1].y-(COMPASS_HEIGHT-2)/2);
			double r=sqrt(x*x+y*y);
#if DEBUG_CLICK
			lbprintf(_T("x=%.2f y=%.2f r=%.2f"),x,y,r);
#endif	    
			if(r < 8) {
				// Zoom in
				_tcscpy(szMapString,_T("Zoom In"));
				zoomin();
			} else if(r < 16.5) {
				// Move
				double theta=atan2(-y,x);
				int direction=(int)floor(4.*theta/pi+4.5);
#if DEBUG_CLICK
				lbprintf(_T("theta=%.2f fac=%.2f dir=%d"),atan2(-y,x)/pi,
					4.*theta/pi +3.5,
					direction);
#endif
				move(direction,MOVEFAC_LARGE);
			} else {
				// Zoom out
				_tcscpy(szMapString,_T("Zoom Out"));
				zoomout();
			}
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		btnDown=FALSE;
		drag=FALSE;
		ReleaseCapture();
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
	}

	return 0;
}

static void paintCompass(HWND hWnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT wRect;
	BITMAP bmp;

	hDC=BeginPaint(hWnd,&ps);
	GetClientRect(hWnd,&wRect);
	GetObject(hCompassBitmap,sizeof(BITMAP),&bmp);
	BitBlt(hDC,wRect.left,wRect.top,
		wRect.right-wRect.left,wRect.bottom-wRect.top,
		hCompassDC,0,0,SRCCOPY );
	EndPaint(hWnd,&ps);
}

/////////////////////////////////////////////////////////////
//////////////// Map Current Position Window ////////////////
/////////////////////////////////////////////////////////////

static void createMapCurPos(HWND hWndParent)                                     
{
	RECT rect;

	if(hMapCurPos) {
		// TBD
	} else {
		// Get the size and position of the parent window.
		GetClientRect(hWndParent,&rect);

		// Register the class
		registerMapCurPosClass();

		// Create the window
		hMapCurPos=CreateWindowEx(0L,
			MAPCURPOSCLASS,                // Class
			TEXT("GPSLINK Current Position"),            // Title
			WS_VISIBLE|WS_CHILD|WS_BORDER|  // Style
			WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
			0,
			0,
			POSITIONSIZE,POSITIONSIZE,
			hWndParent,
			(HMENU)ID_MAP_CURPOS,           // Child window ID
			hInst,
			NULL);

		if(hMapCurPos == NULL ) return;

		// Load the popupmenu
		hCurPosMenu=LoadMenu(hInst,MAKEINTRESOURCE(IDM_PVTMENU));
		hCurPosMenu=GetSubMenu(hCurPosMenu,0);

		// Update the window 
		UpdateWindow(hMapCurPos);
	}
}

static ATOM registerMapCurPosClass(void)
{
	WNDCLASS wc;

	wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc=(WNDPROC)mapCurPosWndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=sizeof(long);
	wc.hInstance=hInst;
	wc.hIcon=NULL;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=MAPCURPOSCLASS;

	return RegisterClass(&wc);
}

static LRESULT CALLBACK mapCurPosWndProc(HWND hWnd, UINT message,
										 WPARAM wParam, LPARAM lParam)
{
	POINT point;
	int wmId,wmEvent;
	double latitude,longitude;
	long xCur,yCur;
#ifdef UNDER_CE
	SHRGINFO shrg;
	DWORD gesture;
#endif

	switch(message) {
	case WM_PAINT:
		paintCurPos(hWnd);
		break;
#ifndef UNDER_CE
	case WM_RBUTTONDOWN:
		// Popup menu
		point.x=LOWORD(lParam);  // (Relative to client area)
		point.y=HIWORD(lParam);
		ClientToScreen(hWnd,&point);
		TrackPopupMenu(hCurPosMenu,
			TPM_LEFTALIGN|TPM_VCENTERALIGN,
			point.x,point.y,0,hWnd,NULL);
		break;
#else
	case WM_LBUTTONDOWN:
		// Check for tap-hold
		point.x=LOWORD(lParam);  // (Relative to client area)
		point.y=HIWORD(lParam);
		shrg.cbSize=sizeof(SHRGINFO);
		shrg.hwndClient=hWnd;
		shrg.ptDown=point;
		shrg.dwFlags=SHRG_RETURNCMD;
		gesture=SHRecognizeGesture(&shrg);
		if(gesture == GN_CONTEXTMENU) {
			// Popup menu
			ClientToScreen(hWnd,&point);
			TrackPopupMenu(hCurPosMenu,
				TPM_LEFTALIGN|TPM_VCENTERALIGN,
				point.x,point.y,0,hWnd,NULL);
		} else {
			MapWindowPoints(hWnd,hMap,&menuPoint,1);	
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
#endif
#if 0
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONUP:
		break;
#endif
	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		wmEvent=HIWORD(wParam); 
		switch(wmId) {       
	case ID_PVTOPTIONS_STOP:
		stopPVTData();
		break;
	case ID_PVTOPTIONS_NEWWAYPOINT:
		// Get screen coordinates from position lat/lon
		longitude=position.getLongitude();
		latitude=position.getLatitude();
		calculatePixel(latitude,longitude,&xCur,&yCur);
		// Convert screen coordinates to map pixel coordinates
		xClickMap=xCur/scale+xOff;
		yClickMap=yCur/scale+yOff;
		doEditWaypoint(POS_WPT);
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
	}

	return 0;
}

static void paintCurPos(HWND hWnd)
{
	HDC hdc;
	HBRUSH hOldBrush;
	PAINTSTRUCT ps;
	RECT wRect;

	// Get graphics context
	hdc=BeginPaint(hWnd,&ps);

	// Begin paint
	GetClientRect(hWnd,&wRect);
	hOldBrush=(HBRUSH)SelectObject(hdc,CreateSolidBrush(mapPosColor));
	Rectangle(hdc,0,0,wRect.right,wRect.bottom);

	// End paint
	DeleteObject(SelectObject(hdc,hOldBrush));
	EndPaint(hWnd,&ps);
}

static void drawCurrentPosition(CurPosMode mode)
{
	double longitude,latitude;
	long x,y;
	static BOOL didError=FALSE;

	if(!pvtDataInProgress || !hMap) return;

	// Create the current position window
	if(!hMapCurPos) {
		createMapCurPos(hMap);
		if(!hMapCurPos) {
			// Print an error message once
			if(!didError) {
				errMsg(_T("Could not create current position window"));
				didError=TRUE;
			}
			return;
		}
	}
	didError=FALSE;

	// Switch depending on the mode
	switch(mode) {
	case CUR_POS_START:
	case CUR_POS_CONTINUE:
		longitude=position.getLongitude();
		latitude=position.getLatitude();
#if 0
		// HOME waypoint
		longitude=-87.946309;
		latitude=41.593666;
#endif
		calculatePixel(latitude,longitude,&x,&y);

		MoveWindow(hMapCurPos,x-POSITIONOFFSET,y-POSITIONOFFSET,
			POSITIONSIZE,POSITIONSIZE,TRUE);
		ShowWindow(hMapCurPos,SW_SHOW);
		break;
	case CUR_POS_END:
		ShowWindow(hMapCurPos,SW_HIDE);
		break;
	}
}

// Mesage handler for calibrate dialog
LRESULT CALLBACK calibrateDlgProc(HWND hDlg, UINT message,
								  WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
	RECT rc;
#endif
	int wmId;
	HWND hwButton=NULL;
	HWND hwEdit=NULL;
	HWND hwStatic=NULL;
	HWND hwListBox=NULL;
	TCHAR szText[MAX_ENTRY_SIZE];
	CMapPoint *pPoint;
	int sel;
	int index;

	switch (message) {
	case WM_RESETCALPOINT:
		hwEdit=GetDlgItem(hDlg,IDC_XENTRY);
		Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
		_stprintf(szText,_T("%.6f"),xClickMap);
		Edit_SetText(hwEdit,szText);
		hwEdit=GetDlgItem(hDlg,IDC_YENTRY);
		Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
		_stprintf(szText,_T("%.6f"),yClickMap);
		Edit_SetText(hwEdit,szText);
		return TRUE;

	case WM_INITDIALOG:
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN;
		shidi.hDlg = hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);

		// Resize it because WCE seems to put an extra border at the
		// bottom
		GetWindowRect(hDlg,&rc);
		rc.bottom-=MENU_HEIGHT;
		MoveWindow(hDlg,rc.left,rc.top,
			rc.right-rc.left,rc.bottom-rc.top,FALSE);
#endif
		// Edit controls
		hwEdit=GetDlgItem(hDlg,IDC_XENTRY);
		Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
		_stprintf(szText,_T("%.6f"),xClickMap);
		Edit_SetText(hwEdit,szText);
		hwEdit=GetDlgItem(hDlg,IDC_YENTRY);
		Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
		_stprintf(szText,_T("%.6f"),yClickMap);
		Edit_SetText(hwEdit,szText);
		if(pMap) {
			double latitude,longitude;

			pMap->calculateLatLon(xClickMap,yClickMap,
				&latitude,&longitude);
			hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
			Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
			_stprintf(szText,_T("%.6f"),latitude);
			Edit_SetText(hwEdit,szText);
			hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
			Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
			_stprintf(szText,_T("%.6f"),longitude);
			Edit_SetText(hwEdit,szText);
		} else {
			hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
			Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
			Edit_SetText(hwEdit,_T(""));
			hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
			Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
			Edit_SetText(hwEdit,_T(""));
		}
		hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
		Edit_LimitText(hwEdit,MAX_ENTRY_SIZE);
		nextCalNumber=getNextNumber(CAL_NUMBER);
		_stprintf(szText,_T("CAL%03d"),nextCalNumber);
		Edit_SetText(hwEdit,szText);
		// Calibration point list box
		hwListBox=GetDlgItem(hDlg,IDC_CALLIST);
		ListBox_ResetContent(hwListBox);  
		if(pMap) {
			const tsDLList<CMapPoint> *pointList=pMap->getList();
			tsDLIterBD<CMapPoint> iter(pointList->first());
			tsDLIterBD<CMapPoint> eol;
			index=0;
			while(iter != eol) {
				pPoint=(CMapPoint *)iter;
				if(pPoint->getIdent()) {
					ListBox_AddString(hwListBox,pPoint->getIdent());
					ListBox_SetItemData(hwListBox,index,(LPARAM)pPoint);
					index++;
				}
				iter++;
			}
		}
		ListBox_SetCurSel(hwListBox,0);
		// Waypoint list box
		hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
		ListBox_ResetContent(hwListBox);  
		if(waypointList.getCount()) {
			tsDLIterBD<CBlock> iter(waypointList.first());
			tsDLIterBD<CBlock> eol;
			index=0;
			while(iter != eol) {
				CBlock *pBlock=(CBlock *)iter;
				if(pBlock->getIdent()) {
					LPWSTR unicodeString=NULL;
					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						ListBox_AddString(hwListBox,unicodeString);
						ListBox_SetItemData(hwListBox,index,(LPARAM)pBlock);
						free(unicodeString);
					}
					index++;
				}
				iter++;
			}
		}
		ListBox_SetCurSel(hwListBox,0);
#ifdef UNDER_CE
		// Add the SIPPREF control with a unique ID for a child of this dialog
		CreateWindow(_T("SIPPREF"),_T(""),WS_CHILD,-10,-10,5,5,hDlg,
			(HMENU)ID_SIPPREF,hInst,0);
#endif
		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		switch(wmId) {       
	case IDADD:
	case IDREPLACE:
	case IDDELETE:
		if(!pMap) break;
		hwListBox=GetDlgItem(hDlg,IDC_CALLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR && wmId != IDADD) break;
		pPoint=(CMapPoint *)ListBox_GetItemData(hwListBox,sel);
		if(pPoint) {
			double latitude,longitude,xPix,yPix;
			TCHAR *stopString;

			hwEdit=GetDlgItem(hDlg,IDC_XENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			xPix=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_YENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			yPix=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			latitude=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			longitude=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			// Note name is stored in szText, don't use it any more
			szText[MAX_ENTRY_SIZE-1]='\0';

			if(wmId == IDADD) {
				pMap->addPoint(latitude,longitude,xPix,yPix,szText);
				// Replace the default name if it was used
				TCHAR oldName[MAX_ENTRY_SIZE];
				hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
				Edit_GetText(hwEdit,oldName,MAX_ENTRY_SIZE);
				oldName[MAX_ENTRY_SIZE-1]='\0';
				_stprintf(szText,_T("CAL%03d"),nextCalNumber);
				if(!_tcscmp(oldName,szText)) {
					nextCalNumber=getNextNumber(CAL_NUMBER);	
					_stprintf(szText,_T("CAL%03d"),nextCalNumber);
					Edit_SetText(hwEdit,szText);
				}
			} else if(wmId == IDREPLACE) {
				// Set the values
				pMap->setPoint(pPoint,latitude,longitude,xPix,yPix,
					szText);
			} else if(wmId == IDDELETE) {
				pMap->deletePoint(pPoint,latitude,longitude,xPix,yPix,
					szText);
			}
			// Recalibrate
			pMap->calibrate();
			isValid=pMap->isValid();
			resetTransformation();
			// Cause the map window to update
			InvalidateRect(hMap,NULL,TRUE);
			UpdateWindow(hMap);
			// Update calibration point list box
			hwListBox=GetDlgItem(hDlg,IDC_CALLIST);
			ListBox_ResetContent(hwListBox);  
			if(pMap) {
				const tsDLList<CMapPoint> *pointList=pMap->getList();
				tsDLIterBD<CMapPoint> iter(pointList->first());
				tsDLIterBD<CMapPoint> eol;
				index=0;
				while(iter != eol) {
					pPoint=(CMapPoint *)iter;
					if(pPoint->getIdent()) {
						ListBox_AddString(hwListBox,pPoint->getIdent());
						ListBox_SetItemData(hwListBox,index,(LPARAM)pPoint);
						index++;
					}
					iter++;
				}
			}
			ListBox_SetCurSel(hwListBox,0);
		}
		break;
	case IDCOPYALLCAL:
	case IDCOPYCAL:
		if(!pMap) break;
		hwListBox=GetDlgItem(hDlg,IDC_CALLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		pPoint=(CMapPoint *)ListBox_GetItemData(hwListBox,sel);
		if(pPoint) {
			hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
			_stprintf(szText,_T("%.6f"),pPoint->getLatitude());
			Edit_SetText(hwEdit,szText);
			hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
			_stprintf(szText,_T("%.6f"),pPoint->getLongitude());
			Edit_SetText(hwEdit,szText);
			hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
			_stprintf(szText,_T("%s"),pPoint->getIdent());
			Edit_SetText(hwEdit,szText);
			if(wmId == IDCOPYALLCAL) {
				hwEdit=GetDlgItem(hDlg,IDC_XENTRY);
				_stprintf(szText,_T("%d"),(int)(pPoint->getXPix()+.5));
				Edit_SetText(hwEdit,szText);
				hwEdit=GetDlgItem(hDlg,IDC_YENTRY);
				_stprintf(szText,_T("%d"),(int)(pPoint->getYPix()+.5));
				Edit_SetText(hwEdit,szText);
			}
		}
		break;
	case IDCOPYWPT:
		if(waypointList.getCount()) {
			hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
			sel=ListBox_GetCurSel(hwListBox);  
			if(sel == LB_ERR) break;
			CBlock *pBlock=(CBlock *)ListBox_GetItemData(hwListBox,sel);
			if(pBlock) {
				hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
				_stprintf(szText,_T("%.6f"),pBlock->getLatitude());
				Edit_SetText(hwEdit,szText);
				hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
				_stprintf(szText,_T("%.6f"),pBlock->getLongitude());
				Edit_SetText(hwEdit,szText);
				hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
				_stprintf(szText,_T("%s"),pBlock->getIdent());
				LPWSTR unicodeString=NULL;
				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					Edit_SetText(hwEdit,unicodeString);
					free(unicodeString);
				} else {
					Edit_SetText(hwEdit,_T("Error"));
				}
			}
		}
		break;
	case IDOK:
	case IDCANCEL:
#ifdef UNDER_CE
		// Resize the map window so it will show
		if(hMap) {
			RECT rcMain,rcDlg;
			GetClientRect(hWndMain,&rcMain);
			GetClientRect(hCalDlg,&rcDlg);
			LONG height=rcDlg.bottom-rcDlg.top;
			MoveWindow(hMap,0,0,rcMain.right-rcMain.left,
				rcMain.bottom-rcMain.top,TRUE);
			// Move the center up
			yOff-=.5*height/scale;
			resetTransformation();
		} 
#endif
		DestroyWindow(hCalDlg);
		hCalDlg=0;
		return TRUE;
		}
	}
	return FALSE;
}

#if DO_GAMMA
// Mesage handler for gamma dialog
LRESULT CALLBACK gammaDlgProc(HWND hDlg, UINT message,
							  WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
	RECT rc;
#endif
	int wmId;
	HWND hwCtl=NULL;
	TCHAR szText[MAX_ENTRY_SIZE];
	int val=0, pos=0;
	static BOOL adjustGammaOld=FALSE;
	static WORD gammaOld=10000;
	static COLORADJUSTMENT colorAdjustOld={0};
	static BOOL preview=TRUE;

	switch (message) {
	case WM_INITDIALOG:
		adjustGammaInProgress=TRUE;
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN;
		shidi.hDlg = hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);

		// Resize it because WCE seems to put an extra border at the
		// bottom
		GetWindowRect(hDlg,&rc);
		rc.bottom-=MENU_HEIGHT;
		MoveWindow(hDlg,rc.left,rc.top,
			rc.right-rc.left,rc.bottom-rc.top,FALSE);
#endif
		// Initialize COLORADJUSTMENT if necessary
		if(!colorAdjust.caSize) resetGamma();
		// Save incoming values
		adjustGammaOld=adjustGamma;
		gammaOld=gamma;
		colorAdjustOld=colorAdjust;
		// Initialize controls
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		Edit_LimitText(hwCtl,MAX_ENTRY_SIZE);
		_stprintf(szText,_T("%d"),gamma);
		Edit_SetText(hwCtl,szText);
		hwCtl=GetDlgItem(hDlg,IDC_GAMMASLIDER);
		SendMessage(hwCtl,TBM_SETRANGE,TRUE,
			MAKELONG(GAMMA_MIN/GAMMA_MULT,GAMMA_MAX/GAMMA_MULT));
		SendMessage(hwCtl,TBM_SETPOS,(WPARAM)TRUE,(LONG)(gamma/GAMMA_MULT));
		hwCtl=GetDlgItem(hDlg,IDC_PREVIEW);
		Button_SetCheck(hwCtl,preview?BST_CHECKED:BST_UNCHECKED);
#ifdef UNDER_CE
		// Add the SIPPREF control with a unique ID for a child of this dialog
		CreateWindow(_T("SIPPREF"),_T(""),WS_CHILD,-10,-10,5,5,hDlg,
			(HMENU)ID_SIPPREF,hInst,0);
#endif
		return TRUE;

	case WM_HSCROLL:
		hwCtl=GetDlgItem(hDlg,IDC_GAMMASLIDER);
		if((HWND)lParam == hwCtl) {
			pos=GAMMA_MULT*SendMessage(hwCtl,TBM_GETPOS,(WPARAM)0,(LPARAM)0);
			hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
			Edit_GetText(hwCtl,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			val=_ttoi(szText);
			if(val != pos) {
				_stprintf(szText,_T("%d"),pos);
				Edit_SetText(hwCtl,szText);
			}
		}
		return FALSE;	

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		switch(wmId) {       
	case IDDEFAULT:
		resetGamma();
		hwCtl=GetDlgItem(hDlg,IDC_GAMMASLIDER);
		SendMessage(hwCtl,TBM_SETPOS,(WPARAM)TRUE,(LONG)(gamma/GAMMA_MULT));
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		_stprintf(szText,_T("%d"),gamma);
		Edit_SetText(hwCtl,szText);
		break;
	case IDRESTORE:
		gamma=gammaOld;
		colorAdjust=colorAdjustOld;
		hwCtl=GetDlgItem(hDlg,IDC_GAMMASLIDER);
		SendMessage(hwCtl,TBM_SETPOS,(WPARAM)TRUE,(LONG)(gamma/GAMMA_MULT));
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		_stprintf(szText,_T("%d"),gamma);
		Edit_SetText(hwCtl,szText);
		break;
	case IDC_LIGHT:
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		val=GAMMA_LIGHT;
		_stprintf(szText,_T("%d"),GAMMA_LIGHT);
		Edit_SetText(hwCtl,szText);
		break;
	case IDC_MEDIUM:
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		val=GAMMA_MEDIUM;
		_stprintf(szText,_T("%d"),GAMMA_MEDIUM);
		Edit_SetText(hwCtl,szText);
		break;
	case IDC_GAMMAENTRY:
		hwCtl=GetDlgItem(hDlg,IDC_GAMMAENTRY);
		Edit_GetText(hwCtl,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		val=_ttoi(szText);
		// Readjust the values
		if(val < GAMMA_MIN) gamma=GAMMA_MIN;
		else if(val > GAMMA_MAX) gamma=GAMMA_MAX;
		else gamma=(WORD)val;
		colorAdjust.caRedGamma=colorAdjust.caBlueGamma=colorAdjust.caGreenGamma=
			gamma;
		adjustGamma=!(gamma == GAMMA_DEFAULT);
		if(preview && pMap && pMap != &pMapDefault) {
			// Make it repaint
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		hwCtl=GetDlgItem(hDlg,IDC_GAMMASLIDER);
		SendMessage(hwCtl,TBM_SETPOS,(WPARAM)TRUE,(LONG)(gamma/GAMMA_MULT));
		break;
	case IDC_PREVIEW:
		hwCtl=GetDlgItem(hDlg,IDC_PREVIEW);
		preview=Button_GetCheck(hwCtl);
		if(preview && pMap && pMap != &pMapDefault) {
			// Make it repaint
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		break;
	case IDCANCEL:
		gamma=gammaOld;
		adjustGamma=adjustGammaOld;
		colorAdjust=colorAdjustOld;
		// Fall through
	case IDOK:
		adjustGammaInProgress=FALSE;
#ifdef UNDER_CE
		// Resize the map window so it will show
		if(hMap) {
			RECT rcMain,rcDlg;
			GetClientRect(hWndMain,&rcMain);
			GetClientRect(hGammaDlg,&rcDlg);
			LONG height=rcDlg.bottom-rcDlg.top;
			MoveWindow(hMap,0,0,rcMain.right-rcMain.left,
				rcMain.bottom-rcMain.top,TRUE);
			// Move the center up
			yOff-=.5*height/scale;
			resetTransformation();
		}
#endif
		DestroyWindow(hGammaDlg);
		hGammaDlg=0;
		InvalidateRect(hMap,NULL,FALSE);
		UpdateWindow(hMap);
		return TRUE;
		}
	}
	return FALSE;
}
#endif

// Mesage handler for waypoint dialog
LRESULT CALLBACK editWaypointDlgProc(HWND hDlg, UINT message,
									 WPARAM wParam, LPARAM lParam)
{
	static BOOL useSpecifiedLatLon;
	static CBlockList *blockList;
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	int wmId,wmEvent;
	HWND hwButton=NULL;
	HWND hwEdit=NULL;
	HWND hwStatic=NULL;
	HWND hwListBox=NULL;
	HWND hwComboBox=NULL;
	TCHAR szText[MAX_ENTRY_SIZE];
	char  ansiText[MAX_ENTRY_SIZE];
	CBlock *pBlockSel=NULL,*pBlock=NULL;
	int sel;
	int index;
	int i;
	BOOL setDone;

	switch (message) {
	case WM_INITDIALOG:
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN |
			SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);
#endif

		// Symbol combo box
		hwComboBox=GetDlgItem(hDlg,IDC_SYMBOL);
		ComboBox_ResetContent(hwComboBox);  
		for(i=0; i < nSymbolsEtrex; i++) {
			char *symbolName=getEtrexSymbolKeyName(i);
			LPWSTR unicodeString=NULL;
			ansiToUnicode(symbolName,&unicodeString);
			if(unicodeString) {
				ComboBox_AddString(hwComboBox,unicodeString);
				free(unicodeString);
			}
		}

		// Edit controls
		pBlockSel=NULL;
		sel=-1;
		if(editDialogType == NEW_WPT) {
			blockList=&waypointList;
			nextWptNumber=getNextNumber(WPT_NUMBER);
			// Get the lat/lon from the map
			if(pMap) {
				double latitude,longitude,altitude;
				pMap->calculateLatLon(xClickMap,yClickMap,
					&latitude,&longitude);
				altitude=0;
				sprintf(ansiText,"REF%03d",nextWptNumber);
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					ansiText,DEFAULT_SYMBOL);
			} else {
				sprintf(ansiText,"REF%03d",nextWptNumber);
				editWaypointSetValues(hDlg,0,0,0,ansiText,DEFAULT_SYMBOL);
			}
			hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
			useSpecifiedLatLon=TRUE;
			Button_SetCheck(hwButton,useSpecifiedLatLon);
			Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
			hwStatic=GetDlgItem(hDlg,IDC_WAYPOINTTYPE);
			DestroyWindow(hwStatic);
		} else if(editDialogType == POS_WPT) {
			blockList=&waypointList;
			// Should not be called if position is invalid
			double latitude,longitude,altitude;
			// Get the lat/lon from the position
			longitude=position.getLongitude();
			latitude=position.getLatitude();
			altitude=position.getAltitude();
			nextPosNumber=getNextNumber(POS_NUMBER);
			sprintf(ansiText,"POS%03d",nextPosNumber);
			editWaypointSetValues(hDlg,latitude,longitude,altitude,
				ansiText,DEFAULT_POS_SYMBOL);
			hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
			useSpecifiedLatLon=TRUE;
			Button_SetCheck(hwButton,useSpecifiedLatLon);
			Button_SetText(hwButton,_T("Use Lat/Lon/Alt from Position"));
			hwStatic=GetDlgItem(hDlg,IDC_WAYPOINTTYPE);
			DestroyWindow(hwStatic);
		} else if(editDialogType == LV_WPT) {
			blockList=pMenuBlock->getParentList();
			double latitude,longitude,altitude;
			longitude=pMenuBlock->getLongitude();
			latitude=pMenuBlock->getLatitude();
			altitude=pMenuBlock->getAltitude();
			char *name=pMenuBlock->getIdent();
			int sym=((CWaypoint *)pMenuBlock)->getSymbol();
			editWaypointSetValues(hDlg,latitude,longitude,altitude,
				name,sym);
			if(blockList) {
				sel=blockList->getPos(pMenuBlock)-1;
			} else {
				sel=-1;
			}
			hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
			DestroyWindow(hwButton);
			hwStatic=GetDlgItem(hDlg,IDC_WAYPOINTTYPE);
			if(blockList == &waypointList) {
				Static_SetText(hwStatic,_T("Regular Waypoint"));
			} else {
				Static_SetText(hwStatic,_T("Route Waypoint"));
			}
		} else if(editDialogType == EDIT_WPT) {
			blockList=&waypointList;
			// Use closest waypoint
			if(pMap) {
				double minDistance;
				BOOL status=findClosestWaypoint(&sel,&pBlockSel,&minDistance );
				if(!status) {
					errMsg(_T("Cannot find nearest waypoint"));
					EndDialog(hDlg,LOWORD(wParam));
					return TRUE;
				}
			}
			// Set the entries
			if(sel >= 0 && pBlockSel) {
				double latitude,longitude,altitude;
				latitude=pBlockSel->getLatitude();
				longitude=pBlockSel->getLongitude();
				altitude=pBlockSel->getAltitude();
				char *name=pBlockSel->getIdent();
				int sym=((CWaypoint *)pBlockSel)->getSymbol();
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					name,sym);

				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
			} else {
				editWaypointSetValues(hDlg,0,0,0,"",DEFAULT_SYMBOL);
				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
			}
			hwStatic=GetDlgItem(hDlg,IDC_WAYPOINTTYPE);
			DestroyWindow(hwStatic);
		} else if(editDialogType == EDIT_RTE_WPT) {
			// Use closest route waypoint
			blockList=NULL;
			if(pMap) {
				double minDistance;
				BOOL status=findClosestRouteWaypoint(&sel,&pBlockSel,
					&minDistance );
				if(!status) {
					errMsg(_T("Cannot find nearest route waypoint"));
					EndDialog(hDlg,LOWORD(wParam));
					return TRUE;
				}
			}
			// Set the entries
			if(sel >= 0 && pBlockSel) {
				blockList=pBlockSel->getParentList();
				double latitude,longitude,altitude;
				latitude=pBlockSel->getLatitude();
				longitude=pBlockSel->getLongitude();
				altitude=pBlockSel->getAltitude();
				char *name=pBlockSel->getIdent();
				int sym=((CWaypoint *)pBlockSel)->getSymbol();
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					name,sym);

				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
			} else {
				editWaypointSetValues(hDlg,0,0,0,"",DEFAULT_SYMBOL);
				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
			}
			hwStatic=GetDlgItem(hDlg,IDC_WAYPOINTTYPE);
			DestroyWindow(hwStatic);
		} else {
			errMsg(_T("Invalid editDialogType"));
			EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
		}

		// Waypoint list box
		hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
		ListBox_ResetContent(hwListBox);  
		if(blockList && blockList->getCount()) {
			tsDLIterBD<CBlock> iter(blockList->first());
			tsDLIterBD<CBlock> eol;
			index=0;
			while(iter != eol) {
				CBlock *pBlock=(CBlock *)iter;
				if(pBlock->getIdent()) {
					LPWSTR unicodeString=NULL;
					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						ListBox_AddString(hwListBox,unicodeString);
						ListBox_SetItemData(hwListBox,index,(LPARAM)pBlock);
						free(unicodeString);
					}
					index++;
				}
				iter++;
			}
		}
		ListBox_SetCurSel(hwListBox,sel);
#ifdef UNDER_CE
		// Add the SIPPREF control with a unique ID for a child of this dialog
		CreateWindow(_T("SIPPREF"),_T(""),WS_CHILD,-10,-10,5,5,hDlg,
			(HMENU)ID_SIPPREF,hInst,0);
#endif
		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		wmEvent=HIWORD(wParam); 
		switch(wmId) {       
	case IDC_CHECKLATLON:
		hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
		useSpecifiedLatLon=Button_GetCheck(hwButton);
		// Reset the latitude values
		setDone=FALSE;
		if(useSpecifiedLatLon) {
			if(editDialogType == POS_WPT) {
				// Should not be called if position is invalid
				double latitude,longitude,altitude;
				// Get the lat/lon from the position
				longitude=position.getLongitude();
				latitude=position.getLatitude();
				altitude=position.getAltitude();
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					NULL,-1);
				setDone=TRUE;
			} else {
				if(pMap) {
					// Get the lat/lon from the map
					double latitude,longitude,altitude;
					pMap->calculateLatLon(xClickMap,yClickMap,
						&latitude,&longitude);
					// Get the altitude from the selected block
					hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
					sel=ListBox_GetCurSel(hwListBox);  
					if(sel != LB_ERR) {
						CBlock *pBlock=(CBlock *)ListBox_GetItemData(hwListBox,sel);
						if(pBlock) {
							altitude=pBlock->getAltitude();
						} else {
							altitude=0.;
						}
					} else {
						altitude=0.;
					}
					editWaypointSetValues(hDlg,latitude,longitude,altitude,
						NULL,-1);
					setDone=TRUE;
				}
			}
		} else {
			// Reset from selected
			hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
			sel=ListBox_GetCurSel(hwListBox);  
			if(sel == LB_ERR) break;
			CBlock *pBlock=(CBlock *)ListBox_GetItemData(hwListBox,sel);
			if(pBlock) {
				double latitude,longitude,altitude;
				latitude=pBlock->getLatitude();
				longitude=pBlock->getLongitude();
				altitude=pBlock->getAltitude();
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					NULL,-1);
				setDone=TRUE;
			}
		}
		// If still not set, use defaults
		if(!setDone) {
			editWaypointSetValues(hDlg,0,0,0,NULL,-1);
			setDone=TRUE;
		}
		return TRUE;
	case IDC_WPTLIST:
		switch(wmEvent) {
	case LBN_DBLCLK:
		{
			hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
			sel=ListBox_GetCurSel(hwListBox);  
			if(sel == LB_ERR) break;
			CBlock *pBlock=(CBlock *)ListBox_GetItemData(hwListBox,sel);
			if(pBlock) {
				double latitude,longitude,altitude;
				latitude=pBlock->getLatitude();
				longitude=pBlock->getLongitude();
				altitude=pBlock->getAltitude();
				char *name=pBlock->getIdent();
				int sym=((CWaypoint *)pBlock)->getSymbol();
				editWaypointSetValues(hDlg,latitude,longitude,altitude,
					name,sym);
				// Reset the check box
				if(editDialogType != LV_WPT) {
					hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
					useSpecifiedLatLon=FALSE;
					Button_SetCheck(hwButton,useSpecifiedLatLon);
				}
				return TRUE;
			} else {
				return FALSE;
			}
		}
	default:
		return FALSE;
		}
	case IDADD:
	case IDREPLACE:
	case IDDELETE:
		hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR && wmId != IDADD) break;
		pBlock=(CBlock *)ListBox_GetItemData(hwListBox,sel);
		if(pBlock) {
			double latitude,longitude,altitude;
			TCHAR *stopString;

			hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			latitude=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			longitude=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_ALTENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			szText[MAX_ENTRY_SIZE-1]='\0';
			altitude=_tcstod(szText,&stopString);

			hwEdit=GetDlgItem(hDlg,IDC_NAMEENTRY);
			Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
			// Note name is stored in szText, don't use it any more
			szText[MAX_ENTRY_SIZE-1]='\0';

			TCHAR szSymbol[MAX_ENTRY_SIZE];
			hwComboBox=GetDlgItem(hDlg,IDC_SYMBOL);
			ComboBox_GetText(hwComboBox,szSymbol,MAX_ENTRY_SIZE);
			szSymbol[MAX_ENTRY_SIZE-1]='\0';

			if(wmId == IDADD) {
				// Add it as checked
				char *ansiName=NULL,*ansiSymbol=NULL;
				unicodeToAnsi(szText,&ansiName);
				unicodeToAnsi(szSymbol,&ansiSymbol);
				CWaypoint *pWaypoint;

				if(ansiName && ansiSymbol) {
					pWaypoint=new CWaypoint(TRUE,
						ansiName,latitude,longitude,altitude,ansiSymbol);
					free(ansiName);
					free(ansiSymbol);
				} else if(ansiName) {
					pWaypoint=new CWaypoint(TRUE,
						ansiName,latitude,longitude,altitude,"Flag");
					free(ansiName);
				} else if(ansiSymbol) {
					pWaypoint=new CWaypoint(TRUE,
						"Error",latitude,longitude,altitude,ansiSymbol);
					free(ansiSymbol);
				} else {
					pWaypoint=new CWaypoint(TRUE,
						"Error",latitude,longitude,altitude,"Flag");
				}
				if(!pWaypoint) {
					lbprintf(_T("Cannot create waypoint"));
					break;
				}
				if(!blockList) {
					errMsg(_T("Does not belong to a list"));
				} else {
					if(pWaypoint->isDuplicate(blockList)) {
						errMsg(_T("Not added, is a duplicate"));
						delete pWaypoint;
					} else {
						blockList->addBlock((CBlock *)pWaypoint);
						char *ptr=pWaypoint->getIdent();
						if(strstr(ptr,"REF") == ptr) {
							nextWptNumber=getNextNumber(WPT_NUMBER);
						} else if(strstr(ptr,"POS") == ptr) {
							nextPosNumber=getNextNumber(POS_NUMBER);
						}
					}
				}
			} else if(wmId == IDREPLACE) {
				// Set the values
				char *ansiName=NULL,*ansiSymbol=NULL;

				unicodeToAnsi(szText,&ansiName);
				if(ansiName) {
					pBlock->setIdent(ansiName);
					free(ansiName);
				} else {
					errMsg(_T("Error setting name"));
				}

				unicodeToAnsi(szSymbol,&ansiSymbol);
				if(ansiSymbol) {
					CWaypoint *pWaypoint=(CWaypoint *)pBlock;
					pWaypoint->setSymbol(ansiSymbol);
					free(ansiSymbol);
				} else {
					errMsg(_T("Error setting symbol"));
				}

				pBlock->setLatitude(latitude);
				pBlock->setLongitude(longitude);
				pBlock->setAltitude(altitude);
			} else if(wmId == IDDELETE) {
				if(blockList) blockList->removeBlock(pBlock,TRUE);
			}
			// Update the waypoint list view
			if(hWptLV) wptCreateLV(hWndMain);
			// Cause the map window to update
			InvalidateRect(hMap,NULL,TRUE);
			UpdateWindow(hMap);
			// Update waypoint list box
			hwListBox=GetDlgItem(hDlg,IDC_WPTLIST);
			ListBox_ResetContent(hwListBox);  
			if(blockList && blockList->getCount()) {
				tsDLIterBD<CBlock> iter(blockList->first());
				tsDLIterBD<CBlock> eol;
				index=0;
				while(iter != eol) {
					CBlock *pBlock=(CBlock *)iter;
					if(pBlock->getIdent()) {
						LPWSTR unicodeString=NULL;
						ansiToUnicode(pBlock->getIdent(),&unicodeString);
						if(unicodeString) {
							ListBox_AddString(hwListBox,unicodeString);
							ListBox_SetItemData(hwListBox,index,(LPARAM)pBlock);
							free(unicodeString);
						}
						index++;
					}
					iter++;
				}
			}
			ListBox_SetCurSel(hwListBox,sel);
		}
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
	case IDOK:
	case IDCANCEL:
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
		}
	}
	return FALSE;
}

// Mesage handler for trackpoint dialog
LRESULT CALLBACK editTrackpointDlgProc(HWND hDlg, UINT message,
									   WPARAM wParam, LPARAM lParam)
{
	static BOOL useSpecifiedLatLon;
	static CBlockList *blockList;
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	int wmId,wmEvent;
	HWND hwButton=NULL;
	HWND hwEdit=NULL;
	HWND hwStatic=NULL;
	HWND hwListBox=NULL;
	HWND hwComboBox=NULL;
	TCHAR szText[MAX_ENTRY_SIZE];
	CBlock *pBlockSel=NULL,*pBlock=NULL,*pTrackBlock=NULL;
	int sel;
	int index;
	BOOL isNewTrack;
	BOOL status;

	switch (message) {
	case WM_INITDIALOG:
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN |
			SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);
#endif

		// Be sure the indices are set
		if(!hTrkLV) calculateTrackIndices();

		// Edit controls
		if(editDialogType == LV_TPT) {
			blockList=pMenuBlock->getParentList();
			double latitude,longitude,altitude;
			longitude=pMenuBlock->getLongitude();
			latitude=pMenuBlock->getLatitude();
			altitude=pMenuBlock->getAltitude();
			index=pMenuBlock->getIndex();
			isNewTrack=((CTrackpoint *)pMenuBlock)->isNewTrack();
			editTrackpointSetValues(hDlg,latitude,longitude,altitude,
				index,isNewTrack,SET_ALL);
			hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
			DestroyWindow(hwButton);
			hwStatic=GetDlgItem(hDlg,IDC_TRACKNAME);
			pTrackBlock=getTrackFromTrackpoint(pMenuBlock);
			if(pTrackBlock) {
				LPWSTR unicodeString=NULL;
				ansiToUnicode(pTrackBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					Static_SetText(hwStatic,unicodeString);
				} else {
					Static_SetText(hwStatic,_T("Error"));
				}
			} else {
				Static_SetText(hwStatic,_T("Unknown Track"));
			}
		} else if(editDialogType == EDIT_TPT) {
			// Use closest trackpoint
			blockList=NULL;
			if(pMap) {
				double minDistance;
				status=findClosestTrackpoint(&sel,&pBlockSel,
					&minDistance );
				if(!status) {
					errMsg(_T("Cannot find nearest trackpoint"));
					EndDialog(hDlg,LOWORD(wParam));
					return TRUE;
				}
			}
			// Set the entries
			if(sel >= 0 && pBlockSel) {
				blockList=pBlockSel->getParentList();
				double latitude,longitude,altitude;
				latitude=pBlockSel->getLatitude();
				longitude=pBlockSel->getLongitude();
				altitude=pBlockSel->getAltitude();
				index=pBlockSel->getIndex();
				isNewTrack=((CTrackpoint *)pBlockSel)->isNewTrack();
				editTrackpointSetValues(hDlg,latitude,longitude,altitude,
					index,isNewTrack,SET_ALL);

				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
				hwStatic=GetDlgItem(hDlg,IDC_TRACKNAME);
				pTrackBlock=getTrackFromTrackpoint(pBlockSel);
				if(pTrackBlock) {
					LPWSTR unicodeString=NULL;
					ansiToUnicode(pTrackBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						Static_SetText(hwStatic,unicodeString);
					} else {
						Static_SetText(hwStatic,_T("Error"));
					}
				} else {
					Static_SetText(hwStatic,_T("Unknown Track"));
				}
			} else {
				editTrackpointSetValues(hDlg,0,0,0,0,TRUE,SET_ALL);
				hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
				useSpecifiedLatLon=FALSE;
				Button_SetCheck(hwButton,useSpecifiedLatLon);
				Button_SetText(hwButton,_T("Use Lat/Lon from Cursor"));
				hwStatic=GetDlgItem(hDlg,IDC_TRACKNAME);
				Static_SetText(hwStatic,_T("Unknown Track"));
			}
		} else {
			errMsg(_T("Invalid editDialogType"));
			EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
		}

#ifdef UNDER_CE
		// Add the SIPPREF control with a unique ID for a child of this dialog
		CreateWindow(_T("SIPPREF"),_T(""),WS_CHILD,-10,-10,5,5,hDlg,
			(HMENU)ID_SIPPREF,hInst,0);
#endif
		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		wmEvent=HIWORD(wParam); 
		switch(wmId) {       
	case IDC_CHECKLATLON:
		hwButton=GetDlgItem(hDlg,IDC_CHECKLATLON);
		useSpecifiedLatLon=Button_GetCheck(hwButton);
		// Fall through
	case IDC_INDEXENTRY:
		hwEdit=GetDlgItem(hDlg,IDC_INDEXENTRY);
		Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		index=_ttoi(szText);
		status=getBlockFromIndex(index,blockList,&pBlock);
		if(!status) {
			editTrackpointSetValues(hDlg,0.0,0.0,0.0,0,TRUE,
				SET_ALL_EXC_INDEX);
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			return TRUE;
		}
		if(pBlock) {
			double latitude,longitude,altitude;
			if(useSpecifiedLatLon) {
				pMap->calculateLatLon(xClickMap,yClickMap,
					&latitude,&longitude);
			} else {
				latitude=pBlock->getLatitude();
				longitude=pBlock->getLongitude();
			}
			altitude=pBlock->getAltitude();
			index=pBlock->getIndex();
			isNewTrack=((CTrackpoint *)pBlock)->isNewTrack();
			editTrackpointSetValues(hDlg,latitude,longitude,altitude,
				index,isNewTrack,SET_ALL_EXC_INDEX);
		}
		return TRUE;
	case IDADD:
	case IDREPLACE:
	case IDDELETE:
		hwEdit=GetDlgItem(hDlg,IDC_INDEXENTRY);
		Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		index=_ttoi(szText);
		status=getBlockFromIndex(index,blockList,&pBlock);
		if(!status) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			errMsg(_T("Invalid index"));
			return TRUE;
		}
		if(!pBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			errMsg(_T("Bad data"));
			return TRUE;
		}
		double latitude,longitude,altitude;
		TCHAR *stopString;

		hwEdit=GetDlgItem(hDlg,IDC_LATENTRY);
		Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		latitude=_tcstod(szText,&stopString);

		hwEdit=GetDlgItem(hDlg,IDC_LONENTRY);
		Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		longitude=_tcstod(szText,&stopString);

		hwEdit=GetDlgItem(hDlg,IDC_ALTENTRY);
		Edit_GetText(hwEdit,szText,MAX_ENTRY_SIZE);
		szText[MAX_ENTRY_SIZE-1]='\0';
		altitude=_tcstod(szText,&stopString);

		hwButton=GetDlgItem(hDlg,IDC_TRACKSTART);
		isNewTrack=Button_GetCheck(hwButton);

		if(wmId == IDADD) {
			// Add after the index given with check on
			CTrackpoint *pTrackpoint=new CTrackpoint(TRUE);
			if(!pTrackpoint) {
				PlaySound(_T("Critical"),NULL,SND_SYNC);
				errMsg(_T("Cannot create trackpoint"));
				return TRUE;
			}
			pTrackpoint->setLatitude(latitude);
			pTrackpoint->setLongitude(longitude);
			pTrackpoint->setAltitude(altitude);
			pTrackpoint->setIdent(isNewTrack?"Start":"Continue");
			if(!blockList) {
				PlaySound(_T("Critical"),NULL,SND_SYNC);
				errMsg(_T("Does not belong to a list"));
				return TRUE;
			} else {
				blockList->insertBlockAfter(pTrackpoint,pBlock);
			}
		} else if(wmId == IDREPLACE) {
			pBlock->setLatitude(latitude);
			pBlock->setLongitude(longitude);
			pBlock->setAltitude(altitude);
			pBlock->setIdent(isNewTrack?"Start":"Continue");
		} else if(wmId == IDDELETE) {
			if(blockList) blockList->removeBlock(pBlock,TRUE);
		}
		// Update the track list view
		calculateTrackIndices();
		if(hTrkLV) trkCreateLV(hWndMain);
		// Cause the map window to update
		InvalidateRect(hMap,NULL,TRUE);
		UpdateWindow(hMap);
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
	case IDOK:
	case IDCANCEL:
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
		}
	}
	return FALSE;
}

static void editWaypointSetValues(HWND hDlg, double lat, double lon, double alt,
								  char *name, int sym)
{
	HWND hwCtrl;
	TCHAR szText[MAX_ENTRY_SIZE];

	hwCtrl=GetDlgItem(hDlg,IDC_LATENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.6f"),lat);
	Edit_SetText(hwCtrl,szText);
	hwCtrl=GetDlgItem(hDlg,IDC_LONENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.6f"),lon);
	Edit_SetText(hwCtrl,szText);
	hwCtrl=GetDlgItem(hDlg,IDC_ALTENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.0f"),alt);
	Edit_SetText(hwCtrl,szText);
	if(name) {
		hwCtrl=GetDlgItem(hDlg,IDC_NAMEENTRY);
		LPWSTR unicodeString=NULL;
		ansiToUnicode(name,&unicodeString);
		if(unicodeString) {
			Edit_SetText(hwCtrl,unicodeString);
			free(unicodeString);
		} else {
			_stprintf(szText,_T("Error"));
			Edit_SetText(hwCtrl,szText);
		}
	}
	if(sym >= 0) {
		char *symbolName=getSymbolName(sym);
		hwCtrl=GetDlgItem(hDlg,IDC_SYMBOL);
		LPWSTR unicodeString=NULL;
		ansiToUnicode(symbolName,&unicodeString);
		if(unicodeString) {
			ComboBox_SetText(hwCtrl,unicodeString);
			free(unicodeString);
		} else {
			ComboBox_SetText(hwCtrl,_T("Flag"));
		}
	}
}

static void editTrackpointSetValues(HWND hDlg, double lat, double lon, double alt,
									int index, BOOL isNewTrack, TrackSetValuesType type)
{
	HWND hwCtrl;
	TCHAR szText[MAX_ENTRY_SIZE];

	hwCtrl=GetDlgItem(hDlg,IDC_LATENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.6f"),lat);
	Edit_SetText(hwCtrl,szText);

	hwCtrl=GetDlgItem(hDlg,IDC_LONENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.6f"),lon);
	Edit_SetText(hwCtrl,szText);

	if(type == SET_LATLON) return;

	hwCtrl=GetDlgItem(hDlg,IDC_ALTENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%.6f"),alt);
	Edit_SetText(hwCtrl,szText);

	hwCtrl=GetDlgItem(hDlg,IDC_TRACKSTART);
	Button_SetCheck(hwCtrl,isNewTrack);
	hwCtrl=GetDlgItem(hDlg,IDC_TRACKCONTINUE);
	Button_SetCheck(hwCtrl,!isNewTrack);

	if(type == SET_ALL_EXC_INDEX) return;

	hwCtrl=GetDlgItem(hDlg,IDC_INDEXENTRY);
	Edit_LimitText(hwCtrl,MAX_ENTRY_SIZE);
	_stprintf(szText,_T("%d"),index);
	Edit_SetText(hwCtrl,szText);
}

/////////////////////////////////////////////////////////////
///////////////////// Support Routines //////////////////////
/////////////////////////////////////////////////////////////

void positionToLatLon(double latitude, double longitude)
{
	long x,y;
	double facX=.5,facY=.5;

	calculatePixel(latitude,longitude,&x,&y);
	xOff+=(x-facX*width)/scale;
	yOff+=(y-facY*height)/scale;
	resetTransformation();
	// Cause the map window to update
	InvalidateRect(hMap,NULL,TRUE);
	UpdateWindow(hMap);
}

static void positionToPoint(PositionType type)
{
	double facX=.5,facY;

	switch(type) {
	case CENTER_TOP:
		facY=.25;
		break;
	case CENTER_CENTER:
		facY=.50;
		break;
	case CENTER_BOTTOM:
		facY=.75;
		break;
	case TOP_LEFT:
		facX=facY=0.;
		break;	
	}

	xOff+=(menuPoint.x-facX*width)/scale;
	yOff+=(menuPoint.y-facY*height)/scale;
	resetTransformation();
}

// Zoom in
static void zoomin(void)
{
	double scale0=scale;

	scale*=ZOOMFAC;
	xOff+=.5*(scale-scale0)/(scale*scale0)*width;
	yOff+=.5*(scale-scale0)/(scale*scale0)*height;
	resetTransformation();
}

// Zoom out
static void zoomout(void)
{
	double scale0=scale;

	scale/=ZOOMFAC;
	xOff+=.5*(scale-scale0)/(scale*scale0)*width;
	yOff+=.5*(scale-scale0)/(scale*scale0)*height;
	resetTransformation();
}

static void zoomSpecified(double factor)
{
	double scale0=scale;

	if(!factor) return;
	scale=factor;
	xOff+=.5*(scale-scale0)/(scale*scale0)*width;
	yOff+=.5*(scale-scale0)/(scale*scale0)*height;
	resetTransformation();
}

// Zoom to normal map size
static void zoomNormal(void)
{
	double scale0=scale;

	scale=1.0;
	xOff+=.5*(scale-scale0)/(scale*scale0)*width;
	yOff+=.5*(scale-scale0)/(scale*scale0)*height;
	resetTransformation();
}

// Zoom to specified pixel coordinates
static void zoom(double x0, double y0, double x1, double y1)
{
	double scale0=scale,scalex,scaley;

	// Check for zero-length box
	if(x0 == x1) {
		errMsg(_T("Zoom window has zero width"));
		return;
	}
	if(y0 == y1) {
		errMsg(_T("Zoom window has zero height"));
		return;
	}

	// Pick scale to be lesser of x and y scale
	scalex=width/(x1-x0)*scale;
	scaley=height/(y1-y0)*scale;
	scale=min(scalex,scaley);
	xOff+=.5*((x1+x0)/scale0-width/scale);
	yOff+=.5*((y1+y0)/scale0-height/scale);
	resetTransformation();
}

static void move(int direction, double movefac)
{
	switch(direction) {
	case 1:
		// sw
		_tcscpy(szMapString,_T("SW"));
		movedown(movefac);
		moveleft(movefac);
		break;
	case 2:
		// s
		_tcscpy(szMapString,_T("S"));
		movedown(movefac);
		break;
	case 3:
		// se
		_tcscpy(szMapString,_T("SE"));
		movedown(movefac);
		moveright(movefac);
		break;
	case 4:
		// e
		_tcscpy(szMapString,_T("E"));
		moveright(movefac);
		break;
	case 5:
		// ne
		_tcscpy(szMapString,_T("NE"));
		moveup(movefac);
		moveright(movefac);
		break;
	case 6:
		// n
		_tcscpy(szMapString,_T("N"));
		moveup(movefac);
		break;
	case 7:
		// nw
		_tcscpy(szMapString,_T("NW"));
		moveup(movefac);
		moveleft(movefac);
		break;
	default:
		// w
		_tcscpy(szMapString,_T("W"));
		moveleft(movefac);
		break;
	}
}

static void moveup(double movefac)
{
	yOff-=movefac*height/scale;
	resetTransformation();
}

static void movedown(double movefac)
{
	yOff+=movefac*height/scale;
	resetTransformation();
}

static void moveright(double movefac)
{
	xOff+=movefac*width/scale;
	resetTransformation();
}

static void moveleft(double movefac)
{
	xOff-=movefac*width/scale;
	resetTransformation();
}

// Draws cross in pixel coordinates
static void drawCross(HDC hdc, long X, long Y)
{
	POINT point[2];

	point[0].x=X-SYMBSIZE;
	point[0].y=Y;
	point[1].x=X+SYMBSIZE;
	point[1].y=Y;
	Polyline(hdc,point,2);
	point[0].x=X;
	point[0].y=Y-SYMBSIZE;
	point[1].x=X;
	point[1].y=Y+SYMBSIZE;
	Polyline(hdc,point,2);
}

// Draws line in pixel coordinates
static void drawLine(HDC hdc, long X1, long Y1, long X2, long Y2)
{
	POINT point[2];

	point[0].x=X1;
	point[0].y=Y1;
	point[1].x=X2;
	point[1].y=Y2;
	Polyline(hdc,point,2);
}

// Draws rectangle in pixel coordinates
static void drawSquare(HDC hdc, long X, long Y)
{
#if 1
	Rectangle(hdc,X-SQUARESIZE,Y-SQUARESIZE,
		X+SQUARESIZE+1,Y+SQUARESIZE+1);
#else
	POINT point[2];

	point[0].x=X-SQUARESIZE;
	point[0].y=Y;
	point[1].x=X+SQUARESIZE;
	point[1].y=Y;
	Polyline(hdc,point,2);
	point[0].x=X;
	point[0].y=Y-SQUARESIZE;
	point[1].x=X;
	point[1].y=Y+SQUARESIZE;
	Polyline(hdc,point,2);
#endif
}

// Draws box in pixel coordinates for rubberband box
static void drawBoxOutline(HWND hwnd, POINT beg, POINT end)
{
	HDC hdc=GetDC(hwnd);

	SetROP2(hdc,R2_NOT);
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	Rectangle(hdc,beg.x,beg.y,end.x,end.y);
	ReleaseDC(hwnd,hdc);
}

// Determine the lat,lon envelope
static void getLatLonEnvelope(double *pLatMin, double *pLonMin,
							  double *pLatMax, double *pLonMax)
{
	double latMin,lonMin,latMax,lonMax; 
	double latMin0,lonMin0,latMax0,lonMax0; 

	latMin=lonMin=DBL_MAX;
	latMax=lonMax=-DBL_MAX;
	latMin0=latMin;
	lonMin0=lonMin;
	latMax0=latMax;
	lonMax0=lonMax;

	// Tracks
	if(plotTracks) {
		tsDLIterBD<CBlock> iter(trackList.first());
		tsDLIterBD<CBlock> eol;
		// Do the trackpoints
		while(iter != eol) {
			CBlock *pBlock=iter;
			CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			if(pBlock->isChecked()) {
				tsDLIterBD<CBlock> iter1(blockList->first());
				while(iter1 != eol) {
					CBlock *pBlock1=iter1;
					if(pBlock1->isChecked()) {
						double lon=pBlock1->getLongitude();
						double lat=pBlock1->getLatitude();
						if(lat < latMin) latMin=lat;
						if(lon < lonMin) lonMin=lon;
						if(lat > latMax) latMax=lat;
						if(lon > lonMax) lonMax=lon;
					}
					++iter1;
				}
			}
			++iter;
		}
	}

	// Waypoints
	if(plotWaypoints) {
		tsDLIterBD<CBlock> iter(waypointList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			if(pBlock->isChecked()) {
				double lon=pBlock->getLongitude();
				double lat=pBlock->getLatitude();
				if(lat < latMin) latMin=lat;
				if(lon < lonMin) lonMin=lon;
				if(lat > latMax) latMax=lat;
				if(lon > lonMax) lonMax=lon;
			}
			++iter;
		}
	}

	// Routes
	if(plotRoutes) {
		tsDLIterBD<CBlock> iter(routeList.first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			CBlock *pBlock=iter;
			CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			if(pBlock->isChecked()) {
				// Do the waypoints
				tsDLIterBD<CBlock> iter1(blockList->first());
				while(iter1 != eol) {
					CBlock *pBlock1=iter1;
					if(pBlock1->isChecked()) {
						double lon=pBlock1->getLongitude();
						double lat=pBlock1->getLatitude();
						if(lat < latMin) latMin=lat;
						if(lon < lonMin) lonMin=lon;
						if(lat > latMax) latMax=lat;
						if(lon > lonMax) lonMax=lon;
					}
					++iter1;
				}
			}
			++iter;
		}
	}

	// Set the values depending on if they are different from the
	// (unphysical) starting values
	if(latMin != latMin0) *pLatMin=latMin-MAPLIMITSSCALEFAC*(latMax-latMin);
	else *pLatMin=LATMIN;
	if(lonMin != lonMin0) *pLonMin=lonMin-MAPLIMITSSCALEFAC*(lonMax-lonMin);
	else *pLonMin=LONMIN;
	if(latMax != latMax0) *pLatMax=latMax+MAPLIMITSSCALEFAC*(latMax-latMin);
	else *pLatMax=LATMAX;
	if(lonMax != lonMax0) *pLonMax=lonMax+MAPLIMITSSCALEFAC*(lonMax-lonMin);
	else *pLonMax=LONMAX;
}

// Adjusts pixel coordinates to maintain aspect ratio
static BOOL adjustAspectRatio(double aspect, double *pLatMin, double *pLonMin,
							  double *pLatMax, double *pLonMax)
{
	double x0=*pLonMin,y0=*pLatMin,x1=*pLonMax,y1=*pLatMax;
	double width=x1-x0;
	double height=y1-y0;

	if(!aspect || !width || !height) {
		return FALSE;
	}
	double aspect1=height/width;
	if(aspect1 < aspect) {
		// Expand y
		double heightNew=width*aspect;
		y0-=.5*(heightNew-height);
		y1+=.5*(heightNew-height);
	} else {
		// Expand x
		double widthNew=height/aspect;
		x0-=.5*(widthNew-width);
		x1+=.5*(widthNew-width);
	}
	*pLonMin=x0; *pLatMin=y0; *pLonMax=x1; *pLatMax=y1;

	return TRUE;
}

// Restores the window to no zooming, scaled to fit points
static BOOL restore(void)
{
	RECT rect;
	BOOL status;

	if(!pMap) {
		return FALSE;
	}

	// Get window dimensions
	GetClientRect(hMap,&rect);
	width=(double)(rect.right-rect.left);
	height=(double)(rect.bottom-rect.top);
	if(!width) {
		errMsg(_T("Window has zero width"));
		return FALSE;
	}
	if(!height) {
		errMsg(_T("Window has zero height"));
		return FALSE;
	}

	// Get the limits of the current set of points
	double latMin=0.0,latMax=0.0,lonMin=0.0,lonMax=0.0;
	getLatLonEnvelope(&latMin,&lonMin,&latMax,&lonMax);

	// Check for min=max
	if(latMin == latMax) {
		latMax+=LATLON_EXTEND;
		latMin-=LATLON_EXTEND;
	}
	if(lonMin == lonMax) {
		lonMax+=LATLON_EXTEND;
		lonMin-=LATLON_EXTEND;
	}

	// Check parameters
	if(latMin >= latMax || lonMin >= lonMax) {
		errMsg(_T("Map dimensions are invalid\n")
			_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
			latMin,latMax,lonMin,lonMax);
		return FALSE;
	}

	// Set the calibration for the default map or an uncalibrated map
	if(pMap == &pMapDefault || !pMap->getCount()) {
		// Adjust the aspect ratio to that of the window
		double aspect=height/width;
		if(!adjustAspectRatio(aspect,&latMin,&lonMin,&latMax,&lonMax)) {
			latMin=LATMIN;
			lonMin=LONMIN;
			latMax=LATMAX;
			lonMax=LONMAX;
		}

		// Check parameters again
		if(latMin >= latMax || lonMin >= lonMax) {
			errMsg(_T("Map dimensions are invalid\n")
				_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
				latMin,latMax,lonMin,lonMax);
			return FALSE;
		}

		// Reset the calibration points
		pMap->clearPoints();
		pMap->addPoint(latMax,lonMin,0,0,_T("ul"));
		pMap->addPoint(latMin,lonMin,0,rect.bottom,_T("ll"));
		pMap->addPoint(latMin,lonMax,rect.right,rect.bottom,_T("lr"));
		pMap->addPoint(latMax,lonMax,rect.right,0,_T("ur"));
		pMap->calibrate();
	} else {
		// Just calibrate
		pMap->calibrate();
	}
	isValid=pMap->isValid();

	// Reset the transformation
	scale=1.0;
	xOff=yOff=0.0;
	status=resetTransformation();
	if(!status) {
		errMsg(_T("Cannot set up the coordinate transformation"));
		return FALSE;
	}

	// Find the pixel coordinates corresponding to these limits
	long xPix,yPix,xPixMax,xPixMin,yPixMax,yPixMin;
	status=calculatePixel(latMax,lonMin,&xPix,&yPix);
	if(!status) {
		errMsg(_T("Cannot convert latitude, longitude to pixels\n")
			_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
			latMin,latMax,lonMin,lonMax);
		return FALSE;
	}
	xPixMax=xPixMin=xPix;
	yPixMax=yPixMin=yPix;
	status=calculatePixel(latMin,lonMin,&xPix,&yPix);
	if(!status) {
		errMsg(_T("Cannot convert latitude, longitude to pixels\n")
			_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
			latMin,latMax,lonMin,lonMax);
		return FALSE;
	}
	xPixMax=max(xPixMax,xPix);
	xPixMin=min(xPixMin,xPix);
	yPixMax=max(yPixMax,yPix);
	yPixMin=min(yPixMin,yPix);
	status=calculatePixel(latMin,lonMax,&xPix,&yPix);
	if(!status) { 
		errMsg(_T("Cannot convert latitude, longitude to pixels\n")
			_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
			latMin,latMax,lonMin,lonMax);
		return FALSE;
	}
	xPixMax=max(xPixMax,xPix);
	xPixMin=min(xPixMin,xPix);
	yPixMax=max(yPixMax,yPix);
	yPixMin=min(yPixMin,yPix);
	status=calculatePixel(latMax,lonMax,&xPix,&yPix);
	if(!status) {
		errMsg(_T("Cannot convert latitude, longitude to pixels\n")
			_T("  latMin=%g latMax=%g\n  lonMin=%g lonMax=%g"),
			latMin,latMax,lonMin,lonMax);
		return FALSE;
	}
	xPixMax=max(xPixMax,xPix);
	xPixMin=min(xPixMin,xPix);
	yPixMax=max(yPixMax,yPix);
	yPixMin=min(yPixMin,yPix);

	// Zoom to the limits (will fix aspect ratio if necessary)
	zoom((double)xPixMin,(double)yPixMin,(double)xPixMax,(double)yPixMax);

	return TRUE;
}

static BOOL resetTransformation(void)
{
	if(!pMap || !pMap->isValid()) {
		return FALSE;
	}

	Matrix a0,ainv0,b0;
	Matrix offset(2,1);

	offset.setval(1,1,xOff);
	offset.setval(2,1,yOff);

	pMap->getMatrices(a0,ainv0,b0);
	a=a0/scale;
	ainv=scale*ainv0;
	b=a0*offset+b0;

	return TRUE;
}

// Get lat/lon from screen coordinates
static BOOL calculateLatLon(double xPix, double yPix,
							double *latitude, double *longitude)
{
	Matrix l,xs(2,1,0.0);

	if(!isValid || !pMap || !pMap->isValid()) return FALSE;

	xs.setval(1,1,xPix);
	xs.setval(2,1,yPix);
	l=a*xs+b;
	*latitude=l.getval(1,1);
	*longitude=l.getval(2,1);

	return TRUE;
}

// Get screen cooordinates from lat/lon
static BOOL calculatePixel(double latitude, double longitude,
						   long *xPix, long *yPix)
{
	Matrix l(2,1,0.0),xs;

	if(!isValid || !pMap || !pMap->isValid()) return FALSE;

	l.setval(1,1,latitude);
	l.setval(2,1,longitude);
	xs=ainv*(l-b);
	*xPix=(long)(xs.getval(1,1)+.5);
	*yPix=(long)(xs.getval(2,1)+.5);

	return TRUE;
}

void doEditWaypoint(EditDialogType type)
{
	editDialogType=type;
	DialogBox(hInst,(LPCTSTR)IDD_EDITWAYPOINT,hWndMain,
		(DLGPROC)editWaypointDlgProc);
}

void doEditTrackpoint(EditDialogType type)
{
	editDialogType=type;
	DialogBox(hInst,(LPCTSTR)IDD_EDITTRACKPOINT,hWndMain,
		(DLGPROC)editTrackpointDlgProc);
}

void changeMapImage(CMap *pNewMap)
{
	// If there are calibration points calibrate it
	if(pNewMap->getCount()) {
		pNewMap->calibrate();
		if(!pNewMap->isValid()) {
			errMsg(_T("Could not calibrate map:\n%s"),
				pNewMap->getFileName());
		}
	}

	// If there is no map yet, set it and return
	if(!hMap) {
		pMap=pNewMap;
		return;
	}

	// Get the window coordinates
	RECT rect;
	GetClientRect(hMap,&rect);

	// Get the lat/lon for the current corners
	double latA,lonA,latC,lonC;
	calculateLatLon(0.0,0.0,&latA,&lonA);
	calculateLatLon((double)(rect.right-rect.left),
		(double)(rect.bottom-rect.top),&latC,&lonC);
	if(latC >= latA || lonC <= lonA) {
		errMsg(_T("Bad map dimensions, resetting"));
		pMap=pNewMap;
		createMapWindow(hWndMain);
		return;
	}

	// If the map is not calibrated, use these points to calibrate it
	if(!pNewMap->getCount()) {
		pNewMap->addPoint(latA,lonA,0,0,_T("ul"));
		pNewMap->addPoint(latC,lonA,0,rect.bottom,_T("ll"));
		pNewMap->addPoint(latC,lonC,rect.right,rect.bottom,_T("lr"));
		pNewMap->addPoint(latA,lonC,rect.right,0,_T("ur"));
		pNewMap->calibrate();
		if(!pNewMap->isValid()) {
			errMsg(_T("Could not calibrate map:\n%s"),
				pNewMap->getFileName());
		}
	}

	// Determine the offsets and scale to keep the map where it is now
	double xA,yA,xC,yC;
	// Calibrated, get the offsets from the ul
	pNewMap->calculatePixel(latA,lonA,&xA,&yA);
	// Get the scale from the lr
	pNewMap->calculatePixel(latC,lonC,&xC,&yC);
	if(xC <= xA || yC <= yA) {
		errMsg(_T("Cannot position map, resetting"));
		pMap=pNewMap;
		createMapWindow(hWndMain);
		return;
	}
#if 0
	// Restores the map with the new image
	pMap=pNewMap;
	createMapWindow(hWndMain);
#else
	xOff=xA;
	yOff=yA;
	double scaleX,scaleY;
	scaleX=width/(xC-xOff);
	scaleY=height/(yC-yOff);
	if(scaleX <= 0 || scaleY <= 0) {
		errMsg(_T("Cannot scale map, resetting"));
		pMap=pNewMap;
		createMapWindow(hWndMain);
		return;
	}
	// Use the mean
	scale=sqrt(scaleX*scaleY);
	pMap=pNewMap;
	resetTransformation();

	// Repaint
	InvalidateRect(hMap,NULL,FALSE);
	UpdateWindow(hMap);
#endif
}

static int getNextNumber(IncrementalNumberType type)
{
	CBlockList *pList;
	int number;
	char prefix[4];
	char name[MAX_ENTRY_SIZE];
	BOOL found;

	switch(type) {
	case CAL_NUMBER:
		number=nextCalNumber;
		strcpy(prefix,"CAL");
		break;
	case WPT_NUMBER:
		pList=&waypointList;
		number=nextWptNumber;
		strcpy(prefix,"REF");
		break;
	case POS_NUMBER:
		pList=&waypointList;
		number=nextPosNumber;
		strcpy(prefix,"POS");
		break;
	}

	if(type == CAL_NUMBER) {
		found=TRUE;
		while(found) {
			const tsDLList<CMapPoint> *pointList=pMap->getList();
			tsDLIterBD<CMapPoint> iter(pointList->first());
			tsDLIterBD<CMapPoint> eol;
			sprintf(name,"%s%03d",prefix,number);
			while(iter != eol) {
				CMapPoint *pPoint=iter;
				char *ansiName=NULL;
				unicodeToAnsi(pPoint->getIdent(),&ansiName);
				if(ansiName) {
					if(!strcmp(name,ansiName)) {
						found=TRUE;
						number++;
						free(ansiName);
						break;
					}
					free(ansiName);
				}
				iter++;
			}
			found=FALSE;
		}
	} else {
		found=TRUE;
		while(found) {
			tsDLIterBD<CBlock> iter(waypointList.first());
			tsDLIterBD<CBlock> eol;
			sprintf(name,"%s%03d",prefix,number);
			while(iter != eol) {
				CBlock *pBlock=iter;
				if(!strcmp(name,pBlock->getIdent())) {
					found=TRUE;
					number++;
					break;
				}
				iter++;
			}
			found=FALSE;
		}
	}

	return number;
}

// Find closest waypoint to (xClickMap,yClickMap)
// index is zero based for listbox
static BOOL findClosestWaypoint(int *pSel, CBlock **ppBlockSel,
								double *pDist)
{
	double latitude,longitude;
	int index;

	if(!pMap) return FALSE;

	pMap->calculateLatLon(xClickMap,yClickMap,
		&latitude,&longitude);
	*pDist=DBL_MAX;
	*ppBlockSel=NULL;
	*pSel=-1;
	if(!waypointList.getCount()) return TRUE;
	tsDLIterBD<CBlock> iter(waypointList.first());
	tsDLIterBD<CBlock> eol;
	index=0;
	while(iter != eol) {
		CBlock *pBlock=(CBlock *)iter;
		if(pBlock->isChecked()) {
			double distance=greatCircleDistance(latitude,longitude,
				pBlock->getLatitude(),pBlock->getLongitude());
			if(distance < *pDist) {
				*pDist=distance;
				*ppBlockSel=pBlock;
				*pSel=index;
			}
		}
		index++;
		iter++;
	}
	return TRUE;
}

// Find closest route waypoint to (xClickMap,yClickMap)
// index is zero based for listbox
static BOOL findClosestRouteWaypoint(int *pSel, CBlock **ppBlockSel,
									 double *pDist)
{
	double latitude,longitude;
	int index;

	if(!pMap) return FALSE;

	pMap->calculateLatLon(xClickMap,yClickMap,
		&latitude,&longitude);
	*pDist=DBL_MAX;
	*ppBlockSel=NULL;
	*pSel=-1;
	if(!routeList.getCount()) return TRUE;
	// Loop over routes
	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
		if(pBlock->isChecked()) {
			// Loop over waypoints
			tsDLIterBD<CBlock> iter1(blockList->first());
			index=0;
			while(iter1 != eol) {
				CBlock *pBlock1=(CBlock *)iter1;
				if(pBlock1->isChecked()) {
					double distance=greatCircleDistance(latitude,longitude,
						pBlock1->getLatitude(),pBlock1->getLongitude());
					if(distance < *pDist) {
						*pDist=distance;
						*ppBlockSel=pBlock1;
						*pSel=index;
					}
				}
				index++;
				iter1++;
			}
		}
		iter++;
	}
	return TRUE;
}

// Find closest trackpoint to (xClickMap,yClickMap)
// index is zero based for listbox
static BOOL findClosestTrackpoint(int *pSel, CBlock **ppBlockSel,
								  double *pDist)
{
	double latitude,longitude;
	int index;

	if(!pMap) return FALSE;

	pMap->calculateLatLon(xClickMap,yClickMap,&latitude,&longitude);
	*pDist=DBL_MAX;
	*ppBlockSel=NULL;
	*pSel=-1;

	if(!trackList.getCount()) return TRUE;

	// Loop over tracks
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *pBlockList=((CTrack *)pBlock)->getBlockList();
		if(pBlock->isChecked()) {
			// Loop over trackpoints
			tsDLIterBD<CBlock> iter1(pBlockList->first());
			index=0;
			while(iter1 != eol) {
				CBlock *pBlock1=(CBlock *)iter1;
				if(pBlock1->isChecked()) {
					double distance=greatCircleDistance(latitude,longitude,
						pBlock1->getLatitude(),pBlock1->getLongitude());
					if(distance < *pDist) {
						*pDist=distance;
						*ppBlockSel=pBlock1;
						*pSel=index;
					}
				}
				index++;
				iter1++;
			}
		}
		iter++;
	}
	return TRUE;
}

static BOOL getBlockFromIndex(int index, CBlockList *pBlockList,
							  CBlock **ppBlock)
{
	CBlock *pBlock;

	if(pBlockList) {
		tsDLIterBD<CBlock> iter(pBlockList->first());
		tsDLIterBD<CBlock> eol;
		while(iter != eol) {
			pBlock=(CBlock *)iter;
			int index1=pBlock->getIndex();
			if(index1 == index) {
				*ppBlock=pBlock;
				return TRUE;
			}
			iter++;
		}
	}

	// Didn't find it
	*ppBlock=NULL;
	return FALSE;
}

static CBlock *getRouteFromWaypoint(CBlock *pBlock)
{
	if(!pBlock) return NULL;
	CBlockList *pBlockList=pBlock->getParentList();

	// Loop over routes
	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *pBlockList1=((CRoute *)pBlock)->getBlockList();
		if(pBlockList1 == pBlockList) {
			return pBlock;
		}
		iter++;
	}

	return NULL;
}

CBlock *getTrackFromTrackpoint(CBlock *pBlock)
{
	if(!pBlock) return NULL;
	CBlockList *pBlockList=pBlock->getParentList();

	// Loop over tracks
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		CBlockList *pBlockList1=((CTrack *)pBlock)->getBlockList();
		if(pBlockList1 == pBlockList) {
			return pBlock;
		}
		iter++;
	}

	return NULL;
}

static void calculateTrackIndices(void)
{
	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	int trkIndex=1;
	while(iter != eol) {
		int index=1;
		CBlock *pBlock=iter;
		CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
		// Do the trackpoints
		tsDLIterBD<CBlock> iter1(blockList->first());
		CBlock *pBlock0=NULL;
		while(iter1 != eol) {
			CBlock *pBlock1=iter1;
			pBlock1->setIndex(index++);
			++iter1;
		}
		pBlock->setIndex(trkIndex++);
		iter++;
	}
}

static void doGoToLVWaypoint(void)
{
	int sel=-1;
	int index;
	CBlock *pBlockSel;
	double minDist;
	int count;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	if(!pMap) goto FINISH;

	double latitude,longitude;
	pMap->calculateLatLon(xClickMap,yClickMap,
		&latitude,&longitude);

	// Create the listview 
	if(!hWptLV) {
		// Create it
		wptCreateLV(hWndMain);
		if(!hWptLV) {
			errMsg(_T("Could not create waypoint ListView"));
			goto FINISH;
		}
	}

	// Get the item count
	count=ListView_GetItemCount(hWptLV);
	if(count <= 0) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		goto FINISH;
	}

	// Loop over the items in the listview
	minDist=DBL_MAX;
	pBlockSel=NULL;
	sel=-1;
	for(index=0; index < count; index++) {
		CBlock *pBlock=lvGetBlockFromItem(hWptLV,index);
		if(pBlock && pBlock->isChecked() &&
			pBlock->getType() == TYPE_WPT) {
				double dist=greatCircleDistance(latitude,longitude,
					pBlock->getLatitude(),pBlock->getLongitude());
				if(dist < minDist) {
					minDist=dist;
					pBlockSel=pBlock;
					sel=index;
				}
		}
	}

	// Unselect everything
	ListView_SetItemState(hWptLV,-1,0x0000,
		LVIS_SELECTED|LVIS_FOCUSED|LVIS_CUT|LVIS_DROPHILITED);

	// Set the the item to be selected and make it visible
	if(pBlockSel && sel > -1) {
		ListView_SetItemState(hWptLV,
			sel,LVIS_SELECTED|LVIS_FOCUSED,
			LVIS_SELECTED|LVIS_FOCUSED);
		ListView_EnsureVisible(hWptLV,sel,FALSE);
	} else {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
	}

FINISH:
	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;

	viewWindow(hWptLV);
}

static void doGoToLVRouteWaypoint(void)
{
	int sel=-1;
	int index;
	CBlock *pBlockSel;
	double minDist;
	int count;
	BOOL parentChecked;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	if(!pMap) goto FINISH;

	double latitude,longitude;
	pMap->calculateLatLon(xClickMap,yClickMap,
		&latitude,&longitude);

	// Create the listview 
	showRouteWaypoints=TRUE;
	if(!hRteLV || !showRouteWaypointsOld) {
		// Create it
		rteCreateLV(hWndMain);
		if(!hRteLV) {
			errMsg(_T("Could not create route ListView"));
			goto FINISH;
		}
	}

	// Get the item count
	count=ListView_GetItemCount(hRteLV);
	if(count <= 0) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		goto FINISH;
	}

	// Loop over the items in the listview
	minDist=DBL_MAX;
	pBlockSel=NULL;
	sel=-1;
	parentChecked=FALSE;
	for(index=0; index < count; index++) {
		CBlock *pBlock=lvGetBlockFromItem(hRteLV,index);
		// See if the route is checked
		if(pBlock && pBlock->getType() == TYPE_RTE) {
			parentChecked=pBlock->isChecked();
		}
		if(parentChecked && pBlock && pBlock->isChecked() &&
			pBlock->getType() == TYPE_WPT) {
				double dist=greatCircleDistance(latitude,longitude,
					pBlock->getLatitude(),pBlock->getLongitude());
				if(dist < minDist) {
					minDist=dist;
					pBlockSel=pBlock;
					sel=index;
				}
		}
	}

	// Unselect everything
	ListView_SetItemState(hRteLV,-1,0x0000,
		LVIS_SELECTED|LVIS_FOCUSED|LVIS_CUT|LVIS_DROPHILITED);

	// Set the the item to be selected and make it visible
	if(pBlockSel && sel > -1) {
		ListView_SetItemState(hRteLV,
			sel,LVIS_SELECTED|LVIS_FOCUSED,
			LVIS_SELECTED|LVIS_FOCUSED);
		ListView_EnsureVisible(hRteLV,sel,FALSE);
	} else {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
	}

FINISH:
	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;

	viewWindow(hRteLV);
}

static void doGoToLVTrackpoint(void)
{
	int sel=-1;
	int index;
	CBlock *pBlockSel;
	double minDist;
	int count;
	BOOL parentChecked;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	if(!pMap) goto FINISH;

	double latitude,longitude;
	pMap->calculateLatLon(xClickMap,yClickMap,
		&latitude,&longitude);

	// Create the listview 
	showTrackpoints=TRUE;
	if(!hTrkLV || !showTrackpointsOld) {
		trkCreateLV(hWndMain);
		if(!hTrkLV) {
			errMsg(_T("Could not create track ListView"));
			goto FINISH;
		}
	}

	// Get the item count
	count=ListView_GetItemCount(hTrkLV);
	if(count <= 0) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		goto FINISH;
	}

	// Loop over the items in the listview
	minDist=DBL_MAX;
	pBlockSel=NULL;
	sel=-1;
	parentChecked=FALSE;
	for(index=0; index < count; index++) {
		CBlock *pBlock=lvGetBlockFromItem(hTrkLV,index);
		// See if the track is checked
		if(pBlock && pBlock->getType() == TYPE_TRK) {
			parentChecked=pBlock->isChecked();
		}
		if(parentChecked && pBlock && pBlock->isChecked() &&
			pBlock->getType() == TYPE_TPT) {
				double dist=greatCircleDistance(latitude,longitude,
					pBlock->getLatitude(),pBlock->getLongitude());
				if(dist < minDist) {
					minDist=dist;
					pBlockSel=pBlock;
					sel=index;
				}
		}
	}

	// Unselect everything
	ListView_SetItemState(hTrkLV,-1,0x0000,
		LVIS_SELECTED|LVIS_FOCUSED|LVIS_CUT|LVIS_DROPHILITED);

	// Set the the item to be selected and make it visible
	if(pBlockSel && sel > -1) {
		ListView_SetItemState(hTrkLV,
			sel,LVIS_SELECTED|LVIS_FOCUSED,
			LVIS_SELECTED|LVIS_FOCUSED);
		ListView_EnsureVisible(hTrkLV,sel,FALSE);
	} else {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
	}

FINISH:
	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;

	viewWindow(hTrkLV);
}

#if DO_GAMMA
static void resetGamma(void) {
	adjustGamma=FALSE;
	gamma=GAMMA_DEFAULT;

	colorAdjust.caSize=sizeof(colorAdjust);
	colorAdjust.caRedGamma=colorAdjust.caBlueGamma=colorAdjust.caGreenGamma=
		gamma;
	colorAdjust.caFlags=NULL;
#ifdef UNDER_CE
	// ILLUMINANT_DEVICE_DEFAULT is not defined in wingdi.h
	colorAdjust.caIlluminantIndex=0;
#else
	colorAdjust.caIlluminantIndex=ILLUMINANT_DEVICE_DEFAULT;
#endif
	colorAdjust.caReferenceBlack=0;
	colorAdjust.caReferenceWhite=10000;
	colorAdjust.caContrast=0;
	colorAdjust.caBrightness=0;
	colorAdjust.caColorfulness=0;
	colorAdjust.caRedGreenTint=0;
}
#endif
