// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Turn off deprecation warnings
#pragma warning (disable: 4996)

// Require Windows 10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN    // Exclude rarely-used stuff from Windows headers
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#include <afxdtctl.h>
#include <afxcmn.h>
#include <afxhtml.h>
#include <afxtempl.h>
#include <afxdlgs.h>
#include <afxole.h>
#include <afxocc.h>
#include <afxmt.h>
#include <afxpriv.h>

#include <atlbase.h>
#include <atlenc.h>

#include <dlgs.h>
#include <msxml2.h>
#include <richole.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <textserv.h>
#include <tom.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <wininet.h>
