#include "stdafx.h"
#include "ReportHtml.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Internet Explorer window, used to set the context menu
class IEWnd : public CWnd
{
  DECLARE_DYNAMIC(IEWnd)

protected:
  DECLARE_MESSAGE_MAP()

  ReportHtml* m_html;
  HMENU m_menu;
  UINT m_zoomId;

public:
  IEWnd(ReportHtml* html, HMENU menu, UINT zoomId);

  afx_msg LRESULT OnInitMenuPopup(WPARAM, LPARAM);
};

IMPLEMENT_DYNCREATE(ReportHtml, CHtmlView)

ReportHtml::ReportHtml() : m_consumer(NULL),
  m_setFocus(true), m_goToFound(false), m_notify(true),
  m_scriptExternal(this), m_scriptProject(this)
{
}

BEGIN_MESSAGE_MAP(ReportHtml, CHtmlView)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
  ON_COMMAND(ID_EDIT_FIND, OnEditFind)
END_MESSAGE_MAP()

HRESULT ReportHtml::OnGetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved)
{
  // Has an alternate registry path been set?
  if (m_registryPath.IsEmpty())
    return S_FALSE;

  // Copy to an allocated string and return
  CStringW registryPath((LPCSTR)m_registryPath);
  *pchKey = (LPOLESTR)::CoTaskMemAlloc((registryPath.GetLength()+1)*sizeof(WCHAR));
  wcscpy(*pchKey,registryPath);
  return S_OK;
}

void ReportHtml::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD, LPCTSTR, CByteArray&, LPCTSTR, BOOL* pbCancel)
{
  *pbCancel = FALSE;
  if (m_consumer)
  {
    if (strncmp(lpszURL,"source:",7) == 0)
    {
      // Got a source: URL
      m_consumer->SourceLink(lpszURL);
      *pbCancel = TRUE;
    }
    else if (strncmp(lpszURL,"inform:",7) == 0)
    {
      // Got an inform: documentation URL
      m_consumer->DocLink(theApp.GetUrlProtocol().TranslateUrl(CStringW(lpszURL)));
      *pbCancel = TRUE;
    }
  }

  if (*pbCancel == FALSE)
  {
    // If notification is on, tell the parent window that the URL has changed
    if (m_notify)
    {
      if (strncmp(lpszURL,"javascript:",11) != 0)
      {
        m_url = lpszURL;
        GetParent()->PostMessage(WM_USERNAVIGATE);
      }
    }
  }

  // Reset the notification status to the default, i.e. enabled
  m_notify = true;
}

void ReportHtml::OnDocumentComplete(LPCTSTR lpszURL)
{
  CHtmlView::OnDocumentComplete(lpszURL);

  // Make this the active window, except for blank URLs
  if (m_setFocus && (strcmp(lpszURL,"about:blank") != 0))
    SetFocusOnContent();

  // Highlight found text
  if (!m_find.IsEmpty())
  {
    IDispatch* disp = GetHtmlDocument();
    CComQIPtr<IHTMLDocument2> doc(disp);
    disp->Release();
    if (doc == NULL)
      return;

    CComPtr<IHTMLElement> element;
    doc->get_body(&element);
    if (element == NULL)
      return;
    CComQIPtr<IHTMLBodyElement> body(element);
    if (body == NULL)
      return;

    CComPtr<IHTMLTxtRange> range;
    body->createTextRange(&range);
    if (range == NULL)
      return;

    bool first = m_goToFound;
    VARIANT_BOOL found = VARIANT_TRUE;
    while (found == VARIANT_TRUE)
    {
      range->findText(CComBSTR(m_find),0,0,&found);
      if (found == VARIANT_TRUE)
      {
        COLORREF colour = theApp.GetColour(InformApp::ColourHighlight);
        CString colourStr;
        colourStr.Format("#%02X%02X%02X",GetRValue(colour),GetGValue(colour),GetBValue(colour));

        VARIANT_BOOL result;
        range->execCommand(CComBSTR("BackColor"),VARIANT_FALSE,CComVariant(colourStr),&result);

        if (first)
        {
          range->scrollIntoView(VARIANT_TRUE);
          first = false;
        }

        long moved;
        range->collapse(VARIANT_FALSE);
        range->moveEnd(CComBSTR("textedit"),1,&moved);
      }
    }

    // Don't highlight any more
    m_find.Empty();
    m_goToFound = false;
  }
}

void ReportHtml::OnStatusTextChange(LPCTSTR)
{
  // Do nothing
}

