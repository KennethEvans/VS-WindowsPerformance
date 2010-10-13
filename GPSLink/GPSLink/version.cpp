// Version information files

#include "StdAfx.h"

#include "utils.h"

#define DEBUG_RESOURCES 0
#define WRITE_FILE 0
#define USE_TESTS 0

// Function prototypes

#if USE_TESTS
void listResources(TCHAR *fileName);
#endif

// Version that doesn't use VersionInfo routines
BOOL getModuleFileVersion(TCHAR *versionString, int len)
{
	HMODULE hModule;
	HRSRC hResInfo; 

	// Get the module handle of this module (Could omit this step and use
	// NULL for hModule below)
	hModule=GetModuleHandle(NULL);
	if(!hModule) { 
		errMsg(_T("Could not get module handle")); 
		return FALSE;
	} 

	// Get the resource.  We assume from hacking that 1 is the correct
	// name to use.  If not, they will have to enumerated.
	hResInfo=FindResource(hModule,(LPTSTR)1,RT_VERSION); 
	if(!hResInfo) { 
		errMsg(_T("Could not find version resource")); 
		return FALSE;
	}

	// Load the resource
	HGLOBAL hResLoad=LoadResource(hModule,hResInfo);
	if(!hResLoad) {
		errMsg(_T("Could not load resource"));
		return FALSE;
	}

	// Lock the resource (Not necessary on WCE, can use hResLoad for the
	// data
	LPBYTE hData=(BYTE *)LockResource(hResLoad);
	if(!hData) {
		errMsg(_T("Could not lock resource"));
		return FALSE;
	}

	// Write the version string
	DWORD dwFileVersionMS=*(DWORD *)(hData+0x30);
	DWORD dwFileVersionLS=*(DWORD *)(hData+0x30+sizeof(DWORD));
	_stprintf(versionString,_T("%d.%d.%d.%d"),
		HIWORD(dwFileVersionMS),LOWORD(dwFileVersionMS),
		HIWORD(dwFileVersionLS),LOWORD(dwFileVersionLS));

	// Check the length after the fact
	int usedLen=_tcslen(versionString)+1;
	if(usedLen > len) {
		errMsg(_T("Overwrote version string.  Used %d.  Given %d"));
		return FALSE;
	}

	return TRUE;
}

#if 0
// Version using Version Info API not available on WCE
BOOL getModuleFileVersion1(TCHAR *versionString, int len)
{
#ifdef UNDER_CE
	return FALSE;
#else
	TCHAR moduleName[MAX_PATH];
	BYTE *versionData=NULL;
	BOOL status;
	DWORD fileNameSize,versionSize;
	DWORD dummy;

#if DEBUG_RESOURCES
	listResources(_T("c:\\scratch\\resinfo.txt"));
#endif    

	// Get the filename of this module
	fileNameSize=GetModuleFileName(NULL,moduleName,MAX_PATH);
	if(fileNameSize <= 0) {
		errMsg(_T("Could not get module name"));
		return FALSE;
	}

	// Get the required size for the version info
	versionSize=GetFileVersionInfoSize(moduleName,&dummy);
	if(versionSize <= 0) {
		errMsg(_T("Version info not available"));
		return FALSE;
	}

	// Get the version info
	versionData=new BYTE[versionSize];
	if(!versionData) {
		errMsg(_T("Could not allocate space for versionData"));
		return FALSE;
	}
	status=GetFileVersionInfo(moduleName,dummy,
		versionSize,versionData);
	if(!status) {
		errMsg(_T("Could not get version info"));
		if(versionData) delete [] versionData;
		return FALSE;
	}

	// Get the root block
	LPVOID lpvi;
	UINT iLen;
	status=VerQueryValue(versionData,_T("\\"),&lpvi,&iLen);
	if(!status || iLen < 4) {
		errMsg(_T("Could not get version info value"));
		if(versionData) delete [] versionData;
		return FALSE;
	}
	VS_FIXEDFILEINFO *lpvInfo=(VS_FIXEDFILEINFO *)lpvi;

	// Print the string
	_stprintf(versionString,_T("%d.%d.%d.%d"),
		HIWORD(lpvInfo->dwFileVersionMS),LOWORD(lpvInfo->dwFileVersionMS),
		HIWORD(lpvInfo->dwFileVersionLS),LOWORD(lpvInfo->dwFileVersionLS));

	// Check the length after the fact
	int usedLen=_tcslen(versionString)+1;
	if(usedLen > len) {
		errMsg(_T("Overwrote version string.  Used %d.  Given %d"));
		if(versionData) delete [] versionData;
		return FALSE;
	}

	return TRUE;
#endif
}
#endif

