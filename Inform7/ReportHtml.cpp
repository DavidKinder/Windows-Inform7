#include "stdafx.h"
#include "ReportHtml.h"
#include "ExtensionFrame.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"
#include "TextFormat.h"

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Settings for all browser instances
CefSettings cefSettings;
CefBrowserSettings cefBrowserSettings;

// Implementation of inform: handlers
class I7SchemeHandler : public CefResourceHandler
{
public:
  I7SchemeHandler() : m_data(NULL), m_dataLen(0), m_dataOffset(0)
  {
  }

  ~I7SchemeHandler()
  {
    delete[] m_data;
  }

  bool Open(CefRefPtr<CefRequest> request, bool& handle_request, CefRefPtr<CefCallback>)
  {
    ASSERT(m_data == NULL);
    ASSERT(m_dataLen == 0);

    handle_request = true;
    CString path = ConvertUrlToPath(request->GetURL().ToString().c_str());
    if (!path.IsEmpty())
    {
      CFileStatus status;
      if (CFile::GetStatus(path,status))
      {
        CFile reqFile;
        if (reqFile.Open(path,CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite))
        {
          m_dataLen = status.m_size;
          m_data = new char[m_dataLen];
          int64 read = reqFile.Read(m_data,m_dataLen);
          ASSERT(read == m_dataLen);
          reqFile.Close();

          CString ext = ::PathFindExtension(path);
          ext.MakeLower();
          if (ext == ".gif")
            m_dataType = "image/gif";
          else if ((ext == ".jpg") || (ext == ".jpeg"))
            m_dataType = "image/jpeg";
          else if (ext == ".png")
            m_dataType = "image/png";
          else if ((ext == ".tif") || (ext == ".tiff"))
            m_dataType = "image/tiff";
          else
            m_dataType = "text/html";
          return true;
        }
      }
    }
    return false;
  }

  void GetResponseHeaders(
    CefRefPtr<CefResponse> response, int64& response_length, CefString&)
  {
    response->SetMimeType(m_dataType);
    response->SetStatus(200);
    response_length = m_dataLen;
  }

  void Cancel()
  {
  }

  bool Read(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefResourceReadCallback>)
  {
    if (m_dataOffset < m_dataLen)
    {
      int64 transfer = m_dataLen - m_dataOffset;
      if (transfer > bytes_to_read)
        transfer = bytes_to_read;

      memcpy(data_out,m_data + m_dataOffset,transfer);
      m_dataOffset += transfer;
      bytes_read = transfer;
      return true;
    }
    bytes_read = 0;
    return false;
  }

  static CString ConvertUrlToPath(const char* url)
  {
    if (strncmp(url,"inform:",7) != 0)
      return false;

    CString appDir = theApp.GetAppDir();
    CStringW fileName = TextFormat::UTF8ToUnicode(Unescape(url+8));
    fileName.TrimRight(L"/");

    static const char* dirs[] =
    {
      "\\Documentation",
      "\\Documentation\\doc_images",
      "\\Documentation\\sections",
      "\\Images"
    };

    for (int i = 0; i < sizeof dirs / sizeof dirs[0]; i++)
    {
      CString path;
      path.Format("%s%s%S",appDir.GetString(),dirs[i],fileName.GetString());
      if (FileExists(path))
        return path;
    }

    if (fileName.Left(11) == L"/extensions")
    {
      fileName = fileName.Mid(11);

      CString path;
      path.Format("%s\\Inform\\Documentation%S",
        theApp.GetHomeDir().GetString(),fileName.GetString());
      if (FileExists(path))
        return path;

      path.Format("%s\\Documentation%S",appDir.GetString(),fileName.GetString());
      if (FileExists(path))
        return path;
    }

    ASSERT(FALSE);
    return "";
  }

private:
  IMPLEMENT_REFCOUNTING(I7SchemeHandler);

  static CString Unescape(const char* input)
  {
    int len = (int)strlen(input);
    CString output;
    output.Preallocate(len);

    static const char* hex = "0123456789ABCDEF";

    int i = 0;
    while (i < len)
    {
      if ((input[i] == L'%') && (i < len-2))
      {
        const char* hex1 = strchr(hex,toupper(input[i+1]));
        const char* hex2 = strchr(hex,toupper(input[i+2]));
        if ((hex1 != NULL) && (hex2 != NULL))
        {
          char ch = ((hex1-hex)<<4)+(hex2-hex);
          output.AppendChar(ch);
          i += 3;
          continue;
        }
      }
      output.AppendChar(input[i++]);
    }
    return output;
  }

