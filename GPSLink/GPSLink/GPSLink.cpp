// GPSLink.cpp * Connects to GPS receiver

#include "StdAfx.h"

#define DEBUG_CLIPBOARD 1
#define DEBUG_LIST 0
#ifdef UNDER_CE
#  define DO_GPSL_TO_GPX_CONVERT 0
#  define DO_COMMAND_LINE 0
#else
// Define this to open files specified on the command line
#  define DO_COMMAND_LINE 1
// Define this to read a GPSL file name from the command line, and write
// a GPX file with the results.  There is no prompt to overwrite and also
// minimal error handling.  It thus creates a GPX file with the same information
// as the GPSL file.  The original intent was to fix errors in GPX files and to
// insure there is a GPX file for each GPSL file. It does not convert GPSL files
// that have a map in them. DO_COMMAND_LINE must be also be true.  Only one file
// will be processed and it must be a GPSL file.  This is a temporary enhancement
// and should not be left defined.
#  define DO_GPSL_TO_GPX_CONVERT 0
#endif

// To use the WC_SIPPREF control, set USE_SIPPREF to 1 below
//   and add the following line to the .rc file at the end of
//   the dialog resource for dialogs that take character input
// CONTROL         "",-1,WC_SIPPREF,NOT WS_VISIBLE,-10,-10,5,5
//   or implement it via CreateWindow.
#define USE_SIPPREF 1

#include "resource.h"
#include "file.h"
#include "GPSLink.h"
#include "version.h"

#define MAX_LOADSTRING 100

#define DELIMITER "Delimiter"
#define GMTOFFSET "GMTOffset"
#define MAX_COM_SIZE 25

#define WIN32_WIDTH 800
#define WIN32_HEIGHT 600

