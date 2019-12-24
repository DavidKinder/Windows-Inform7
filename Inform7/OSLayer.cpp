#include "stdafx.h"
#include "OSLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only OSLayer object
OSLayer theOS;

OSLayer::OSLayer()
{
  m_kernelDll = 0;
  m_userDll = 0;
  m_folderDll = 0;
  m_shellDll = 0;
  m_shellApiDll = 0;
  m_themeDll = 0;
  m_comCtlDll = 0;
  m_dwmDll = 0;
}

void OSLayer::Init(void)
{
  m_kernelDll = ::LoadLibrary("kernel32.dll");
  m_userDll = ::LoadLibrary("user32.dll");
  m_folderDll = ::LoadLibrary("shfolder.dll");
  m_shellDll = ::LoadLibrary("shell32.dll");
  m_shellApiDll = ::LoadLibrary("shlwapi.dll");
  m_themeDll = ::LoadLibrary("uxtheme.dll");
  m_comCtlDll = ::LoadLibrary("comctl32.dll");
  m_dwmDll = ::LoadLibrary("dwmapi.dll");
}

DWORD OSLayer::GetDllVersion(const char* dllName)
{
  DWORD version = 0;

  HINSTANCE dll = ::LoadLibrary(dllName);
  if (dll != 0)
  {
    DLLGETVERSIONPROC dllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(dll,"DllGetVersion");
    if (dllGetVersion != NULL)
    {
      DLLVERSIONINFO dvi;
      ::ZeroMemory(&dvi,sizeof dvi);
      dvi.cbSize = sizeof dvi;

      if (SUCCEEDED((*dllGetVersion)(&dvi)))
        version = DLLVERSION(dvi.dwMajorVersion,dvi.dwMinorVersion);
    }
    ::FreeLibrary(dll);
  }
  return version;
}

bool OSLayer::IsDebuggerPresent(void)
{
  if (m_kernelDll)
  {
    typedef BOOL(__stdcall *ISDEBUGGERPRESENT)();

    ISDEBUGGERPRESENT isDebuggerPresent =
      (ISDEBUGGERPRESENT)::GetProcAddress(m_kernelDll,"IsDebuggerPresent");
    if (isDebuggerPresent)
      return (*isDebuggerPresent)() != FALSE;
  }
  return false;
}

HANDLE OSLayer::CreateJobObject(LPSECURITY_ATTRIBUTES jobAttrs, LPCSTR name)
{
  if (m_kernelDll)
  {
    typedef HANDLE(__stdcall *CREATEJOBOBJECT)(LPSECURITY_ATTRIBUTES, LPCSTR);

    CREATEJOBOBJECT createJobObject =
      (CREATEJOBOBJECT)::GetProcAddress(m_kernelDll,"CreateJobObjectA");
    if (createJobObject)
      return (*createJobObject)(jobAttrs,name);
  }
  return 0;
}

bool OSLayer::SetInformationJobObject(HANDLE job, JOBOBJECTINFOCLASS jobClass, LPVOID jobInfo, DWORD jobInfoLen)
{
  if (m_kernelDll)
  {
    typedef BOOL(__stdcall *SETINFORMATIONJOBOBJECT)(HANDLE, JOBOBJECTINFOCLASS, LPVOID, DWORD);

    SETINFORMATIONJOBOBJECT setInformationJobObject =
      (SETINFORMATIONJOBOBJECT)::GetProcAddress(m_kernelDll,"SetInformationJobObject");
    if (setInformationJobObject)
      return ((*setInformationJobObject)(job,jobClass,jobInfo,jobInfoLen) != 0);
  }
  return 0;
}

bool OSLayer::AssignProcessToJobObject(HANDLE job, HANDLE process)
{
  if (m_kernelDll)
  {
    typedef BOOL(__stdcall *ASSIGNPROCESSTOJOBOBJECT)(HANDLE, HANDLE);

    ASSIGNPROCESSTOJOBOBJECT assignProcessToJobObject =
      (ASSIGNPROCESSTOJOBOBJECT)::GetProcAddress(m_kernelDll,"AssignProcessToJobObject");
    if (assignProcessToJobObject)
      return ((*assignProcessToJobObject)(job,process) != 0);
  }
  return 0;
}

int OSLayer::DrawText(CDC* dc, LPCWSTR text, int count, CRect& rect, UINT format)
{
  return ::DrawTextW(dc->GetSafeHdc(),text,count,rect,format);
}

WCHAR OSLayer::ToUnicode(UINT virtKey, UINT scanCode, UINT flags)
{
  BYTE state[256];
  if (::GetKeyboardState(state))
  {
    WCHAR buffer[16];
    if (::ToUnicode(virtKey,scanCode,state,buffer,16,flags) == 1)
      return buffer[0];
  }
  return 0;
}