  static bool FileExists(const char* path)
  {
    return (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
  }

private:
  char* m_data;
  int64 m_dataLen;
  int64 m_dataOffset;
  std::string m_dataType;
};

// Application level callbacks for all browser instances
class I7CefApp : public CefApp,
  public CefBrowserProcessHandler,
  public CefSchemeHandlerFactory
{
public:
  I7CefApp()
  {
  }

  void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
  {
    registrar->AddCustomScheme(
      "inform",CEF_SCHEME_OPTION_STANDARD|CEF_SCHEME_OPTION_CORS_ENABLED);
  }

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
  {
    return this;
  }

  void OnContextInitialized()
  {
    CefRegisterSchemeHandlerFactory("inform","",this);
  }

  CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser>,
    CefRefPtr<CefFrame>, const CefString&, CefRefPtr<CefRequest>)
  {
    return new I7SchemeHandler();
  }

private:
  IMPLEMENT_REFCOUNTING(I7CefApp);
};

// Handler implementations for each browser instance
class I7CefClient : public CefClient, public CefRequestHandler, public CefLoadHandler
{
public:
  I7CefClient()
  {
  }

  void SetObject(ReportHtml* obj)
  {
    m_object = obj;
  }

  CefRefPtr<CefRequestHandler> GetRequestHandler()
  {
    return this;
  }

  bool OnBeforeBrowse(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>,
    CefRefPtr<CefRequest> request, bool user_gesture, bool)
  {
    if (m_object)
      return m_object->OnBeforeBrowse(
        request->GetURL().ToString().c_str(),user_gesture);
    return false;
  }

  CefRefPtr<CefLoadHandler> GetLoadHandler()
  {
    return this;
  }

  void OnLoadError(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, ErrorCode, const CefString&,
    const CefString& failedUrl)
  {
    if (m_object)
      return m_object->OnLoadError(failedUrl.ToString().c_str());
  }

private:
  IMPLEMENT_REFCOUNTING(I7CefClient);

  ReportHtml* m_object;
};

// Private implementation data not exposed in the class header file
struct ReportHtml::Private : public CefRefPtr<CefBrowser>
{
  CefRefPtr<I7CefClient> client;
  CefRefPtr<CefBrowser> browser;
};

IMPLEMENT_DYNCREATE(ReportHtml, CWnd)

static std::string GetUTF8Path(const char* root, const char* path)
{
  CString fullPath;
  fullPath.Format("%s%s",root,path);
  CString utf8Path = TextFormat::UnicodeToUTF8(CStringW(fullPath));
  return utf8Path.GetString();
}

bool ReportHtml::InitWebBrowser(void)
{
  // If this is a CEF sub-process, call CEF straight away
  CefMainArgs cefArgs(::GetModuleHandle(0));
  CefRefPtr<CefApp> app(new I7CefApp());
  if (CefExecuteProcess(cefArgs,app.get(),NULL) >= 0)
    return false;

  // Initialize settings
  cefSettings.no_sandbox = true;
  cefSettings.command_line_args_disabled = true;
  CString dir = theApp.GetAppDir();
  CefString(&cefSettings.resources_dir_path).FromString(GetUTF8Path(dir,"\\Chrome"));
  CefString(&cefSettings.locales_dir_path).FromString(GetUTF8Path(dir,"\\Chrome\\locales"));
  dir = theApp.GetHomeDir() + "\\Inform\\ceflog.txt";
  ::DeleteFile(dir);
  CefString(&cefSettings.log_file).FromString(GetUTF8Path(dir,""));

  // Initialize CEF
  if (!CefInitialize(cefArgs,cefSettings,app.get(),NULL))
  {
    AfxMessageBox("Failed to initialize Chrome Extension Framework",MB_ICONSTOP|MB_OK);
    exit(0);
  }
  return true;
}

void ReportHtml::ShutWebBrowser(void)
{
  CefShutdown();
}

void ReportHtml::DoWebBrowserWork(void)
{
  for (int i = 0; i < 10; i++)
    CefDoMessageLoopWork();
}

void ReportHtml::UpdateWebBrowserPreferences(void)
{
}

ReportHtml::ReportHtml()
{
  m_private = new Private();
}

ReportHtml::~ReportHtml()
{
  Detach();
  if (m_private->client.get())
    m_private->client->SetObject(NULL);
  delete m_private;
}

