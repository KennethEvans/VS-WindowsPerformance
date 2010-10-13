/*------------------------------------------
Based on Petzold:
POPFILE.C -- Popup Editor File Functions
------------------------------------------*/

#include "stdafx.h"
#include <Commdlg.h>
#include "file.h"

// Global variables

static OPENFILENAME ofn;
static BOOL initialized=FALSE;
// These need to be coordinated with InputFilterIndex
TCHAR szOpenFilter[] =
_T("GPX Files (*.gpx)\0*.gpx\0") \
_T("GPSLink Files (*.gpsl)\0*.gpsl\0")  \
_T("CSV Files (*.csv)\0*.csv\0") \
_T("JPEG (*.jpg)\0*.jpg\0") \
_T("BMP (*.bmp)\0*.bmp\0") \
_T("GIF (*.gif)\0*.gif\0") \
_T("PNG (*.png)\0*.png\0") \
_T("XBM (*.xbm)\0*.xbm\0") \
_T("All Files (*.*)\0*.*\0") \
_T("\0");
// These need to be coordinated with OututFilterIndex
TCHAR szSaveFilter[] =
_T("GPX Files (*.gpx)\0*.gpx\0") \
_T("GPSLink Files (*.gpsl)\0*.gpsl\0")  \
_T("CSV Files (*.csv)\0*.csv\0") \
_T("Street Atlas 8 Files (*.sa8)\0*.sa8\0") \
_T("GPSU Files (*.gpsu)\0*.gpsu\0") \
_T("All Files (*.*)\0*.*\0") \
_T("\0");

void fileInitialize(HWND hwnd)
{
	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szOpenFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;          // Set in Open and Close functions
	ofn.nMaxFile          = MAX_PATH;
	ofn.lpstrFileTitle    = NULL;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = MAX_PATH;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;             // Set in Open and Close functions
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = _T("txt");
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;
}

BOOL fileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	// Initialize ofn if not done yet
	if(!initialized) {
		fileInitialize(hwnd);
		initialized = TRUE;
	}

	ofn.lpstrFilter       = szOpenFilter;
	ofn.hwndOwner         = hwnd;
	ofn.lpstrFile         = pstrFileName;
	ofn.lpstrFileTitle    = pstrTitleName;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	return GetOpenFileName(&ofn);
}

BOOL fileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	// Initialize ofn if not done yet
	if(!initialized) {
		fileInitialize(hwnd);
		initialized = TRUE;
	}

	ofn.lpstrFilter       = szSaveFilter;
	ofn.hwndOwner         = hwnd;
	ofn.lpstrFile         = pstrFileName;
	ofn.lpstrFileTitle    = pstrTitleName;
	ofn.Flags             = OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}

DWORD getFilterIndex(void)
{
	return ofn.nFilterIndex;
}

void setFilterIndex(int index)
{
	ofn.nFilterIndex=index;
}