int OSLayer::MessageBox(CWnd* wnd, LPCWSTR text, LPCWSTR caption, UINT type)
{
  return ::MessageBoxW(wnd->GetSafeHwnd(),text,caption,type);
}

bool OSLayer::GetComboBoxInfo(CComboBox* box, COMBOBOXINFO* info)
{
  if (m_userDll)
  {
    typedef BOOL(__stdcall *PFNGETCOMBOBOXINFO)(HWND, COMBOBOXINFO*);

    PFNGETCOMBOBOXINFO getComboBoxInfo = (PFNGETCOMBOBOXINFO)
      ::GetProcAddress(m_userDll,"GetComboBoxInfo");
    if (getComboBoxInfo != NULL)
    {
      if ((*getComboBoxInfo)(box->GetSafeHwnd(),info))
        return true;
    }
  }
  return false;
}

CString OSLayer::SHGetFolderPath(CWnd* wnd, int folder, HANDLE token, DWORD flags)
{
  if (m_folderDll)
  {
    typedef HRESULT(__stdcall *PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);

    PFNSHGETFOLDERPATHA getFolderPath = (PFNSHGETFOLDERPATHA)
      ::GetProcAddress(m_folderDll,"SHGetFolderPathA");
    if (getFolderPath != NULL)
    {
      char path[MAX_PATH];
      if (SUCCEEDED((*getFolderPath)(wnd->GetSafeHwnd(),folder,token,flags,path)))
        return path;
    }
  }
  return "";
}

void OSLayer::SHAutoComplete(CWnd* edit, DWORD flags)
{
  if (m_shellApiDll)
  {
    typedef HRESULT(__stdcall *PFNSHAUTOCOMPLETE)(HWND, DWORD);

    PFNSHAUTOCOMPLETE autoComplete = (PFNSHAUTOCOMPLETE)
      ::GetProcAddress(m_shellApiDll,"SHAutoComplete");
    if (autoComplete != NULL)
      (*autoComplete)(edit->GetSafeHwnd(),flags);
  }
}

int OSLayer::SHCreateDirectoryEx(CWnd* wnd, LPCSTR path)
{
  if (m_shellDll)
  {
    typedef int(__stdcall *PSHCREATEDIRECTORYEXA)(HWND, LPCSTR, SECURITY_ATTRIBUTES*);

    PSHCREATEDIRECTORYEXA createDirectory = (PSHCREATEDIRECTORYEXA)
      ::GetProcAddress(m_shellDll,"SHCreateDirectoryExA");
    if (createDirectory != NULL)
      return (*createDirectory)(wnd->GetSafeHwnd(),path,NULL);
  }

  // If SHCreateDirectoryExA is not present, fall back to CreateDirectory
  if (::CreateDirectory(path,NULL) == 0)
    return ::GetLastError();
  return ERROR_SUCCESS;
}

HRESULT OSLayer::SHCreateItemFromParsingName(LPCWSTR path, IBindCtx* bc, REFIID iid, void** val)
{
  if (m_shellDll)
  {
    typedef HRESULT(__stdcall *PFNSHCREATEITEMFROMPARSINGNAME)(PCWSTR, IBindCtx*, REFIID, void**);

    PFNSHCREATEITEMFROMPARSINGNAME createItem = (PFNSHCREATEITEMFROMPARSINGNAME)
      ::GetProcAddress(m_shellDll,"SHCreateItemFromParsingName");
    if (createItem != NULL)
      return (*createItem)(path,bc,iid,val);
  }
  return E_NOTIMPL;
}

bool OSLayer::IsAppThemed(void)
{
  if (m_themeDll)
  {
    typedef BOOL(__stdcall *ISAPPTHEMED)();

    ISAPPTHEMED isAppThemed =
      (ISAPPTHEMED)::GetProcAddress(m_themeDll,"IsAppThemed");
    if (isAppThemed)
      return (*isAppThemed)() != FALSE;
  }
  return false;
}

HTHEME OSLayer::OpenThemeData(CWnd* wnd, LPCWSTR classList)
{
  if (m_themeDll)
  {
    typedef HTHEME(__stdcall *OPENTHEMEDATA)(HWND, LPCWSTR);

    OPENTHEMEDATA openThemeData =
      (OPENTHEMEDATA)::GetProcAddress(m_themeDll,"OpenThemeData");
    if (openThemeData)
      return (*openThemeData)(wnd->GetSafeHwnd(),classList);
  }
  return 0;
}

void OSLayer::CloseThemeData(HTHEME theme)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *CLOSETHEMEDATA)(HTHEME);

    CLOSETHEMEDATA closeThemeData =
      (CLOSETHEMEDATA)::GetProcAddress(m_themeDll,"CloseThemeData");
    if (closeThemeData)
      (*closeThemeData)(theme);
  }
}

