#include "stdafx.h"
#include "ExtensionFrame.h"

#include "Inform.h"
#include "Messages.h"
#include "NewDialogs.h"
#include "ProjectFrame.h"
#include "TextFormat.h"
#include "WelcomeLauncher.h"

#include "Dialogs.h"
#include "DpiFunctions.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ExtensionFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(ExtensionFrame, MenuBarFrameWnd)
  ON_WM_CREATE()
  ON_WM_ACTIVATE()
  ON_WM_CLOSE()
  ON_WM_SETTINGCHANGE()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)

  ON_MESSAGE(WM_PROJECTEDITED, OnProjectEdited)

  ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
  ON_COMMAND(ID_FILE_SAVE, OnFileSave)
  ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)

  ON_UPDATE_COMMAND_UI(ID_WINDOW_LIST, OnUpdateWindowList)
  ON_COMMAND_RANGE(ID_WINDOW_LIST, ID_WINDOW_LIST+8, OnWindowList)
END_MESSAGE_MAP()

ExtensionFrame::ExtensionFrame()
{
}

void ExtensionFrame::SetDarkMode(DarkMode* dark)
{
  MenuBarFrameWnd::SetDarkMode(dark);
  m_edit.SetDarkMode(dark);
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

  // Turn on dark mode, if needed
  SetDarkMode(DarkMode::GetEnabled(REGISTRY_INFORM));

  // Set window text for accessibility
  m_coolBar.SetWindowText("Toolbar area");
  m_menuBar.SetWindowText("Menus");
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

  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  if (frames.GetSize() == 1)
    theApp.WriteOpenProjectsOnExit();

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

void ExtensionFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
  MenuBarFrameWnd::OnSettingChange(uFlags,lpszSection);

  if ((m_dark != NULL) != DarkMode::IsEnabled(REGISTRY_INFORM))
    theApp.SendAllFrames(InformApp::LightDarkMode,0);
}

LRESULT ExtensionFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  MoveWindow((LPRECT)lparam,TRUE);
  UpdateDPI((int)HIWORD(wparam));
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
      "under \"My Documents\", and not where Inform has been installed.",
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
    theApp.RunLegacyExtensionCensus();
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
  CMenu* windowMenu = GetSubMenu(GetMenu(),3,"Window");
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
    else if (frames[i]->IsKindOf(RUNTIME_CLASS(WelcomeLauncherFrame)))
      name = frames[i]->GetTitle();

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

void ExtensionFrame::StartNew(CWnd* parent, const char* dir, const ProjectSettings& settings)
{
  NewExtensionDialog dialog(parent,dir);
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
    version = L"Version (none)";

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
  frame->SetFromRegistryPath(REGISTRY_INFORM_WINDOW);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();
  return frame;
}

bool ExtensionFrame::RemoveI7X(CString& path)
{
  // Does the extension file name have an '.i7x' ending?
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
      if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
      {
        SourceSettingsRegistry set(registryKey,this);
        m_edit.LoadSettings(set,GetBackColour(set));
        m_edit.PrefsChanged();
      }
    }
    break;
  case InformApp::Spelling:
    m_edit.UpdateSpellCheck();
    break;
  case InformApp::LightDarkMode:
    SetDarkMode(DarkMode::GetEnabled(REGISTRY_INFORM));
    SendChanged(InformApp::Preferences,0);
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
    if (registryKey.QueryBinaryValue("Placement 96dpi",&place,&len) == ERROR_SUCCESS)
    {
      DPI::ContextUnaware dpiUnaware;
      SetWindowPlacement(&place);
    }
    else if (registryKey.QueryBinaryValue("Placement",&place,&len) == ERROR_SUCCESS)
      SetWindowPlacement(&place);

    // Allow the source editor to load settings
    SourceSettingsRegistry set(registryKey,this);
    m_edit.LoadSettings(set,GetBackColour(set));
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

COLORREF ExtensionFrame::GetBackColour(SourceSettings& set)
{
  // Use the extension paper colour for the background
  DWORD enabled = 1;
  set.GetDWord("Syntax Colouring",enabled);
  if (enabled)
  {
    DWORD colour;
    if (set.GetDWord("Ext Paper Colour",colour))
      return (COLORREF)colour;
  }

  DarkMode* dark = DarkMode::GetActive(this);
  return dark ? dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack);
}
