// Console routines

#include "stdafx.h"

#define DEBUG_EOP 0

#define USE_HSCROLL 1
#define USE_TAHOMA 0

#include "console.h"
#include "resource.h"
#include "utils.h"

#define LB_MAXLINES 8192
#define LB_MAXCHARS 80
// LB_AT_END is the last full visible line, starting at 1.  Default is
// below.  Is recalculated by lbSetEndOfPage.  May need to be reset if
// the window changes.
#define LB_AT_END 19

// For dumpBytes
#define BYTES_PER_LINE 8

// Static global variables
static int endOfPage=LB_AT_END;
static int maxLines=LB_MAXLINES;
static int maxChars=LB_MAXCHARS;
static BOOL useHScroll=TRUE;

// Global variables
HWND hLB=NULL;                // ListBox handle
BOOL redraw=TRUE;

void lbSetRedraw(BOOL val)
{
	redraw=val;
	SendMessage(hLB,WM_SETREDRAW,val,0);
	if(redraw) {
		UpdateWindow(hLB);
	}
}

void createConsole(HINSTANCE hInst, HWND hWndParent)
{
	RECT rect;

	// Set the font
#ifndef UNDER_CE
#if USE_TAHOMA
	// Change font to Tahoma
	LOGFONT lf;
	HFONT hFont;

	// Initialize lf (Defaults correspond to 0's)
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 14;
	_tcscpy(lf.lfFaceName,_T("Tahoma"));
	hFont = CreateFontIndirect(&lf);
#endif
#endif

	// Get the size and position of the parent window.
	GetClientRect(hWndParent,&rect);

#if 0
	int i1=WS_VISIBLE;
	int i2=WS_CHILD;
	int i3=WS_BORDER;
	int i4=WS_EX_CLIENTEDGE;
	int i5=LBS_NOINTEGRALHEIGHT;
	int i6=LBS_NOSEL;
	int i7=WS_VSCROLL;
	int i8=LBS_MULTICOLUMN;
	int i9=WS_VISIBLE|WS_CHILD|WS_BORDER|WS_EX_CLIENTEDGE|	// Styles
		LBS_NOINTEGRALHEIGHT|LBS_NOSEL|WS_VSCROLL;
#endif

	// Create the list box window
	// Note that WS_EX_CLIENTEDGE is the same as LBS_MULTICOLUMN
	// (0x00000200).  Don't use it.
	hLB=CreateWindowEx(0L,
		_T("LISTBOX"),              // Listbox class
		TEXT(""),                   // No default text
		WS_VISIBLE|WS_CHILD|WS_BORDER|  // Styles
#if USE_HSCROLL
		WS_HSCROLL|
#endif
		LBS_NOINTEGRALHEIGHT|LBS_NOSEL|WS_VSCROLL,
		rect.top,rect.left,
		rect.right-rect.left,rect.bottom-rect.top,
		hWndParent,
		(HMENU)ID_CONSOLE,
		hInst,
		NULL);

	if(hLB) {
#ifndef UNDER_CE
#if USE_TAHOMA
		// Set the font and cause redraw
		SendMessage(hLB,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(TRUE,0));
#endif
#endif

#if USE_HSCROLL
		if(useHScroll) {
			// Set the scroll extent assuming 5 pixels per char (avg spacing
			// for 14-pixel Tahoma)
			ListBox_SetHorizontalExtent(hLB,5*maxChars);
		} else {
			ListBox_SetHorizontalExtent(hLB,0);
		}
#endif

		// Set the end of page
		lbSetEndOfPage();
	}
}

int lbprintf(const TCHAR *format, ... )
{
	va_list vargs;
	static TCHAR lstring[1024];     // Danger: Fixed size
	TCHAR *ptr,*pos;
	int end,end1,top;
	int endVisible=0;

	if(!hLB) return 1;

	va_start(vargs,format);
	(void)_vstprintf(lstring,format,vargs);
	va_end(vargs);

	// Turn off redraw for long strings
	if(redraw) {
		SendMessage(hLB,WM_SETREDRAW,FALSE,0);
	}

	// Determine if the end is visible
	end=ListBox_GetCount(hLB)-1;
	top=ListBox_GetTopIndex(hLB);
	if(end < endOfPage) endVisible=1;
	else if(end-top < endOfPage) endVisible=1;
	else endVisible=0;

	// Search for \n
	ptr=lstring;
	while(1) {
		pos=_tcschr(ptr,'\n');
		if(pos) {
			*pos='\0';
			ListBox_AddString(hLB,ptr);
			ptr=pos+1;
		} else {
			ListBox_AddString(hLB,ptr);
			break;
		}
	}

	// Limit lines in list box
	while(ListBox_GetCount(hLB) > maxLines) {
		ListBox_DeleteString(hLB,0);
	}

	// Reset to make the last line visible if it was before
	end1=-1;
	if(endVisible) {
		end1=ListBox_GetCount(hLB)-1;
		ListBox_SetCaretIndex(hLB,end1);
	}
#if DEBUG_EOP && 0
	errMsg(_T("top=%d end=%d end1=%d EOP=%d endVisible=%d"),
		top,end,end1,endOfPage,endVisible);
#endif

	// Turn on redraw
	if(redraw) {
		SendMessage(hLB,WM_SETREDRAW,TRUE,0);
	}

	// WCE hangs on long output if you don't put this in
	UpdateWindow(hLB);

	return 0;
}

