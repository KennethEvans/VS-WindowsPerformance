// Header for version routines

#ifndef _INC_VERSION_H
#define _INC_VERSION_H

#ifndef UNDER_CE
#include <tchar.h>
#endif

// Function prototypes

BOOL getModuleFileVersion(TCHAR *versionString, int len);

#endif // _INC_VERSION_H
