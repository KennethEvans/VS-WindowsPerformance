// Header for console routines

#ifndef _INC_CONSOLE_H
#define _INC_CONSOLE_H

// ListBox handle
extern HWND hLB;

// Basic routines
void createConsole(HINSTANCE hInst, HWND hWndParent);
int lbprintf(const TCHAR *format,... );
void lbclear(void);

// Call before createConsole to change defaults
void lbSetMaxLines(int maxLinesIn);
int lbGetMaxLines(void);
void lbSetMaxChars(int maxCharsIn);
int lbGetMaxChars(void);
void lbUseHScroll(BOOL useHscrollIn);

// Use to speed up and prevent flashing during long output
void lbSetRedraw(BOOL val);

// Use to reset the end of page value after resize
void lbSetEndOfPage(void);

// Utilities
void dumpLBToFile(TCHAR *szFileName);
void dumpBytes(BYTE *buf, int nBuf);

#endif // _INC_CONSOLE_H