void lbclear(void)
{
	if(hLB) {
		ListBox_ResetContent(hLB);
	}
}

void lbSetMaxLines(int maxLinesIn)
{
	maxLines=maxLinesIn;
}

int lbGetMaxLines(void)
{
	return maxLines;
}

void lbSetMaxChars(int maxCharsIn)
{
	maxChars=maxCharsIn;
}

int lbGetMaxChars(void)
{
	return maxChars;
}

#if USE_HSCROLL
void lbUseHScroll(BOOL useHscrollIn)
{
	useHScroll=useHscrollIn;
}
#endif

void lbSetEndOfPage(void)
{
	if(!hLB) return;

	// Get the client area size
	RECT rect;
	GetClientRect(hLB,&rect);
	int cHeight=rect.bottom-rect.top;

#if USE_HSCROLL
	// Get the horizontal scrollbar height
	if(useHScroll) {
		int sHeight=GetSystemMetrics(SM_CYHSCROLL);
		cHeight-=sHeight;
	}
#endif

	// Get the item height
	int iHeight=ListBox_GetItemHeight(hLB,0);
	if(!iHeight) {
		errMsg(_T("Listbox item height is zero"));
		iHeight=1;
	}

	// Calculate the number of full lines available
	endOfPage=cHeight/iHeight;

	// Extend the cHeight to an integral number of iHeights
	if(endOfPage*iHeight < cHeight) endOfPage++;
	if(endOfPage < 1) endOfPage=1;

#if DEBUG_EOP
	errMsg(_T("iHeight=%d cHeight=%d endOfPage=%d"),
		iHeight,cHeight,endOfPage);
#endif
}

void dumpLBToFile(TCHAR *szFileName)
{
	char *ansiFileName=NULL;
	char *ansiLine=NULL;
	DWORD nLines;
	DWORD result;
	FILE *fp;
	DWORD i;

	lbprintf(_T("\nWriting file:"));
	lbprintf(szFileName);

	// Convert to ANSI
	result=unicodeToAnsi(szFileName,&ansiFileName);
	if(!ansiFileName) {
		lbprintf(_T("Cannot convert unicode filename string"));
		goto CLEANUP;
	}

	// Open the file
	fp=fopen(ansiFileName,"w");
	if(!fp) {
		lbprintf(_T("Cannot write file:\n  %s"),szFileName);
		goto CLEANUP;
	}

	// Loop over the lines in the listbox
	nLines=ListBox_GetCount(hLB);
	TCHAR szLine[100];
	for(i=0; i < nLines; i++) {
		ansiLine=NULL;
		ListBox_GetText(hLB,i,szLine);
		// Convert to ANSI
		result=unicodeToAnsi(szLine,&ansiLine);
		if(!ansiLine) {
			fclose(fp);
			fp=NULL; 
			lbprintf(_T("Cannot convert line"));
			goto CLEANUP;
		} else {
			fprintf(fp,"%s\n",ansiLine);
			free(ansiLine);
		}
	}

	lbprintf(_T("File written"));

CLEANUP:
	fclose(fp);
	fp=NULL;
	if(ansiFileName) free(ansiFileName);
}

void dumpBytes(BYTE *buf, int nBuf)
{
	int i,j,j1,k,k1,index;
	TCHAR szNumbers[BYTES_PER_LINE*4+1];  // *3 should be right, 4 is safety
	TCHAR szText[2*BYTES_PER_LINE+1];
	TCHAR szLine[BYTES_PER_LINE*6+2];
	TCHAR szSpace[BYTES_PER_LINE*6+2];

	for(i=0; i < nBuf; i+=BYTES_PER_LINE) {
		if(i >= nBuf) break;
		for(j=0, j1=0; j < BYTES_PER_LINE; j++) {
			index=i+j;
			if(index >= nBuf) break;
			// Print the hex part
			_stprintf(szNumbers+3*j,_T(" %02x"),buf[index]);
			// Print the char equivalent part
			int ch=(int)buf[index];
			TCHAR wch=buf[index];
			if(wch == '%') {
				// Handle % as %% (requires %%%%)
				_stprintf(szText+j+j1,_T("%%%%"));
				j1++;
			} else if(ch < 33 || ch > 127) {  // Note space is printed as .
				_stprintf(szText+j+j1,_T("."));
			} else {
				_stprintf(szText+j+j1,_T("%c"),wch);
			}
		}
		// Make the right number of spaces to locate the text part
		szSpace[0]='\0';
		for(k=j,k1=0; k < BYTES_PER_LINE; k++) {
			szSpace[k1++]=' ';
			szSpace[k1++]=' ';
			szSpace[k1++]=' ';
		}
		szSpace[k1]='\0';
		_stprintf(szLine,_T("%s %s%s"),szNumbers,szSpace,szText);
		lbprintf(szLine);
	}
}