HRESULT ReportHtml::OnShowContextMenu(DWORD dwID,  LPPOINT ppt, LPUNKNOWN pcmdTarget, LPDISPATCH)
{
  // Get COM interfaces
  CComQIPtr<IOleWindow> oleWindow(pcmdTarget);
  if (oleWindow == NULL)
    return S_FALSE;

  // Get the window to use as the menu's parent
  HWND window;
  oleWindow->GetWindow(&window);

  // Get the context menu
  CMenu menu;
  menu.LoadMenu(IDR_HTMLMENU);

  // Get the current zoom value
  CComVariant zoom;
  ExecWB(OLECMDID_ZOOM,OLECMDEXECOPT_DONTPROMPTUSER,NULL,&zoom); 

  // Subclass the IE window to catch menu messages
  IEWnd ieWnd(this,menu,zoom.lVal+ID_ZOOM_SMALLEST);
  ieWnd.SubclassWindow(window);

  // Show the menu
  int select = menu.GetSubMenu(0)->TrackPopupMenuEx(
    TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,ppt->x,ppt->y,CWnd::FromHandle(window),NULL);
  ieWnd.UnsubclassWindow();

  switch (select)
  {
  case ID_NAVIGATE_BACK:
    Panel::GetPanel(this)->TabNavigate(false);
    break;
  case ID_NAVIGATE_FORE:
    Panel::GetPanel(this)->TabNavigate(true);
    break;
  case ID_EDIT_COPY:
    ExecWB(OLECMDID_COPY,OLECMDEXECOPT_DONTPROMPTUSER,NULL,NULL);
    break;
  case ID_EDIT_SELECT_ALL:
    ExecWB(OLECMDID_SELECTALL,OLECMDEXECOPT_DONTPROMPTUSER,NULL,NULL);
    break;
  case ID_FILE_PRINT:
    ExecWB(OLECMDID_PRINT,OLECMDEXECOPT_PROMPTUSER,NULL,NULL); 
    break;
  case ID_MENU_PROPERTIES:
    ExecWB(OLECMDID_PROPERTIES,OLECMDEXECOPT_DODEFAULT,NULL,NULL); 
    break;
  default:
    if ((select >= ID_ZOOM_SMALLEST) && (select <= ID_ZOOM_LARGEST))
    {
      // Change the text size
      zoom = (int)(select-ID_ZOOM_SMALLEST);
      ExecWB(OLECMDID_ZOOM,OLECMDEXECOPT_DONTPROMPTUSER,&zoom,NULL); 
    }
    break;
  }
  return S_OK;
}

HRESULT ReportHtml::OnGetExternal(LPDISPATCH *lppDispatch)
{
  *lppDispatch = m_scriptExternal.GetIDispatch(TRUE);
  return S_OK;
}

HRESULT ReportHtml::OnTranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
{
  if ((lpMsg != NULL) && (lpMsg->message == WM_SYSKEYDOWN))
  {
    switch (lpMsg->wParam)
    {
    case VK_LEFT:
      GetParent()->PostMessage(WM_COMMAND,ID_NAVIGATE_BACK);
      return S_OK;
    case VK_RIGHT:
      GetParent()->PostMessage(WM_COMMAND,ID_NAVIGATE_FORE);
      return S_OK;
    }
  }
  return S_FALSE;
}

void ReportHtml::OnEditSelectAll()
{
  ExecWB(OLECMDID_SELECTALL,OLECMDEXECOPT_DONTPROMPTUSER,NULL,NULL);
}

void ReportHtml::OnEditFind()
{
  IDispatch* disp = GetHtmlDocument();
  CComQIPtr<IOleCommandTarget> target(disp);
  disp->Release();

  if (target != NULL)
  {
    // From the Microsoft Knowledge Base, article Q175513
    static const GUID CGID_IWebBrowser =
      { 0xED016940L,0xBD5B,0x11CF,{0xBA,0x4E,0x00,0xC0,0x4F,0xD7,0x08,0x16}};
    target->Exec(&CGID_IWebBrowser,1,0,NULL,NULL);
  }
}

void ReportHtml::OnUpdateEditFind(CCmdUI* pCmdUI)
{
  IDispatch* disp = GetHtmlDocument();
  CComQIPtr<IOleCommandTarget> target(disp);
  disp->Release();

  pCmdUI->Enable(target != NULL);
}

CString ReportHtml::m_registryPath;