BOOL ReportHtml::Create(LPCSTR, LPCSTR, DWORD style,
  const RECT& rect, CWnd* parentWnd, UINT id, CCreateContext*)
{
  CefWindowInfo windowInfo;
  windowInfo.SetAsChild(parentWnd->GetSafeHwnd(),rect);
  windowInfo.style = style;
  windowInfo.menu = (HMENU)(UINT_PTR)id;

  CefRefPtr<I7CefClient> client(new I7CefClient());
  CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
    windowInfo,client,"",cefBrowserSettings,NULL,NULL);
  if (browser.get() == NULL)
    return FALSE;

  m_private->client = client;
  client->SetObject(this);
  m_private->browser = browser;
  Attach(browser->GetHost()->GetWindowHandle());
  return TRUE;
}

void ReportHtml::Navigate(const char* url, bool focus, const wchar_t* find)
{
  // Stop any current page loading
  if (m_private->browser->IsLoading())
    m_private->browser->StopLoad();

  m_url = url;
  m_private->browser->GetMainFrame()->LoadURL(url);
}

CString ReportHtml::GetURL(void)
{
  return m_url;
}

void ReportHtml::Refresh(void)
{
  m_private->browser->Reload();
}

bool ReportHtml::OnBeforeBrowse(const char* url, bool user)
{
  if (m_consumer)
  {
    if (strncmp(url,"source:",7) == 0)
    {
      // Got a source: URL
      m_consumer->SourceLink(url);
      return true;
    }
    else if (strncmp(url,"library:",8) == 0)
    {
      // Got a library: URL
      m_consumer->LibraryLink(url);
      return true;
    }
    else if (strncmp(url,"skein:",6) == 0)
    {
      // Got a skin: URL
      m_consumer->SkeinLink(url);
      return true;
    }
    else if (strncmp(url,"inform:",7) == 0)
    {
      // Got an inform: documentation URL
      CString path = I7SchemeHandler::ConvertUrlToPath(url);
      if (m_consumer->DocLink(theApp.PathToUrl(path)))
        return true;
    }
  }

  // Open links to the Int-Fiction forum (from the Public Library) in a browser
  if (strncmp(url,"http://www.intfiction.org/",26) == 0)
  {
    ::ShellExecute(0,NULL,url,NULL,NULL,SW_SHOWNORMAL);
    return true;
  }

  // If this navigation is due to the user, tell the parent window that the URL has changed
  if (user)
  {
    if (strncmp(url,"javascript:",11) != 0)
    {
      m_url = url;
      GetParent()->PostMessage(WM_USERNAVIGATE);
    }
  }
  return false;
}

void ReportHtml::OnLoadError(const char* url)
{
  if (m_consumer)
    m_consumer->LinkError(url);
}

void ReportHtml::SetLinkConsumer(LinkConsumer* consumer)
{
  m_consumer = consumer;
}

///////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(ReportHtml, CWnd)
/*
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
  ON_COMMAND(ID_EDIT_FIND, OnEditFind)
  ON_WM_TIMER()
*/
END_MESSAGE_MAP()

