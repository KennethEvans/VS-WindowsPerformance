#include "stdafx.h"
#include "utils.h"

/**************************** ansiToUnicode *******************************/
DWORD ansiToUnicode(LPCSTR pszA, LPWSTR *ppszW)
// Function to convert a char string to a unicode string.  If *ppszW
// is NULL, space is allocated.  In which case the unicode string
// must be freed by the calling program, and you can check *ppszW
// for error instead of the return value.  It is the caller's
// responsibility to insure there is enough space, otherwise.
{
	int sizeW;
	DWORD dwError;
	int retVal;
	int allocate=0;

	// Check if ANSI input is null
	if (pszA == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	// Determine number of wide characters to be allocated for the
	// Unicode string
	sizeW = MultiByteToWideChar(CP_ACP,0,pszA,-1,NULL,0);

	// Allocate space if the unicode pointer is null, otherwise use the
	// caller's space
	if(!*ppszW) {
		allocate=1;
		*ppszW = (LPWSTR)malloc(sizeW*sizeof(WCHAR));
		if(!*ppszW) return ERROR_OUTOFMEMORY;
	}

	// Covert to Unicode
	retVal = MultiByteToWideChar(CP_ACP,0,pszA,-1,*ppszW,
		sizeW);
	if(!retVal) {
		dwError = GetLastError();
		if(allocate) {
			free(*ppszW);
			*ppszW = NULL;
		}
		return dwError;
	}

	return NOERROR;
}
/**************************** unicodeToAnsi *******************************/
DWORD unicodeToAnsi(LPCWSTR pszW, LPSTR *ppszA)
// Function to convert a unicode string to an ansi string.  If
// *ppszA is NULL, space is allocated.  In which case the ansi
// string must be freed by the calling program, and you can check
// *ppszA for error instead of the return value.  It is the caller's
// responsibility to insure there is enough space, otherwise.
{
	int sizeA;
	DWORD dwError;
	int retVal;
	int allocate=0;

	// Check if Unicode input is null
	if (pszW == NULL) {
		return ERROR_INVALID_PARAMETER;
	}

	// Determine number of characters to be allocated for the
	// ANSI string
	sizeA = WideCharToMultiByte(CP_ACP,0,pszW,-1,NULL,0,NULL,NULL);

	// Allocate space if the ansi pointer is null, otherwise use the
	// caller's space
	if(!*ppszA) {
		allocate=1;
		*ppszA = (LPSTR)malloc(sizeA);
		if(!*ppszA) return ERROR_OUTOFMEMORY;
	}

	// Covert to Ansi
	retVal = WideCharToMultiByte(CP_ACP,0,pszW,-1,
		*ppszA,sizeA,NULL,NULL);
	if(!retVal) {
		dwError = GetLastError();
		if(allocate) {
			free((void *)*ppszA);
			*ppszA = NULL;
		}
		return dwError;
	}

	return NOERROR;
}
/**************************** errMsg **************************************/
int errMsg(const TCHAR *format, ...)
{
	va_list vargs;

	va_start(vargs,format);
	_vstprintf(szPrintString,format,vargs);
	va_end(vargs);

	if(szPrintString[0] == '\0') return 0;

	// Display the string.
	MessageBox(NULL,szPrintString,_T("Warning"),
		MB_OK|MB_ICONWARNING|MB_TOPMOST);

	return 0;
}
/**************************** infoMsg *************************************/
int infoMsg(const TCHAR *format, ...)
{
	va_list vargs;

	va_start(vargs,format);
	_vstprintf(szPrintString,format,vargs);
	va_end(vargs);

	if(szPrintString[0] == '\0') return 0;

	// Display the string.
	MessageBox(NULL,szPrintString,_T("Information"),
		MB_OK|MB_ICONINFORMATION|MB_TOPMOST);

	return 0;
}
/**************************** sysErrMsg ***********************************/
void sysErrMsg(LPTSTR lpHead)
{
	LPTSTR lpMsgBuf=NULL;
	DWORD error = GetLastError();
	DWORD status;

	status=FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf, // Use & and cast for FORMAT_MESSAGE_ALLOCATE_BUFFER
		0,
		NULL 
		);
	// Process any inserts in lpMsgBuf
	// ...
	if(status && lpMsgBuf) {
		_stprintf(szPrintString,_T("%s (Error %d) %s"),lpHead,error,lpMsgBuf);
		MessageBox(NULL,szPrintString,_T("Warning"),
			MB_OK|MB_ICONWARNING|MB_TOPMOST);
		// Free the buffer
	} else {
		_stprintf(szPrintString,_T("%s (Error %d) No information is available"),
			lpHead,error);
		MessageBox(NULL,szPrintString,_T("Warning"),
			MB_OK|MB_ICONWARNING|MB_TOPMOST);
	}
	LocalFree((HLOCAL)lpMsgBuf);
}
