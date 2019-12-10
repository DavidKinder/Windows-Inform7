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

// Test whether the given file exists
static bool FileExists(const char* path)
{
  return (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
}

// Implementation of inform: handler
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

  // Open an inform: request. If the file exists read it in and handle the request
  // straight away.
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

  // Show the resource as ready and set the appropriate MIME type
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

  // Copy out part of the file that we have read in Open()
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

  // Convert an inform: URL to a file path
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

  // Remove HTML-like escapes (e.g. "%20") from the input string
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

private:
  char* m_data;
  int64 m_dataLen;
  int64 m_dataOffset;
  std::string m_dataType;
};

// Implementation of handler for JavaScript calls
class I7JavaScriptHandler : public CefV8Handler
{
public:
  // The handler constructor takes a reference to the frame for sending process messages
  I7JavaScriptHandler(CefRefPtr<CefFrame> frame) : m_frame(frame)
  {
  }

  // Handle executing one of our JavaScript callback functions. Those that cannot be implemented
  // in the render process are passed to the browser process via a process message.
  bool Execute(const CefString& name, CefRefPtr<CefV8Value>,
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString&)
  {
    if (name == "selectView")
      SendMessage("I7.selectView",arguments);
    else if (name == "pasteCode")
      SendMessage("I7.pasteCode",arguments);
    else if (name == "createNewProject")
      SendMessage("I7.createNewProject",arguments);
    else if (name == "openFile")
      SendMessage("I7.openFile",arguments);
    else if (name == "openUrl")
    {
      // We don't need anything from the browser process for this,
      // so just pass the URL to the shell.
      std::string url = GetStringArgument(arguments,0).ToString();
      ::ShellExecute(0,0,url.c_str(),NULL,NULL,SW_SHOWNORMAL);
    }
    else if (name == "askInterfaceForLocalVersion")
    {
      CefString author = GetStringArgument(arguments,0);
      CefString title = GetStringArgument(arguments,1);
      CefString compare = GetStringArgument(arguments,2);
      CefString result = AskForLocalVersion(
        author.ToString().c_str(),title.ToString().c_str(),compare.c_str());
      retval = CefV8Value::CreateString(result);
    }
    else if (name == "askInterfaceForLocalVersionText")
    {
      CefString author = GetStringArgument(arguments,0);
      CefString title = GetStringArgument(arguments,1);
      CefString result = AskForLocalVersionText(
        author.ToString().c_str(),title.ToString().c_str());
      retval = CefV8Value::CreateString(result);
    }
    else if (name == "downloadMultipleExtensions")
    {
      if (arguments.size() == 1)
      {
        CefRefPtr<CefV8Value> argument = arguments[0];
        if (argument->IsArray())
        {
          CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("I7.downloadExts");
          CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
          int msgArgIdx = 0;
          for (int i = 0; i < argument->GetArrayLength(); i += 3)
          {
            CefRefPtr<CefV8Value> element = argument->GetValue(i+1);
            if (element.get() && element->IsString())
              msgArgs->SetString(msgArgIdx++,element->GetStringValue());
          }
          m_frame->SendProcessMessage(PID_BROWSER,msg);
        }
      }
    }
    return true;
  }

private:
  // Given a name and a list of JavaScript arguments, send a process message to the
  // browser process containing those arguments. Only strings are currently supported.
  void SendMessage(const char* name, const CefV8ValueList& arguments)
  {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(name);
    CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
    int msgArgIdx = 0;
    for (int i = 0; i < arguments.size(); i++)
    {
      CefRefPtr<CefV8Value> argument = arguments[i];
      if (argument->IsString())
        msgArgs->SetString(msgArgIdx++,argument->GetStringValue());
    }
    m_frame->SendProcessMessage(PID_BROWSER,msg);
  }

  // Return the string form of a JavaScript function call argument
  CefString GetStringArgument(const CefV8ValueList& arguments, int index)
  {
    if (arguments.size() > index)
    {
      CefRefPtr<CefV8Value> argument = arguments[index];
      if (argument->IsString())
        return argument->GetStringValue();
    }
    return "";
  }

  // Find the installed extension, if present, and compare its version against
  // the supplied version.
  CefString AskForLocalVersion(const char* author, const char* title, LPCWSTR compare)
  {
    CString path;
    path.Format("%s\\Internal\\Extensions\\%s\\%s.i7x",(LPCSTR)theApp.GetAppDir(),author,title);
    if (FileExists(path))
      return "!";

    path.Format("%s\\Inform\\Extensions\\%s\\%s.i7x",(LPCSTR)theApp.GetHomeDir(),author,title);
    if (FileExists(path))
    {
      CStringW extLine = ExtensionFrame::ReadExtensionFirstLine(path);
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
                return "<";
              else if (extNumber > compareNumber)
                return ">";
            }
          }

