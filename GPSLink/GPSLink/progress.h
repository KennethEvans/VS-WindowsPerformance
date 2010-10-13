// Header for progress routines

#ifndef _INC_PROGRESS_H
#define _INC_PROGRESS_H

// Function prototypes
void progressCreateDialog(HINSTANCE hInst, HWND hWndParent);
void progressSetProgress(TCHAR *header, int rcvd, int max);
void progressDestroyDialog(void);

// Global variables
extern BOOL progressAbort;

#endif // _INC_PROGRESS_H
