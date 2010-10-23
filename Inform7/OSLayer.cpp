#include "stdafx.h"
#include "OSLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only OSLayer object
OSLayer theOS;

OSLayer::OSLayer()
{
  m_osVer.dwOSVersionInfoSize = sizeof m_osVer;
  ::GetVersionEx(&m_osVer);

  m_kernelDll = 0;
  m_userDll = 0;
  m_folderDll = 0;
  m_shellDll = 0;
  m_shellApiDll = 0;
  m_themeDll = 0;
}

void OSLayer::Init(void)
{
  m_kernelDll = ::LoadLibrary("kernel32.dll");
  m_userDll = ::LoadLibrary("user32.dll");
  m_folderDll = ::LoadLibrary("shfolder.dll");
  m_shellDll = ::LoadLibrary("shell32.dll");
  m_shellApiDll = ::LoadLibrary("shlwapi.dll");
  m_themeDll = ::LoadLibrary("uxtheme.dll");
}

bool OSLayer::IsWindows9X(void)
{
  return (m_osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}

int OSLayer::GetWindowsVersion(void)
{
  return m_osVer.dwMajorVersion;
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

int OSLayer::DrawText(CDC* dc, LPCWSTR text, int count, CRect& rect, UINT format)
{
  if (m_osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
  {
    // Call ANSI function on Windows 9X
    CString textA(text,count);
    return ::DrawTextA(dc->GetSafeHdc(),(LPCSTR)textA,textA.GetLength(),rect,format);
  }
  else
  {
    // Call Unicode function on Windows NT
    return ::DrawTextW(dc->GetSafeHdc(),text,count,rect,format);
  }
}

WCHAR OSLayer::ToUnicode(UINT virtKey, UINT scanCode, UINT flags)
{
  BYTE state[256];
  if (::GetKeyboardState(state))
  {
    if (m_osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
      WORD buffer[16];
      if (::ToAscii(virtKey,scanCode,state,buffer,flags) == 1)
      {
        CHAR charA = (CHAR)buffer[0];
        WCHAR charW = 0;
        if (::MultiByteToWideChar(CP_ACP,0,&charA,1,&charW,1) == 1)
          return charW;
      }
    }
    else
    {
      WCHAR buffer[16];
      if (::ToUnicode(virtKey,scanCode,state,buffer,16,flags) == 1)
        return buffer[0];
    }
  }
  return 0;
}

int OSLayer::MessageBox(CWnd* wnd, LPCWSTR text, LPCWSTR caption, UINT type)
{
  if (m_osVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
  {
    CString textA(text);
    CString captionA(caption);
    return ::MessageBoxA(wnd->GetSafeHwnd(),textA,captionA,type);
  }
  else
    return ::MessageBoxW(wnd->GetSafeHwnd(),text,caption,type);
}

CString OSLayer::SHGetFolderPath(CWnd* wnd, int folder, HANDLE token, DWORD flags)
{
  if (m_folderDll)
  {
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
