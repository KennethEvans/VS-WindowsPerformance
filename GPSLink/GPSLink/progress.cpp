// Progress dialog

#include "stdafx.h"
#include "resource.h"
#include "console.h"
#include "GPSLink.h"

#define DEBUG_PROGRESS 0

#define PROGRESS_STRINGSIZE 80

// Function prototypes
static LRESULT CALLBACK Progress(HWND hDlg, UINT msg, WPARAM wParam,
								 LPARAM lParam);

// Global variables
static HWND hProgressDlg=NULL;
static HWND hProgress=NULL;
static HWND hMsg=NULL;
BOOL progressAbort=FALSE;

void progressCreateDialog(HINSTANCE hInst, HWND hWndParent)
{
	progressAbort=FALSE;
	if(!hProgressDlg) {
		hProgressDlg=CreateDialog(hInst,(LPCTSTR)IDD_PROGRESS,hWndParent,
			(DLGPROC)Progress);
		if(!hProgressDlg) return;
	}
#if DEBUG_PROGRESS
	lbprintf(_T("progressCreateDialog:\n")
		_T(" thread=%x hProgressDlg=%x"),
		GetCurrentThreadId(),hProgressDlg);
#endif
	ShowWindow(hProgressDlg,SW_SHOWNORMAL);
	UpdateWindow(hProgressDlg);
}

void progressSetProgress(TCHAR *header, int rcvd, int max)
{
	static TCHAR szProgressString[PROGRESS_STRINGSIZE];
	WORD percent=(WORD)(max?rcvd*100/max:0);

	_stprintf(szProgressString,_T("%s: %d of %d packets"),
		header,rcvd,max);

	if(!hProgressDlg || !hProgress) return;
	Static_SetText(hMsg,szProgressString);
	SendMessage(hProgress,PBM_SETPOS,(WPARAM)percent,0);
	UpdateWindow(hProgressDlg);
}

void progressDestroyDialog(void)
{
	if(!hProgressDlg) return;
#if 0
	// Destroy Window doesn't work if called from another thread
	BOOL status=DestroyWindow(hProgressDlg);
	if(status) {
		DWORD dwError=GetLastError();
		lbprintf(_T("Error destroying progress dialog [%d]"),dwError);
	}
#else
	// Use SendMessage because DestroyWindow does not work if it was
	// created in a different thread
	SendMessage(hProgressDlg,WM_CLOSE,0,0);
#endif

#if DEBUG_PROGRESS
	lbprintf(_T("progressDestroyDialog:\n")
		_T(" thread=%x hProgressDlg=%x"),
		GetCurrentThreadId(),hProgressDlg);
#endif

	hProgressDlg=hProgress=hMsg=NULL;
}

// Mesage handler for the Progress dialog
static LRESULT CALLBACK Progress(HWND hDlg, UINT msg, WPARAM wParam,
								 LPARAM lParam)
{
#if 0
#ifdef UNDER_CE
	SHINITDLGINFO shidi;
#endif
#endif

	switch(msg) {
	case WM_INITDIALOG:
#if 0
#ifdef UNDER_CE
		//On Rapier devices you normally create all Dialog's as
		//fullscreen dialog's with an OK button in the upper corner.
		//You should get/set any program settings during each modal
		//dialog creation and destruction.  Create a Done button and
		//size it.
		shidi.dwMask=SHIDIM_FLAGS;
#if 1
		shidi.dwFlags=SHIDIF_DONEBUTTON|SHIDIF_SIPDOWN|SHIDIF_SIZEDLGFULLSCREEN;
#endif
		shidi.hDlg=hDlg;
		//Initialzes the dialog based on the dwFlags parameter
		SHInitDialog(&shidi);
		return TRUE;
#endif
#endif

		// Store the dialog handle
		hProgressDlg=hDlg;

		// Initialize the progress control
		hProgress=GetDlgItem(hDlg,IDC_PROGRESS);
		SendMessage(hProgress,PBM_SETRANGE,0,MAKELPARAM(0,100)); 
		SendMessage(hProgress,PBM_SETPOS,(WPARAM)0,0); 

		// Initialize the message control
		hMsg=GetDlgItem(hDlg,IDC_PROGRESS_MSG);
		Static_SetText(hMsg,_T("Ready"));
		break;

	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL) {
			abortCurrentTransfer();
			return TRUE;
		}
		break;

	case WM_CLOSE:
		progressAbort=TRUE;
		DestroyWindow(hDlg);
		hProgressDlg=hProgress=hMsg=NULL;
		break;
	}

	return FALSE;
}
