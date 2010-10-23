// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Turn off deprecation warnings
#pragma warning (disable: 4996)

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
#include <shfolder.h>
#include <shobjidl.h>
#include <textserv.h>
#include <tom.h>
#include <tmschema.h>
#include <uxtheme.h>

// Definitions for Windows Vista

#ifndef __IModalWindow_INTERFACE_DEFINED__

MIDL_INTERFACE("b4db1657-70d7-485e-8e3e-6fcb5a5c1802")
IModalWindow : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE Show(HWND) = 0;
};

#endif // __IModalWindow_INTERFACE_DEFINED__

#ifndef __IShellItem_INTERFACE_DEFINED__

typedef enum _SIGDN
{
  SIGDN_FILESYSPATH	= 0x80058000
}
SIGDN;

typedef DWORD SICHINTF;

MIDL_INTERFACE("43826d1e-e718-42ee-bc55-a1e261c37bfe")
IShellItem : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE BindToHandler(IBindCtx*, REFGUID, REFIID, void**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetParent(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDisplayName(SIGDN, LPOLESTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetAttributes(SFGAOF, SFGAOF*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Compare(IShellItem*, SICHINTF, int*) = 0;
};

#endif // __IShellItem_INTERFACE_DEFINED__

#ifndef __IFileDialog_INTERFACE_DEFINED__

enum tagFILEOPENDIALOGOPTIONS
{
  FOS_PICKFOLDERS     = 0x0000020,
  FOS_DONTADDTORECENT = 0x2000000
};

typedef struct _COMDLG_FILTERSPEC
{
  LPCWSTR pszName;
  LPCWSTR pszSpec;
}
COMDLG_FILTERSPEC;

typedef enum _FDAP {} FDAP;
typedef enum _FDE_OVERWRITE_RESPONSE {} FDE_OVERWRITE_RESPONSE;
typedef enum _FDE_SHAREVIOLATION_RESPONSE {} FDE_SHAREVIOLATION_RESPONSE;

typedef DWORD CDCONTROLSTATEF;

typedef interface IFileDialogEvents IFileDialogEvents;
typedef interface IShellItemFilter IShellItemFilter;

MIDL_INTERFACE("42f85136-db7e-439c-85f1-e4075d135fc8")
IFileDialog : public IModalWindow
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetFileTypes(UINT, const COMDLG_FILTERSPEC*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileTypeIndex(UINT) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFileTypeIndex(UINT*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Advise(IFileDialogEvents*, DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE Unadvise(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetOptions(DWORD) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetOptions(DWORD*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultFolder(IShellItem*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFolder(IShellItem*) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFolder(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentSelection(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileName(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFileName(LPWSTR*) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetTitle(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetOkButtonLabel(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFileNameLabel(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetResult(IShellItem**) = 0;
  virtual HRESULT STDMETHODCALLTYPE AddPlace(IShellItem*, FDAP) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultExtension(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE Close(HRESULT) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetClientGuid(REFGUID) = 0;
  virtual HRESULT STDMETHODCALLTYPE ClearClientData(void) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFilter(IShellItemFilter*) = 0;
};

MIDL_INTERFACE("973510db-7d7f-452b-8975-74a85828d354")
IFileDialogEvents : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE OnFileOk(IFileDialog*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnFolderChanging(IFileDialog*, IShellItem*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnFolderChange(IFileDialog*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnSelectionChange(IFileDialog*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnTypeChange(IFileDialog*) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) = 0;
};

class DECLSPEC_UUID("DC1C5A9C-E88A-4dde-A5A1-60F82A20AEF7") FileOpenDialog;
class DECLSPEC_UUID("C0B4E2F3-BA21-4773-8DBA-335EC946EB8B") FileSaveDialog;

#endif // __IFileDialog_INTERFACE_DEFINED__

#if (_WIN32_WINNT < 0x0600)

typedef enum _BP_BUFFERFORMAT
{
  BPBF_COMPATIBLEBITMAP,
  BPBF_DIB,
  BPBF_TOPDOWNDIB,
  BPBF_TOPDOWNMONODIB
}
BP_BUFFERFORMAT;

#endif // _WIN32_WINNT < 0x0600