// Function prototypes
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK aboutDlgProc(HWND hDlg, UINT message,
							  WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK fileOptionsDlgProc(HWND hDlg, UINT message,
									WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK colorsDlgProc(HWND hDlg, UINT message,
							   WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK comPortDlgProc(HWND hDlg, UINT message,
								WPARAM wParam, LPARAM lParam);
void viewWindow(HWND hWnd);
static void connect(void);
static void disconnect(void);
static void getID(void);
static void abortTransfer(void);
static void getWaypoints(void);
static void getRoutes(void);
static void getTracks(void);
static void sendWaypoints(void);
static void sendRoutes(void);
static void sendTracks(void);
static void sendAll(void);
static void openFile(void);
static void parseCommandLine(void);
static void saveFile(void);
static void resetCurrentWindow(void);
static void resizeChildWindows(WPARAM wParam, LPARAM lParam);
static void sort(DataType type);

// Global variables
TCHAR szPrintString[PRINT_STRING_SIZE];
HWND hWndMain=NULL;           // The main window
HWND hWptLV=NULL;             // Waypoint ListView handle
HWND hRteLV=NULL;             // Route ListView handle
HWND hTrkLV=NULL;             // Track ListView handle
HWND hMap=NULL;               // Map window handle
HWND hCurWnd=NULL;            // Window that is currently in view
HANDLE hPort=INVALID_HANDLE_VALUE;  // Serial port handle
HANDLE hReadThread=NULL;      // Handle to the read thread
HCURSOR specialCursor=NULL;
FILE *fp;
#ifdef _WIN32_WCE_EMULATION   // Name of COM port
TCHAR lptszComPort[MAX_COM_SIZE]={_T("COM2:")};    // No colon?
#elif defined(UNDER_CE)
TCHAR lptszComPort[MAX_COM_SIZE]={_T("COM1:")};   // Colon needed
#else
TCHAR lptszComPort[MAX_COM_SIZE]={_T("COM4:")};
#endif
CBlock *pMenuBlock=NULL;
CMap pMapDefault;
CMap *pMap=&pMapDefault;
CMap *pMapFile=NULL;
BOOL saveWpt=TRUE;
BOOL saveRte=TRUE;
BOOL saveTrk=TRUE;
BOOL saveMap=FALSE;
BOOL saveMapShortName=TRUE;
BOOL saveCheckedOnly=FALSE;
BOOL showTrackpoints=FALSE;
BOOL showTrackpointsOld=showTrackpoints;
BOOL showRouteWaypoints=TRUE;
BOOL showRouteWaypointsOld=showRouteWaypoints;
HMENU hLvPopupMenu=NULL;

// Global Variables for this file
BOOL doFileSave=FALSE;
HINSTANCE hInst=NULL;                // The current instance
HWND hWndCB=NULL;                    // The command bar handle
TCHAR szTitle[MAX_LOADSTRING];       // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING]; // The window class name
TCHAR szFileName[_MAX_PATH];         // The current filename
#if DO_COMMAND_LINE
// This is only implemented for not UNDER_CE
LPSTR glpCmdLine;
#endif

#define NCOLORS 8
COLORREF colors[NCOLORS]={
	RGB(0,0,0),
	RGB(255,255,255),
	RGB(255,0,0),
	RGB(0,255,0),
	RGB(0,0,255),
	RGB(0,255,255),
	RGB(255,255,0),
	RGB(255,0,255),
};
TCHAR *colorNames[NCOLORS]={
	_T("Black"),
	_T("White"),
	_T("Red"),
	_T("Green"),
	_T("Blue"),
	_T("Cyan"),
	_T("Yellow"),
	_T("Magenta"),
};
int wptColorIndex=0;
int rteColorIndex=2;
int trkColorIndex=4;
int posColorIndex=7;
int calColorIndex=7;
COLORREF mapWptColor=colors[wptColorIndex];
COLORREF mapRteColor=colors[rteColorIndex];
COLORREF mapTrkColor=colors[trkColorIndex];
COLORREF mapCalColor=colors[posColorIndex];
COLORREF mapPosColor=colors[calColorIndex];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
#ifdef UNDER_CE
				   LPTSTR    lpCmdLine,
#else
				   LPSTR    lpCmdLine,
#endif
				   int nCmdShow)
{
	MSG msg;

#if DO_COMMAND_LINE
	// Save the command line as a global variable
	glpCmdLine = lpCmdLine;
#endif

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_CLASS, szWindowClass, MAX_LOADSTRING);
	// Look for running instance of application
	if(FindWindow(szWindowClass, szTitle)) {
		SetForegroundWindow(FindWindow(szWindowClass, szTitle));
		return FALSE;
	}

	// Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	// Main message loop:
	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

	wc.style          = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc    = (WNDPROC) WndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = hInstance;
	wc.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wc.hCursor        = 0;
	wc.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
#ifdef UNDER_CE
	wc.lpszMenuName=0;
#else
	wc.lpszMenuName=(LPTSTR)IDM_MAIN_MENUBAR;
#endif
	wc.lpszClassName  = szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// Store instance handle in our global variable
	hInst = hInstance;

#ifdef UNDER_CE
#if USE_SIPPREF
	// Initialize the extra controls (WC_SIPPREF)
	SHInitExtraControls();
#endif
#endif

	// Register the class
	MyRegisterClass(hInstance, szWindowClass);

#ifdef UNDER_CE
	hWndMain=CreateWindow(szWindowClass,szTitle,
		WS_VISIBLE|WS_CLIPCHILDREN,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL,NULL,hInstance,NULL);
#else
	// WCE is 240x320 using 8 pt. Tahoma 
	// Dialogs don't have OK button, but CR works
	int xFrame=GetSystemMetrics(SM_CXFRAME);
	int yFrame=GetSystemMetrics(SM_CYFRAME);
	int yMenu=GetSystemMetrics(SM_CYMENU);
	int yCaption=GetSystemMetrics(SM_CYCAPTION);
	hWndMain = CreateWindow(szWindowClass, szTitle,
		WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WIN32_WIDTH+2*xFrame,
		WIN32_HEIGHT-2*MENU_HEIGHT+yCaption+yMenu+2*yFrame,
		NULL,NULL,hInstance,NULL);
#endif
	if(!hWndMain) {	
		DWORD dwError=GetLastError();
		return FALSE;
	}

	// When the main window is created using CW_USEDEFAULT the height of
	// the menubar (if one is created is not taken into account).  So we
	// resize the window after creating it if a menubar is present.
	{
		RECT rc;
		GetWindowRect(hWndMain, &rc);
		rc.bottom -= MENU_HEIGHT;
		if(hWndCB)
			MoveWindow(hWndMain, rc.left, rc.top,
			rc.right-rc.left, rc.bottom-rc.top, FALSE);
	}

	// Create the console and center it under the commandbar.  Do this
	// here instead of in response to WM_CREATE so it will not get a
	// resize message when the size of hWndMain is not determined yet
	createConsole(hInst,hWndMain);
	lbprintf(_T("Welcome to GPSLink"));

	// Initialize time routines
	timeInit();
	lbprintf(_T("GMT Offset=%ld hr"),getTimeOffset()/3600);

	// Load the listview popupmenu
	hLvPopupMenu=LoadMenu(hInst,MAKEINTRESOURCE(IDM_LVPOPUPMENU));
	hLvPopupMenu=GetSubMenu(hLvPopupMenu,0);

	ShowWindow(hWndMain,nCmdShow);
	UpdateWindow(hWndMain);

#if DO_COMMAND_LINE
	// This block reads filenames from the command line and opens them.
	parseCommandLine();
#endif

	return TRUE;
}

static void parseCommandLine(void) {
	if(!glpCmdLine || glpCmdLine[0] == '\0') return;
	size_t len = strlen(glpCmdLine) + 1;

	// Make a copy of the command line
	char *cmdLine = new char[len];
	strcpy(cmdLine,glpCmdLine);

	// Count the items
	char * pattern = " ,\t";
	char *token = strtok(cmdLine, pattern);
	int nItems = 0;
	while(token) {
		nItems++;
		token = strtok(NULL,pattern);
	}

	// Make an array to hold the items
	char **fileList=new char*[nItems];

	// Reparse cmdLine and fill in the array of file names
	// This is necessary as strtok will get reused while processing the files
	lbprintf(_T("Parsing command-line files:"));
	int index=0;
	strcpy(cmdLine,glpCmdLine);
	token = strtok(cmdLine, pattern);
	while(token) {
		// Handle quotes
		len=strlen(token);
		if(len > 0 && token[len-1] == '"') token[len-1] = '\0';
		len=strlen(token);
		if(len > 0 && token[0] == '"') token++;
		len=strlen(token) + 1;
		// Add the file name to the list
		fileList[index] = new char[len];
		strcpy(fileList[index],token);
		index++;
		token = strtok(NULL,pattern);
	}
	delete cmdLine;

	// Process the list
	TCHAR *unicodeString = NULL;
	char *ptr;
	for(int i=0; i<nItems; i++) {
		ansiToUnicode(fileList[i],&unicodeString);
		if(unicodeString) {
#if 0
			lbprintf(unicodeString);
#endif
			if(ptr=strstr(fileList[i],".gpsl")) {
				readGPSLFile(unicodeString);
			} else if(ptr=strstr(fileList[i],".gpx")) {
				readGPXFile(unicodeString);
			} 
			free(unicodeString);
			unicodeString = NULL;
#if DO_GPSL_TO_GPX_CONVERT
			// Create a GPX file
			lbprintf(_T("Creating a GPX file from the first command-line file"));
			// Only use the first file
			if(i > 0) {
				// Should not get here
				lbprintf(_T("Not processing remaining command-line files"));
				break;
			}
			// The first file must be a GPSL file
			if(!(ptr=strstr(fileList[i],".gpsl"))) {
				lbprintf(_T("First file is not a GPSL file"));
				break;
			}
			// Check if there is a map in the file
			if(pMapFile) {
				lbprintf(_T("Not converted because there is a map"));
				break;
			}
			// Make the GPX name
			len = strlen(fileList[0]) + 1;
			char *gpxName = new char[len];
			strcpy(gpxName,fileList[0]);
			ptr=strstr(gpxName,".gpsl");
			*ptr = 0;
			strcpy(ptr,".gpx");
			ansiToUnicode(gpxName, &unicodeString);
			// Write the file
			saveWpt=TRUE;
			saveRte=TRUE;
			saveTrk=TRUE;
			saveMap=FALSE;
			writeGPXFile(unicodeString);
			readGPXFile(unicodeString);
			free(unicodeString);
			unicodeString = NULL;
			delete gpxName;
			if(nItems > 0) {
				lbprintf(_T("Remaining command-line files will be ignored"));
			}
			break;
#endif DO_GPSL_TO_GPX_CONVERT
		}
	}

	// Clean up
	if(fileList) {
		for(int i=0; i < nItems; i++) {
			delete fileList[i];
			fileList[i]=NULL;
		}
		delete fileList;
		fileList=NULL;
	}

	lbprintf(_T("Finished parsing command-line files"));
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch(message) {
	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		wmEvent=HIWORD(wParam); 
		// Parse the menu selections:
		switch(wmId) {
	case ID_HELP_ABOUT:
		DialogBox(hInst,(LPCTSTR)IDD_ABOUTBOX,hWnd,
			(DLGPROC)aboutDlgProc);
		break;
	case ID_FILE_OPEN:
		openFile();
		break;
	case ID_FILE_SAVE:
	case ID_FILE_SAVEAS:
		saveFile();
		break;
	case ID_FILE_QUIT:
		if(hPort != INVALID_HANDLE_VALUE) disconnect();
		DestroyWindow(hWnd);
		break;
	case ID_EDIT_CHECK_EVERYTHING:
	case ID_EDIT_CHECK_ALLBOXES:
		checkBlocks(TYPE_WPT,TRUE,FALSE);
		checkBlocks(TYPE_RTE,TRUE,TRUE);
		checkBlocks(TYPE_TRK,TRUE,TRUE);
		break;
	case ID_EDIT_CHECK_ALLWPTRTETRK:
		checkBlocks(TYPE_WPT,TRUE,FALSE);
		checkBlocks(TYPE_RTE,TRUE,FALSE);
		checkBlocks(TYPE_TRK,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLWAYPOINTS:
		checkBlocks(TYPE_WPT,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLROUTES:
		checkBlocks(TYPE_RTE,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLTRACKS:
		checkBlocks(TYPE_TRK,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLTRACKPOINTS:
		checkBlocks(TYPE_TRK,TRUE,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLROUTEWAYPOINTS:
		checkBlocks(TYPE_RTE,TRUE,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_ALLSELECTED:
		checkSelectedBlocks(TRUE);
		break;
	case ID_EDIT_CHECK_NOSELECTED:
		checkSelectedBlocks(FALSE);
		break;
	case ID_EDIT_CHECK_NOBOXES:
		checkBlocks(TYPE_WPT,FALSE,FALSE);
		checkBlocks(TYPE_RTE,FALSE,TRUE);
		checkBlocks(TYPE_TRK,FALSE,TRUE);
		break;
	case ID_EDIT_CHECK_NOWPTRTETRK:
		checkBlocks(TYPE_WPT,FALSE,FALSE);
		checkBlocks(TYPE_RTE,FALSE,FALSE);
		checkBlocks(TYPE_TRK,FALSE,FALSE);
		break;
	case ID_EDIT_CHECK_NOWAYPOINTS:
		checkBlocks(TYPE_WPT,FALSE,FALSE);
		break;
	case ID_EDIT_CHECK_NOROUTES:
		checkBlocks(TYPE_RTE,FALSE,FALSE);
		break;
	case ID_EDIT_CHECK_NOTRACKS:
		checkBlocks(TYPE_TRK,FALSE,FALSE);
		break;
	case ID_EDIT_CHECK_NOTRACKPOINTS:
		checkBlocks(TYPE_TRK,FALSE,TRUE,FALSE);
		break;
	case ID_EDIT_CHECK_NOROUTEWAYPOINTS:
		checkBlocks(TYPE_RTE,FALSE,TRUE,FALSE);
		break;
	case ID_EDIT_DELETE_ALL:
		deleteBlocks(TYPE_WPT);
		deleteBlocks(TYPE_RTE);
		deleteBlocks(TYPE_TRK);
		break;
	case ID_EDIT_DELETE_ALLWAYPOINTS:
		deleteBlocks(TYPE_WPT);
		break;
	case ID_EDIT_DELETE_ALLROUTES:
		deleteBlocks(TYPE_RTE);
		break;
	case ID_EDIT_DELETE_ALLTRACKS:
		deleteBlocks(TYPE_TRK);
		break;
	case ID_EDIT_DELETE_ALLCHECKED:
		deleteCheckedBlocks(TRUE);
		break;
	case ID_EDIT_DELETE_ALLUNCHECKED:
		deleteCheckedBlocks(FALSE);
		break;
	case ID_EDIT_DELETE_ALLSELECTED:
		removeSelectedBlocks(EDIT_DELETE);
		break;
	case ID_EDIT_CUT:
		removeSelectedBlocks(EDIT_CUT);
		break;
	case ID_EDIT_COPY:
		removeSelectedBlocks(EDIT_COPY);
		break;
	case ID_EDIT_PASTE_ATTOP:
	case ID_EDIT_PASTE_BEFORE:
	case ID_EDIT_PASTE_ON:
	case ID_EDIT_PASTE_AFTER:
	case ID_EDIT_PASTE_ATBOTTOM:
		pasteBlocks(wParam);
		break;
	case ID_TOOLS_SORT_WAYPOINTS:
		sort(TYPE_WPT);
		break;
	case ID_TOOLS_SORT_ROUTES:
		sort(TYPE_RTE);
		break;
	case ID_TOOLS_SORT_TRACKS:
		sort(TYPE_TRK);
		break;
	case ID_TOOLS_SORT_ALL:
		sort(TYPE_WPT);
		sort(TYPE_RTE);
		sort(TYPE_TRK);
		break;
	case ID_TOOLS_CLEAR:
		lbclear();
		break;
	case ID_TOOLS_RESET:
		resetCurrentWindow();
		break;
	case ID_TOOLS_MOB:
		if(!position.isValid()) {
			errMsg(_T("Current position is not valid"));
		} else {
			doEditWaypoint(POS_WPT);
		}
		break;
	case ID_TOOLS_OPTIONS_COLORS:
		DialogBox(hInst,(LPCTSTR)IDD_COLORS,hWnd,
			(DLGPROC)colorsDlgProc);
		break;
	case ID_TOOLS_OPTIONS_COMPORT:
		DialogBox(hInst,(LPCTSTR)IDD_COMPORT,hWnd,
			(DLGPROC)comPortDlgProc);
		break;
	case ID_TOOLS_ABORTTRANSFER:
		abortTransfer();
		break;
	case ID_TOOLS_GETID:
		getID();
		break;
	case ID_TOOLS_DOWNLOAD_WAYPOINTS:
		getWaypoints();
		break;
	case ID_TOOLS_DOWNLOAD_ROUTES:
		getRoutes();
		break;
	case ID_TOOLS_DOWNLOAD_TRACKS:
		getTracks();
		break;
	case ID_TOOLS_DOWNLOAD_ALL:
		getWaypoints();
		getRoutes();
		getTracks();
		break;
	case ID_TOOLS_UPLOAD_WAYPOINTS:
		sendWaypoints();
		break;
	case ID_TOOLS_UPLOAD_ROUTES:
		sendRoutes();
		break;
	case ID_TOOLS_UPLOAD_TRACKS:
		sendTracks();
		break;
	case ID_TOOLS_UPLOAD_ALL:
		sendWaypoints();
		sendRoutes();
		sendTracks();
		break;
	case ID_TOOLS_PVTDATA_START:
		startPVTData();
		break;
	case ID_TOOLS_PVTDATA_STOP:
		stopPVTData();
		break;
	case ID_TOOLS_CONNECT:
		connect();
		break;
	case ID_TOOLS_DISCONNECT:
		disconnect();
		break;
	case ID_VIEW_WAYPOINTS:
		if(!hWptLV) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			wptCreateLV(hWndMain);
			if(!hWptLV) {
				errMsg(_T("Could not create waypoint ListView"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hWptLV);
#if DEBUG_LIST
		listWaypoints();
#endif
		break;
	case ID_VIEW_ROUTESUMMARY:
		showRouteWaypoints=FALSE;
		if(!hRteLV || showRouteWaypointsOld) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			rteCreateLV(hWndMain);
			if(!hRteLV) {
				errMsg(_T("Could not create route ListView"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hRteLV);
#if DEBUG_LIST
		listRoutes();
#endif
		break;
	case ID_VIEW_ROUTES:
		showRouteWaypoints=TRUE;
		if(!hRteLV || !showRouteWaypointsOld) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			rteCreateLV(hWndMain);
			if(!hRteLV) {
				errMsg(_T("Could not create route ListView"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hRteLV);
#if DEBUG_LIST
		listRoutes();
#endif
		break;
	case ID_VIEW_TRACKSUMMARY:
		showTrackpoints=FALSE;
		if(!hTrkLV || showTrackpointsOld) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			trkCreateLV(hWndMain);
			if(!hTrkLV) {
				errMsg(_T("Could not create track ListView"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hTrkLV);
#if DEBUG_LIST
		listTracks();
#endif
		break;
	case ID_VIEW_TRACKS:
		showTrackpoints=TRUE;
		if(!hTrkLV || !showTrackpointsOld) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			trkCreateLV(hWndMain);
			if(!hTrkLV) {
				errMsg(_T("Could not create track ListView"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hTrkLV);
#if DEBUG_LIST
		listTracks();
#endif
		break;
	case ID_VIEW_MAP:
		if(!hMap) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			createMapWindow(hWndMain);
			if(!hMap) {
				errMsg(_T("Could not create map window"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hMap);
		break;
	case ID_VIEW_CONSOLE:
		viewWindow(hLB);
		break;
	case ID_LVPOPUP_GOTO:
		if(!hMap) {
			// Change the cursor
			specialCursor=LoadCursor(NULL,IDC_WAIT);
			HCURSOR hOrigCursor=SetCursor(specialCursor);

			// Create it
			createMapWindow(hWndMain);
			if(!hMap) {
				errMsg(_T("Could not create map window"));
				break;
			}

			// Change the cursor back
			SetCursor(hOrigCursor);
			specialCursor=NULL;
		}
		viewWindow(hMap);
		if(pMenuBlock) {
			positionToLatLon(pMenuBlock->getLatitude(),
				pMenuBlock->getLongitude());
			pMenuBlock=NULL;
		}
		break;
	case ID_LVPOPUP_CUT:
		removeSelectedBlocks(EDIT_CUT);
		break;
	case ID_LVPOPUP_COPY:
		removeSelectedBlocks(EDIT_COPY);
		break;
	case ID_LVPOPUP_PASTE_BEFORE:
		pasteBlocks(ID_EDIT_PASTE_BEFORE);
		break;
	case ID_LVPOPUP_PASTE_ON:
		pasteBlocks(ID_EDIT_PASTE_ON);
		break;
	case ID_LVPOPUP_PASTE_AFTER:
		pasteBlocks(ID_EDIT_PASTE_AFTER);
		break;
	case ID_LVPOPUP_DELETE:
		removeSelectedBlocks(EDIT_DELETE);
		break;
	case ID_LVPOPUP_EDIT:
		if(!pMenuBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			break;
		}
		switch(pMenuBlock->getType()) {
	case TYPE_WPT:
		doEditWaypoint(LV_WPT);
		break;
	case TYPE_TPT:
		doEditTrackpoint(LV_TPT);
		break;
	default:
		infoMsg(_T("You can edit the name by selecting the item, ")
			_T("then clicking once"));
		}
		break;
	case ID_LVPOPUP_CONVERTS:
		if(!pMenuBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			break;
		}
		switch(pMenuBlock->getType()) {
	case TYPE_TPT:
		trkConvertS();
		break;
	default:
		infoMsg(_T("Not a trackpoint"));
		}
		break;
	case ID_LVPOPUP_CONVERTALLS:
		if(!pMenuBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			break;
		}
		switch(pMenuBlock->getType()) {
	case TYPE_TRK:
	case TYPE_TPT:
		trkConvertAllS(pMenuBlock->getType());
		break;
	default:
		infoMsg(_T("Not a track or trackpoint"));
		}
		break;
	case ID_LVPOPUP_PREVS:
		if(!pMenuBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			break;
		}
		switch(pMenuBlock->getType()) {
	case TYPE_TPT:
		trkNextS(BACK);
		break;
	default:
		infoMsg(_T("Not a trackpoint"));
		}
		break;
	case ID_LVPOPUP_NEXTS:
		if(!pMenuBlock) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			break;
		}
		switch(pMenuBlock->getType()) {
	case TYPE_TPT:
		trkNextS(FORWARD);
		break;
	default:
		infoMsg(_T("Not a trackpoint"));
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

#ifdef UNDER_CE
	case WM_CREATE:
		//Create 'Rapier' type MenuBar
		SHMENUBARINFO mbi;

		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize     = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hWnd;
		mbi.nToolBarId = IDM_MAIN_MENUBAR;
		mbi.hInstRes   = hInst;
		mbi.nBmpId     = 0;
		mbi.cBmpImages = 0;	

		if(!SHCreateMenuBar(&mbi))
			MessageBox(hWnd, L"SHCreateMenuBar Failed", L"Error", MB_OK);
		hWndCB = mbi.hwndMB;
		break;
#endif

	case WM_SIZE:
		resizeChildWindows(wParam,lParam);
		break;

	case WM_NOTIFY:
		return(notifyHandler(hWnd,message,wParam,lParam));

#if 0
		// KE: This is apparently not necessary and don't really need
		// the specialCursor
	case WM_SETCURSOR:
		if(specialCursor == NULL || LOWORD(lParam) != HTCLIENT) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		} 
		SetCursor(specialCursor);
		return TRUE;
#endif

	case WM_DESTROY:
		if(hWndCB) DestroyWindow(hWndCB);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Mesage handler for the About box
LRESULT CALLBACK aboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	TCHAR szText[80];
	HWND hCtrl=NULL;
	TCHAR fileVersion[80];
	BOOL status;

	switch(msg) {
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
		// Fill in the version
		hCtrl=GetDlgItem(hDlg,IDC_VERSION);
		status=getModuleFileVersion(fileVersion,80);
		if(!status) {
			Static_SetText(hCtrl,_T("Version 1.0"));
		} else {
			_stprintf(szText,_T("Version: %s"),fileVersion);
			Static_SetText(hCtrl,szText);
		}

		// Fill in the build date
		hCtrl=GetDlgItem(hDlg,IDC_BUILDDATE);
		_tprintf(szText,"Built %s %s",_T(__DATE__),_T(__TIME__));
		Static_SetText(hCtrl,szText);

		return TRUE;

	case WM_COMMAND:
		if((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL)) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

// Mesage handler for file options dialog
LRESULT CALLBACK fileOptionsDlgProc(HWND hDlg, UINT message,
									WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	int wmId;
	HWND hCtrl=NULL;

	switch (message) {
	case WM_INITDIALOG:
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);
#endif
		// Set defaults
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_WPT);
		Button_SetCheck(hCtrl,saveWpt?BST_CHECKED:BST_UNCHECKED);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_RTE);
		Button_SetCheck(hCtrl,saveRte?BST_CHECKED:BST_UNCHECKED);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_TRK);
		Button_SetCheck(hCtrl,saveTrk?BST_CHECKED:BST_UNCHECKED);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_CHECKEDONLY);
		Button_SetCheck(hCtrl,saveCheckedOnly?BST_CHECKED:BST_UNCHECKED);

		hCtrl=GetDlgItem(hDlg,IDC_SAVE_MAP);
		Button_SetCheck(hCtrl,saveMap?BST_CHECKED:BST_UNCHECKED);
		EnableWindow(hCtrl,pMap != &pMapDefault);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_MAPSHORTNAME);
		Button_SetCheck(hCtrl,saveMapShortName?BST_CHECKED:BST_UNCHECKED);
		EnableWindow(hCtrl,pMap != &pMapDefault);
		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		switch(wmId) {       
	case IDOK:
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_WPT);
		saveWpt=Button_GetCheck(hCtrl);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_RTE);
		saveRte=Button_GetCheck(hCtrl);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_TRK);
		saveTrk=Button_GetCheck(hCtrl);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_MAP);
		saveMap=Button_GetCheck(hCtrl);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_MAPSHORTNAME);
		saveMapShortName=Button_GetCheck(hCtrl);
		hCtrl=GetDlgItem(hDlg,IDC_SAVE_CHECKEDONLY);
		saveCheckedOnly=Button_GetCheck(hCtrl);

		EndDialog(hDlg,LOWORD(wParam));
		doFileSave=TRUE;
		return TRUE;
	case IDCANCEL:
		EndDialog(hDlg,LOWORD(wParam));
		doFileSave=FALSE;
		return TRUE;
		}
	}
	return FALSE;
}

// Mesage handler for colors dialog
LRESULT CALLBACK colorsDlgProc(HWND hDlg, UINT message,
							   WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	int wmId;
	HWND hwListBox=NULL;
	int sel,i;

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

		hwListBox=GetDlgItem(hDlg,IDC_WPTCOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,wptColorIndex);

		hwListBox=GetDlgItem(hDlg,IDC_RTECOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,rteColorIndex);

		hwListBox=GetDlgItem(hDlg,IDC_TRKCOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,trkColorIndex);

		hwListBox=GetDlgItem(hDlg,IDC_POSCOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,posColorIndex);

		hwListBox=GetDlgItem(hDlg,IDC_POSCOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,posColorIndex);

		hwListBox=GetDlgItem(hDlg,IDC_CALCOLORLIST);
		ListBox_ResetContent(hwListBox);
		for(i=0; i < NCOLORS; i++) {
			ListBox_AddString(hwListBox,colorNames[i]);
		}
		ListBox_SetCurSel(hwListBox,calColorIndex);

		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		switch(wmId) {       
	case IDCANCEL:
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
	case IDOK:
		hwListBox=GetDlgItem(hDlg,IDC_WPTCOLORLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		wptColorIndex=sel;
		mapWptColor=colors[wptColorIndex];

		hwListBox=GetDlgItem(hDlg,IDC_RTECOLORLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		rteColorIndex=sel;
		mapRteColor=colors[rteColorIndex];

		hwListBox=GetDlgItem(hDlg,IDC_TRKCOLORLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		trkColorIndex=sel;
		mapTrkColor=colors[trkColorIndex];

		hwListBox=GetDlgItem(hDlg,IDC_CALCOLORLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		calColorIndex=sel;
		mapCalColor=colors[calColorIndex];

		hwListBox=GetDlgItem(hDlg,IDC_POSCOLORLIST);
		sel=ListBox_GetCurSel(hwListBox);  
		if(sel == LB_ERR) break;
		posColorIndex=sel;
		mapPosColor=colors[posColorIndex];

		if(hMap) {
			InvalidateRect(hMap,NULL,TRUE);
			UpdateWindow(hMap); 
		}

		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
		}
	}
	return FALSE;
}

// Mesage handler for COM Port dialog
LRESULT CALLBACK comPortDlgProc(HWND hDlg, UINT message,
								WPARAM wParam, LPARAM lParam)
{
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
	int wmId;
	HWND hwCtrl=NULL;
	TCHAR szText[MAX_COM_SIZE];

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

		hwCtrl=GetDlgItem(hDlg,IDC_COM1);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM2);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM3);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM4);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_OTHER);
		Button_SetCheck(hwCtrl,FALSE);
		if(!_tcscmp(lptszComPort,_T("COM1:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM1);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(lptszComPort,_T("COM2:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM2);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(lptszComPort,_T("COM3:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM3);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(lptszComPort,_T("COM4:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM4);
			Button_SetCheck(hwCtrl,TRUE);
		} else {
			hwCtrl=GetDlgItem(hDlg,IDC_OTHER);
			Button_SetCheck(hwCtrl,TRUE);
		}
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_LimitText(hwCtrl,MAX_COM_SIZE);
		Edit_SetText(hwCtrl,lptszComPort);

		return TRUE;

	case WM_COMMAND:
		wmId=LOWORD(wParam); 
		switch(wmId) {       
	case IDC_COM1:
		hwCtrl=GetDlgItem(hDlg,IDC_COM1);
		Button_SetCheck(hwCtrl,TRUE);
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_SetText(hwCtrl,_T("COM1:"));
		return TRUE;
	case IDC_COM2:
		hwCtrl=GetDlgItem(hDlg,IDC_COM2);
		Button_SetCheck(hwCtrl,TRUE);
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_SetText(hwCtrl,_T("COM2:"));
		return TRUE;
	case IDC_COM3:
		hwCtrl=GetDlgItem(hDlg,IDC_COM3);
		Button_SetCheck(hwCtrl,TRUE);
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_SetText(hwCtrl,_T("COM3:"));
		return TRUE;
	case IDC_COM4:
		hwCtrl=GetDlgItem(hDlg,IDC_COM4);
		Button_SetCheck(hwCtrl,TRUE);
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_SetText(hwCtrl,_T("COM4:"));
		return TRUE;
	case IDC_OTHER:
		hwCtrl=GetDlgItem(hDlg,IDC_OTHER);
		Button_SetCheck(hwCtrl,TRUE);
		return TRUE;
	case IDC_EDIT:
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_GetText(hwCtrl,szText,MAX_COM_SIZE);
		szText[MAX_COM_SIZE-1]=_T('\0');
		hwCtrl=GetDlgItem(hDlg,IDC_COM1);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM2);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM3);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_COM4);
		Button_SetCheck(hwCtrl,FALSE);
		hwCtrl=GetDlgItem(hDlg,IDC_OTHER);
		Button_SetCheck(hwCtrl,FALSE);
		if(!_tcscmp(szText,_T("COM1:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM1);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(szText,_T("COM2:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM2);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(szText,_T("COM3:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM3);
			Button_SetCheck(hwCtrl,TRUE);
		} else if(!_tcscmp(szText,_T("COM4:"))) {
			hwCtrl=GetDlgItem(hDlg,IDC_COM4);
			Button_SetCheck(hwCtrl,TRUE);
		} else {
			hwCtrl=GetDlgItem(hDlg,IDC_OTHER);
			Button_SetCheck(hwCtrl,TRUE);
		}
		return TRUE;
	case IDCANCEL:
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
	case IDOK:
		hwCtrl=GetDlgItem(hDlg,IDC_EDIT);
		Edit_GetText(hwCtrl,szText,MAX_COM_SIZE);
		_tcsncpy(lptszComPort,szText,MAX_COM_SIZE);
		lptszComPort[MAX_COM_SIZE-1]=_T('\0');
		lbprintf(_T("COM Port=%s"),lptszComPort);
		if(hPort != INVALID_HANDLE_VALUE) disconnect();
		EndDialog(hDlg,LOWORD(wParam));
		return TRUE;
		}
	}
	return FALSE;
}

static void connect(void)
{
	BOOL status;

	if(hPort != INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is already open"));
		return;
	}
	status=PortInitialize(lptszComPort);
	stopPVTData();
	if(!status) {
		errMsg(_T("Error initializing %s"),lptszComPort);
		PortClose();
	}
}

static void disconnect(void)
{
	BOOL status;

	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	// Stop sending any PVT data
	stopPVTData();
	status=PortClose();
	if(!status) {
		errMsg(_T("Error closing port"));
	}
}

static void getID(void)
{
	BOOL status;

	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	status=GetID();
	if(!status) {
		errMsg(_T("Error closing port"));
	}
}

void startPVTData(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Start_Pvt_Data);
}

void stopPVTData(void)
{
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Stop_Pvt_Data);
}

static void getWaypoints(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Transfer_Wpt);
}

static void abortTransfer(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	abortCurrentTransfer();
}

static void getRoutes(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Transfer_Rte);
}

static void getTracks(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Transfer_Trk);
}

static void sendRoutes(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Upload_Rte);
}

static void sendTracks(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Upload_Trk);
}

static void sendWaypoints(void)
{
	if(hPort == INVALID_HANDLE_VALUE) connect();
	if(hPort == INVALID_HANDLE_VALUE) {
		PlaySound(_T("Critical"),NULL,SND_SYNC);
		errMsg(_T("Port is not open"));
		return;
	}
	queueCommand(Cmnd_Upload_Wpt);
}

static void saveFile(void)
{
	char delimiter='\t';
	BOOL status;
	OutputFilterIndex index;

	// Pop up the file save options dialog
	doFileSave=FALSE;
	DialogBox(hInst,(LPCTSTR)IDD_FILEOPTS,hWndMain,
		(DLGPROC)fileOptionsDlgProc);
	if(!doFileSave) return;

	// Pop up the file save dialog
	status=fileSaveDlg(hWndMain,szFileName,NULL);
	if(!status) return;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Switch depending on the file type
	index=(OutputFilterIndex)getFilterIndex();
	switch(index) {
	case GPX_OutputFile:
		writeGPXFile(szFileName);
		break;
	case GPSU_OutputFile:
		writeGPSUFile(szFileName);
		break;
	case SA8_OutputFile:
		writeSA8File(szFileName);
		break;
	case CSV_OutputFile:
	case GPSL_OutputFile:
	default:
		// Set delimiter
		if(index == CSV_OutputFile) delimiter=',';
		else delimiter='\t';
		writeGPSLFile(szFileName,delimiter);
	}

	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;
}

static void openFile(void)
{
	BOOL status;
	InputFilterIndex index;

	// Popup the file open dialog
	status=fileOpenDlg(hWndMain,szFileName,NULL);
	if(!status) return;

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Switch depending on the file type
	index=(InputFilterIndex)getFilterIndex();
	switch(index) {
	case JPEG_InputFile:
	case BMP_InputFile:
	case GIF_InputFile:
	case PNG_InputFile:
	case XBM_InputFile:
		readImageFile(szFileName);
		break;
	case GPSL_InputFile:
	case CSV_InputFile:
	default:
		readGPSLFile(szFileName);
		break;
	}

	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;
}

void viewWindow(HWND hWnd)
{
	// Update as other child windows to hWinMain are added to view menu
	HWND childWnd[]={
		hLB,
		hWptLV,
		hRteLV,
		hTrkLV,
		hMap,
	};
	int i,n=sizeof(childWnd)/sizeof(HWND);

	for(i=0; i < n; i++) {
		if(!childWnd[i]) continue;
		if(childWnd[i] == hWnd) {
			ShowWindow(childWnd[i],SW_SHOW);
			UpdateWindow(childWnd[i]);
		} else {
			ShowWindow(childWnd[i],SW_HIDE);
		}
	}
	hCurWnd=hWnd;
}

static void resetCurrentWindow(void)
{
	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Refresh the current view
	if(hCurWnd == hWptLV) wptCreateLV(hWndMain);
	else if(hCurWnd == hRteLV) rteCreateLV(hWndMain);
	else if(hCurWnd == hTrkLV) trkCreateLV(hWndMain);
	else if(hCurWnd == hMap) createMapWindow(hWndMain);
	else if(hCurWnd == hLB) lbclear();


	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;
}

static void resizeChildWindows(WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	GetClientRect(hWndMain,&rc);

	// Resize the current view
	if(hWptLV) {
		MoveWindow(hWptLV,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);
		//	SendMessage(hWptLV,WM_SIZE,wParam,lParam);
	} 
	if(hRteLV) {
		MoveWindow(hRteLV,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);
		//	SendMessage(hRteLV,WM_SIZE,wParam,lParam);
	} 
	if(hTrkLV) {
		MoveWindow(hTrkLV,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);
		//	SendMessage(hTrkLV,WM_SIZE,wParam,lParam);
	} 
	if(hMap) {
		MoveWindow(hMap,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);
		//	SendMessage(hMap,WM_SIZE,wParam,lParam);
	} 
	if(hLB) {
		MoveWindow(hLB,0,0,rc.right-rc.left,rc.bottom-rc.top,TRUE);
		//	SendMessage(hLB,WM_SIZE,wParam,lParam);
		lbSetEndOfPage();
	} 
}

static void sort(DataType type)
{
	CBlockList *pList;

	// Determine the list
	switch(type) {
	case TYPE_WPT:
		pList=&waypointList;
		break;
	case TYPE_RTE:
		pList=&routeList;
		break;
	case TYPE_TRK:
		pList=&trackList;
		break;
	}
	unsigned count=pList->getCount();
	if(count <= 0) return;

	// Allocate arrays
	CBlock **blocks=new CBlock *[count];
	char **names=new char *[count];
	int *indx=new int[count];
	if(!blocks || !names || !indx) {
		errMsg(_T("Cannot allocate memory for sort"));	
		if(blocks) delete [] blocks;
		if(names) delete [] names;
		if(indx) delete [] indx;
		return;
	}

	// Change the cursor
	specialCursor=LoadCursor(NULL,IDC_WAIT);
	HCURSOR hOrigCursor=SetCursor(specialCursor);

	// Assign the arrays
	int index=0;
	tsDLIterBD<CBlock> iter(pList->first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		blocks[index]=pBlock;
		names[index]=pBlock->getIdent();
		index++;
		iter++;
	}

	// Sort
	hsort(names,indx,count,FALSE);

	// Empty the list without deleting the blocks and refill it
	pList->empty(FALSE);
	for(unsigned i=0; i < count; i++) {
		pList->addBlock(blocks[indx[i]]);
	}

	// Free the arrays
	if(blocks) delete [] blocks;
	if(names) delete [] names;
	if(indx) delete [] indx;

	// Restore the listviews
	switch(type) {
	case TYPE_WPT:
		if(hWptLV) wptCreateLV(hWndMain);
		break;
	case TYPE_RTE:
		if(hRteLV) rteCreateLV(hWndMain);
		break;
	case TYPE_TRK:
		if(hTrkLV) trkCreateLV(hWndMain);
		break;
	}

	// Change the cursor back
	SetCursor(hOrigCursor);
	specialCursor=NULL;
}