          int c = extVersion.Compare(compare);
          if (c == 0)
            return "=";
          else if (c < 0)
            return "<";
          else
            return ">";
        }
      }
    }
    return "";
  }

  // Find the installed extension, if present, and return its version string.
  CefString AskForLocalVersionText(const char* author, const char* title)
  {
    // Find the extension
    CString path;
    path.Format("%s\\Internal\\Extensions\\%s\\%s.i7x",(LPCSTR)theApp.GetAppDir(),author,title);
    if (!FileExists(path))
    {
      path.Format("%s\\Inform\\Extensions\\%s\\%s.i7x",(LPCSTR)theApp.GetHomeDir(),author,title);
      if (!FileExists(path))
        return "";
    }

    CStringW extLine = ExtensionFrame::ReadExtensionFirstLine(path);
    if (!extLine.IsEmpty())
    {
      CStringW extName, extAuthor, extVersion;
      if (ExtensionFrame::IsValidExtension(extLine,extName,extAuthor,extVersion))
        return (LPCWSTR)extVersion;
    }
    return "";
  }

  IMPLEMENT_REFCOUNTING(I7JavaScriptHandler);

  CefRefPtr<CefFrame> m_frame;
};

// Application level callbacks for all browser instances
class I7CefApp : public CefApp,
  public CefBrowserProcessHandler,
  public CefRenderProcessHandler,
  public CefSchemeHandlerFactory
{
public:
  I7CefApp()
  {
  }

  // Register our custom inform: scheme
  void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
  {
    registrar->AddCustomScheme(
      "inform",CEF_SCHEME_OPTION_STANDARD|CEF_SCHEME_OPTION_CORS_ENABLED);
  }

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
  {
    return this;
  }

  // Implement CefBrowserProcessHandler to register our custom inform: scheme
  void OnContextInitialized()
  {
    CefRegisterSchemeHandlerFactory("inform","",this);
  }

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
  {
    return this;
  }

  // Implement CefRenderProcessHandler to add JavaScript function callbacks
  void OnContextCreated(CefRefPtr<CefBrowser>,
    CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
  {
    // Create an object with our JavaScript methods
    CefRefPtr<CefV8Value> obj = CefV8Value::CreateObject(NULL,NULL);
    CefRefPtr<CefV8Handler> handler = new I7JavaScriptHandler(frame);
    AddMethod(obj,"selectView",handler);
    AddMethod(obj,"pasteCode",handler);
    AddMethod(obj,"createNewProject",handler);
    AddMethod(obj,"openUrl",handler);
    AddMethod(obj,"askInterfaceForLocalVersion",handler);
    AddMethod(obj,"askInterfaceForLocalVersionText",handler);
    AddMethod(obj,"downloadMultipleExtensions",handler);

    // Add our object as 'window.Project'
    context->GetGlobal()->SetValue("Project",obj,V8_PROPERTY_ATTRIBUTE_NONE);

    // For backward compatability, also add as 'window.external.Project'
    CefRefPtr<CefV8Value> extObj = CefV8Value::CreateObject(NULL,NULL);
    extObj->SetValue("Project",obj,V8_PROPERTY_ATTRIBUTE_NONE);
    context->GetGlobal()->SetValue("external",extObj,V8_PROPERTY_ATTRIBUTE_NONE);
  }

  CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser>,
    CefRefPtr<CefFrame>, const CefString&, CefRefPtr<CefRequest>)
  {
    return new I7SchemeHandler();
  }

private:
  // Add the named function to the given JavaScript object
  void AddMethod(CefRefPtr<CefV8Value> obj, const char* name, CefRefPtr<CefV8Handler> handler)
  {
    obj->SetValue(name,CefV8Value::CreateFunction(name,handler),V8_PROPERTY_ATTRIBUTE_NONE);
  }

  IMPLEMENT_REFCOUNTING(I7CefApp);
};

// Handler implementations for each browser instance
class I7CefClient : public CefClient, public CefRequestHandler, public CefLoadHandler
{
public:
  I7CefClient()
  {
  }

  // Set the application callback object
  void SetObject(ReportHtml* obj)
  {
    m_object = obj;
  }

