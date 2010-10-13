// Header file for Pocket PC utilities

#ifndef _INC_UTILS_H
#define _INC_UTILS_H

#ifndef UNDER_CE
#include <tchar.h>
#endif

#define PRINT_STRING_SIZE 1024

#include <windows.h>
#include <stdlib.h>

// Function prototypes
DWORD ansiToUnicode(LPCSTR pszA, LPWSTR *ppszW);
DWORD unicodeToAnsi(LPCWSTR pszW, LPSTR *ppszA);
int errMsg(const TCHAR *format, ...);
int infoMsg(const TCHAR *format, ...);
void sysErrMsg(LPTSTR lpHead);
int wsaErrMsg(const TCHAR *format, ...);

// Global string for print routines.  Must be defined somewhere.
extern TCHAR szPrintString[PRINT_STRING_SIZE];

#endif // _INC_UTILS_H