/*
void ReportHtml::OnDocumentComplete(LPCTSTR lpszURL)
{
  CHtmlView::OnDocumentComplete(lpszURL);

  // Make this the active window, except for blank URLs
  if (m_setFocus && (strcmp(lpszURL,"about:blank") != 0))
    SetFocusOnContent();

  // Highlight found text
  if (strchr(lpszURL,'#') != NULL)
  {
    HighlightFound(false);
    m_findTimer = 5;
    SetTimer(1,200,NULL);
  }
  else
  {
    HighlightFound(true);
    m_find.Empty();
  }
}

HRESULT ReportHtml::OnShowContextMenu(DWORD dwID,  LPPOINT ppt, LPUNKNOWN pcmdTarget, LPDISPATCH)
{
  // Get COM interfaces
  CComQIPtr<IOleWindow> oleWindow(pcmdTarget);
  if (oleWindow == NULL)
    return S_FALSE;

  // Get the window to use as the menu's parent
  HWND window;
  if (FAILED(oleWindow->GetWindow(&window)))
    return S_FALSE;

  // Get the context menu
  CMenu menu;
  menu.LoadMenu(IDR_HTMLMENU);

  // Subclass the IE window to catch menu messages
  IEWnd ieWnd(this,menu);
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

void ReportHtml::SetIEPreferences(const char* path)
{
    CRegKey scriptsKey;
    if (scriptsKey.Create(webKey,"International\\Scripts\\3") == ERROR_SUCCESS)
    {
      // Set the font names
      scriptsKey.SetStringValue("IEPropFontName",theApp.GetFontName(InformApp::FontDisplay));
      scriptsKey.SetStringValue("IEFixedFontName",theApp.GetFontName(InformApp::FontFixedWidth));

      // Set the approximate font size
      int ptSize = theApp.GetFontSize(InformApp::FontDisplay);
      if (ptSize >= 14)
        scriptsKey.SetDWORDValue("IEFontSize",4);
      else if (ptSize >= 12)
        scriptsKey.SetDWORDValue("IEFontSize",3);
      else if (ptSize >= 10)
        scriptsKey.SetDWORDValue("IEFontSize",2);
      else if (ptSize >= 9)
        scriptsKey.SetDWORDValue("IEFontSize",1);
      else
        scriptsKey.SetDWORDValue("IEFontSize",0);
    }
  }

  // If the registry path hasn't been changed, this is a preferences update,
  // so cause all Internet Explorer windows to update
  if (path == NULL)
  {
    DWORD_PTR result;
    SendMessageTimeout(HWND_BROADCAST,
      WM_SETTINGCHANGE,0x1F,(LPARAM)"Software\\Microsoft\\Internet Explorer",SMTO_BLOCK,1000,&result);
  }
}
*/

void ReportHtml::SetFocusOnContent(void)
{
/*
  CPoint point(0,0);
  ClientToScreen(&point);
  CWnd* wnd = WindowFromPoint(point);
  if (wnd != NULL)
    wnd->SetFocus();
*/
}

void ReportHtml::SetFocusFlag(bool focus)
{
//  m_setFocus = focus;
}

/*
void ReportHtml::Navigate(const char* url, bool focus, const wchar_t* find)
{
  m_setFocus = focus;

  if (find != NULL)
    m_find = find;
  else
    m_find.Empty();
}
*/

void ReportHtml::Invoke(LPCWSTR method, VARIANT* arg)
{
/*
  IDispatch* disp = GetHtmlDocument();
  CComQIPtr<IHTMLDocument> doc(disp);
  disp->Release();

  CComPtr<IDispatch> script;
  doc->get_Script(&script);
  script.Invoke1(method,arg);
*/
}

