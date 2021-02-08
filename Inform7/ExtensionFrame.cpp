#include "stdafx.h"
#include "ExtensionFrame.h"
#include "ProjectFrame.h"
#include "Inform.h"
#include "Messages.h"
#include "SourceSettings.h"
#include "TextFormat.h"
#include "NewDialogs.h"
#include "Dialogs.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ExtensionFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(ExtensionFrame, MenuBarFrameWnd)
  ON_WM_CREATE()
  ON_WM_ACTIVATE()
  ON_WM_CLOSE()
  ON_WM_SIZE()
  ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)

  ON_MESSAGE(WM_PROJECTEDITED, OnProjectEdited)

  ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
  ON_COMMAND(ID_FILE_SAVE, OnFileSave)
  ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)

  ON_UPDATE_COMMAND_UI(ID_WINDOW_LIST, OnUpdateWindowList)
  ON_COMMAND_RANGE(ID_WINDOW_LIST, ID_WINDOW_LIST+8, OnWindowList)
END_MESSAGE_MAP()

static UINT indicators[] =
{
  ID_SEPARATOR,
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM,
};

ExtensionFrame::ExtensionFrame()
{
}

int ExtensionFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (MenuBarFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  if (!CreateMenuBar(IDR_EXTFRAME,0))
    return -1;

  // Create the editing window
  if (!m_edit.Create(this,AFX_IDW_PANE_FIRST,theApp.GetColour(InformApp::ColourBack),true))
  {
    TRACE("Failed to create source edit control\n");
    return -1;
  }
  m_edit.SetFocus();

  // Set the application icon
  theApp.SetIcon(this);

  return 0;
}

BOOL ExtensionFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if (!MenuBarFrameWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}

void ExtensionFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  MenuBarFrameWnd::OnActivate(nState,pWndOther,bMinimized);

  if (nState == WA_ACTIVE)
  {
    // Only consider reopening if the extension has not been edited
    if (!m_edit.IsEdited())
    {
      CFileStatus status;
      if (CFile::GetStatus(m_extension,status))
      {
        // Reopen the extension if it has been changed by another application
        if (status.m_mtime > m_edit.GetFileTime())
          OpenFile(m_extension);
      }
    }

    // Set the focus on the extension's source
    m_edit.SetFocus();
  }
}

void ExtensionFrame::OnClose()
{
  if (IsProjectEdited() && IsUserExtension())
  {
    // Ask the user before discarding the extension
    CString msg;
    msg.Format(
      "Closing %s will discard any changes.\n"
      "Do you want to save this extension first?",(LPCSTR)GetDisplayName(false));
    switch (MessageBox(msg,INFORM_TITLE,MB_YESNOCANCEL|MB_ICONEXCLAMATION))
    {
    case IDYES:
      m_extension.IsEmpty() ? OnFileSaveAs() : OnFileSave();
      break;
    case IDNO:
      // Do nothing
      break;
    case IDCANCEL:
      // Don't close the window
      return;
    }
  }

  // If there are any secondary frame windows and this is the main frame,
  // promote one of the secondaries to be the new main frame.
  theApp.FrameClosing(this);
  MenuBarFrameWnd::OnClose();
}