void OSLayer::DrawThemeBackground(
  HTHEME theme, CDC* dc, int partId, int stateId, const RECT* rect)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *DRAWTHEMEBACKGROUND)(HTHEME, HDC, int, int,
      const RECT*, const RECT*);

    DRAWTHEMEBACKGROUND drawThemeBackground =
      (DRAWTHEMEBACKGROUND)::GetProcAddress(m_themeDll,"DrawThemeBackground");
    if (drawThemeBackground)
      (*drawThemeBackground)(theme,dc->GetSafeHdc(),partId,stateId,rect,NULL);
  }
}

void OSLayer::DrawThemeText(
  HTHEME theme, CDC* dc, int partId, int stateId, const CStringW& text,
  DWORD flags, DWORD flags2, const RECT* rect)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *DRAWTHEMETEXT)(HTHEME, HDC, int, int,
      LPCWSTR, int, DWORD, DWORD, const RECT *);

    DRAWTHEMETEXT drawThemeText =
      (DRAWTHEMETEXT)::GetProcAddress(m_themeDll,"DrawThemeText");
    if (drawThemeText)
    {
      (*drawThemeText)(theme,dc->GetSafeHdc(),partId,stateId,text,text.GetLength(),
        flags,flags2,rect);
    }
  }
}

void OSLayer::GetThemeBackgroundContentRect(
  HTHEME theme, CDC* dc, int partId, int stateId, RECT* rect)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *GETTHEMEBACKCONTENTRECT)(HTHEME, HDC, int, int,
      const RECT *, RECT *);

    GETTHEMEBACKCONTENTRECT getThemeBackRect =
      (GETTHEMEBACKCONTENTRECT)::GetProcAddress(m_themeDll,"GetThemeBackgroundContentRect");
    if (getThemeBackRect)
    {
      CRect result;
      (*getThemeBackRect)(theme,dc->GetSafeHdc(),partId,stateId,rect,&result);
      *rect = result;
    }
  }
}

COLORREF OSLayer::GetThemeColor(HTHEME theme, int partId, int stateId, int propId)
{
  COLORREF color = RGB(0,0,0);
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *GETTHEMECOLOR)(HTHEME, int, int, int, COLORREF*);

    GETTHEMECOLOR getThemeColor =
      (GETTHEMECOLOR)::GetProcAddress(m_themeDll,"GetThemeColor");
    if (getThemeColor)
      (*getThemeColor)(theme,partId,stateId,propId,&color);
  }
  return color;
}

void OSLayer::GetThemePartSize(HTHEME theme,
  CDC* dc, int partId, int stateId, THEMESIZE ts, SIZE* sz)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *GETTHEMEPARTSIZE)
      (HTHEME, HDC, int, int, RECT*, THEMESIZE, SIZE*);

    GETTHEMEPARTSIZE getThemePartSize =
      (GETTHEMEPARTSIZE)::GetProcAddress(m_themeDll,"GetThemePartSize");
    if (getThemePartSize)
      (*getThemePartSize)(theme,dc->GetSafeHdc(),partId,stateId,NULL,ts,sz);
  }
}

void OSLayer::GetThemeMargins(HTHEME theme,
  CDC* dc, int partId, int stateId, int propId, MARGINS* margins)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *GETTHEMEMARGINS)
      (HTHEME, HDC, int, int, int, RECT*, MARGINS*);

    GETTHEMEMARGINS getThemeMargins =
      (GETTHEMEMARGINS)::GetProcAddress(m_themeDll,"GetThemeMargins");
    if (getThemeMargins)
      (*getThemeMargins)(theme,dc->GetSafeHdc(),partId,stateId,propId,NULL,margins);
  }
}

int OSLayer::GetThemeInt(HTHEME theme, int partId, int stateId, int propId)
{
  int i = 0;
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *GETTHEMEINT)(HTHEME, int, int, int, int*);

    GETTHEMEINT getThemeInt =
      (GETTHEMEINT)::GetProcAddress(m_themeDll,"GetThemeInt");
    if (getThemeInt)
      (*getThemeInt)(theme,partId,stateId,propId,&i);
  }
  return i;
}

void OSLayer::BufferedPaintInit(void)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *BUFFEREDPAINTINIT)(void);

    BUFFEREDPAINTINIT bufferedPaintInit =
      (BUFFEREDPAINTINIT)::GetProcAddress(m_themeDll,"BufferedPaintInit");
    if (bufferedPaintInit)
      (*bufferedPaintInit)();
  }
}

