// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

// ImgDecmp header
#include "imgdecmp.h"

// Xerces
#ifndef UNDER_CE
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMAttr.hpp>
XERCES_CPP_NAMESPACE_USE
#endif

#ifdef UNDER_CE
// WCE only
#include <Aygshell.h>
#include <Sipapi.h>
// #include <WinVer.h>
#else
// WIN32 only
#include <Mmsystem.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#endif

// This is not defined in commctl.h for original Pocket PC
#ifndef ListView_SetCheckState
#define ListView_SetCheckState(hwndLV, i, data) \
{ LV_ITEM _ms_lvi;\
  _ms_lvi.stateMask = LVIS_STATEIMAGEMASK;\
  _ms_lvi.state = (data+1) << 12;\
  SNDMSG((hwndLV), LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);\
}
#endif

// Local Header Files

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