BOOL ExtensionFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CPushRoutingFrame push(this);

  // First try the edit window
  if (m_edit.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through frame
  if (CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through application
  if (AfxGetApp()->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return FALSE;
}

void ExtensionFrame::OnUpdateFrameTitle(BOOL)
{
  CString title;
  title.Format("%s - %s",(LPCSTR)GetDisplayName(true),m_strTitle);
  AfxSetWindowText(GetSafeHwnd(),title);
}

void ExtensionFrame::GetMessageString(UINT nID, CString& rMessage) const
{
  if ((nID >= ID_WINDOW_LIST) && (nID <= ID_WINDOW_LIST+8))
  {
    CArray<CFrameWnd*> frames;
    theApp.GetWindowFrames(frames);

    int i = nID-ID_WINDOW_LIST;
    if (i < frames.GetSize())
    {
      if (frames[i]->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
      {
        rMessage.Format("Switch to the \"%s\" project",
          ((ProjectFrame*)frames[i])->GetDisplayName(false));
      }
      else if (frames[i]->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
      {
        rMessage.Format("Switch to the extension \"%s\"",
          ((ExtensionFrame*)frames[i])->GetDisplayName(false));
      }
      return;
    }
  }

  MenuBarFrameWnd::GetMessageString(nID,rMessage);
}

LRESULT ExtensionFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
  if ((wParam == AFX_IDS_IDLEMESSAGE) && (lParam == NULL))
  {
    static CString msg;
    if (msg.IsEmpty())
      msg.Format(AFX_IDS_IDLEMESSAGE,NI_BUILD);
    return MenuBarFrameWnd::OnSetMessageString(0,(LPARAM)(LPCSTR)msg);
  }
  return MenuBarFrameWnd::OnSetMessageString(wParam,lParam);
}

LRESULT ExtensionFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  MoveWindow((LPRECT)lparam,TRUE);
  return 0;
}

LRESULT ExtensionFrame::OnProjectEdited(WPARAM wparam, LPARAM lparam)
{
  DelayUpdateFrameTitle();
  return 0;
}

void ExtensionFrame::OnFileClose()
{
  SendMessage(WM_CLOSE);
}

void ExtensionFrame::OnUpdateFileSave(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(IsUserExtension());
}

void ExtensionFrame::OnFileSave()
{
  if (m_extension.IsEmpty())
    return;

  // Check we are not saving under the program directory
  if (!IsUserExtension())
  {
    MessageBox(
      "Extensions should be saved in the \"Inform\\Extensions\" area\n"
      "under \"My Documents\", and not where Inform 7 has been installed.",
      INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Choose a temporary file name to save under
  CString saveName = m_extension;
  saveName += ".save";

  {
    // Open the file for saving
    CFile extFile;
    if (extFile.Open(saveName,CFile::modeCreate|CFile::modeWrite) == FALSE)
    {
      CString msg;
      msg.Format("Failed to save extension to \n\"%s\"",(LPCSTR)m_extension);
      MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
      return;
    }

    // Save the extension from the edit control
    if (!m_edit.SaveFile(&extFile))
      return;
  }

  ::DeleteFile(m_extension);
  if (::MoveFile(saveName,m_extension))
  {
    DeleteOldExtension(m_extension);

    theApp.FindExtensions();
    theApp.SendAllFrames(InformApp::Extensions,0);
  }
}

void ExtensionFrame::OnFileSaveAs()
{
  // Ask the user for a new file name
  SimpleFileDialog dialog(FALSE,"i7x",m_extension,OFN_HIDEREADONLY|OFN_ENABLESIZING,
    "Inform extensions (*.i7x)|*.i7x|All Files (*.*)|*.*||",this);
  dialog.m_ofn.lpstrTitle = "Save the extension";
  if (dialog.DoModal() != IDOK)
    return;

  m_extension = dialog.GetPathName();
  OnFileSave();
}

void ExtensionFrame::OnUpdateWindowList(CCmdUI *pCmdUI)
{
  CMenu* windowMenu = GetMenu()->GetSubMenu(3);
  int maximum = 9;

  // Remove any existing items in the window list
  for (int i = 0; i < maximum; i++)
    windowMenu->RemoveMenu(ID_WINDOW_LIST+i,MF_BYCOMMAND);

  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);

  if (frames.GetSize() < maximum)
    maximum = (int)frames.GetSize();

  // Add a menu for each window frame open
  for (int i = 0; i < maximum; i++)
  {
    CString name, menu;

    if (frames[i]->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
      name = ((ProjectFrame*)frames[i])->GetDisplayName(true);
    else if (frames[i]->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
      name = ((ExtensionFrame*)frames[i])->GetDisplayName(true);

    menu.Format("&%d %s",i+1,(LPCSTR)name);

    UINT flags = (frames[i] == this) ? MF_CHECKED|MF_STRING : MF_STRING;
    windowMenu->AppendMenu(flags,ID_WINDOW_LIST+i,menu);
  }
}

void ExtensionFrame::OnWindowList(UINT nID)
{
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);

  int index = nID-ID_WINDOW_LIST;
  if ((index >= 0) && (index < frames.GetSize()))
    frames[index]->ActivateFrame();
}

void ExtensionFrame::StartNew(CWnd* parent, const ProjectSettings& settings)
{
  NewExtensionDialog dialog(parent);
  if (dialog.DoModal() != IDOK)
    return;

  CStringW initialCode;
  initialCode.Format(L"%S by %s begins here.\n\n%S ends here.\n",
    dialog.GetName(),dialog.GetAuthor(),dialog.GetName());

  ExtensionFrame* frame = NewFrame(settings);
  frame->m_extension = dialog.GetPath();

  frame->m_edit.PasteCode(initialCode);
}

void ExtensionFrame::StartExisting(const char* path, const ProjectSettings& settings)
{
  // Is the extension already open?
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    if (frames[i]->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
    {
      ExtensionFrame* extFrame = (ExtensionFrame*)frames[i];
      if (extFrame->m_extension.CompareNoCase(path) == 0)
      {
        extFrame->ActivateFrame();
        return;
      }
    }
  }

  if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
  {
    ExtensionFrame* frame = NewFrame(settings);
    frame->OpenFile(path);
  }
}

bool ExtensionFrame::StartHighlight(const char* url, COLORREF colour, const ProjectSettings& settings)
{
  // Try to decode the URL
  int line = 0;
  char urlPath[_MAX_PATH];
  if (sscanf(url,"source:%[^#]#line%d",urlPath,&line) == 2)
  {
    line--;
    if (line >= 0)
    {
      // Create a path from the decoded URL
      CString path(urlPath);
      path.Replace("%20"," ");

      // Check that the file exists
      if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
      {
        // Canonicalize the path
        CFileFind find;
        if (find.FindFile(path))
        {
          find.FindNextFile();
          path = find.GetFilePath();
        }

        // Check if the extension is already open
        CArray<CFrameWnd*> frames;
        theApp.GetWindowFrames(frames);
        for (int i = 0; i < frames.GetSize(); i++)
        {
          if (frames[i]->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
          {
            ExtensionFrame* extFrame = (ExtensionFrame*)frames[i];
            if (extFrame->m_extension.CompareNoCase(path) == 0)
            {
              // Show the window with the highlight
              extFrame->m_edit.Highlight(line,colour,true);
              extFrame->ShowWindow(SW_SHOW);
              return true;
            }
          }
        }

        // Open a new window
        ExtensionFrame* extFrame = NewFrame(settings);
        extFrame->OpenFile(path);

        // Highlight the text
        extFrame->m_edit.Highlight(line,colour,true);
      }
    }
    return true;
  }
  return false;
}

void ExtensionFrame::StartSelect(const char* path, const CHARRANGE& range, const ProjectSettings& settings)
{
  // Check if the extension is already open
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    if (frames[i]->IsKindOf(RUNTIME_CLASS(ExtensionFrame)))
    {
      ExtensionFrame* extFrame = (ExtensionFrame*)frames[i];
      if (extFrame->m_extension.CompareNoCase(path) == 0)
      {
        // Show the window with the highlight
        extFrame->m_edit.Select(range,true);
        extFrame->ShowWindow(SW_SHOW);
        return;
      }
    }
  }

  if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
  {
    // Open a new window
    ExtensionFrame* extFrame = NewFrame(settings);
    extFrame->OpenFile(path);

    // Select the text
    extFrame->m_edit.Select(range,true);
  }
}

void ExtensionFrame::InstallExtensions(CFrameWnd* parent)
{
  // Ask the user for one or more extensions
  SimpleFileDialog dialog(TRUE,"i7x",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT,
    "Inform extensions (*.i7x)|*.i7x|All Files (*.*)|*.*||",parent);
  dialog.m_ofn.lpstrTitle = "Select the extensions to install";
  dialog.m_ofn.nMaxFile = 16384;
  dialog.m_ofn.lpstrFile = (LPSTR)_alloca(dialog.m_ofn.nMaxFile);
  memset(dialog.m_ofn.lpstrFile,0,dialog.m_ofn.nMaxFile);
  if (dialog.DoModal() != IDOK)
    return;

  // Iterate over the selected extensions
  CStringArray paths;
  POSITION pos = dialog.GetStartPosition();
  while (pos != NULL)
    paths.Add(dialog.GetNextPathName(pos));
  InstallExtensions(parent,paths);
}

void ExtensionFrame::InstallExtensions(CFrameWnd* parent, CStringArray& paths)
{
  // Iterate over the extensions
  CStringW lastExt;
  int installed = 0, total = 0;
  for (int i = 0; i < paths.GetSize(); i++)
  {
    total++;

    // Get the first line of the extension
    const CString& path = paths.GetAt(i);
    CStringW extLine = ReadExtensionFirstLine(path);
    if (extLine.IsEmpty())
      continue;

    // Check for a valid extension
    CStringW extName, extAuthor, extVersion;
    if (!IsValidExtension(extLine,extName,extAuthor,extVersion))
    {
      CString msg;
      msg.Format(
        "The file \"%s\"\n"
        "does not seem to be an extension. Extensions should be\n"
        "saved as UTF-8 format text files, and should start with a\n"
        "line of one of these forms:\n\n"
        "<Extension> by <Author> begins here.\n"
        "Version <Version> of <Extension> by <Author> begins here.",
        (LPCSTR)path);
      parent->MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
      continue;
    }

    // Work out the path to copy the extension to
    CString target;
    target.Format("%s\\Inform\\Extensions\\%S",
      (LPCSTR)theApp.GetHomeDir(),(LPCWSTR)extAuthor);
    ::CreateDirectory(target,NULL);
    target.AppendFormat("\\%S.i7x",(LPCWSTR)extName);

    // Check if the extension already exists
    bool exists = (::GetFileAttributes(target) != INVALID_FILE_ATTRIBUTES);
    if (!exists)
    {
      CString oldTarget(target);
      if (RemoveI7X(oldTarget))
        exists = (::GetFileAttributes(oldTarget) != INVALID_FILE_ATTRIBUTES);
    }
    if (exists)
    {
      CString msg;
      msg.Format(
        "A version of the extension %S by %S is already installed.\n"
        "Do you want to overwrite the installed extension with this new one?",
        (LPCWSTR)extName,(LPCWSTR)extAuthor);
      if (parent->MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_YESNO) != IDYES)
        continue;
    }

    // Copy the extension
    if (::CopyFile(path,target,FALSE))
    {
      DeleteOldExtension(target);
      lastExt.Format(L"\"%s\" by %s (%s)",(LPCWSTR)extName,(LPCWSTR)extAuthor,(LPCWSTR)extVersion);
      installed++;
    }
  }

  // Update the extensions menu and documentation
  parent->SendMessage(WM_RUNCENSUS,1);
  ShowInstalledMessage(parent,installed,total,lastExt);
  theApp.FindExtensions();
  theApp.SendAllFrames(InformApp::Extensions,0);
}

// Implementation of IBindStatusCallback used to wait for the downloading of
// extensions to complete
class WaitForDownload : public IBindStatusCallback
{
public:
  WaitForDownload() : m_refCount(0)
  {
    m_timeout = ::GetTickCount() + 10000;
  }

  void waitForEnd(void)
  {
    theApp.RunMessagePump();
    while (m_refCount > 0)
    {
      theApp.RunMessagePump();
      ::Sleep(50);
    }
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    if (ppvObj == NULL)
      return E_INVALIDARG;

    if ((riid == IID_IUnknown) || (riid == IID_IBindStatusCallback))
    {
      *ppvObj = (LPVOID)this;
      AddRef();
      return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef(void)
  {
    return ::InterlockedIncrement(&m_refCount);
  }

  ULONG STDMETHODCALLTYPE Release(void)
  {
    return ::InterlockedDecrement(&m_refCount);
  }

  HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD, IBinding*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE GetPriority(LONG*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnLowResource(DWORD)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnProgress(ULONG, ULONG, ULONG, LPCWSTR)
  {
    if (::GetTickCount() > m_timeout)
      return E_ABORT;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hr, LPCWSTR)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO*)
  {
    if (grfBINDF)
    {
      // Always download, never use cache
      *grfBINDF |= BINDF_GETNEWESTVERSION;
      *grfBINDF &= ~BINDF_GETFROMCACHE_IF_NET_FAIL;
    }
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID, IUnknown*)
  {
    return E_NOTIMPL;
  }

private:
  volatile long m_refCount;
  volatile DWORD m_timeout;
};

void ExtensionFrame::DownloadExtensions(CFrameWnd* parent, CStringArray* urls)
{
  {
    CWaitCursor wc;

    CStringW lastExt;
    int installed = 0, total = (int)urls->GetSize();
    for (int i = 0; i < total; i++)
    {
      SetDownloadProgress(parent,total,i,installed);
      if (parent->SendMessage(WM_WANTSTOP) != 0)
        break;

      CString url = urls->GetAt(i);
      if (url.Left(8) != "library:")
        continue;

      // Get the ID of the download
      int idIdx = url.Find("?id=");
      if (idIdx <= 0)
        continue;
      int id = atoi(((LPCSTR)url)+idIdx+4);

      // Determine the path for downloaded extension files
      CString downPath;
      ::GetTempPath(MAX_PATH,downPath.GetBuffer(MAX_PATH));
      downPath.ReleaseBuffer();
      int downLen = downPath.GetLength();
      if (downLen > 0)
      {
        if (downPath.GetAt(downLen-1) != '\\')
          downPath.AppendChar('\\');
      }
      downPath.Append("PubLibDownload.i7x");

      // Determine the URL for the extension
      url.Format("http://www.emshort.com/pl%s",(LPCSTR)url.Mid(8));

      // Make sure there is no matching IE cache entry
      ::DeleteUrlCacheEntry(url);

      // Download from the public library
      WaitForDownload wait;
      if (FAILED(::URLDownloadToFile(NULL,url,(LPCSTR)downPath,0,&wait)))
        continue;
      wait.waitForEnd();

      // Check for a valid extension
      CStringW extLine = ReadExtensionFirstLine(downPath);
      if (!extLine.IsEmpty())
      {
        CStringW extName, extAuthor, extVersion;
        if (IsValidExtension(extLine,extName,extAuthor,extVersion))
        {
          // Work out the path to copy the extension to
          CString target;
          target.Format("%s\\Inform\\Extensions\\%S",
            (LPCSTR)theApp.GetHomeDir(),(LPCWSTR)extAuthor);
          ::CreateDirectory(target,NULL);
          target.AppendFormat("\\%S.i7x",(LPCWSTR)extName);

          // Copy the extension
          if (::CopyFile(downPath,target,FALSE))
          {
            DeleteOldExtension(target);
            theApp.AddToExtensions(CString(extAuthor),CString(extName),target);
            theApp.SendAllFrames(InformApp::DownloadedExt,id);
            lastExt.Format(L"\"%s\" by %s (%s)",(LPCWSTR)extName,(LPCWSTR)extAuthor,(LPCWSTR)extVersion);
            installed++;
          }
        }
      }

      // Delete the downloaded file
      ::DeleteFile(downPath);
    }
    SetDownloadProgress(parent,total,total,installed);

    // Notify the user of what happened
    parent->SendMessage(WM_RUNCENSUS,0);
    parent->SendMessage(WM_PROGRESS,-1);
    ShowInstalledMessage(parent,installed,total,lastExt);
  }

  // Update the extensions menu
  theApp.FindExtensions();
  theApp.SendAllFrames(InformApp::Extensions,0);
}

CStringW ExtensionFrame::ReadExtensionFirstLine(const char* path)
{
  // Get the first line of the file
  CStdioFile extFile;
  if (!extFile.Open(path,CFile::modeRead|CFile::typeBinary))
    return L"";
  CString extLineUTF8;
  if (!extFile.ReadString(extLineUTF8))
    return L"";
  extFile.Close();
  extLineUTF8.Trim();

  // Check for a line-end
  int lfPos = extLineUTF8.FindOneOf("\r\n");
  if (lfPos != -1)
    extLineUTF8.Truncate(lfPos);

  // Check for a Unicode line-end
  int uniPos = extLineUTF8.Find("\xE2\x80\xA8");
  if (uniPos != -1)
    extLineUTF8.Truncate(uniPos);

  // Check for a UTF-8 BOM
  if (extLineUTF8.GetLength() >= 3)
  {
    if (extLineUTF8.Left(3) == "\xEF\xBB\xBF")
      extLineUTF8 = extLineUTF8.Mid(3);
  }

  // Convert from UTF-8 to Unicode
  return TextFormat::UTF8ToUnicode(extLineUTF8);
}

bool ExtensionFrame::IsValidExtension(const CStringW& firstLine,
  CStringW& name, CStringW& author, CStringW& version)
{
  CArray<CStringW> tokens;
  int pos = 0;

  // Split the first line into tokens
  CStringW token = firstLine.Tokenize(L" \t",pos);
  while (token.IsEmpty() == FALSE)
  {
    tokens.Add(token);
    token = firstLine.Tokenize(L" \t",pos);
  }

  // Remove leading "Version XYZ of", if present
  if (tokens.GetSize() == 0)
    return false;
  if (tokens[0] == L"Version")
  {
    if (tokens.GetSize() < 3)
      return false;
    if (tokens[2] != L"of")
      return false;
    version.Format(L"%s %s",tokens[0],tokens[1]);
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
  }
  else
    version = L"Version 1";

  // Remove trailing "begins here", if present
  int size = (int)tokens.GetSize();
  if (size < 2)
    return false;
  if (tokens[size-1] != L"here.")
    return false;
  if ((tokens[size-2] != L"begin") && (tokens[size-2] != L"begins"))
    return false;
  tokens.SetSize(size-2);

  // Remove leading "the" from the name, if present
  if (tokens.GetSize() == 0)
    return false;
  if ((tokens[0] == L"The") || (tokens[0] == L"the"))
    tokens.RemoveAt(0);

  // Extract the name and author
  bool gotName = false;
  bool gotBracket = false;
  while (tokens.GetSize() > 0)
  {
    if (tokens[0] == L"by")
    {
      gotName = true;
    }
    else if (gotName)
    {
      if (author.GetLength() > 0)
        author.AppendChar(L' ');
      author.Append(tokens[0]);
    }
    else if (!gotBracket)
    {
      if (tokens[0].GetAt(0) == '(')
        gotBracket = true;
      else
      {
        if (name.GetLength() > 0)
          name.AppendChar(L' ');
        name.Append(tokens[0]);
      }
    }
    tokens.RemoveAt(0);
  }

  if (name.IsEmpty() || author.IsEmpty() || version.IsEmpty())
    return false;
  return true;
}

ExtensionFrame* ExtensionFrame::NewFrame(const ProjectSettings& settings)
{
  ExtensionFrame* frame = new ExtensionFrame;
  theApp.NewFrame(frame);

  frame->LoadFrame(IDR_EXTFRAME,WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE,NULL,NULL);
  frame->SetFromRegistryPath(REGISTRY_PATH_WINDOW);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();
  return frame;
}

bool ExtensionFrame::RemoveI7X(CString& path)
{
  // Does the extension file name have the new '.i7x' ending?
  if (path.GetLength() > 4)
  {
    if (path.Right(4).CompareNoCase(".i7x") == 0)
    {
      path = path.Left(path.GetLength()-4);
      return true;
    }
  }
  return false;
}

void ExtensionFrame::DeleteOldExtension(CString path)
{
  // Does the extension file name have the new '.i7x' ending?
  if (RemoveI7X(path))
    ::DeleteFile(path);
}

void ExtensionFrame::SetDownloadProgress(CFrameWnd* parent, int total, int current, int installed)
{
  CString status;
  status.Format("Downloaded and installed %d of %d extensions",installed,total);
  if (current > installed)
    status.AppendFormat(" (%d failed)",current-installed);

  parent->SendMessage(WM_PROGRESS,(int)(100 * ((double)current / (double)total)),(LPARAM)(LPCSTR)status);
}

void ExtensionFrame::ShowInstalledMessage(CWnd* parent, int installed, int total, LPCWSTR lastExt)
{
  CStringW head, msg;
  if (total > installed)
  {
    // One or more errors
    if (installed > 0)
    {
      if (installed == 1)
        msg.Format(L"One extension installed successfully, %d failed.",total-installed);
      else
        msg.Format(L"%d extensions installed successfully, %d failed.",installed,total-installed);
    }
    else
    {
      if (total > 1)
        msg.Format(L"Failed to install %d extensions.",total);
      else
        msg = "Failed to install extension.";
    }
  }
  else
  {
    // No errors
    head = L"Installation complete.";
    if (installed > 1)
      msg.Format(L"%d extensions installed successfully.",installed);
    else
      msg.Format(L"Extension %s installed successfully.",lastExt);
  }
  ::TaskDialog(parent->GetSafeHwnd(),0,
    L_INFORM_TITLE,head,msg,TDCBF_OK_BUTTON,TD_INFORMATION_ICON,NULL);
}

CString ExtensionFrame::GetDisplayName(bool fullName)
{
  CString name;
  if (m_extension.IsEmpty())
    name = "Untitled";
  else
  {
    CString extName(m_extension);
    RemoveI7X(extName);
    int start = extName.ReverseFind('\\');
    name = extName.Mid(start+1);
  }
  if (IsProjectEdited() && fullName)
    name += '*';
  return name;
}

void ExtensionFrame::SendChanged(InformApp::Changed changed, int value)
{
  switch (changed)
  {
  case InformApp::Preferences:
    {
      CRegKey registryKey;
      if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
      {
        SourceSettingsRegistry set(registryKey);
        m_edit.LoadSettings(set,GetBackColour(registryKey));
        m_edit.PrefsChanged();
      }
    }
    break;
  case InformApp::Spelling:
    m_edit.UpdateSpellCheck();
    break;
  }
}

void ExtensionFrame::OpenFile(const char* path)
{
  // Set the file name
  m_extension = path;

  // Open the file
  CFile extFile;
  if (extFile.Open(m_extension,CFile::modeRead))
  {
    // Update the control
    m_edit.OpenFile(&extFile);
  }
}

void ExtensionFrame::SetFromRegistryPath(const char* path)
{
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,path,KEY_READ) == ERROR_SUCCESS)
  {
    // Restore the window state
    WINDOWPLACEMENT place;
    ULONG len = sizeof WINDOWPLACEMENT;
    if (registryKey.QueryBinaryValue("Placement",&place,&len) == ERROR_SUCCESS)
      SetWindowPlacement(&place);

    // Allow the source editor to load settings
    SourceSettingsRegistry set(registryKey);
    m_edit.LoadSettings(set,GetBackColour(registryKey));
  }
}

bool ExtensionFrame::IsProjectEdited(void)
{
  return m_edit.IsEdited();
}

bool ExtensionFrame::IsUserExtension(void)
{
  // Check we are not saving under the program directory
  CString appDir = theApp.GetAppDir();
  return (strncmp(m_extension,appDir,appDir.GetLength()) != 0);
}

COLORREF ExtensionFrame::GetBackColour(CRegKey& key)
{
  // Use the source paper colour for the background
  DWORD colour;
  if (key.QueryDWORDValue("Source Paper Colour",colour) == ERROR_SUCCESS)
    return (COLORREF)colour;
  return theApp.GetColour(InformApp::ColourBack);
}