/*
void ReportHtml::HighlightFound(bool goToFound)
{
  if (m_find.IsEmpty())
    return;

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

  bool first = goToFound;
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
}

void ReportHtml::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == 1)
  {
    HighlightFound(false);

    if (m_findTimer > 0)
      m_findTimer--;
    if (m_findTimer <= 0)
    {
      // Don't highlight any more
      KillTimer(1);
      m_find.Empty();
    }
  }
}

// COM object for the Javascript project object

BEGIN_DISPATCH_MAP(ScriptProject,CCmdTarget)
  DISP_FUNCTION(ScriptProject,"selectView",SelectView,VT_EMPTY,VTS_BSTR)
  DISP_FUNCTION(ScriptProject,"pasteCode",PasteCode,VT_EMPTY,VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"createNewProject",CreateNewProject,VT_EMPTY,VTS_WBSTR VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"openFile",OpenFile,VT_EMPTY,VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"openUrl",OpenUrl,VT_EMPTY,VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"askInterfaceForLocalVersion",ExtCompareVersion,VT_BSTR,VTS_WBSTR VTS_WBSTR VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"askInterfaceForLocalVersionText",ExtGetVersion,VT_BSTR,VTS_WBSTR VTS_WBSTR)
  DISP_FUNCTION(ScriptProject,"downloadMultipleExtensions",ExtDownload,VT_EMPTY,VTS_VARIANT)
END_DISPATCH_MAP()

void ScriptProject::SelectView(LPCSTR view)
{
  m_html->GetParentFrame()->SendMessage(
    WM_SELECTVIEW,(WPARAM)view,(LPARAM)m_html->GetSafeHwnd());
}

void ScriptProject::PasteCode(LPCWSTR code)
{
  CStringW theCode = UnescapeUnicode(code);
  m_html->GetParentFrame()->SendMessage(WM_PASTECODE,(WPARAM)(LPCWSTR)theCode);
}

void ScriptProject::CreateNewProject(LPCWSTR title, LPCWSTR code)
{
  CStringW theTitle = UnescapeUnicode(title);
  CStringW theCode = UnescapeUnicode(code);
  m_html->GetParentFrame()->SendMessage(WM_NEWPROJECT,
    (WPARAM)(LPCWSTR)theCode,(LPARAM)(LPCWSTR)theTitle);
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

BSTR ScriptProject::ExtCompareVersion(LPCWSTR author, LPCWSTR title, LPCWSTR compare)
{
  const InformApp::ExtLocation* ext = theApp.GetExtension(CString(author),CString(title));
  if (ext != NULL)
  {
    if (ext->system)
      return ::SysAllocString(L"!");

    CStringW extLine = ExtensionFrame::ReadExtensionFirstLine(ext->path.c_str());
    if (!extLine.IsEmpty())
    {
      CStringW extName, extAuthor, extVersion;
      if (ExtensionFrame::IsValidExtension(extLine,extName,extAuthor,extVersion))
      {
        int extNumber, compareNumber;
        if (swscanf(extVersion,L"Version %d",&extNumber) == 1)
        {
          if (swscanf(compare,L"Version %d",&compareNumber) == 1)
          {
            if (extNumber < compareNumber)
              return ::SysAllocString(L"<");
            else if (extNumber > compareNumber)
              return ::SysAllocString(L">");
          }
        }

        int c = extVersion.Compare(compare);
        if (c == 0)
          return ::SysAllocString(L"=");
        else if (c < 0)
          return ::SysAllocString(L"<");
        else
          return ::SysAllocString(L">");
      }
    }
  }
  return ::SysAllocString(L"");
}

BSTR ScriptProject::ExtGetVersion(LPCWSTR author, LPCWSTR title)
{
  const InformApp::ExtLocation* ext = theApp.GetExtension(CString(author),CString(title));
  if (ext != NULL)
  {
    CStringW extLine = ExtensionFrame::ReadExtensionFirstLine(ext->path.c_str());
    if (!extLine.IsEmpty())
    {
      CStringW extName, extAuthor, extVersion;
      if (ExtensionFrame::IsValidExtension(extLine,extName,extAuthor,extVersion))
        return extVersion.AllocSysString();
    }
  }
  return ::SysAllocString(L"");
}

void ScriptProject::ExtDownload(VARIANT& extArray)
{
  if (extArray.vt != VT_DISPATCH)
    return;

  DISPID lengthId;
  OLECHAR* lengthName = L"length";
  if (FAILED(extArray.pdispVal->GetIDsOfNames(IID_NULL,&lengthName,1,LOCALE_SYSTEM_DEFAULT,&lengthId)))
    return;

  COleDispatchDriver driver(extArray.pdispVal,FALSE);
  long length = 0;
  driver.GetProperty(lengthId,VT_I4,&length);
  if (length <= 0)
    return;

  CStringArray* libraryUrls = new CStringArray();
  for (long i = 0; i < length; i += 3)
  {
    DISPID indexId;
    CStringW indexName;
    indexName.Format(L"%d",i+1);
    LPCWSTR indexStr = indexName;
    if (SUCCEEDED(extArray.pdispVal->GetIDsOfNames(IID_NULL,(LPOLESTR*)&indexStr,1,LOCALE_SYSTEM_DEFAULT,&indexId)))
    {
      CString libraryUrl;
      driver.GetProperty(indexId,VT_BSTR,&libraryUrl);
      libraryUrls->Add(libraryUrl);
    }
  }
  m_html->GetParentFrame()->PostMessage(WM_EXTDOWNLOAD,(WPARAM)libraryUrls);
}

CStringW ScriptProject::UnescapeUnicode(LPCWSTR input)
{
  size_t len = wcslen(input);
  CStringW output;
  output.Preallocate((int)len);
  for (size_t i = 0; i < len; i++)
  {
    wchar_t c = input[i];
    if (c == '[')
    {
      int unicode = 0;
      if (swscanf(input+i,L"[=0x%x=]",&unicode) == 1)
      {
        output.AppendChar((wchar_t)unicode);
        i += 9;
        continue;
      }
    }
    output.AppendChar(c);
  }
  return output;
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

  ::EnableMenuItem(m_menu,ID_FILE_PRINT,MF_ENABLED|MF_BYCOMMAND);
  ::EnableMenuItem(m_menu,ID_MENU_PROPERTIES,MF_ENABLED|MF_BYCOMMAND);

  return result;
  return 0;
}
*/
