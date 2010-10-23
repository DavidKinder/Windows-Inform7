#include "stdafx.h"
#include "ExtensionFrame.h"
#include "ProjectFrame.h"
#include "Inform.h"
#include "Messages.h"
#include "TextFormat.h"
#include "NewDialogs.h"
#include "Dialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ExtensionFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(ExtensionFrame, MenuBarFrameWnd)
  ON_WM_CREATE()
  ON_WM_ACTIVATE()
  ON_WM_CLOSE()
  ON_WM_SIZE()

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
  if (!m_edit.Create(this,AFX_IDW_PANE_FIRST))
  {
    TRACE("Failed to create source edit control\n");
    return -1;
  }
  m_edit.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
  m_edit.SetFocus();

  // Create the status bar
  if (!m_statusBar.Create(this) ||
      !m_statusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT)))
  {
    TRACE("Failed to create status bar\n");
    return -1;
  }

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
    m_edit.SetFocus();
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
        rMessage.Format("Switch to the \"%s\" extension project",
          ((ExtensionFrame*)frames[i])->GetDisplayName(false));
      }
      return;
    }
  }

  MenuBarFrameWnd::GetMessageString(nID,rMessage);
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

  // Open the file for saving
  CFile extFile;
  if (extFile.Open(m_extension,CFile::modeCreate|CFile::modeWrite) == FALSE)
  {
    CString msg;
    msg.Format("Failed to save extension to \n\"%s\"",(LPCSTR)m_extension);
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Save the extension from the edit control
  if (m_edit.SaveFile(&extFile))
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
  CMenu* windowMenu = GetMenu()->GetSubMenu(2);
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

void ExtensionFrame::StartNew(CWnd* parent)
{
  NewExtensionDialog dialog(parent);
  if (dialog.DoModal() != IDOK)
    return;

  CStringW initialCode;
  initialCode.Format(L"%S by %s begins here.\r\r%S ends here.\n",
    dialog.GetName(),dialog.GetAuthor(),dialog.GetName());

  ExtensionFrame* frame = NewFrame();
  frame->m_extension = dialog.GetPath();

  frame->m_edit.PasteCode(initialCode);
}

void ExtensionFrame::StartExisting(const char* path)
{
  ExtensionFrame* frame = NewFrame();
  frame->OpenFile(path);
}

bool ExtensionFrame::StartHighlight(const char* url, COLORREF colour)
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
        ExtensionFrame* extFrame = NewFrame();
        extFrame->OpenFile(path);

        // Highlight the text
        extFrame->m_edit.Highlight(line,colour,true);
      }
    }
    return true;
  }
  return false;
}

bool ExtensionFrame::InstallExtensions(CWnd* parent)
{
  // Ask the user for one or more extensions
  SimpleFileDialog dialog(TRUE,"i7x",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_ALLOWMULTISELECT,
    "Inform extensions (*.i7x)|*.i7x|All Files (*.*)|*.*||",parent);
  dialog.m_ofn.lpstrTitle = "Select the extensions to install";
  dialog.m_ofn.nMaxFile = 16384;
  dialog.m_ofn.lpstrFile = (LPSTR)_alloca(dialog.m_ofn.nMaxFile);
  memset(dialog.m_ofn.lpstrFile,0,dialog.m_ofn.nMaxFile);
  if (dialog.DoModal() != IDOK)
    return false;

  // Iterate over the selected extensions
  POSITION pos = dialog.GetStartPosition();
  while (pos != NULL)
  {
    // Get the extension file path
    CString path = dialog.GetNextPathName(pos);

    // Get the first line of the file
    CStdioFile extFile;
    if (!extFile.Open(path,CFile::modeRead|CFile::typeBinary))
      continue;
    CString extLineUTF8;
    if (!extFile.ReadString(extLineUTF8))
      continue;
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
    CStringW extLine = TextFormat::UTF8ToUnicode(extLineUTF8);

    // Check for a valid extension
    CStringW extName, extAuthor;
    if (!IsValidExtension(extLine,extName,extAuthor))
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
    target.Format("%s\\Inform\\Extensions\\%S\\%S.i7x",
      (LPCSTR)theApp.GetHomeDir(),(LPCWSTR)extAuthor,(LPCWSTR)extName);

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
      DeleteOldExtension(target);
  }

  // Update the extensions menu and documentation
  theApp.FindExtensions();
  theApp.SendAllFrames(InformApp::Extensions,0);
  theApp.RunCensus(true);
  return true;
}

ExtensionFrame* ExtensionFrame::NewFrame(void)
{
  ExtensionFrame* frame = new ExtensionFrame;
  theApp.NewFrame(frame);

  frame->LoadFrame(IDR_EXTFRAME,WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE,NULL,NULL);
  frame->SetFromRegistryPath(REGISTRY_PATH_WINDOW);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();

  return frame;
}

bool ExtensionFrame::IsValidExtension(const CStringW& firstLine, CStringW& name, CStringW& author)
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
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
    tokens.RemoveAt(0);
  }

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

  if (name.IsEmpty())
    return false;
  if (author.IsEmpty())
    return false;
  return true;
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

CString ExtensionFrame::GetDisplayName(bool showEdited)
{
  CString name = m_extension;
  RemoveI7X(name);
  if (name.IsEmpty())
    name = "Untitled";

  if (IsProjectEdited() && showEdited)
    name += '*';

  int start = name.ReverseFind('\\');
  return ((LPCSTR)name)+start+1;
}

void ExtensionFrame::SendChanged(InformApp::Changed changed, int value)
{
  switch (changed)
  {
  case InformApp::Preferences:
    {
      CRegKey registryKey;
      if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_PATH_WINDOW,KEY_READ) == ERROR_SUCCESS)
        m_edit.LoadSettings(registryKey,true);
    }
    break;
  case InformApp::SourceTextSize:
    m_edit.SetTextSize(value);
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
    m_edit.LoadSettings(registryKey,false);
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
