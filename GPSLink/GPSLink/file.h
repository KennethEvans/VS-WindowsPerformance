// Header for file routines

#ifndef _INC_FILE_H
#define _INC_FILE_H

#ifndef UNDER_CE
#include <tchar.h>
#endif

typedef enum {
    Other_OutputFile=0,
    // The first pair of strings is index 1
    GPX_OutputFile,
	GPSL_OutputFile,
    CSV_OutputFile,
    SA8_OutputFile,
    GPSU_OutputFile,
} OutputFilterIndex;

typedef enum {
    Other_InputFile=0,
    // The first pair of strings is index 1
    GPX_InputFile,
    GPSL_InputFile,
    CSV_InputFile,
    JPEG_InputFile,
    BMP_InputFile,
    GIF_InputFile,
    PNG_InputFile,
    XBM_InputFile,
} InputFilterIndex;

void fileInitialize(HWND hwnd);
BOOL fileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
BOOL fileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
void setFilterIndex(int index);
DWORD getFilterIndex(void);

#endif // _INC_FILE_H