void ReportHtml::SetRegistryPath(const char* path)
{
  m_registryPath = path;

  // Open the web browser registry key
  CRegKey webKey;
  DWORD disposition;
  LONG result = webKey.Create(HKEY_CURRENT_USER,path,
    REG_NONE,REG_OPTION_NON_VOLATILE,KEY_READ|KEY_WRITE,NULL,&disposition);

  // If the key already exists, don't create any settings under it
  if ((result == ERROR_SUCCESS) && (disposition == REG_CREATED_NEW_KEY))
  {
    CRegKey webSettingsKey;
    if (webSettingsKey.Create(webKey,"Settings") == ERROR_SUCCESS)
    {
      // Show all hyperlinks as blue
      webSettingsKey.SetStringValue("Anchor Color","0,0,255");
      webSettingsKey.SetStringValue("Anchor Color Visited","0,0,255");
      webSettingsKey.SetStringValue("Background Color","255,255,255");
      webSettingsKey.SetStringValue("Text Color","0,0,0");
    }

    CRegKey webMainKey;
    if (webMainKey.Create(webKey,"Main") == ERROR_SUCCESS)
    {
      // Use Windows colours
      webMainKey.SetStringValue("Use_DlgBox_Colors","no");
    }
  }

  // Update font keys as necessary
  if (result == ERROR_SUCCESS)
  {
    CRegKey internationalKey;
    if (internationalKey.Create(webKey,"International") == ERROR_SUCCESS)
    {
      CRegKey scriptsKey;
      if (scriptsKey.Create(internationalKey,"Scripts") == ERROR_SUCCESS)
      {
        CRegKey script3Key;
        if (script3Key.Create(scriptsKey,"3") == ERROR_SUCCESS)
        {
          // Only set font size if not already set
          ULONG len;
          if (script3Key.QueryBinaryValue("IEFontSize",NULL,&len) != ERROR_SUCCESS)
          {
            INT32 size = (theApp.GetFontPointSize() > 9) ? 2 : 1;
            script3Key.SetBinaryValue("IEFontSize",&size,sizeof size);
          }

          // Always update font names
          script3Key.SetStringValue("IEPropFontName",theApp.GetFontName());
          script3Key.SetStringValue("IEFixedFontName",theApp.GetFixedFontName());
        }
      }
    }
  }
}

void ReportHtml::SetLinkConsumer(LinkConsumer* consumer)
{
  m_consumer = consumer;
}

void ReportHtml::SetFocusOnContent(void)
{
  CPoint point(0,0);
  ClientToScreen(&point);
  CWnd* wnd = WindowFromPoint(point);
  if (wnd != NULL)
    wnd->SetFocus();
}

void ReportHtml::SetFocusFlag(bool focus)
{
  m_setFocus = focus;
}

void ReportHtml::Navigate(const char* url, bool focus, const wchar_t* find)
{
  // Stop any current page loading
  if (GetBusy())
    Stop();

  m_url = url;
  m_setFocus = focus;

  if (find != NULL)
  {
    m_find = find;

    // Go to the first found occurence if the link does not contain a section reference
    m_goToFound = (strchr(url,'#') == NULL);
  }
  else
  {
    m_find.Empty();
    m_goToFound = false;
  }

  // Don't send a navigation notification for programmatic navigation
  m_notify = false;
  CHtmlView::Navigate(url);
}

CString ReportHtml::GetURL(void)
{
  return m_url;
}

// COM object for the root Javascript external object

BEGIN_DISPATCH_MAP(ScriptExternal,CCmdTarget)
  DISP_PROPERTY_EX(ScriptExternal,"Project",GetProject,SetProject,VT_UNKNOWN)
END_DISPATCH_MAP()

ScriptExternal::ScriptExternal(ReportHtml* html) : m_html(html)
{
  EnableAutomation();
}

LPUNKNOWN ScriptExternal::GetProject(void)
{
  return m_html->m_scriptProject.GetIDispatch(TRUE);
}

void ScriptExternal::SetProject(LPUNKNOWN)
{
}

// COM object for the Javascript project object

BEGIN_DISPATCH_MAP(ScriptProject,CCmdTarget)
  DISP_FUNCTION(ScriptProject,"selectView",SelectView,VT_EMPTY,VTS_BSTR)
  DISP_FUNCTION(ScriptProject,"pasteCode",PasteCode,VT_EMPTY,VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"openFile",OpenFile,VT_EMPTY,VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"openUrl",OpenUrl,VT_EMPTY,VTS_WBSTR)