#if USE_TESTS
/////////////////////////////////////////////////////////////////
// Test routines ////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Declare callback functions. 
BOOL EnumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG lParam); 
BOOL EnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName,
				   LONG lParam); 
BOOL EnumLangsFunc(HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName,
				   WORD wLang, LONG lParam); 

// Global variables
static char szBuffer[80]; // print buffer for EnumResourceTypes 
static DWORD cbWritten;   // number of bytes written to res. info. file 
static int cbString;      // length of string in sprintf 
static HANDLE hFile;

void listResources(TCHAR *fileName)
{
	HMODULE hMod;

#if USE_LOADLIBRARY
	// Load the .EXE whose resources you want to list. 
	hMod=LoadLibrary(_T("hand.exe")); 
	if(hMod == NULL) { 
		errMsg(_T("Could not load module"),_T("hand.exe")); 
		return;
	} 
#else
	hMod=GetModuleHandle(NULL);
	if(!hMod) { 
		errMsg(_T("Could not get module handle")); 
		return;
	} 
#endif

#if WRITE_FILE
	// Create a file to contain the resource info. 
	hFile=CreateFile(fileName,          // name of file 
		GENERIC_READ | GENERIC_WRITE,      // access mode 
		0,                                 // share mode 
		(LPSECURITY_ATTRIBUTES) NULL,      // no security 
		CREATE_ALWAYS,                     // create flags 
		FILE_ATTRIBUTE_NORMAL,             // file attributes 
		(HANDLE) NULL);                    // no template 
	if(hFile == INVALID_HANDLE_VALUE) { 
		errMsg(_T("Could not open file %s"),fileName); 
		return;
	} 

	// Find all of the loaded file's resources
	{
		TCHAR moduleName[MAX_PATH];
		DWORD fileNameSize;
		fileNameSize=GetModuleFileName(NULL,moduleName,MAX_PATH);
		if(fileNameSize <= 0) {
			errMsg(_T("Could not get module name"));
			cbString=sprintf(szBuffer, 
				"This module contains the following resources:\n\n"); 
		} else {
			DWORD result;
			char *ansiString=NULL;
			result=unicodeToAnsi(moduleName,&ansiString);
			if(ansiString) {
				cbString=sprintf(szBuffer, 
					"%s\nThis module contains the following resources:\n\n",
					ansiString); 
			} else {
				cbString=sprintf(szBuffer, 
					"%s\nThis module contains the following resources:\n\n",
					"Error"); 
			}
		}
	}
	WriteFile(hFile,      // file to hold resource info. 
		szBuffer,           // what to write to the file 
		(DWORD)cbString,    // number of bytes in szBuffer 
		&cbWritten,         // number of bytes written 
		NULL);              // no overlapped I/O 
#endif

	EnumResourceTypes(hMod,            // module handle 
		(ENUMRESTYPEPROC)EnumTypesFunc,  // callback function 
		0);                              // extra parameter 

#if USE_LOADLIBRARY
	// Unload the executable file whose resources were 
	// enumerated and close the file created to contain 
	// the resource information. 
	FreeLibrary(hMod); 
#endif
#if WRITE_FILE
	CloseHandle(hFile);
#endif
}