  // Implement CefClient to handle process messages from the renderer process
  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame>, CefProcessId, CefRefPtr<CefProcessMessage> message)
  {
    if (CefCurrentlyOn(TID_UI))
    {
      if (m_object)
      {
        // Implement our JavaScript callback functions that affect the front-end
        CefString name = message->GetName();
        if (name == "I7.selectView")
        {
          std::string view = GetStringArgument(message,0).ToString();
          m_object->GetParentFrame()->SendMessage(WM_SELECTVIEW,
            (WPARAM)view.c_str(),(LPARAM)m_object->GetSafeHwnd());
        }
        else if (name == "I7.pasteCode")
        {
          CStringW code = UnescapeUnicode(GetStringArgument(message,0).ToString().c_str());
          m_object->GetParentFrame()->SendMessage(WM_PASTECODE,(WPARAM)code.GetString());
        }
        else if (name == "I7.createNewProject")
        {
          CStringW title = UnescapeUnicode(GetStringArgument(message,0).ToString().c_str());
          CStringW code = UnescapeUnicode(GetStringArgument(message,1).ToString().c_str());
          m_object->GetParentFrame()->SendMessage(WM_NEWPROJECT,
            (WPARAM)(LPCWSTR)code,(LPARAM)(LPCWSTR)title);
        }
        else if (name == "I7.openFile")
        {
          std::string path = GetStringArgument(message,0).ToString();
          OpenFile(path.c_str());
        }
        else if (name == "I7.downloadExts")
        {
          CStringArray* libraryUrls = new CStringArray();
          for (int i = 0; i < message->GetArgumentList()->GetSize(); i++)
          {
            CString libraryUrl = GetStringArgument(message,i).ToString().c_str();
            libraryUrls->Add(libraryUrl);
          }
          m_object->GetParentFrame()->PostMessage(WM_EXTDOWNLOAD,(WPARAM)libraryUrls);
        }
      }
    }
    else
      ASSERT(FALSE);
    return false;
  }

  CefRefPtr<CefRequestHandler> GetRequestHandler()
  {
    return this;
  }

  // Implement CefRequestHandler to generate notifications on the page changing
  bool OnBeforeBrowse(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request, bool user_gesture, bool)
  {
    if (CefCurrentlyOn(TID_UI))
    {
      if (m_object && frame->IsMain())
        return m_object->OnBeforeBrowse(
          request->GetURL().ToString().c_str(),user_gesture);
    }
    else
      ASSERT(FALSE);
    return false;
  }

  CefRefPtr<CefLoadHandler> GetLoadHandler()
  {
    return this;
  }

  // Implement CefLoadHandler to generate notifications on a page load completing
  void OnLoadEnd(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame> frame, int)
  {
    if (CefCurrentlyOn(TID_UI))
    {
      if (m_object && frame->IsMain())
        return m_object->OnLoadEnd();
    }
    else
      ASSERT(FALSE);
  }

  // Implement CefLoadHandler to generate notifications on a page load error
  void OnLoadError(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame> frame,
    ErrorCode, const CefString&, const CefString& failedUrl)
  {
    if (CefCurrentlyOn(TID_UI))
    {
      if (m_object && frame->IsMain())
        return m_object->OnLoadError(failedUrl.ToString().c_str());
    }
    else
      ASSERT(FALSE);
  }