END_DISPATCH_MAP()

ScriptProject::ScriptProject(ReportHtml* html) : m_html(html)
{
  EnableAutomation();
}

void ScriptProject::SelectView(LPCSTR view)
{
  m_html->GetParentFrame()->SendMessage(
    WM_SELECTVIEW,(WPARAM)view,(LPARAM)m_html->GetSafeHwnd());
}

void ScriptProject::PasteCode(LPCWSTR code)
{
  m_html->GetParentFrame()->SendMessage(WM_PASTECODE,(WPARAM)code);
}

void ScriptProject::OpenFile(LPCWSTR path)
{
  CString pathA(path);
  DWORD attrs = ::GetFileAttributes(pathA);

  // Older versions of Windows Explorer don't like forward slashes
  pathA.Replace('/','\\');

  if (attrs != INVALID_FILE_ATTRIBUTES)
  {
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
      // For directories, open an Explorer window
      ::ShellExecute(0,"explore",pathA,NULL,NULL,SW_SHOWNORMAL);
    }
    else
    {
      // For files, open them with the default action
      SHELLEXECUTEINFO exec;
      ::ZeroMemory(&exec,sizeof exec);
      exec.cbSize = sizeof exec;
      exec.fMask = SEE_MASK_FLAG_NO_UI;
      exec.hwnd = m_html->GetParentFrame()->GetSafeHwnd();
      exec.lpVerb = "open";
      exec.lpFile = pathA;
      exec.nShow = SW_SHOWNORMAL;
      if (!::ShellExecuteEx(&exec))
      {
        // If there is no default association, let the user choose
        if (::GetLastError() == ERROR_NO_ASSOCIATION)
        {
          exec.fMask &= ~SEE_MASK_FLAG_NO_UI;
          exec.lpVerb = "openas";
          ::ShellExecuteEx(&exec);
        }
      }
    }
  }
}

void ScriptProject::OpenUrl(LPCWSTR url)
{
  // Open an Explorer window
  CString urlA(url);
  ::ShellExecute(0,0,urlA,NULL,NULL,SW_SHOWNORMAL);
}

// Internet Explorer window, used to set the context menu

IMPLEMENT_DYNAMIC(IEWnd, CWnd)

BEGIN_MESSAGE_MAP(IEWnd, CWnd)
  ON_MESSAGE(WM_INITMENUPOPUP, OnInitMenuPopup)
END_MESSAGE_MAP()

IEWnd::IEWnd(ReportHtml* html, HMENU menu, UINT zoomId)
  : m_html(html), m_menu(menu), m_zoomId(zoomId)
{
}

LRESULT IEWnd::OnInitMenuPopup(WPARAM, LPARAM)
{
  Panel* panel = Panel::GetPanel(this);
  LRESULT result = Default();

  ::EnableMenuItem(m_menu,ID_NAVIGATE_BACK,
    (panel->CanTabNavigate(false) ? MF_ENABLED : MF_GRAYED)|MF_BYCOMMAND);
  ::EnableMenuItem(m_menu,ID_NAVIGATE_FORE,
    (panel->CanTabNavigate(true) ? MF_ENABLED : MF_GRAYED)|MF_BYCOMMAND);

  OLECMDF cmdf = m_html->QueryStatusWB(OLECMDID_COPY);
  ::EnableMenuItem(m_menu,ID_EDIT_COPY,
    ((cmdf & OLECMDF_ENABLED) ? MF_ENABLED : MF_GRAYED)|MF_BYCOMMAND);
  ::EnableMenuItem(m_menu,ID_EDIT_SELECT_ALL,MF_ENABLED|MF_BYCOMMAND);

  for (UINT id = ID_ZOOM_SMALLEST; id <= ID_ZOOM_LARGEST; id++)
  {
    ::EnableMenuItem(m_menu,id,MF_ENABLED|MF_BYCOMMAND);
    if (id == m_zoomId)
      ::CheckMenuItem(m_menu,id,MF_CHECKED|MF_BYCOMMAND);
    else
      ::CheckMenuItem(m_menu,id,MF_UNCHECKED|MF_BYCOMMAND);
  }

  ::EnableMenuItem(m_menu,ID_FILE_PRINT,MF_ENABLED|MF_BYCOMMAND);
  ::EnableMenuItem(m_menu,ID_MENU_PROPERTIES,MF_ENABLED|MF_BYCOMMAND);

  return result;
}