CDC* OSLayer::BeginBufferedPaint(HDC dc,
  const RECT* target, BP_BUFFERFORMAT format, HANDLE* pb)
{
  if (m_themeDll)
  {
    typedef HANDLE(__stdcall *BEGINBUFFEREDPAINT)
      (HDC, const RECT*, BP_BUFFERFORMAT, VOID*, HDC*);

    BEGINBUFFEREDPAINT beginBufferedPaint =
      (BEGINBUFFEREDPAINT)::GetProcAddress(m_themeDll,"BeginBufferedPaint");
    if (beginBufferedPaint)
    {
      HDC dcBuffer = 0;
      *pb = (*beginBufferedPaint)(dc,target,format,NULL,&dcBuffer);
      if (dcBuffer != 0)
        return CDC::FromHandle(dcBuffer);
    }
  }
  return CDC::FromHandle(dc);
}

void OSLayer::EndBufferedPaint(HANDLE pb, BOOL updateTarget)
{
  if (m_themeDll)
  {
    typedef HRESULT(__stdcall *ENDBUFFEREDPAINT)(HANDLE, BOOL);

    ENDBUFFEREDPAINT endBufferedPaint =
      (ENDBUFFEREDPAINT)::GetProcAddress(m_themeDll,"EndBufferedPaint");
    if (endBufferedPaint)
      (*endBufferedPaint)(pb,updateTarget);
  }
}

int OSLayer::TaskDialog(CWnd* wnd, LPCWSTR main, LPCWSTR content, LPCWSTR caption, UINT msgBoxType)
{
  if (m_comCtlDll)
  {
    typedef HRESULT(__stdcall *TASKDIALOG)
      (HWND, HINSTANCE, PCWSTR, PCWSTR, PCWSTR, TASKDIALOG_COMMON_BUTTON_FLAGS, PCWSTR, int*);

    TASKDIALOG taskDialog = (TASKDIALOG)
      ::GetProcAddress(m_comCtlDll,"TaskDialog");
    if (taskDialog != NULL)
    {
      TASKDIALOG_COMMON_BUTTON_FLAGS buttons = TDCBF_OK_BUTTON;
      switch (msgBoxType & 0xF)
      {
      case MB_OK:
        buttons = TDCBF_OK_BUTTON;
        break;
      case MB_YESNO:
        buttons = (TASKDIALOG_COMMON_BUTTON_FLAGS)
          (TDCBF_YES_BUTTON|TDCBF_NO_BUTTON);
        break;
      default:
        ASSERT(FALSE);
        break;
      }

      PCWSTR icon = 0;
      switch (msgBoxType & 0xF0)
      {
      case MB_ICONINFORMATION:
        icon = TD_INFORMATION_ICON;
        break;
      case MB_ICONWARNING:
        icon = TD_WARNING_ICON;
        break;
      default:
        ASSERT(FALSE);
        break;
      }

      int result = 0;
      if (SUCCEEDED((*taskDialog)(wnd->GetSafeHwnd(),0,caption,main,content,buttons,icon,&result)))
        return result;
      return 0;
    }
  }

  CStringW msg;
  if (wcslen(main) > 0)
    msg.Format(L"%s\n\n%s",main,content);
  else
    msg = content;
  return MessageBox(wnd,msg,caption,msgBoxType);
}

int OSLayer::TaskDialogIndirect(const TASKDIALOGCONFIG* config, BOOL* verify)
{
  if (m_comCtlDll)
  {
    typedef HRESULT(__stdcall *TASKDIALOGINDIRECT)
      (const TASKDIALOGCONFIG*, int*, int*, BOOL*);

    TASKDIALOGINDIRECT taskDialogIndirect = (TASKDIALOGINDIRECT)
      ::GetProcAddress(m_comCtlDll,"TaskDialogIndirect");
    if (taskDialogIndirect != NULL)
    {
      int btn = 0;
      if (SUCCEEDED((*taskDialogIndirect)(config,&btn,NULL,verify)))
        return btn;
    }
  }
  return 0;
}

bool OSLayer::DwmGetWindowAttribute(CWnd* wnd, DWORD attr, PVOID attrPtr, DWORD attrSize)
{
  if (m_dwmDll)
  {
    typedef HRESULT(__stdcall *PFNDWMGETWINDOWATTRIBUTE)(HWND, DWORD, PVOID, DWORD);
    PFNDWMGETWINDOWATTRIBUTE dwmGetWindowAttribute =
      (PFNDWMGETWINDOWATTRIBUTE)::GetProcAddress(m_dwmDll,"DwmGetWindowAttribute");
    if (dwmGetWindowAttribute)
    {
      if (SUCCEEDED((*dwmGetWindowAttribute)(wnd->GetSafeHwnd(),attr,attrPtr,attrSize)))
        return true;
    }
  }
  return false;
}