private:
  IMPLEMENT_REFCOUNTING(I7CefClient);

  // Given a process message, extract a string argument from it
  CefString GetStringArgument(CefRefPtr<CefProcessMessage> message, int index)
  {
    CefRefPtr<CefListValue> arguments = message->GetArgumentList();
    if (arguments->GetSize() > index)
    {
      if (arguments->GetType(index) == VTYPE_STRING)
        return arguments->GetString(index);
    }
    return "";
  }

  // Remove NI-generated escapes (e.g. "[=0x20=]") from the input string
  CStringW UnescapeUnicode(const char* input)
  {
    size_t len = strlen(input);
    CStringW output;
    output.Preallocate((int)len);
    for (size_t i = 0; i < len; i++)
    {
      char c = input[i];
      if (c == '[')
      {
        int unicode = 0;
        if (sscanf(input+i,"[=0x%x=]",&unicode) == 1)
        {
          output.AppendChar((char)unicode);
          i += 9;
          continue;
        }
      }
      output.AppendChar(c);
    }
    return output;
  }

  // Open the given file or directory in Windows Explorer
  void OpenFile(const char* path)
  {
    DWORD attrs = ::GetFileAttributes(path);
    if (attrs != INVALID_FILE_ATTRIBUTES)
    {
      if (attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
        // For directories, open an Explorer window
        ::ShellExecute(0,"explore",path,NULL,NULL,SW_SHOWNORMAL);
      }
      else
      {
        // For files, open them with the default action
        SHELLEXECUTEINFO exec;
        ::ZeroMemory(&exec,sizeof exec);
        exec.cbSize = sizeof exec;
        exec.fMask = SEE_MASK_FLAG_NO_UI;
        exec.hwnd = m_object->GetParentFrame()->GetSafeHwnd();
        exec.lpVerb = "open";
        exec.lpFile = path;
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

  ReportHtml* m_object;
};

// Private implementation data not exposed in the class header file
struct ReportHtml::Private : public CefRefPtr<CefBrowser>
{
  CefRefPtr<I7CefClient> client;
  CefRefPtr<CefBrowser> browser;
};

IMPLEMENT_DYNCREATE(ReportHtml, CWnd)

// Given a root and a path, combine the two into a UTF-8 encoded string
static std::string GetUTF8Path(const char* root, const char* path)
{
  CString fullPath;
  fullPath.Format("%s%s",root,path);
  CString utf8Path = TextFormat::UnicodeToUTF8(CStringW(fullPath));
  return utf8Path.GetString();
}

// Initialize CEF
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

// Shut down CEF
void ReportHtml::ShutWebBrowser(void)
{
  CefShutdown();
}

// Do an iteration of the CEF message loop
void ReportHtml::DoWebBrowserWork(void)
{
  for (int i = 0; i < 10; i++)
  {
    // Do some work ...
    CefDoMessageLoopWork();

    // If the main window has gone, stop!
    if (AfxGetMainWnd() == NULL)
    {
      TRACE("Main window gone after CefDoMessageLoopWork(), shutting down\n");
      ::ExitProcess(0);
    }
  }
}

// Notify all web browser instances that preferences have been changed
void ReportHtml::UpdateWebBrowserPreferences(void)
{
}

ReportHtml::ReportHtml() : m_setFocus(false), m_consumer(NULL)
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

// Create a CEF instance and attach it to this object
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

// Navigate to the given URL
void ReportHtml::Navigate(const char* url, bool focus, const wchar_t* find)
{
  // Stop any current page loading
  if (m_private->browser->IsLoading())
    m_private->browser->StopLoad();

  m_url = url;
  m_setFocus = focus;
  m_private->browser->GetMainFrame()->LoadURL(url);
}

// Get the current URL for this control
CString ReportHtml::GetURL(void)
{
  return m_url;
}

// Refresh the currently loaded page
void ReportHtml::Refresh(void)
{
  m_private->browser->Reload();
}

// Execute the given JavaScript code
void ReportHtml::RunJavaScript(const char* code)
{
  m_private->browser->GetMainFrame()->ExecuteJavaScript(code,"",0);
}

// Notify any consumer of a navigation event
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

  if (strncmp(url,"http",4) == 0)
  {
    // Web links to anything other than the Public Library go to a separate browser
    if (strncmp(url,"http://www.emshort.com/pl",25) != 0)
    {
      ::ShellExecute(0,NULL,url,NULL,NULL,SW_SHOWNORMAL);
      return true;
    }
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

// Notify any consumer of a load completion event
void ReportHtml::OnLoadEnd(void)
{
  if (m_consumer)
    m_consumer->LinkDone();

  if (m_setFocus && (m_url != "about:blank"))
    SetFocusOnContent();
}

// Notify any consumer of a load error event
void ReportHtml::OnLoadError(const char* url)
{
  if (m_consumer)
    m_consumer->LinkError(url);
}

// Set the consumer of web link related events
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

BEGIN_MESSAGE_MAP(ReportHtml, CWnd)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
END_MESSAGE_MAP()

void ReportHtml::OnEditCopy()
{
  m_private->browser->GetMainFrame()->Copy();
}

void ReportHtml::OnEditSelectAll()
{
  m_private->browser->GetMainFrame()->SelectAll();
}

/*
void ReportHtml::OnDocumentComplete(LPCTSTR lpszURL)
{
  CHtmlView::OnDocumentComplete(lpszURL);

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

/*
void ReportHtml::Navigate(const char* url, bool focus, const wchar_t* find)
{
  if (find != NULL)
    m_find = find;
  else
    m_find.Empty();
}
*/

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
