#pragma once

#define DLLVERSION(major,minor) MAKELONG(minor,major)

class OSLayer
{
public:
  OSLayer();
  void Init(void);

  bool IsWindows9X(void);
  int GetWindowsVersion(void);
  DWORD GetDllVersion(const char* dllName);

  bool IsDebuggerPresent(void);

  int DrawText(CDC* dc, LPCWSTR text, int count, CRect& rect, UINT format);
  WCHAR ToUnicode(UINT virtKey, UINT scanCode, UINT flags);
  int MessageBox(CWnd* wnd, LPCWSTR text, LPCWSTR caption, UINT type);

  CString SHGetFolderPath(CWnd* wnd, int folder, HANDLE token, DWORD flags);
  void SHAutoComplete(CWnd* edit, DWORD flags);
  int SHCreateDirectoryEx(CWnd* wnd, LPCSTR path);
  HRESULT SHCreateItemFromParsingName(LPCWSTR path, IBindCtx* bc, REFIID iid, void** val);

  bool IsAppThemed(void);
  HTHEME OpenThemeData(CWnd* wnd, LPCWSTR classList);
  void CloseThemeData(HTHEME theme);
  void DrawThemeBackground(
    HTHEME theme, CDC* dc, int partId, int stateId, const RECT* rect);
  void DrawThemeText(
    HTHEME theme, CDC* dc, int partId, int stateId, const CStringW& text,
    DWORD flags, DWORD flags2, const RECT* rect);
  void GetThemeBackgroundContentRect(
    HTHEME theme, CDC* dc, int partId, int stateId, RECT* rect);
  COLORREF GetThemeColor(HTHEME theme, int partId, int stateId, int propId);
  void GetThemePartSize(HTHEME theme,
    CDC* dc, int partId, int stateId, THEMESIZE ts, SIZE* sz);
  void GetThemeMargins(HTHEME theme,
    CDC* dc, int partId, int stateId, int propId, MARGINS* margins);
  int GetThemeInt(HTHEME theme, int partId, int stateId, int propId);

  void BufferedPaintInit(void);
  CDC* BeginBufferedPaint(HDC dc,
    const RECT* target, BP_BUFFERFORMAT format, HANDLE* pb);
  void EndBufferedPaint(HANDLE pb, BOOL updateTarget);

private:
  OSVERSIONINFO m_osVer;
  HMODULE m_kernelDll;
  HMODULE m_userDll;
  HMODULE m_folderDll;
  HMODULE m_shellDll;
  HMODULE m_shellApiDll;
  HMODULE m_themeDll;
};

extern OSLayer theOS;