//    FUNCTION: EnumTypesFunc(HMODULE, LPSTR, LONG) 
// 
//    PURPOSE:  Resource type callback 
BOOL EnumTypesFunc( 
				   HMODULE hModule,   // module handle 
				   LPTSTR lpType,    // address of resource type 
				   LONG lParam)      // extra parameter, could be 
				   // used for error checking 
{ 
#if WRITE_FILE
	int cbString; 
	// Write the resource type to a resource information file. 
	// The type may be a string or an unsigned decimal 
	// integer, so test before printing. 
	if((ULONG)lpType & 0xFFFF0000) { 
		DWORD result;
		char *ansiString=NULL;
		result=unicodeToAnsi(lpType,&ansiString);
		if(ansiString) {
			cbString=sprintf(szBuffer,"Type: %s\n",ansiString); 
		} else {
			cbString=sprintf(szBuffer,"Type: %s\n","ERROR"); 
		}
	} else {
		char *desc;
		char *id;
		switch((USHORT)lpType) {
	  case RT_ACCELERATOR:
		  id="RT_ACCELERATOR";
		  desc="Accelerator table";
		  break;	
	  case RT_ANICURSOR:
		  id="RT_ANICURSOR";
		  desc="Animated cursor";
		  break;	
	  case RT_ANIICON:
		  id="RT_ANIICON";
		  desc="Animated icon";
		  break;	
	  case RT_BITMAP:
		  id="RT_BITMAP";
		  desc="Bitmap resource";
		  break;	
	  case RT_CURSOR:
		  id="RT_CURSOR";
		  desc="Hardware-dependent cursor resource";
		  break;	
	  case RT_DIALOG:
		  id="RT_DIALOG";
		  desc="Dialog box";
		  break;	
	  case RT_DLGINCLUDE:
		  id="RT_DLGINCLUDE";
		  desc=" ";
		  break;	
	  case RT_FONT:
		  id="RT_FONT";
		  desc="Font resource";
		  break;	
	  case RT_FONTDIR:
		  id="RT_FONTDIR";
		  desc="Font directory resource";
		  break;	
	  case RT_GROUP_CURSOR:
		  id="RT_GROUP_CURSOR";
		  desc="Hardware-independent cursor resource";
		  break;	
	  case RT_GROUP_ICON:
		  id="RT_GROUP_ICON";
		  desc="Hardware-independent icon resource";
		  break;	
	  case RT_HTML:
		  id="RT_HTML";
		  desc="HTML ";
		  break;	
	  case RT_ICON:
		  id="RT_ICON";
		  desc="Hardware-dependent icon resource";
		  break;	
#if 0
	  case RT_MANIFEST:
		  id="RT_MANIFEST";
		  desc="Whistler: Fusion XML Manifest";
		  break;	
#endif
	  case RT_MENU:
		  id="RT_MENU";
		  desc="Menu resource";
		  break;	
	  case RT_MESSAGETABLE:
		  id="RT_MESSAGETABLE";
		  desc="Message-table entry";
		  break;	
	  case RT_PLUGPLAY:
		  id="RT_PLUGPLAY";
		  desc="Plug and Play resource";
		  break;	
	  case RT_RCDATA:
		  id="RT_RCDATA";
		  desc="Application-defined resource (raw data)";
		  break;	
	  case RT_STRING:
		  id="RT_STRING";
		  desc="String-table entry";
		  break;	
	  case RT_VERSION:
		  id="RT_VERSION";
		  desc="Version resource";
		  break;	
	  case RT_VXD:
		  id="RT_VXD";
		  desc="VXD";
		  break;
	  default:
		  id="Unknown";
		  desc="Unknown";
		  break;
		}
		cbString=sprintf(szBuffer,"Type: %u %s \"%s\"\n",
			(USHORT)lpType,id,desc);
	} 
	WriteFile(hFile,szBuffer,(DWORD)cbString,&cbWritten,NULL); 
#endif

	// Find the names of all resources of type lpType. 
	EnumResourceNames(hModule, 
		lpType, 
		(ENUMRESNAMEPROC)EnumNamesFunc, 
		0); 

	return TRUE; 
}

//    FUNCTION: EnumNamesFunc(HMODULE, LPSTR, LPSTR, LONG) 
// 
//    PURPOSE:  Resource name callback 
BOOL EnumNamesFunc( 
				   HMODULE hModule,   // module handle 
				   LPCTSTR lpType,   // address of resource type 
				   LPTSTR lpName,    // address of resource name 
				   LONG lParam)      // extra parameter, could be 
				   // used for error checking 
{ 
#if WRITE_FILE
	int cbString; 
	// Write the resource name to a resource information file. 
	// The name may be a string or an unsigned decimal 
	// integer, so test before printing. 
	if((ULONG)lpName & 0xFFFF0000)  { 
		DWORD result;
		char *ansiString=NULL;
		result=unicodeToAnsi(lpType,&ansiString);
		if(ansiString) {
			cbString=sprintf(szBuffer,"    Name: %s\n",ansiString); 
		} else {
			cbString=sprintf(szBuffer,"    Name: %s\n","ERROR"); 
		}
	} else  { 
		cbString=sprintf(szBuffer,"    Name: %u\n", 
			(USHORT)lpName); 
	}

	WriteFile(hFile,szBuffer,(DWORD) cbString,&cbWritten,NULL); 
#endif

	// Find the languages of all resources of type 
	// lpType and name lpName. 
	EnumResourceLanguages(hModule, 
		lpType, 
		lpName, 
		(ENUMRESLANGPROC)EnumLangsFunc, 
		0); 

	return TRUE; 
} 

//    FUNCTION: EnumLangsFunc(HMODULE, LPSTR, LPSTR, WORD, LONG) 
// 
//    PURPOSE:  Resource language callback 
BOOL EnumLangsFunc( 
				   HMODULE hModule,  // module handle 
				   LPCTSTR lpType,  // address of resource type 
				   LPCTSTR lpName,  // address of resource name 
				   WORD wLang,      // resource language 
				   LONG lParam)     // extra parameter, could be 
				   // used for error checking 
{ 
#if WRITE_FILE
	HRSRC hResInfo; 
	char szBuffer[80]; 
	int cbString=0; 

	hResInfo=FindResourceEx(hModule,lpType,lpName,wLang); 
	// Write the resource language to the resource information file. 
	cbString=sprintf(szBuffer,"        Language: %u\n",(USHORT)wLang); 
	WriteFile(hFile,szBuffer,(DWORD)cbString,
		&cbWritten,NULL); 
	// Write the resource handle and size to buffer. 
	cbString=sprintf(szBuffer,
		"        hResInfo=%lx, Size=%lu\n",
		hResInfo,
		SizeofResource(hModule,hResInfo)); 
	WriteFile(hFile,szBuffer,(DWORD)cbString,&cbWritten,NULL);
	if(lpType == RT_VERSION) {
		HGLOBAL hResLoad=LoadResource(hModule,hResInfo);
		if(!hResLoad) {
			errMsg(_T("Could not load resource"));
		} else {
			LPBYTE hData=(BYTE *)LockResource(hResLoad);
			if(!hData) {
				errMsg(_T("Could not lock resource"));
			} else {
#if 0
				WriteFile(hFile,hData,SizeofResource(hModule,hResInfo),
					&cbWritten,NULL);
#endif
				DWORD dwFileVersionMS=*(DWORD *)(hData+0x30);
				DWORD dwFileVersionLS=*(DWORD *)(hData+0x30+sizeof(DWORD));
				DWORD dwProdVersionMS=*(DWORD *)(hData+0x30+2*sizeof(DWORD));
				DWORD dwProdVersionLS=*(DWORD *)(hData+0x30+3*sizeof(DWORD));
				cbString=sprintf(szBuffer,
					"File Version: %d.%d.%d.%d Product Version: %d.%d.%d.%d\n",
					HIWORD(dwFileVersionMS),LOWORD(dwFileVersionMS),
					HIWORD(dwFileVersionLS),LOWORD(dwFileVersionLS),
					HIWORD(dwProdVersionMS),LOWORD(dwProdVersionMS),
					HIWORD(dwProdVersionLS),LOWORD(dwProdVersionLS));
				WriteFile(hFile,szBuffer,(DWORD)cbString,&cbWritten,NULL);
			}
		}
	}
#endif

	return TRUE; 
} 
#endif
