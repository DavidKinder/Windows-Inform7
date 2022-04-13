#include "stdafx.h"
#include "ProjectFrame.h"

#include "BookFrame.h"
#include "ExtensionFrame.h"

#include "Messages.h"
#include "NewDialogs.h"
#include "ProjectDirDialog.h"
#include "TextFormat.h"
#include "WelcomeLauncher.h"

#include "Dialogs.h"
#include "DpiFunctions.h"
#include "Build.h"

#include "TabDoc.h"
#include "TabExtensions.h"
#include "TabIndex.h"
#include "TabResults.h"
#include "TabStory.h"
#include "TabTesting.h"

#include <sys/stat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_MENU_EXTENSIONS 1000

#define I7XP_TBAR_SPACER_POS 4
#define I7XP_TBAR_SPACER_COUNT 6

IMPLEMENT_DYNAMIC(ProjectFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(ProjectFrame, MenuBarFrameWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_ACTIVATE()
  ON_WM_CLOSE()
  ON_WM_SIZE()
  ON_WM_SETCURSOR()
  ON_WM_MEASUREITEM()
  ON_WM_DRAWITEM()
  ON_WM_SETTINGCHANGE()
  ON_WM_TIMER()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)

  ON_CBN_SELCHANGE(IDC_EXAMPLE_LIST, OnChangedExample)
  ON_UPDATE_COMMAND_UI(IDC_EXAMPLE_LIST, OnUpdateIfNotBusy)

  ON_MESSAGE(WM_PLAYSKEIN, OnPlaySkein)
  ON_MESSAGE(WM_GAMERUNNING, OnGameRunning)
  ON_MESSAGE(WM_GAMEWAITING, OnGameWaiting)
  ON_MESSAGE(WM_PANEHEADING, OnPaneHeading)
  ON_MESSAGE(WM_SELECTSIDE, OnSelectSide)
  ON_MESSAGE(WM_SELECTVIEW, OnSelectView)
  ON_MESSAGE(WM_PASTECODE, OnPasteCode)
  ON_MESSAGE(WM_RUNTIMEPROB, OnRuntimeProblem)
  ON_MESSAGE(WM_SEARCHSOURCE, OnSearchSource)
  ON_MESSAGE(WM_SEARCHDOC, OnSearchDoc)
  ON_MESSAGE(WM_SHOWSKEIN, OnShowSkein)
  ON_MESSAGE(WM_ANIMATESKEIN, OnAnimateSkein)
  ON_MESSAGE(WM_TERPFAILED, OnTerpFailed)
  ON_MESSAGE(WM_PROJECTDIR, OnProjectDir)
  ON_MESSAGE(WM_PLAYNEXTTHREAD, OnPlayNextThread)
  ON_MESSAGE(WM_CANPLAYALL, OnCanPlayAll)
  ON_MESSAGE(WM_PROJECTEDITED, OnProjectEdited)
  ON_MESSAGE(WM_EXTDOWNLOAD, OnExtDownload)
  ON_MESSAGE(WM_PROGRESS, OnProgress)
  ON_MESSAGE(WM_NEWPROJECT, OnCreateNewProject)
  ON_MESSAGE(WM_PROJECTEXT, OnProjectExt)
  ON_MESSAGE(WM_PROJECTTYPE, OnProjectType)
  ON_MESSAGE(WM_STORYACTIVE, OnStoryActive)
  ON_MESSAGE(WM_WANTSTOP, OnWantStop)
  ON_MESSAGE(WM_RUNCENSUS, OnRunCensus)
  ON_MESSAGE(WM_STORYNAME, OnStoryName)
  ON_MESSAGE(WM_REPLAYALL, OnReplayAll)

  ON_COMMAND(ID_FILE_NEW, OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_COMMAND(ID_FILE_INSTALL_EXT, OnFileInstallExt)
  ON_COMMAND(ID_FILE_INSTALL_FOLDER, OnFileInstallFolder)
  ON_UPDATE_COMMAND_UI(ID_FILE_INSTALL_XP, OnUpdateIfNotBusy)
  ON_COMMAND(ID_FILE_INSTALL_XP, OnFileInstallExtProject)
  ON_COMMAND(ID_FILE_NEW_EXT, OnFileNewExt)
  ON_COMMAND(ID_FILE_NEW_XP, OnFileNewExtProject)
  ON_COMMAND_RANGE(ID_OPEN_EXTENSIONS_LIST, ID_OPEN_EXTENSIONS_LIST+MAX_MENU_EXTENSIONS-1, OnFileOpenExt)
  ON_COMMAND_RANGE(ID_NEW_EXTENSIONS_LIST, ID_NEW_EXTENSIONS_LIST+MAX_MENU_EXTENSIONS-1, OnFileNewXPFromExt)
  ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateIfNotBusy)
  ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateIfNotBusy)
  ON_COMMAND(ID_FILE_SAVE, OnFileSave)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateIfNotBusy)
  ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
  ON_COMMAND(ID_FILE_IMPORT_SKEIN, OnFileImportSkein)
  ON_COMMAND(ID_FILE_EXPORT_EXT, OnFileExportExtProject)

  ON_COMMAND(ID_EDIT_FIND_IN_FILES, OnEditFindInFiles)

  ON_UPDATE_COMMAND_UI(ID_PLAY_GO, OnUpdateCompile)
  ON_COMMAND(ID_PLAY_GO, OnPlayGo)
  ON_UPDATE_COMMAND_UI(ID_PLAY_REPLAY, OnUpdateCompile)
  ON_COMMAND(ID_PLAY_REPLAY, OnPlayReplay)
  ON_UPDATE_COMMAND_UI(ID_PLAY_TEST, OnUpdateIfNotBusy)
  ON_COMMAND(ID_PLAY_TEST, OnPlayTest)
  ON_UPDATE_COMMAND_UI(ID_PLAY_REFRESH, OnUpdateCompile)
  ON_COMMAND(ID_PLAY_REFRESH, OnPlayRefresh)
  ON_COMMAND(ID_PLAY_LOAD, OnPlayLoad)

  ON_UPDATE_COMMAND_UI_RANGE(ID_RELEASE_GAME, ID_RELEASE_TEST, OnUpdateReleaseGame)
  ON_COMMAND_RANGE(ID_RELEASE_GAME, ID_RELEASE_TEST, OnReleaseGame)
  ON_COMMAND(ID_RELEASE_MATERIALS, OnReleaseMaterials)
  ON_UPDATE_COMMAND_UI(ID_RELEASE_IFICTION, OnUpdateReleaseGame)
  ON_COMMAND(ID_RELEASE_IFICTION, OnReleaseIFiction)

  ON_COMMAND(ID_WINDOW_LEFTPANE, OnWindowLeftPane)
  ON_COMMAND(ID_WINDOW_RIGHTPANE, OnWindowRightPane)
  ON_COMMAND(ID_WINDOW_SWITCH, OnWindowSwitchPanes)
  ON_COMMAND_RANGE(ID_WINDOW_TAB_SOURCE, ID_WINDOW_TAB_SOURCE+7, OnWindowShowTab)
  ON_UPDATE_COMMAND_UI_RANGE(ID_WINDOW_TAB_SOURCE, ID_WINDOW_TAB_SOURCE+7, OnUpdateWindowShowTab)
  ON_COMMAND_RANGE(ID_WINDOW_INDEX_HOME, ID_WINDOW_INDEX_HOME+7, OnWindowShowIndex)
  ON_UPDATE_COMMAND_UI(ID_WINDOW_LIST, OnUpdateWindowList)
  ON_COMMAND_RANGE(ID_WINDOW_LIST, ID_WINDOW_LIST+8, OnWindowList)

  ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
  ON_COMMAND(ID_HELP_EXTENSIONS, OnHelpExtensions)
  ON_COMMAND(ID_HELP_RECIPES, OnHelpRecipes)
  ON_COMMAND(ID_HELP_LICENCE, OnHelpLicence)
  ON_COMMAND(ID_HELP_WINDOWS, OnHelpWindows)

  ON_COMMAND(ID_SEARCH_SOURCE, OnSearchSource)
  ON_COMMAND(ID_SEARCH_DOCS, OnSearchDocs)
END_MESSAGE_MAP()

class IntestOutputSink : public InformApp::OutputSink
{
public:
  void Output(const char* msg)
  {
    for (const char* ptr = msg; *ptr != 0; ptr++)
    {
      if ((*ptr == '\n') || (*ptr == '\r'))
        Done();
      else
        m_current.AppendChar(*ptr);
    }
  }

  bool WantStop(void)
  {
    return false;
  }

  void Done(void)
  {
    if (!m_current.IsEmpty())
    {
      // Convert any "[unicode N]" to the appropriate Unicode character
      int i = 0;
      while (true)
      {
        i = m_current.Find(L"[unicode ",i);
        if (i < 0)
          break;
        int j = m_current.Find(L']',i+9);
        if (j < 0)
          break;
        int c = _wtoi(m_current.Mid(i+9,j-i-9));
        m_current.Delete(i+1,j-i);
        m_current.SetAt(i,c);
        i++;
      }

      results.Add(m_current);
    }
    m_current.Empty();
  }

  CArray<CStringW> results;

private:
  CStringW m_current;
};

ProjectFrame::ProjectFrame(ProjectType projectType)
  : m_projectType(projectType), m_needCompile(true), m_last5StartTime(0),
    m_busy(false), m_I6debug(false), m_game(m_skein), m_finder(this), m_focus(0),
    m_loadFilter(1), m_menuGutter(0), m_menuTextGap(0,0), m_splitter(true)
{
  m_menuBar.SetUseF10(false);
  if (m_projectType == Project_I7XP)
    m_skein.SetFile("");
}

int ProjectFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (MenuBarFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Create a splitter to occupy the client area of the frame
  if (!m_splitter.CreateStatic(this,1,2,WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS))
  {
    TRACE("Failed to create splitter window\n");
    return -1;
  }

  for (int i = 0; i < 2; i++)
  {
    if (!m_splitter.CreateView(0,i,RUNTIME_CLASS(Panel),CSize(0,0),NULL))
    {
      TRACE("Failed to create panel window\n");
      return -1;
    }
  }

  CRect client;
  GetClientRect(client);
  int width = (client.Width()/2)-6;
  m_splitter.SetColumnInfo(0,width,16);
  m_splitter.SetColumnInfo(1,width,16);

  // Set the initial tabs
  GetPanel(1)->SetActiveTab(Panel::Tab_Doc);
  ((TabDoc*)GetPanel(1)->GetTab(Panel::Tab_Doc))->SetFocusFlag(false);
  GetPanel(0)->SetActiveTab(Panel::Tab_Source);

  // Listen for notifications from the tabs
  ((TabResults*)GetPanel(0)->GetTab(Panel::Tab_Results))->SetLinkNotify(this);
  ((TabResults*)GetPanel(1)->GetTab(Panel::Tab_Results))->SetLinkNotify(this);
  ((TabIndex*)GetPanel(0)->GetTab(Panel::Tab_Index))->SetLinkNotify(this);
  ((TabIndex*)GetPanel(1)->GetTab(Panel::Tab_Index))->SetLinkNotify(this);
  ((TabExtensions*)GetPanel(0)->GetTab(Panel::Tab_Extensions))->SetLinkNotify(this);
  ((TabExtensions*)GetPanel(1)->GetTab(Panel::Tab_Extensions))->SetLinkNotify(this);
  ((TabSettings*)GetPanel(0)->GetTab(Panel::Tab_Settings))->SetNotify(this);
  ((TabSettings*)GetPanel(1)->GetTab(Panel::Tab_Settings))->SetNotify(this);

  // Set up the source tabs
  ((TabSource*)GetPanel(1)->GetTab(Panel::Tab_Source))->SetDocument(
    ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source)));

  // Set up the story tab
  ((TabStory*)GetPanel(1)->GetTab(Panel::Tab_Story))->SetGame(&m_game);

  // Set up the testing tabs
  ((TabTesting*)GetPanel(0)->GetTab(Panel::Tab_Testing))->SetSkein(&m_skein);
  ((TabTesting*)GetPanel(1)->GetTab(Panel::Tab_Testing))->SetSkein(&m_skein);

  // Set up the settings tabs
  ((TabSettings*)GetPanel(0)->GetTab(Panel::Tab_Settings))->SetSettings(&m_settings);
  ((TabSettings*)GetPanel(1)->GetTab(Panel::Tab_Settings))->SetSettings(&m_settings);

  // Create the game window
  m_game.Create(this);

  // Create the toolbars
  DWORD style = WS_CHILD|WS_VISIBLE|CBRS_ALIGN_TOP|CBRS_TOOLTIPS|CBRS_FLYBY;
  DWORD ctrlStyle = TBSTYLE_FLAT|TBSTYLE_LIST|TBSTYLE_TRANSPARENT;
  if (!m_toolBar.CreateEx(this,ctrlStyle,style) || !LoadToolBar())
  {
    TRACE("Failed to create main toolbar\n");
    return -1;
  }
  style = CBRS_ALIGN_TOP|CBRS_TOOLTIPS|CBRS_FLYBY;
  if (!m_searchBar.Create(this,style,AFX_IDW_DIALOGBAR))
  {
    TRACE("Failed to create search toolbar\n");
    return -1;
  }

  // Create the menu bar and add the toolbars to it
  if (!CreateMenuBar(IDR_MAINFRAME,0))
  {
    TRACE("Failed to create menu bar\n");
    return -1;
  }
  if (!m_coolBar.AddBar(&m_toolBar,NULL,NULL,RBBS_NOGRIPPER|RBBS_BREAK))
  {
    TRACE("Failed to add toolbar\n");
    return -1;
  }
  if (!m_coolBar.AddBar(&m_searchBar,NULL,NULL,RBBS_NOGRIPPER))
  {
    TRACE("Failed to add search bar\n");
    return -1;
  }

  // Create a progress window
  if (!m_progress.Create(this,WS_CLIPSIBLINGS))
  {
    TRACE("Failed to create progress window\n");
    return -1;
  }

  // Set the application icon
  theApp.SetIcon(this);

  // Create the menu of available extensions
  UpdateMenuParams();
  UpdateExtensionsMenu();

  // Remove menu items that do not apply to the project
  if (m_projectType != Project_I7XP)
    GetMenu()->RemoveMenu(ID_FILE_EXPORT_EXT,MF_BYCOMMAND);

  if (!theApp.GetTestMode())
  {
    // Remove test menu item
    GetMenu()->RemoveMenu(ID_PLAY_LOAD,MF_BYCOMMAND);
  }

  // Set window text for accessibility
  m_coolBar.SetWindowText("Toolbar area");
  m_menuBar.SetWindowText("Menus");
  m_toolBar.SetWindowText("Actions");
  m_searchBar.SetWindowText("Search");
  GetPanel(0)->SetWindowText("Left");
  GetPanel(1)->SetWindowText("Right");
  return 0;
}

BOOL ProjectFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if (!MenuBarFrameWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}

void ProjectFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  MenuBarFrameWnd::OnActivate(nState,pWndOther,bMinimized);

  switch (nState)
  {
  case WA_INACTIVE:
    // Store the focus window
    m_focus = GetFocus()->GetSafeHwnd();
    break;
  case WA_ACTIVE:
    // Check if the source file has been changed by another application
    TabSource* leftSource = (TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source);
    if (leftSource->CheckNeedReopen(m_projectDir))
    {
      // Reopen the source files
      leftSource->OpenProject(m_projectDir,true);
      GetPanel(1)->GetTab(Panel::Tab_Source)->OpenProject(m_projectDir,false);

      // Update elastic tabstops, if in use
      ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source))->UpdateElasticTabStops();

      if (m_projectType == Project_I7XP)
        UpdateExampleList();
    }

    // Restore the focus window
    if (!::IsWindow(m_focus))
      m_focus = 0;
    if (m_focus != 0)
      CWnd::FromHandle(m_focus)->SetFocus();
    break;
  }
}

void ProjectFrame::OnDestroy()
{
  SaveSettings();

  m_game.StopInterpreter(false);
  for (int i = 0; i < m_processes.GetSize(); i++)
    m_processes.GetAt(i).cp.close();
  m_processes.RemoveAll();
  ReportHtml::RemoveContext(this);

  MenuBarFrameWnd::OnDestroy();
}

void ProjectFrame::OnClose()
{
  if (IsProjectEdited())
  {
    // Ask the user before discarding the project
    CString msg;
    msg.Format(
      "Closing %s will discard any changes.\n"
      "Do you want to save this project first?",(LPCSTR)GetDisplayName(false));
    switch (MessageBox(msg,INFORM_TITLE,MB_YESNOCANCEL|MB_ICONEXCLAMATION))
    {
    case IDYES:
      {
        // Try to save the project. If this fails, prompt the user for a directory
        bool saved = SaveProject(m_projectDir);
        while (saved == false)
        {
          ProjectDirDialog dialog(false,m_projectDir,"Save the project",GetProjectFileExt(),this);
          if (dialog.ShowDialog() == IDOK)
            saved = SaveProject(dialog.GetProjectDir());
          else
            saved = true;
        }
      }
      break;
    case IDNO:
      // Do nothing
      break;
    case IDCANCEL:
      // Don't close the window
      return;
    }
  }

  m_finder.Hide();
  m_game.StopInterpreter(false);
  CleanProject();

  // If this is the last frame window, mark this project as to be re-opened
  // the next time the application is run.
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  if (frames.GetSize() == 1)
    theApp.WriteOpenProjectsOnExit();

  // If there are any secondary frame windows and this is the main frame,
  // promote one of the secondaries to be the new main frame.
  theApp.FrameClosing(this);
  MenuBarFrameWnd::OnClose();
}

void ProjectFrame::OnSize(UINT nType, int cx, int cy)
{
  // Get the existing splitter position before the default action for this message
  double split = 0.0;
  if (m_splitter.GetSafeHwnd() != 0)
    split = m_splitter.GetColumnFraction(0);

  MenuBarFrameWnd::OnSize(nType,cx,cy);

  // Reposition and update the toolbars
  if (m_coolBar.GetSafeHwnd() != 0)
  {
    m_coolBar.GetReBarCtrl().MinimizeBand(2);
    m_toolBar.Invalidate();
    m_searchBar.Invalidate();
  }

  if ((m_splitter.GetSafeHwnd() != 0) && (split > 0.0))
  {
    // Adjust the splitter so that the fractional position is constant
    m_splitter.SetColumnFraction(0,split,16);
    m_splitter.RecalcLayout();
  }
}

BOOL ProjectFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  if (theApp.IsWaitCursor())
  {
    theApp.RestoreWaitCursor();
    return TRUE;
  }
  return MenuBarFrameWnd::OnSetCursor(pWnd,nHitTest,message);
}

void ProjectFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT mi)
{
  if (mi->CtlType == ODT_MENU)
  {
    // Custom measurement for extensions menu items
    if (((mi->itemID >= ID_OPEN_EXTENSIONS_LIST) && (mi->itemID < ID_OPEN_EXTENSIONS_LIST+MAX_MENU_EXTENSIONS)) ||
        ((mi->itemID >= ID_NEW_EXTENSIONS_LIST ) && (mi->itemID < ID_NEW_EXTENSIONS_LIST +MAX_MENU_EXTENSIONS)))
    {
      InformApp::ExtLocation* ext = (InformApp::ExtLocation*)mi->itemData;

      // Get the size of the menu text
      CDC* dc = GetDC();
      CFont* oldFont = dc->SelectObject(&m_menuFonts[ext->system ? 1 : 0]);
      CSize sz = dc->GetTextExtent(CString(ext->title.c_str()));
      dc->SelectObject(oldFont);
      ReleaseDC(dc);

      // Add space around the text
      sz.cx += m_menuGutter+(2*m_menuTextGap.cx);
      sz.cy += 2*m_menuTextGap.cy;

      mi->itemWidth = sz.cx;
      mi->itemHeight = sz.cy;
      return;
    }
  }
  CWnd::OnMeasureItem(nIDCtl,mi);
}

void ProjectFrame::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di)
{
  if (di->CtlType == ODT_MENU)
  {
    // Custom drawing for extensions menu items
    if (((di->itemID >= ID_OPEN_EXTENSIONS_LIST) && (di->itemID < ID_OPEN_EXTENSIONS_LIST+MAX_MENU_EXTENSIONS)) ||
        ((di->itemID >= ID_NEW_EXTENSIONS_LIST ) && (di->itemID < ID_NEW_EXTENSIONS_LIST +MAX_MENU_EXTENSIONS)))
    {
      CRect rc(di->rcItem);
      InformApp::ExtLocation* ext = (InformApp::ExtLocation*)di->itemData;

      // Get the theme, if any
      HTHEME theme = 0;
      if (::IsAppThemed())
        theme = ::OpenThemeData(GetSafeHwnd(),L"Menu");

      if (theme != 0)
      {
        // Get a device context for buffered painting
        HDC hdc = 0;
        HANDLE pb = ::BeginBufferedPaint(di->hDC,&(di->rcItem),BPBF_COMPATIBLEBITMAP,NULL,&hdc);
        if (pb == 0)
          return;
        CDC* dc = CDC::FromHandle(hdc);

        // Draw the menu background
        ::DrawThemeBackground(theme,dc->GetSafeHdc(),MENU_POPUPBACKGROUND,0,&rc,NULL);

        // Draw the menu gutter
        rc.right = di->rcItem.left+m_menuGutter;
        ::DrawThemeBackground(theme,dc->GetSafeHdc(),MENU_POPUPGUTTER,0,&rc,NULL);

        // Draw the menu item
        int state = (di->itemState & ODS_SELECTED) ? MPI_HOT : MPI_NORMAL;
        rc.left = di->rcItem.left;
        rc.right = di->rcItem.right;
        ::DrawThemeBackground(theme,dc->GetSafeHdc(),MENU_POPUPITEM,state,&rc,NULL);

        // Select a colour for the menu text, based on the theme's colours
        COLORREF itemColour = 0;
        ::GetThemeColor(theme,MENU_POPUPITEM,state,TMT_TEXTCOLOR,&itemColour);
        if (ext->system)
        {
          COLORREF backColour = 0;
          ::GetThemeColor(theme,MENU_POPUPBACKGROUND,0,TMT_FILLCOLORHINT,&backColour);
          dc->SetTextColor(theApp.BlendedColour(itemColour,4,backColour,1));
        }
        else
          dc->SetTextColor(itemColour);

        // Draw the menu text
        rc.left = di->rcItem.left+m_menuGutter+m_menuTextGap.cx;
        CFont* oldFont = dc->SelectObject(&m_menuFonts[ext->system ? 1 : 0]);
        dc->SetBkMode(TRANSPARENT);
        dc->DrawText(CString(ext->title.c_str()),rc,DT_VCENTER|DT_SINGLELINE);
        dc->SelectObject(oldFont);

        ::EndBufferedPaint(pb,TRUE);
        ::CloseThemeData(theme);
      }
      else
      {
        CDC* dc = CDC::FromHandle(di->hDC);

        if (di->itemState & ODS_SELECTED)
        {
          dc->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
          dc->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
        }
        else
        {
          if (ext->system)
          {
            dc->SetTextColor(theApp.BlendedColour(
              ::GetSysColor(COLOR_MENUTEXT),4,::GetSysColor(COLOR_MENU),1));
          }
          else
            dc->SetTextColor(::GetSysColor(COLOR_MENUTEXT));
          dc->SetBkColor(::GetSysColor(COLOR_MENU));
        }

        dc->ExtTextOut(0,0,ETO_OPAQUE,rc,NULL,0,NULL);
        rc.left = di->rcItem.left+m_menuGutter+m_menuTextGap.cx;
        CFont* oldFont = dc->SelectObject(&m_menuFonts[ext->system ? 1 : 0]);
        dc->DrawText(CString(ext->title.c_str()),rc,DT_VCENTER|DT_SINGLELINE);
        dc->SelectObject(oldFont);
      }
      return;
    }
  }
  CWnd::OnDrawItem(nIDCtl,di);
}

void ProjectFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
  UpdateMenuParams();
}

void ProjectFrame::OnChangedExample()
{
  CString id;
  Example ex = GetCurrentExample();
  if (ex.id != 0)
    id.Format("Skein%c.skein",ex.id);

  if (m_skein.ChangeFile(id,m_projectDir))
  {
    TabTesting* tab = (TabTesting*)(GetPanel(0)->GetTab(Panel::Tab_Testing));
    tab->SkeinChanged();
    GetPanel(1)->GetTab(Panel::Tab_Testing)->OpenProject(m_projectDir,false);
    m_needCompile = true;
  }
}

LRESULT ProjectFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  std::map<CWnd*,double> layout;
  if (m_splitter.GetSafeHwnd() != 0)
  {
    layout.insert(std::make_pair(&m_splitter,m_splitter.GetColumnFraction(0)));
    for (int i = 0; i < 2; i++)
      GetPanel(i)->BeforeUpdateDPI(layout);
  }

  UINT newDpi = (int)HIWORD(wparam);
  MoveWindow((LPRECT)lparam,TRUE);

  UpdateMenuParams();
  UpdateDPI(newDpi);

  // Set the text on the toolbar buttons again to force them to resize
  for (int i = 0; i < m_toolBar.GetToolBarCtrl().GetButtonCount(); i++)
  {
    UINT id = m_toolBar.GetItemID(i);
    if (id != ID_SEPARATOR)
    {
      CString btnText, tipText;
      btnText.LoadString(id);
      AfxExtractSubString(tipText,btnText,1,'\n');
      m_toolBar.SetButtonText(i,tipText);
    }
  }

  // If showing, update the list of examples
  if (m_exampleList.GetSafeHwnd() != 0)
  {
    m_exampleList.SetFont(m_toolBar.GetFont());
    SetExampleListLocation();
  }

  // Update the search bar in the toolbar
  m_searchBar.UpdateDPI();
  REBARBANDINFO bandInfo = { sizeof(REBARBANDINFO),0 };
  bandInfo.fMask = RBBIM_CHILDSIZE;
  CSize size = m_searchBar.CalcFixedLayout(FALSE,TRUE);
  bandInfo.cxMinChild = size.cx;
  bandInfo.cyMinChild = size.cy;
  m_coolBar.GetReBarCtrl().SetBandInfo(2,&bandInfo);
  m_coolBar.GetReBarCtrl().MaximizeBand(1);

  ReportHtml::UpdateWebBrowserPreferences(this);
  if (m_splitter.GetSafeHwnd() != 0)
  {
    for (int i = 0; i < 2; i++)
      GetPanel(i)->UpdateDPI(layout);

    auto layoutIt = layout.find(&m_splitter);
    if (layoutIt != layout.end())
    {
      m_splitter.SetColumnFraction(0,layoutIt->second,16);
      m_splitter.RecalcLayout();
    }
  }

  m_progress.UpdateDPI();
  return 0;
}

BOOL ProjectFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CPushRoutingFrame push(this);
  CWnd* focusWnd = GetFocus();

  // Let the interpreter window process the command first
  if (m_game.IsChild(focusWnd))
  {
    if (m_game.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;

    // If the interpreter did not process the command,
    // make sure it is routed to the correct panel
    if (((TabStory*)GetPanel(1)->GetTab(Panel::Tab_Story))->IsActive())
    {
      if (GetPanel(1)->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
        return TRUE;
    }
  }

  // Then let the panels process the command
  for (int i = 0; i < 2; i++)
  {
    if (GetPanel(i)->IsChild(focusWnd))
    {
      if (GetPanel(i)->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
        return TRUE;
    }
  }

  // Then try the search bar
  if ((m_searchBar.GetSafeHwnd() != 0) && m_searchBar.IsChild(focusWnd))
  {
    if (m_searchBar.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }

  // Then pump through frame
  if (CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through application
  if (AfxGetApp()->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return FALSE;
}

void ProjectFrame::OnUpdateFrameTitle(BOOL)
{
  CString title;
  title.Format("%s - %s",(LPCSTR)GetDisplayName(true),m_strTitle);
  AfxSetWindowText(GetSafeHwnd(),title);
}

LRESULT ProjectFrame::OnPlaySkein(WPARAM wparam, LPARAM)
{
  Skein::Node* current = m_skein.GetCurrent();
  Skein::Node* newNode = (Skein::Node*)wparam;

  // Change the current node
  m_skein.SetCurrent(newNode);

  // Check if the new node is reachable from the current node in the skein
  bool reachable = current->FindAncestor(newNode) != NULL;

  // Build and run if the game is not running, or if the new node is unreachable
  if (!m_game.IsRunning())
    OnPlayReplay();
  else if (!reachable)
    OnPlayReplay();
  else
    m_game.InputFromSkein();

  return 0;
}

LRESULT ProjectFrame::OnGameRunning(WPARAM wparam, LPARAM)
{
  return m_game.IsRunning() ? 1 : 0;
}

LRESULT ProjectFrame::OnGameWaiting(WPARAM wparam, LPARAM)
{
  return m_game.IsWaiting() ? 1 : 0;
}

LRESULT ProjectFrame::OnPaneHeading(WPARAM wparam, LPARAM)
{
  return ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source))->GetTabHeight();
}

LRESULT ProjectFrame::OnSelectSide(WPARAM side, LPARAM)
{
  GetPanel((int)side)->SetActiveTab(GetPanel((int)side)->GetActiveTab());
  return 0;
}

LRESULT ProjectFrame::OnSelectView(WPARAM view, LPARAM wnd)
{
  int panel = 0;
  if (GetPanel(0)->IsChild(CWnd::FromHandle((HWND)wnd)))
    panel = 1;

  CString viewName((const char*)view);
  if (viewName == "source")
    GetPanel(panel)->SetActiveTab(Panel::Tab_Source);
  else if ((viewName == "error") || (viewName == "results"))
    GetPanel(panel)->SetActiveTab(Panel::Tab_Results);
  else if ((viewName == "game") || (viewName == "story"))
    GetPanel(panel)->SetActiveTab(Panel::Tab_Story);
  else if (viewName == "documentation")
    GetPanel(panel)->SetActiveTab(Panel::Tab_Doc);
  else if (viewName == "index")
    GetPanel(panel)->SetActiveTab(Panel::Tab_Index);
  else if ((viewName == "skein") || (viewName == "testing"))
    GetPanel(panel)->SetActiveTab(Panel::Tab_Testing);
  return 0;
}

LRESULT ProjectFrame::OnPasteCode(WPARAM code, LPARAM)
{
  ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source))->PasteCode((const wchar_t*)code);
  return 0;
}

LRESULT ProjectFrame::OnRuntimeProblem(WPARAM problem, LPARAM)
{
  ((TabResults*)GetPanel(1)->GetTab(Panel::Tab_Results))->ShowRuntimeProblem((int)problem);
  GetPanel(1)->SetActiveTab(Panel::Tab_Results);
  return 0;
}

LRESULT ProjectFrame::OnSearchSource(WPARAM text, LPARAM)
{
  m_finder.Show();
  m_finder.FindInSource((LPCWSTR)text);
  return 0;
}

LRESULT ProjectFrame::OnSearchDoc(WPARAM text, LPARAM)
{
  m_finder.Show();
  m_finder.FindInDocs((LPCWSTR)text);
  return 0;
}

LRESULT ProjectFrame::OnShowSkein(WPARAM wparam, LPARAM lparam)
{
  Skein::Node* node = (Skein::Node*)wparam;
  if (m_skein.IsValidNode(node) == false)
    return 0;

  CWnd* wnd = CWnd::FromHandle((HWND)lparam);

  // If the skein is not visible, use the same panel as the calling window
  Panel* panel = NULL;
  if (GetPanel(0)->GetActiveTab() == Panel::Tab_Testing)
    panel = GetPanel(0);
  else if (GetPanel(1)->GetActiveTab() == Panel::Tab_Testing)
    panel = GetPanel(1);
  else
    panel = GetPanel(GetPanel(0)->IsChild(wnd) ? 0 : 1);

  // Move the skein to the given node and show the testing tab
  ((TabTesting*)panel->GetTab(Panel::Tab_Testing))->ShowNode(node,Skein::JustShow);
  panel->SetActiveTab(Panel::Tab_Testing);
  return 0;
}

LRESULT ProjectFrame::OnAnimateSkein(WPARAM wparam, LPARAM lparam)
{
  // Don't animate if using Terminal Services
  bool animate = true;
  if (::GetSystemMetrics(SM_REMOTESESSION) != 0)
    animate = false;

  // Don't animate if the skein isn't shown anywhere
  bool skein0 = (GetPanel(0)->GetActiveTab() == Panel::Tab_Testing);
  bool skein1 = (GetPanel(1)->GetActiveTab() == Panel::Tab_Testing);
  if (!skein0 && !skein1)
    animate = false;

  if (animate)
  {
    for (int pct = 0; pct < 100; pct += 10)
    {
      if (skein0)
        ((TabTesting*)GetPanel(0)->GetTab(Panel::Tab_Testing))->Animate(pct);
      if (skein1)
        ((TabTesting*)GetPanel(1)->GetTab(Panel::Tab_Testing))->Animate(pct);
      ::Sleep(5);
    }
  }

  m_skein.GetRoot()->AnimateClear();
  ((TabTesting*)GetPanel(0)->GetTab(Panel::Tab_Testing))->Animate(-1);
  ((TabTesting*)GetPanel(1)->GetTab(Panel::Tab_Testing))->Animate(-1);

  if (lparam != 0)
  {
    Command* cmd = (Command*)lparam;
    cmd->Run();
    delete cmd;
  }
  return 0;
}

LRESULT ProjectFrame::OnTerpFailed(WPARAM wparam, LPARAM lparam)
{
  ((TabResults*)GetPanel(1)->GetTab(Panel::Tab_Results))->ShowTerpFailed();
  GetPanel(1)->SetActiveTab(Panel::Tab_Results);
  return 0;
}

LRESULT ProjectFrame::OnProjectDir(WPARAM wparam, LPARAM lparam)
{
  return (LRESULT)(LPCSTR)m_projectDir;
}

LRESULT ProjectFrame::OnPlayNextThread(WPARAM wparam, LPARAM lparam)
{
  // Get the next skein thread task, if any
  if (m_playThreads.empty())
    return 0;
  PlaySkein play = m_playThreads.front();
  m_playThreads.pop();

  // Take the appropriate action
  switch (play.action)
  {
  case PlaySkeinThread:
    // Make sure that the node is still valid
    if (m_skein.IsValidNode(play.node) == false)
      return 0;

    // Play the node without recompiling
    m_skein.SetCurrent(play.node);
    m_game.StopInterpreter(false);
    m_skein.Reset(false);
    GetPanel(ChoosePanel(Panel::Tab_Story))->SetActiveTab(Panel::Tab_Story);
    RunProject();
    break;
  case ShowFirstSkeinError:
    {
      // Get the first skein node that is in error
      Skein::Node* node = m_skein.GetFirstDifferent();
      if (node != NULL)
        PostMessage(WM_SHOWSKEIN,(WPARAM)node,0);
      else
        ::MessageBeep(MB_ICONEXCLAMATION);
    }
    break;
  case ShowTestReport:
    {
      BusyProject busy(this);
      GenerateIntestReport("");
    }
    m_progress.LongTaskDone();
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
    break;
  case RunNextTest:
  case ReportThenRunNextTest:
    if (!BusyWantStop())
    {
      if (play.action == ReportThenRunNextTest)
      {
        BusyProject busy(this);
        GenerateIntestReport("");
      }

      int current = m_exampleList.GetCurSel();
      if (current > 0)
      {
        if (current < m_exampleList.GetCount()-1)
        {
          m_exampleList.SetCurSel(current+1);
          OnChangedExample();

          int count = m_exampleList.GetCount()-1;
          CString msg;
          msg.Format("Testing %d of %d",current+1,count);
          m_progress.LongTaskProgress(msg,2*current,2*count);
          TestCurrentExample(true);
        }
        else
        {
          m_exampleList.SetCurSel(0);
          OnChangedExample();

          {
            BusyProject busy(this);
            GenerateIntestCombinedReport();
          }
          GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
        }
      }
    }
    if (m_playThreads.empty())
      m_progress.LongTaskDone();
    break;
  default:
    ASSERT(FALSE);
    break;
  }
  return 1;
}

LRESULT ProjectFrame::OnCanPlayAll(WPARAM wparam, LPARAM lparam)
{
  return (m_skein.GetRoot()->GetNumChildren() > 0);
}

LRESULT ProjectFrame::OnProjectEdited(WPARAM wparam, LPARAM lparam)
{
  if (wparam != 0)
    m_needCompile = true;
  DelayUpdateFrameTitle();
  return 0;
}

LRESULT ProjectFrame::OnExtDownload(WPARAM urls, LPARAM)
{
  BusyProject busy(this);
  m_progress.ShowStop();
  CStringArray* libraryUrls = (CStringArray*)urls;
  ExtensionFrame::DownloadExtensions(this,libraryUrls);
  delete libraryUrls;
  return 0;
}

LRESULT ProjectFrame::OnProgress(WPARAM wp, LPARAM lp)
{
  int pos = (int)wp;
  const char* text = (const char*)lp;

  if (pos >= 0)
  {
    int lastPos = m_progress.GetProgress();
    m_progress.TaskProgress(text,pos);

    if ((lastPos < 95) && (pos >= 95))
      m_last5StartTime = ::GetTickCount();
  }
  else
  {
    // Make the progress bar invisible
    m_progress.TaskDone();
  }
  return 0;
}

LRESULT ProjectFrame::OnCreateNewProject(WPARAM code, LPARAM title)
{
  // Find the parent of the project directory
  CString lastDir = m_projectDir;
  int i = lastDir.ReverseFind('\\');
  if (i != -1)
    lastDir.Truncate(i);
  else
    lastDir.Empty();

  // Ask the user for where to create the project
  CString dir = theApp.PickDirectory("Choose the directory to create the new project in",
    "Location of new project:","Create Project",lastDir,this);
  if (!dir.IsEmpty())
  {
    CString projectDir;
    projectDir.Format("%s\\%S.inform",(LPCSTR)dir,(LPCWSTR)title);

    ProjectFrame* frame = NewFrame(Project_I7);
    ((TabSource*)frame->GetPanel(0)->GetTab(Panel::Tab_Source))->PasteCode((LPCWSTR)code);

    frame->SaveProject(projectDir);
    frame->GetPanel(0)->SetActiveTab(Panel::Tab_Source);
  }
  return 0;
}

LRESULT ProjectFrame::OnProjectExt(WPARAM wparam, LPARAM lparam)
{
  return (LRESULT)GetProjectFileExt();
}

LRESULT ProjectFrame::OnProjectType(WPARAM wparam, LPARAM lparam)
{
  return (LRESULT)m_projectType;
}

LRESULT ProjectFrame::OnStoryActive(WPARAM wparam, LPARAM lparam)
{
  // If the progress window is active, make sure it is still in front
  m_progress.ToFront();

  // Activating the story tab will change the status of the
  // story tab on the other pane, so redraw the entire window
  Invalidate();
  return 0;
}

LRESULT ProjectFrame::OnWantStop(WPARAM wparam, LPARAM lparam)
{
  return WantStop() ? 1 : 0;
}

LRESULT ProjectFrame::OnRunCensus(WPARAM wparam, LPARAM lparam)
{
  InformApp::CreatedProcess ni = theApp.RunCensus();
  if (ni.process != INVALID_HANDLE_VALUE)
  {
    MonitorProcess(ni,
      (wparam != 0) ? ProcessHelpExtensions : ProcessNoAction,"ni (census)");
  }
  return 0;
}

LRESULT ProjectFrame::OnStoryName(WPARAM wparam, LPARAM lparam)
{
  if (m_projectType == Project_I7XP)
  {
    Example ex = GetCurrentExample();
    if (!ex.name.IsEmpty())
      return (LRESULT)(new CString(ex.name));
  }

  CString title = m_projectDir;
  int i = m_projectDir.ReverseFind('\\');
  if (i >= 0)
  {
    int j = title.Find(GetProjectFileExt(),i+1);
    if (j > i+1)
    {
      title = title.Mid(i+1,j-i-1);
      return (LRESULT)(new CString(title));
    }
  }

  return 0;
}

CString ProjectFrame::GetDisplayName(bool fullName)
{
  CString name;
  if ((m_projectType == Project_I7XP) && fullName)
    name = "Extension Project - ";

  if (m_projectDir.IsEmpty())
    name += "Untitled";
  else
  {
    int start = m_projectDir.ReverseFind('\\');
    name += m_projectDir.Mid(start+1);
  }
  if (IsProjectEdited() && fullName)
    name += '*';
  return name;
}

void ProjectFrame::SendChanged(InformApp::Changed changed, int value)
{
  switch (changed)
  {
  case InformApp::Extensions:
    UpdateExtensionsMenu();
    break;
  case InformApp::Preferences:
    {
      CRegKey registryKey;
      if (registryKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ) == ERROR_SUCCESS)
      {
        DWORD I6debug = 0;
        if (registryKey.QueryDWORDValue("Generate I6 Debug",I6debug) == ERROR_SUCCESS)
          m_I6debug = (I6debug != 0);

        GetPanel(0)->PrefsChanged(registryKey);
        GetPanel(1)->PrefsChanged(registryKey);
      }
      m_game.PrefsChanged();
    }
    break;
  case InformApp::Spelling:
    ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source))->UpdateSpellCheck();
    ((TabSource*)GetPanel(1)->GetTab(Panel::Tab_Source))->UpdateSpellCheck();
    break;
  case InformApp::DownloadedExt:
    for (int i = 0; i < 2; i++)
      ((TabExtensions*)GetPanel(i)->GetTab(Panel::Tab_Extensions))->DownloadedExt(value);
    break;
  }
}

CString ProjectFrame::GetSource(void)
{
  return ((TabSource*)GetPanel(0)->GetTab(Panel::Tab_Source))->GetSource();
}

void ProjectFrame::SelectInSource(const CHARRANGE& range)
{
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Source));
  ((TabSource*)panel->GetTab(Panel::Tab_Source))->Select(range);
  panel->SetActiveTab(Panel::Tab_Source);
}

void ProjectFrame::SelectInDocumentation(const char* link, LPCWSTR find)
{
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Doc));
  ((TabDoc*)panel->GetTab(Panel::Tab_Doc))->Show(TextFormat::AnsiToUTF8(link),find);
  panel->SetActiveTab(Panel::Tab_Doc);
}

const ProjectSettings& ProjectFrame::GetSettings(void)
{
  return m_settings;
}

void ProjectFrame::OnFileNew()
{
  SaveSettings();
  StartNewProject(m_projectDir,this);
}

void ProjectFrame::OnFileOpen()
{
  SaveSettings();
  StartExistingProject(m_projectDir,this);
}

void ProjectFrame::OnFileInstallExt()
{
  CWaitCursor wc;
  ExtensionFrame::InstallExtensions(this);
}

void ProjectFrame::OnFileInstallFolder()
{
  // Get the path to the installed extensions directory
  CString path;
  path.Format("%s\\Inform\\Extensions",(LPCSTR)theApp.GetHomeDir());

  // Open an Explorer window
  ::ShellExecute(0,"explore",path,NULL,NULL,SW_SHOWNORMAL);
}

void ProjectFrame::OnFileInstallExtProject()
{
  if (m_busy || (m_projectType != Project_I7XP))
    return;
  if (SaveProject(m_projectDir) == false)
  {
    MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
    return;
  }

  CStringArray paths;
  paths.Add(m_projectDir+"\\Source\\extension.i7x");

  CWaitCursor wc;
  ExtensionFrame::InstallExtensions(this,paths);
}

void ProjectFrame::OnFileNewExt()
{
  SaveSettings();
  ExtensionFrame::StartNew(this,m_settings);
}

void ProjectFrame::OnFileNewExtProject()
{
  SaveSettings();
  StartNewExtProject(m_projectDir,this,NULL);
}

void ProjectFrame::OnFileOpenExt(UINT nID)
{
  int index = nID-ID_OPEN_EXTENSIONS_LIST;
  const std::vector<InformApp::ExtLocation>& extensions = theApp.GetExtensions();
  if ((index >= 0) && (index < (int)extensions.size()))
  {
    SaveSettings();
    ExtensionFrame::StartExisting(extensions[index].path.c_str(),m_settings);
  }
}

void ProjectFrame::OnFileNewXPFromExt(UINT nID)
{
  int index = nID-ID_NEW_EXTENSIONS_LIST;
  const std::vector<InformApp::ExtLocation>& extensions = theApp.GetExtensions();
  if ((index >= 0) && (index < (int)extensions.size()))
  {
    SaveSettings();
    StartNewExtProject(m_projectDir,this,&(extensions[index]));
  }
}

void ProjectFrame::OnFileClose()
{
  if (!m_busy)
    SendMessage(WM_CLOSE);
}

void ProjectFrame::OnFileSave()
{
  if (!m_busy)
  {
    if (!SaveProject(m_projectDir))
      MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
  }
}

void ProjectFrame::OnFileSaveAs()
{
  if (!m_busy)
  {
    // Ask for a project to save as
    ProjectDirDialog dialog(false,m_projectDir,"Save the project",GetProjectFileExt(),this);
    if (dialog.ShowDialog() == IDOK)
    {
      if (!SaveProject(dialog.GetProjectDir()))
        MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
    }
  }
}

void ProjectFrame::OnFileImportSkein()
{
  // Ask the user for a file to import
  SimpleFileDialog dialog(TRUE,"rec",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
    "Interpreter recording files (*.rec;*.*)|*.*||",this);
  dialog.m_ofn.lpstrTitle = "Select the file to import into the skein";
  if (dialog.DoModal() == IDOK)
    m_skein.Import(dialog.GetPathName());
}

void ProjectFrame::OnFileExportExtProject()
{
  if (m_busy || (m_projectType != Project_I7XP))
    return;
  if (SaveProject(m_projectDir) == false)
  {
    MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
    return;
  }

  CString sourcePath;
  CStringW extName, extAuthor;
  if (!GetExtensionInfo(sourcePath,extName,extAuthor))
    return;

  CString saveName(extName);
  saveName += ".i7x";
  SimpleFileDialog dialog(FALSE,"i7x",saveName,OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_OVERWRITEPROMPT,
    "Inform extensions (*.i7x)|*.i7x|All Files (*.*)|*.*||",this);
  dialog.m_ofn.lpstrTitle = "Export this extension";
  if (dialog.DoModal() == IDOK)
    ::CopyFile(sourcePath,dialog.GetPathName(),FALSE);
}

void ProjectFrame::OnEditFindInFiles()
{
  m_finder.Show();
}

void ProjectFrame::OnUpdateIfNotBusy(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(!m_busy && m_playThreads.empty());
}

void ProjectFrame::OnUpdateCompile(CCmdUI *pCmdUI)
{
  switch (m_projectType)
  {
  case Project_I7:
    pCmdUI->Enable(!m_busy && m_playThreads.empty());
    break;
  case Project_I7XP:
    pCmdUI->Enable(!m_busy && m_playThreads.empty() && (m_exampleList.GetCurSel() > 0));
    break;
  default:
    ASSERT(0);
    break;
  }
}

void ProjectFrame::OnPlayGo()
{
  if (CompileProject(false,false,false))
  {
    m_skein.Reset(true);
    GetPanel(ChoosePanel(Panel::Tab_Story))->SetActiveTab(Panel::Tab_Story);
    RunProject();
  }
  else
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
}

void ProjectFrame::OnPlayReplay()
{
  if (CompileProject(false,false,false))
  {
    m_skein.Reset(false);
    GetPanel(ChoosePanel(Panel::Tab_Story))->SetActiveTab(Panel::Tab_Story);
    RunProject();
  }
  else
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
}

void ProjectFrame::OnPlayTest()
{
  // Discard any previous threads to be played
  while (!m_playThreads.empty())
    m_playThreads.pop();

  switch (m_projectType)
  {
  case Project_I7:
    if (CompileProject(false,true,false))
    {
      m_skein.Reset(true);
      m_skein.NewLine(L"test me");
      m_skein.Reset(false);
      GetPanel(ChoosePanel(Panel::Tab_Story))->SetActiveTab(Panel::Tab_Story);
      RunProject();
    }
    break;

  case Project_I7XP:
    {
      // Is this the start of a test of all examples?
      bool testAll = false;
      if (m_exampleList.GetCurSel() == 0)
      {
        {
          BusyProject busy(this);
          if (SaveProject(m_projectDir))
          {
            if (UpdateExampleList())
            {
              if (m_exampleList.GetCount() > 1)
              {
                // If so, select the first example
                m_exampleList.SetCurSel(1);
                OnChangedExample();
                testAll = true;
              }
            }
          }
          else
          {
            MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
            return;
          }
        }
        if (!testAll)
        {
          MessageBox("There are no examples to test.",INFORM_TITLE,MB_OK|MB_ICONERROR);
          return;
        }
      }

      // Compile and test the selected example
      if (testAll)
      {
        int count = m_exampleList.GetCount()-1;
        CString msg;
        msg.Format("Testing 1 of %d",count);
        m_progress.LongTaskProgress(msg,0,2*count);
      }
      else
        m_progress.LongTaskProgress("Testing",0,2);
      m_progress.ShowStop();
      if (testAll)
        GetPanel(ChoosePanel(Panel::Tab_Testing))->SetActiveTab(Panel::Tab_Testing);
      TestCurrentExample(testAll);
      if (m_playThreads.empty())
        m_progress.LongTaskDone();
    }
    break;

  default:
    ASSERT(0);
    break;
  }
}

void ProjectFrame::OnPlayRefresh()
{
  // Get the current focus window
  HWND focus = GetFocus()->GetSafeHwnd();

  // Compile the project and show the index
  if (CompileProject(false,false,true))
    GetPanel(ChoosePanel(Panel::Tab_Index))->SetActiveTab(Panel::Tab_Index);
  else
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);

  // Return the focus to its original point if still visible
  if (::IsWindow(focus) && ::IsWindowVisible(focus))
    ::SetFocus(focus);
}

void ProjectFrame::OnPlayLoad()
{
  SimpleFileDialog dialog(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
    "Z-code games (*.z?;*.zblorb)|*.z?;*.zblorb|Glulx games (*.ulx;*.gblorb)|*.ulx;*.gblorb||",this);
  dialog.m_ofn.lpstrTitle = "Select a story to play";
  dialog.m_ofn.nFilterIndex = m_loadFilter;
  if (dialog.DoModal() != IDOK)
    return;

  CString path = dialog.GetPathName();
  int split = path.ReverseFind('\\');
  if (split == -1)
    return;

  m_game.StopInterpreter(false);
  m_skein.Reset(true);

  GetPanel(ChoosePanel(Panel::Tab_Story))->SetActiveTab(Panel::Tab_Story);
  m_loadFilter = dialog.m_ofn.nFilterIndex;
  bool glulx = (m_loadFilter == 2);
  m_game.RunInterpreter(path.Left(split),path.Mid(split+1),glulx);
}

LRESULT ProjectFrame::OnReplayAll(WPARAM, LPARAM)
{
  // Discard any previous threads to be played
  while (!m_playThreads.empty())
    m_playThreads.pop();

  // Find all the end nodes of threads in the skein
  std::vector<Skein::Node*> ends;
  m_skein.GetThreadEnds(ends);
  if (ends.empty())
    return 0;

  // Get the first node and store the rest
  Skein::Node* firstEnd = ends[0];
  for (size_t i = 1; i < ends.size(); i++)
  {
    PlaySkein play(PlaySkeinThread);
    play.node = ends[i];
    m_playThreads.push(play);
  }

  // Add a final task to show the first skein error
  PlaySkein showError(ShowFirstSkeinError);
  m_playThreads.push(showError);

  // Play the thread leading to the first node
  m_skein.SetCurrent(firstEnd);
  OnPlayReplay();
  return 0;
}

void ProjectFrame::OnUpdateReleaseGame(CCmdUI *pCmdUI)
{
  switch (m_projectType)
  {
  case Project_I7:
    pCmdUI->Enable(!m_busy && m_playThreads.empty());
    break;
  case Project_I7XP:
    pCmdUI->Enable(FALSE);
    break;
  default:
    ASSERT(0);
    break;
  }
}

void ProjectFrame::OnReleaseGame(UINT nID)
{
  if (CompileProject((nID == ID_RELEASE_GAME),false,true))
  {
    CString releasePath;
    const char* blorbExt = NULL;
    const char* filter = NULL;

    if (m_settings.m_output == ProjectSettings::OutputGlulx)
    {
      blorbExt = "gblorb";
      filter = "Glulx games (*.ulx;*.gblorb)|*.ulx;*.gblorb|All Files (*.*)|*.*||";
    }
    else
    {
      blorbExt = "zblorb";
      filter = "Z-code games (.z?;.zblorb)|*.z?;*.zblorb|All Files (*.*)|*.*||";
    }

    // Create a Blorb file, if needed
    int code = 0;
    if (m_settings.m_blorb)
    {
      CString executable, arguments;
      executable.Format("%s\\Compilers\\cblorb",(LPCSTR)theApp.GetAppDir());
      arguments.Format("-windows Release.blurb Build\\output.%s",blorbExt);

      CString output;
      output.Format("%s \\\n    %s\n",(LPCSTR)executable,(LPCSTR)arguments);
      Output(output);

      CString cmdLine;
      cmdLine.Format("\"%s\" %s",(LPCSTR)executable,(LPCSTR)arguments);

      m_outputFileLoc.Empty();
      code = theApp.RunCommand(m_projectDir,cmdLine,"cblorb.exe",*this,true);

      GetPanel(0)->CompileProject(TabInterface::RanCBlorb,code);
      GetPanel(1)->CompileProject(TabInterface::RanCBlorb,code);

      // If cBlorb picked a location to save the file, use it
      releasePath = m_outputFileLoc;
    }

    // Show the result
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);

    // If creating the Blorb file failed, stop
    if (code != 0)
      return;

    // Get the appropriate file name extension
    CString extension = m_settings.m_blorb ? blorbExt : m_settings.GetOutputFormat();

    // Should we ask the user where to copy to?
    if (releasePath.IsEmpty())
    {
      // Work out a suitable output file name
      CString fileName = GetDisplayName(false);
      fileName.Truncate(fileName.ReverseFind('.')+1);
      fileName.Append(extension);

      // Ask the user where to save
      SimpleFileDialog dialog(FALSE,extension,fileName,
        OFN_HIDEREADONLY|OFN_ENABLESIZING|OFN_OVERWRITEPROMPT,filter,this);
      dialog.m_ofn.lpstrTitle = "Save the story for release";
      if (dialog.DoModal() == IDOK)
        releasePath = dialog.GetPathName();
    }

    // Copy the game file
    if (releasePath.IsEmpty() == FALSE)
      ::CopyFile(m_projectDir+"\\Build\\output."+extension,releasePath,FALSE);
  }
  else
    GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
}

void ProjectFrame::OnReleaseMaterials()
{
  // If the path to the ".materials" directory doesn't exist, create it
  CString path = GetMaterialsFolder();
  if (::GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
    ::SHCreateDirectoryEx(GetSafeHwnd(),path,NULL);

  // Open an Explorer window
  ::ShellExecute(0,"explore",path,NULL,NULL,SW_SHOWNORMAL);
}

void ProjectFrame::OnReleaseIFiction()
{
  // Compile the project
  bool fileCreated = false;
  if (CompileProject(false,false,true))
  {
    // Check for an iFiction file
    CString iFictionFile;
    iFictionFile.Format("%s\\Metadata.iFiction",(LPCSTR)m_projectDir);
    if (::GetFileAttributes(iFictionFile) != INVALID_FILE_ATTRIBUTES)
    {
      fileCreated = true;

      // Ask the user where the file is to be copied
      SimpleFileDialog dialog(FALSE,"iFiction",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
        "iFiction metadata (*.iFiction)|*.iFiction|All Files (*.*)|*.*||",this);
      dialog.m_ofn.lpstrTitle = "Save iFiction record";
      if (dialog.DoModal() == IDOK)
        ::CopyFile(iFictionFile,dialog.GetPathName(),FALSE);
    }
  }

  if (!fileCreated)
  {
    MessageBox(
      "The compiler failed to create an iFiction record;\n"
      "check the results page to see why.",INFORM_TITLE,MB_OK|MB_ICONERROR);
  }
}

void ProjectFrame::OnWindowLeftPane()
{
  // Switch the focus to the left panel
  GetPanel(0)->SetActiveTab(GetPanel(0)->GetActiveTab());
}

void ProjectFrame::OnWindowRightPane()
{
  // Switch the focus to the right panel
  GetPanel(1)->SetActiveTab(GetPanel(1)->GetActiveTab());
}

void ProjectFrame::OnWindowSwitchPanes()
{
  // Switch the focus to the other panel
  if (GetPanel(0)->IsChild(GetFocus()))
    GetPanel(1)->SetActiveTab(GetPanel(1)->GetActiveTab());
  else
    GetPanel(0)->SetActiveTab(GetPanel(0)->GetActiveTab());
}

void ProjectFrame::OnWindowShowTab(UINT nID)
{
  int index = nID-ID_WINDOW_TAB_SOURCE;
  Panel::Tabs tab = (Panel::Tabs)(Panel::Tab_Source+index);
  int panel = GetPanel(0)->IsChild(GetFocus()) ? 0 : 1;
  GetPanel(panel)->SetActiveTab(tab);
}

void ProjectFrame::OnUpdateWindowShowTab(CCmdUI *pCmdUI)
{
  int index = pCmdUI->m_nID-ID_WINDOW_TAB_SOURCE;
  Panel::Tabs tab = (Panel::Tabs)(Panel::Tab_Source+index);
  int panel = GetPanel(0)->IsChild(GetFocus()) ? 0 : 1;
  pCmdUI->Enable(GetPanel(panel)->IsTabEnabled(tab));
}

void ProjectFrame::OnWindowShowIndex(UINT nID)
{
  int index = nID-ID_WINDOW_INDEX_HOME;
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Index));
  ((TabIndex*)panel->GetTab(Panel::Tab_Index))->ShowIndex(index);
  panel->SetActiveTab(Panel::Tab_Index);
}

void ProjectFrame::OnUpdateWindowList(CCmdUI *pCmdUI)
{
  CMenu* windowMenu = GetMenu()->GetSubMenu(6);
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
    else if (frames[i]->IsKindOf(RUNTIME_CLASS(BookFrame)))
      name = ((BookFrame*)frames[i])->GetDisplayName();
    else if (frames[i]->IsKindOf(RUNTIME_CLASS(WelcomeLauncherFrame)))
      name = "Welcome Launcher";

    menu.Format("&%d %s",i+1,(LPCSTR)name);

    UINT flags = (frames[i] == this) ? MF_CHECKED|MF_STRING : MF_STRING;
    windowMenu->AppendMenu(flags,ID_WINDOW_LIST+i,menu);
  }
}

void ProjectFrame::OnWindowList(UINT nID)
{
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);

  int index = nID-ID_WINDOW_LIST;
  if ((index >= 0) && (index < frames.GetSize()))
    frames[index]->ActivateFrame();
}

void ProjectFrame::OnHelpIndex()
{
  CString path = theApp.GetAppDir()+"\\Documentation\\index.html";
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Doc));
  ((TabDoc*)panel->GetTab(Panel::Tab_Doc))->Show(TextFormat::AnsiToUTF8(path));
  panel->SetActiveTab(Panel::Tab_Doc);
}

void ProjectFrame::OnHelpLicence()
{
  CString path = theApp.GetAppDir()+"\\Documentation\\licences\\licence.html";
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Doc));
  ((TabDoc*)panel->GetTab(Panel::Tab_Doc))->Show(TextFormat::AnsiToUTF8(path));
  panel->SetActiveTab(Panel::Tab_Doc);
}

void ProjectFrame::OnHelpWindows()
{
  CString path = theApp.GetAppDir()+"\\Documentation\\windows\\Windows.html";
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Doc));
  ((TabDoc*)panel->GetTab(Panel::Tab_Doc))->Show(TextFormat::AnsiToUTF8(path));
  panel->SetActiveTab(Panel::Tab_Doc);
}

void ProjectFrame::OnHelpExtensions()
{
  CString path = theApp.GetHomeDir()+"\\Inform\\Documentation\\Extensions.html";
  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Extensions));
  ((TabExtensions*)panel->GetTab(Panel::Tab_Extensions))->Show(TextFormat::AnsiToUTF8(path));
  panel->SetActiveTab(Panel::Tab_Extensions);
}

void ProjectFrame::OnHelpRecipes()
{
  CString recipePath = theApp.GetAppDir()+"\\Documentation\\Rindex.html";
  if (::GetFileAttributes(recipePath) == INVALID_FILE_ATTRIBUTES)
    recipePath = theApp.GetAppDir()+"\\Documentation\\Rdoc1.html";

  Panel* panel = GetPanel(ChoosePanel(Panel::Tab_Doc));
  ((TabDoc*)panel->GetTab(Panel::Tab_Doc))->Show(TextFormat::AnsiToUTF8(recipePath));
  panel->SetActiveTab(Panel::Tab_Doc);
}

void ProjectFrame::OnSearchSource()
{
  m_searchBar.SearchSource();
}

void ProjectFrame::OnSearchDocs()
{
  m_searchBar.SearchDocs();
}

ProjectFrame* ProjectFrame::NewFrame(ProjectType projectType)
{
  ProjectFrame* frame = new ProjectFrame(projectType);
  theApp.NewFrame(frame);

  frame->LoadFrame(IDR_MAINFRAME,WS_OVERLAPPEDWINDOW|FWS_ADDTOTITLE,NULL,NULL);
  frame->SetFromRegistryPath(REGISTRY_INFORM_WINDOW);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();

  BOOL cues;
  if (::SystemParametersInfo(SPI_GETKEYBOARDCUES,0,&cues,0) == 0)
    cues = TRUE;
  frame->SendMessage(WM_CHANGEUISTATE,MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET,UISF_HIDEFOCUS));

  return frame;
}

ProjectType ProjectFrame::TypeFromDir(const CString& projectDir)
{
  // Get the last part of the path
  int i = projectDir.ReverseFind('\\');
  CString last = projectDir.Mid(i+1);

  // Look at the extension on the last part
  i = last.ReverseFind('.');
  if (last.Mid(i).CompareNoCase(".i7xp") == 0)
    return Project_I7XP;
  return Project_I7;
}

void ProjectFrame::SetFromRegistryPath(const char* path)
{
  if (m_registryKey.Create(HKEY_CURRENT_USER,path) == ERROR_SUCCESS)
  {
    // Restore the window state
    WINDOWPLACEMENT place;
    ULONG len = sizeof WINDOWPLACEMENT;
    if (m_registryKey.QueryBinaryValue("Placement 96dpi",&place,&len) == ERROR_SUCCESS)
    {
      DPI::ContextUnaware dpiUnaware;
      SetWindowPlacement(&place);
    }
    else if (m_registryKey.QueryBinaryValue("Placement",&place,&len) == ERROR_SUCCESS)
      SetWindowPlacement(&place);
    else
    {
      CRect screen;
      ::SystemParametersInfo(SPI_GETWORKAREA,0,(LPRECT)screen,0);
      MoveWindow(0,0,screen.Width()*7/8,screen.Height()*9/10,FALSE);
      CenterWindow();
      ShowWindow(SW_MAXIMIZE);
    }

    // Restore the splitter position
    DWORD splitter;
    if (m_registryKey.QueryDWORDValue("Splitter2",splitter) == ERROR_SUCCESS)
      m_splitter.SetColumnFraction(0,0.001*splitter,16);
    else if (m_registryKey.QueryDWORDValue("Splitter",splitter) == ERROR_SUCCESS)
      m_splitter.SetColumnInfo(0,splitter,16);
    else
      m_splitter.SetColumnFraction(0,0.5,16);
    m_splitter.RecalcLayout();

    // Restore the default project directory for the file dialog
    char dir[MAX_PATH];
    len = sizeof dir;
    if (m_registryKey.QueryStringValue("Last Project",dir,&len) == ERROR_SUCCESS)
      m_projectDir = dir;

    // Restore whether or not to generate Inform 6 debugging output
    DWORD I6debug = 0;
    if (m_registryKey.QueryDWORDValue("Generate I6 Debug",I6debug) == ERROR_SUCCESS)
      m_I6debug = (I6debug != 0);
    
    // Allow tabs to load settings
    GetPanel(0)->LoadSettings(m_registryKey,true);
    GetPanel(1)->LoadSettings(m_registryKey,false);
  }
}

void ProjectFrame::SaveSettings(void)
{
  if ((HKEY)m_registryKey != 0)
  {
    // Save the window state
    WINDOWPLACEMENT place;
    place.length = sizeof place;
    {
      DPI::ContextUnaware dpiUnaware;
      GetWindowPlacement(&place);
    }
    m_registryKey.SetBinaryValue("Placement 96dpi",&place,sizeof WINDOWPLACEMENT);

    // Save the splitter position
    m_registryKey.SetDWORDValue("Splitter2",(DWORD)(0.5+(1000*m_splitter.GetColumnFraction(0))));

    // Save the default project directory for the file dialog
    m_registryKey.SetStringValue("Last Project",m_projectDir);

    // Allow tabs to save settings
    GetPanel(0)->SaveSettings(m_registryKey,true);
    GetPanel(1)->SaveSettings(m_registryKey,false);
  }
}

bool ProjectFrame::StartNewProject(const char* dir, CWnd* parent)
{
  NewProjectDialog dialog(Project_I7,dir,parent);
  if (dialog.DoModal() != IDOK)
    return false;

  CStringW initialCode;
  initialCode.Format(L"\"%S\" by %s\n\n",dialog.GetName(),dialog.GetAuthor());

  ProjectFrame* frame = NewFrame(Project_I7);
  ((TabSource*)frame->GetPanel(0)->GetTab(Panel::Tab_Source))->PasteCode(initialCode);

  frame->SaveProject(dialog.GetPath());
  frame->GetPanel(0)->SetActiveTab(Panel::Tab_Source);
  return true;
}

bool ProjectFrame::StartNewExtProject(const char* dir, CWnd* parent, const InformApp::ExtLocation* fromExt)
{
  NewProjectDialog dialog(Project_I7XP,dir,parent);
  if (fromExt != NULL)
    dialog.FromExt(fromExt->title.c_str(),fromExt->author.c_str());
  if (dialog.DoModal() != IDOK)
    return false;

  ProjectFrame* frame = NewFrame(Project_I7XP);
  CString projectDir = dialog.GetPath();
  if (fromExt)
  {
    ::CreateDirectory(projectDir,NULL);
    ::CreateDirectory(projectDir+"\\Source",NULL);
    ::CopyFile(fromExt->path.c_str(),projectDir+"\\Source\\extension.i7x",FALSE);
    ((TabSource*)frame->GetPanel(0)->GetTab(Panel::Tab_Source))->OpenProject(projectDir,true);
  }
  else
  {
    CStringW initialCode;
    initialCode.Format(L"%S by %s begins here.\n\n%S ends here.\n",
      dialog.GetName(),dialog.GetAuthor(),dialog.GetName());
    ((TabSource*)frame->GetPanel(0)->GetTab(Panel::Tab_Source))->PasteCode(initialCode);
  }

  frame->SaveProject(projectDir);
  frame->GetPanel(0)->SetActiveTab(Panel::Tab_Source);
  frame->UpdateExampleList();
  return true;
}

bool ProjectFrame::StartExistingProject(const char* dir, CWnd* parent)
{
  ProjectDirDialog dialog(true,dir,"Open a project","",parent);
  if (dialog.ShowDialog() != IDOK)
    return false;
  CString project = dialog.GetProjectDir();
  return StartNamedProject(project);
}

bool ProjectFrame::StartNamedProject(const char* project)
{
  // Is the project already open?
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    if (frames[i]->IsKindOf(RUNTIME_CLASS(ProjectFrame)))
    {
      ProjectFrame* projFrame = (ProjectFrame*)frames[i];
      if (projFrame->m_projectDir.CompareNoCase(project) == 0)
      {
        projFrame->ActivateFrame();
        return false;
      }
    }
  }

  if (::GetFileAttributes(project) == INVALID_FILE_ATTRIBUTES)
    return false;

  ProjectFrame* frame = NewFrame(TypeFromDir(project));
  frame->OpenProject(project);
  return true;
}

void ProjectFrame::OpenProject(const char* project)
{
  // Stop the game if running
  m_game.StopInterpreter(true);

  // As this can take a while, make sure the display is updated
  theApp.RunMessagePump();

  // Set the project directory
  m_projectDir = project;
  theApp.AddToRecentFileList(m_projectDir);

  // Rename any old-style " Materials" folder to ".materials"
  int projectExt = m_projectDir.Find(GetProjectFileExt());
  if (projectExt > 0)
  {
    CString fromPath, toPath;
    fromPath.Format("%s Materials",m_projectDir.Left(projectExt));
    toPath.Format("%s.materials",m_projectDir.Left(projectExt));
    ::MoveFile(fromPath,toPath);
  }

  // Open files in the project
  GetPanel(0)->OpenProject(m_projectDir,true);
  GetPanel(1)->OpenProject(m_projectDir,false);
  GetPanel(0)->SetActiveTab(Panel::Tab_Source);

  if (m_projectType == Project_I7XP)
    UpdateExampleList();
}

bool ProjectFrame::SaveProject(const char* project)
{
  theApp.RunMessagePump();

  // Create project directories
  m_projectDir = project;
  ::CreateDirectory(m_projectDir,NULL);
  ::CreateDirectory(m_projectDir+"\\Build",NULL);
  ::CreateDirectory(m_projectDir+"\\Index",NULL);
  ::CreateDirectory(m_projectDir+"\\Source",NULL);

  // Save the project from the left hand panel
  bool saved = GetPanel(0)->SaveProject(m_projectDir,true);
  GetPanel(1)->SaveProject(m_projectDir,false);
  theApp.AddToRecentFileList(m_projectDir);
  return saved;
}

bool ProjectFrame::CompileProject(bool release, bool test, bool force)
{
  BusyProject busy(this);

  // Stop the game if running
  m_game.StopInterpreter(false);

  // Do we need to compile?
  if (!force)
  {
    if (!m_needCompile)
    {
      // If the source or settings files are more recent than the compiler
      // output, we need to compile again.
      CString outPath;
      outPath.Format("%s\\Build\\output.%s",(LPCSTR)m_projectDir,
        (LPCSTR)m_settings.GetOutputFormat());
      CFileStatus outStatus;
      if (CFile::GetStatus(outPath,outStatus))
      {
        TabSource* tab = (TabSource*)(GetPanel(0)->GetTab(Panel::Tab_Source));
        if (tab->GetFileTimestamp(m_projectDir) > outStatus.m_mtime)
          m_needCompile = true;
        if (m_settings.GetFileTimestamp(m_projectDir) > outStatus.m_mtime)
          m_needCompile = true;
      }
    }
    if (!m_needCompile)
      return true;
  }

  // Save the project first
  if (SaveProject(m_projectDir) == false)
  {
    MessageBox("Failed to save project",INFORM_TITLE,MB_OK|MB_ICONERROR);
    return false;
  }

  if (m_projectType == Project_I7XP)
  {
    // Update the list of examples
    if (!UpdateExampleList())
      return false;

    // Copy the extension to the materials folder
    if (!CopyExtensionToMaterials())
    {
      MessageBox("Failed to copy the extension to the materials folder",INFORM_TITLE,MB_OK|MB_ICONERROR);
      return false;
    }
  }

  // Create the UUID file if needed
  CString uuidFile;
  uuidFile.Format("%s\\uuid.txt",(LPCSTR)m_projectDir);
  if (::GetFileAttributes(uuidFile) == INVALID_FILE_ATTRIBUTES)
  {
    UUID uuid;
    ::UuidCreate(&uuid);
    unsigned char* uuidStr;
    ::UuidToString(&uuid,&uuidStr);

    FILE* f = fopen(uuidFile,"wt");
    if (f != NULL)
    {
      fwrite(uuidStr,1,strlen((const char*)uuidStr),f);
      fclose(f);
    }

    ::RpcStringFree(&uuidStr);
  }

  // Start compiling ...
  int code = 0;
  CString failed;

  // Get the current focus window
  HWND focus = GetFocus()->GetSafeHwnd();

  // Notify panels that compilation is starting
  GetPanel(0)->CompileProject(TabInterface::CompileStart,0);
  GetPanel(1)->CompileProject(TabInterface::CompileStart,0);

  m_exampleCompiled = Example();
  if (m_projectType == Project_I7XP)
  {
    // Decide on the example to compile
    m_exampleCompiled = GetCurrentExample();
    if (m_exampleCompiled.id == 0)
      return false;

    // Run intest to extract the example
    IntestOutputSink sink;
    code = theApp.RunCommand(NULL,IntestSourceCommandLine(),"intest.exe",sink,true);
    sink.Done();

    // Read the intest output describing how to map from example story line numbers
    // to extension source line numbers
    for (int i = 0; i < m_exLineOffsets.GetSize();)
    {
      if (m_exLineOffsets.GetAt(i).id == m_exampleCompiled.id)
        m_exLineOffsets.RemoveAt(i);
      else
        i++;
    }
    if (code == 0)
    {
      for (int i = 0; i < sink.results.GetSize(); i++)
      {
        int from, offset;
        if (swscanf(sink.results.GetAt(i),L"%d %d",&from,&offset) == 2)
          m_exLineOffsets.Add(ExLineOffset(m_exampleCompiled.id,from,offset));
      }
    }

    GetPanel(0)->CompileProject(TabInterface::RanIntestSource,code);
    GetPanel(1)->CompileProject(TabInterface::RanIntestSource,code);
  }

  // Run Natural Inform
  if (code == 0)
  {
    m_last5StartTime = ::GetTickCount();
    code = theApp.RunCommand(NULL,NaturalCommandLine(release),"ni.exe",*this,
      m_settings.GetCompilerVersion() == NI_BUILD);
    if (code != 0)
      failed = "i7";

    GetPanel(0)->CompileProject(TabInterface::RanNaturalInform,code);
    GetPanel(1)->CompileProject(TabInterface::RanNaturalInform,code);

    // Warn if Microsoft Defender Antivirus might be slowing down the Natural Inform compiler
    if (!test && (code == 0))
    {
      if (theApp.GetProfileInt("Window","Slow Compile Warn",1) != 0)
      {
        // Check if the last 5% of compiling took more than 15 seconds
        DWORD niLast5Time = ::GetTickCount() - m_last5StartTime;
        if (niLast5Time > GetMaxLast5Time())
        {
          TASKDIALOGCONFIG config = { 0 };
          config.cbSize = sizeof config;
          config.hwndParent = GetSafeHwnd();
          config.dwCommonButtons = TDCBF_OK_BUTTON;
          config.pszWindowTitle = L_INFORM_TITLE;
          config.pszMainIcon = TD_WARNING_ICON;
          config.pszMainInstruction = L"It took longer than expected to convert the story text to Inform 6 code.";
          config.pszContent =
            L"Microsoft Defender Antivirus is known to slow Inform 7 down. If it is enabled then you should "
            L"consider configuring it to exclude from scanning the Inform 7 installation directory, the "
            L"\"My Documents\\Inform\" directory, and any directories containing Inform 7 projects. This can be "
            L"done from the \"Windows Security\" app: go to \"Virus and Threat Protection\", "
            L"then \"Manage Settings\", then \"Add or Remove Exclusions\".";
          config.pszVerificationText = L"Don't warn about this any more";
          config.cxWidth = 300;
          BOOL dontWarn = FALSE;
          if (SUCCEEDED(::TaskDialogIndirect(&config,NULL,NULL,&dontWarn)))
            theApp.WriteProfileInt("Window","Slow Compile Warn",dontWarn ? 0 : 1);
        }
      }
    }
  }

  // Run Inform 6
  if (code == 0)
  {
    SendMessage(WM_PROGRESS,100,(LPARAM)"Creating story file");
    code = theApp.RunCommand(m_projectDir+"\\Build",InformCommandLine(release),"inform6.exe",*this,true);
    if (code != 0)
    {
      failed = "i6";
      SetMessageText("Creating the story file with Inform 6 has failed");
    }

    GetPanel(0)->CompileProject(TabInterface::RanInform6,code);
    GetPanel(1)->CompileProject(TabInterface::RanInform6,code);
  }

  // Show the final exit code
  CString final;
  final.Format("\nCompiler finished with code %d\n",code);
  Output(final);
  SendMessage(WM_PROGRESS,-1);

  // Generate and show results
  if (!WantStop() && (code > 0) && (code < 10))
  {
    switch (m_projectType)
    {
    case Project_I7:
      GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
      break;
    case Project_I7XP:
      if (test)
        GenerateIntestReport(failed);
      else
        GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
      break;
    default:
      ASSERT(0);
      break;
    }
  }

  // Return the focus to its original point if still visible
  if (::IsWindow(focus) && ::IsWindowVisible(focus))
    ::SetFocus(focus);

  // Finished compiling
  if (code == 0)
    m_needCompile = false;
  return (code == 0);
}

void ProjectFrame::RunProject(void)
{
  // Start the interpreter
  m_game.RunInterpreter(m_projectDir+"\\Build",
    "output."+m_settings.GetOutputFormat(),
    m_settings.m_output == ProjectSettings::OutputGlulx);

  // Send out a skein notification now that the game is running
  m_skein.NotifyChange(Skein::ThreadChanged);
}

void ProjectFrame::CleanProject(void)
{
  if ((HKEY)m_registryKey != 0)
  {
    DWORD cleanFiles;
    if (m_registryKey.QueryDWORDValue("Clean Up Files",cleanFiles) == ERROR_SUCCESS)
    {
      if (cleanFiles == 0)
        return;
    }
  }

  CFileFind find;
  BOOL found = find.FindFile(m_projectDir+"\\Build\\*.*");
  while (found)
  {
    found = find.FindNextFile();
    ::DeleteFile(find.GetFilePath());
  }

  bool indexes = true;
  if ((HKEY)m_registryKey != 0)
  {
    DWORD cleanIndexes;
    if (m_registryKey.QueryDWORDValue("Clean Up Indexes",cleanIndexes) == ERROR_SUCCESS)
      indexes = (cleanIndexes != 0);
  }

  if (indexes)
  {
    found = find.FindFile(m_projectDir+"\\Index\\Details\\*.*");
    while (found)
    {
      found = find.FindNextFile();
      ::DeleteFile(find.GetFilePath());
    }

    found = find.FindFile(m_projectDir+"\\Index\\*.*");
    while (found)
    {
      found = find.FindNextFile();
      ::DeleteFile(find.GetFilePath());
    }
  }
}

bool ProjectFrame::IsProjectEdited(void)
{
  return GetPanel(0)->IsProjectEdited();
}

const char* ProjectFrame::GetProjectFileExt(void)
{
  switch (m_projectType)
  {
  case Project_I7:
    return ".inform";
  case Project_I7XP:
    return ".i7xp";
  default:
    ASSERT(0);
    break;
  }
  return 0;
}

void ProjectFrame::UpdateMenuParams(void)
{
  m_menuFonts[0].DeleteObject();
  m_menuFonts[1].DeleteObject();

  // Create the menu item fonts
  int dpi = DPI::getWindowDPI(this);
  DPI::createSystemMenuFont(&(m_menuFonts[0]),dpi,1.0);
  DPI::createSystemMenuFont(&(m_menuFonts[1]),dpi,0.9);

  // Get the theme, if any
  HTHEME theme = 0;
  if (::IsAppThemed())
    theme = ::OpenThemeData(GetSafeHwnd(),L"Menu");

  // Get the menu item spacings
  if (theme != 0)
  {
    CDC* dc = GetDC();

    CSize szC(0,0);
    ::GetThemePartSize(theme,dc->GetSafeHdc(),MENU_POPUPCHECK,0,NULL,TS_TRUE,&szC);
    MARGINS mrgC = {0}, mrgCb = {0};
    ::GetThemeMargins(theme,dc->GetSafeHdc(),MENU_POPUPCHECK,0,TMT_CONTENTMARGINS,NULL,&mrgC);
    ::GetThemeMargins(theme,dc->GetSafeHdc(),MENU_POPUPCHECKBACKGROUND,0,TMT_CONTENTMARGINS,NULL,&mrgCb);
    m_menuGutter = szC.cx+mrgC.cxLeftWidth+mrgC.cxRightWidth+mrgCb.cxLeftWidth+mrgCb.cxRightWidth;

    int gapX = 0;
    ::GetThemeInt(theme,MENU_POPUPBACKGROUND,0,TMT_BORDERSIZE,&gapX);
    m_menuTextGap.cx = gapX;
    CSize sz = dc->GetTextExtent("Test");
    m_menuTextGap.cy =
      szC.cy+mrgC.cyTopHeight+mrgC.cyBottomHeight+mrgCb.cyTopHeight+mrgCb.cyBottomHeight-sz.cy;
    m_menuTextGap.cy = max(0,m_menuTextGap.cy/2);

    ReleaseDC(dc);
    ::CloseThemeData(theme);
  }
  else
  {
    m_menuGutter = ::GetSystemMetrics(SM_CXMENUCHECK)+2;
    m_menuTextGap = CSize(2,2);
  }
}

void ProjectFrame::UpdateExtensionsMenu(void)
{
  CMenu* fileMenu = GetMenu()->GetSubMenu(0);
  CMenu* newExtMenu = fileMenu->GetSubMenu(4)->GetSubMenu(1);
  ASSERT(newExtMenu != NULL);
  CMenu* openExtMenu = fileMenu->GetSubMenu(7);
  ASSERT(openExtMenu != NULL);

  while (newExtMenu->GetMenuItemCount() > 0)
    newExtMenu->RemoveMenu(0,MF_BYPOSITION);
  while (openExtMenu->GetMenuItemCount() > 0)
    openExtMenu->RemoveMenu(0,MF_BYPOSITION);

  int x = -1;
  HMENU newAuthorMenu = 0, openAuthorMenu = 0;
  const std::vector<InformApp::ExtLocation>& extensions = theApp.GetExtensions();
  for (int i = 0; (i < MAX_MENU_EXTENSIONS) && (i < (int)extensions.size()); i++)
  {
    if ((x == -1) || (extensions[i].author != extensions[x].author))
    {
      newAuthorMenu = ::CreatePopupMenu();
      newExtMenu->AppendMenu(MF_POPUP|MF_STRING,(UINT_PTR)newAuthorMenu,extensions[i].author.c_str());
      openAuthorMenu = ::CreatePopupMenu();
      openExtMenu->AppendMenu(MF_POPUP|MF_STRING,(UINT_PTR)openAuthorMenu,extensions[i].author.c_str());
      x = i;
    }

    ASSERT(newAuthorMenu != 0);
    ::AppendMenu(newAuthorMenu,MF_OWNERDRAW,ID_NEW_EXTENSIONS_LIST+i,(LPCSTR)&(extensions[i]));
    ASSERT(openAuthorMenu != 0);
    ::AppendMenu(openAuthorMenu,MF_OWNERDRAW,ID_OPEN_EXTENSIONS_LIST+i,(LPCSTR)&(extensions[i]));
  }
}

CString ProjectFrame::NaturalCommandLine(bool release)
{
  CString dir = theApp.GetAppDir();
  CString format = m_settings.GetOutputFormat();
  CString version = m_settings.GetCompilerVersion();

  CString executable, arguments;
  if (version == NI_BUILD)
  {
    executable.Format("%s\\Compilers\\ni",(LPCSTR)dir);
    arguments.Format(
      "%s%s-internal \"%s\\Internal\" -project \"%s\" -format=%s",
      (release ? "-release " : ""),
      ((m_settings.m_predictable && !release)) ? "-rng " : "",
      (LPCSTR)dir,(LPCSTR)m_projectDir,(LPCSTR)format);
  }
  else if (version >= "6L38")
  {
    executable.Format("%s\\Compilers\\%s\\ni",(LPCSTR)dir,(LPCSTR)version);
    arguments.Format(
      "%s%s-internal \"%s\\Retrospective\\%s\" -project \"%s\" -format=%s",
      (release ? "-release " : ""),
      ((m_settings.m_predictable && !release)) ? "-rng " : "",
      (LPCSTR)dir,(LPCSTR)version,(LPCSTR)m_projectDir,(LPCSTR)format);
  }
  else if (version == "6L02")
  {
    executable.Format("%s\\Compilers\\%s\\ni",(LPCSTR)dir,(LPCSTR)version);
    arguments.Format(
      "%s%s-rules \"%s\\Retrospective\\%s\\Extensions\" -package \"%s\" -extension=%s",
      (release ? "-release " : ""),
      ((m_settings.m_predictable && !release)) ? "-rng " : "",
      (LPCSTR)dir,(LPCSTR)version,(LPCSTR)m_projectDir,(LPCSTR)format);
  }

  CString output;
  output.Format("%s \\\n    %s\n",(LPCSTR)executable,(LPCSTR)arguments);
  Output(output);

  CString cmdLine;
  cmdLine.Format("\"%s\" %s",(LPCSTR)executable,(LPCSTR)arguments);
  return cmdLine;
}

CString ProjectFrame::InformCommandLine(bool release)
{
  CString dir = theApp.GetAppDir();
  CString switches = m_settings.GetInformSwitches(release,m_I6debug);
  CString format = m_settings.GetOutputFormat();

  CString executable, arguments;
  executable.Format("%s\\Compilers\\inform6",(LPCSTR)dir);
  arguments.Format("%s +include_path=..\\Source,.\\ auto.inf output.%s",
    (LPCSTR)switches,(LPCSTR)format);

  CString output;
  output.Format("\n%s \\\n    %s\n",(LPCSTR)executable,(LPCSTR)arguments);
  Output(output);

  CString cmdLine;
  cmdLine.Format("\"%s\" %s",(LPCSTR)executable,(LPCSTR)arguments);
  return cmdLine;
}

CString ProjectFrame::IntestSourceCommandLine(void)
{
  CString dir = theApp.GetAppDir();

  CString executable, arguments;
  executable.Format("%s\\Compilers\\intest",(LPCSTR)dir);
  arguments.Format(
    "-no-history -threads=1 -using -extension \"%s\\Source\\extension.i7x\""
    " -do -source %c -to \"%s\\Source\\story.ni\" -concordance %c",
    (LPCSTR)m_projectDir,(LPCSTR)m_exampleCompiled.id,(LPCSTR)m_projectDir,
    (LPCSTR)m_exampleCompiled.id);

  CString output;
  output.Format("\n%s \\\n    %s\n",(LPCSTR)executable,(LPCSTR)arguments);
  Output(output);

  CString cmdLine;
  cmdLine.Format("\"%s\" %s",(LPCSTR)executable,(LPCSTR)arguments);
  return cmdLine;
}

void ProjectFrame::GenerateIntestReport(CString result)
{
  int nodeCount = 0;
  CString nodeId("0");

  if (result.IsEmpty())
  {
    // Find the node to report, and what the result was
    Skein::Node* current = m_skein.GetCurrent();
    Skein::Node* report = current;
    while (current != NULL)
    {
      if (current->GetExpectedText().IsEmpty())
      {
        result = "cursed";
        report = current;
      }
      else if (current->GetDiffers() != Skein::Node::ExpectedSame)
      {
        if (result.IsEmpty())
          result = "wrong";
        if (result == "wrong")
          report = current;
      }

      current = current->GetParent();
      nodeCount++;
    }
    if (result.IsEmpty())
      result = "right";
    nodeId = report->GetUniqueId();
  }

  // Run intest to generate a problem report
  CString cmdLine;
  cmdLine.Format(
    "\"%s\\Compilers\\intest\" -no-history -threads=1 -using"
    " -extension \"%s\\Source\\extension.i7x\" -do -report %c %s"
    " \"%s\\Build\\Problems.html\" n%s t%d -to \"%s\\Build\\Inform-Report-%d.html\"",
    (LPCSTR)theApp.GetAppDir(),(LPCSTR)m_projectDir,(LPCSTR)m_exampleCompiled.id,(LPCSTR)result,
    (LPCSTR)m_projectDir,(LPCSTR)nodeId,nodeCount,(LPCSTR)m_projectDir,m_exampleCompiled.index);
  IntestOutputSink sink;
  int code = theApp.RunCommand(m_projectDir,cmdLine,"intest.exe",sink,true);
  sink.Done();

  if (code == 0)
  {
    GetPanel(0)->CompileProject(TabInterface::RanIntestReport,m_exampleCompiled.index);
    GetPanel(1)->CompileProject(TabInterface::RanIntestReport,m_exampleCompiled.index);
  }
  else
  {
    CString msg;
    msg.Format("Failed to generate test report\nIntest returned code %d",code);
    MessageBox(msg,INFORM_TITLE,MB_OK|MB_ICONERROR);
  }
}

void ProjectFrame::GenerateIntestCombinedReport(void)
{
  // Run intest to generate a combined report for all the tests
  CString cmdLine;
  cmdLine.Format(
    "\"%s\\Compilers\\intest\" -no-history -threads=1 -using -extension \"%s\\Source\\extension.i7x\""
    " -do -combine \"%s\\Build\\Inform-Report.html\" -%d -to \"%s\\Build\\Problems.html\"",
    (LPCSTR)theApp.GetAppDir(),(LPCSTR)m_projectDir,(LPCSTR)m_projectDir,m_examples.GetSize(),
    (LPCSTR)m_projectDir);
  IntestOutputSink sink;
  int code = theApp.RunCommand(m_projectDir,cmdLine,"intest.exe",sink,true);
  sink.Done();

  if (code == 0)
  {
    GetPanel(0)->CompileProject(TabInterface::RanIntestReport,0);
    GetPanel(1)->CompileProject(TabInterface::RanIntestReport,0);
  }
  else
  {
    CString msg;
    msg.Format("Failed to generate combined test report\nIntest returned code %d",code);
    MessageBox(msg,INFORM_TITLE,MB_OK|MB_ICONERROR);
  }
}

bool ProjectFrame::BusyWantStop(void)
{
  BusyProject busy(this);
  return WantStop();
}

DWORD ProjectFrame::GetMaxLast5Time(void)
{
  DWORD maxTime = 15*1000;
  
  // Scale the maximum time with the size of the Inform 6 source
  CString autoFile;
  autoFile.Format("%s\\Build\\auto.inf",(LPCSTR)m_projectDir);
  struct _stat autoInfo;
  if (_stat(autoFile,&autoInfo) == 0)
    maxTime += autoInfo.st_size / 512;
  return maxTime;
}

void ProjectFrame::MonitorProcess(InformApp::CreatedProcess cp, ProcessAction action, LPCSTR name)
{
  // Add to the list of processes being monitored
  SubProcess sub;
  sub.cp = cp;
  sub.action = action;
  sub.name = name;
  m_processes.Add(sub);

  // If this is the first process, start a timer
  if (m_processes.GetSize() == 1)
    SetTimer(1,200,NULL);
}

bool ProjectFrame::IsProcessRunning(LPCSTR name)
{
  for (int i = 0; i < m_processes.GetSize(); i++)
  {
    const SubProcess& sub = m_processes.GetAt(i);
    if (sub.name == name)
    {
      DWORD result = STILL_ACTIVE;
      ::GetExitCodeProcess(sub.cp.process,&result);
      if (result == STILL_ACTIVE)
        return true;
    }
  }
  return false;
}

void ProjectFrame::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == 1)
  {
    // Look for any processes that have finished
    CArray<SubProcess> finished;
    for (int i = 0; i < m_processes.GetSize();)
    {
      const SubProcess& sub = m_processes.GetAt(i);

      DWORD result = STILL_ACTIVE;
      ::GetExitCodeProcess(sub.cp.process,&result);
      if (result != STILL_ACTIVE)
      {
        finished.Add(sub);
        m_processes.RemoveAt(i);
      }
      else
        i++;
    }

    // If there are no processes left, stop the timer
    if (m_processes.IsEmpty())
      KillTimer(1);

    // Now handle the finished processes
    for (int i = 0; i < finished.GetSize(); i++)
    {
      SubProcess& sub = finished.GetAt(i);

      // Stop monitoring this process
      DWORD result = 0;
      ::GetExitCodeProcess(sub.cp.process,&result);
      theApp.WaitForProcessEnd(sub.cp.process);
      std::string trace = theApp.GetTraceForProcess(sub.cp.processId);
      sub.cp.close();

      // Tell the user if the process was not successful
      if (result != 0)
      {
        if (!trace.empty())
        {
          CString msg;
          msg.Format("\n%s process failed, stack backtrace:\n",(LPCSTR)sub.name);
          Output(msg);
          Output(trace.c_str());
        }

        CString msg;
        msg.Format("%s returned code %d",(LPCSTR)sub.name,(int)result);
        MessageBox(msg,INFORM_TITLE,MB_OK|MB_ICONERROR);
      }
      else
      {
        // Perform the final action, if any
        switch (sub.action)
        {
        case ProcessHelpExtensions:
          // Show the help on installed extensions
          OnHelpExtensions();
          break;
        }
      }
    }
  }
  CWnd::OnTimer(nIDEvent);
}

Panel* ProjectFrame::GetPanel(int column) const
{
  return (Panel*)m_splitter.GetPane(0,column);
}

int ProjectFrame::ChoosePanel(Panel::Tabs newTab)
{
  Panel::Tabs leftTab = GetPanel(0)->GetActiveTab();
  Panel::Tabs rightTab = GetPanel(1)->GetActiveTab();

  // If either panel is showing the same as the new, use that
  if (rightTab == newTab)
    return 1;
  else if (leftTab == newTab)
    return 0;

  // Always try to use the left panel for source
  if (newTab == Panel::Tab_Source)
    return 0;

  // If the right panel is not source, use that
  if (rightTab != Panel::Tab_Source)
    return 1;

  // Use the left panel unless that is source too
  return (leftTab == Panel::Tab_Source) ? 1 : 0;
}

// Implementation of InformApp::OutputSink
void ProjectFrame::Output(const char* msg)
{
  // Split the output message into lines
  const char* start = msg;
  while (*start != '\0')
  {
    const char* end = strchr(start,'\n');
    if (end != NULL)
      end++;
    else
      end = start+strlen(start);

    CString line(start,(int)(end-start));

    // Is this a progress line?
    if (line.Left(2) == "++")
    {
      // Decode progress information
      int percent;
      char progress[256];
      if (sscanf(line,"++ %d%% (%[^)]",&percent,progress) == 2)
        SendMessage(WM_PROGRESS,percent,(LPARAM)progress);
      else if (sscanf(line,"++ Ended: %[^^]",progress) == 1)
        SetMessageText(progress);
    }
    else
    {
      GetPanel(0)->Progress(line);
      GetPanel(1)->Progress(line);
    }

    // Is this a file location line?
    const char* limit1 = strchr(line,'[');
    if ((limit1 != NULL) && (*(limit1+1) == '['))
    {
      const char* limit2 = strchr(limit1,']');
      if ((limit2 != NULL) && (*(limit2+1) == ']'))
        m_outputFileLoc = CString(limit1+2,(int)(limit2-limit1-2));
    }
      
    start = end;
  }
}

// Implementation of InformApp::OutputSink
bool ProjectFrame::WantStop(void)
{
  return m_progress.WantStop();
}

void ProjectFrame::OnSourceLink(const char* url, TabInterface* from, COLORREF highlight)
{
  CString replace_url;
  if (m_projectType == Project_I7XP)
  {
    bool replace = false;
    char exId = m_exampleCompiled.id;
    int line = 0;

    if (sscanf(url,"source:story.ni#line%d",&line) == 1)
      replace = true;
    else if (sscanf(url,"source:story.ni?case=%c#line%d",&exId,&line) == 2)
      replace = true;

    if (replace)
    {
      // Replace link to source code, if needed
      for (INT_PTR i = m_exLineOffsets.GetSize()-1; (i >= 0) && replace_url.IsEmpty(); i--)
      {
        const ExLineOffset& elo = m_exLineOffsets.GetAt(i);
        if ((exId == elo.id) && (line >= elo.from))
        {
          replace_url.Format("source:extension.i7x#line%d",line+elo.offset);
          url = replace_url;
        }
      }
    }
    else
    {
      char urlPath[_MAX_PATH];
      if (sscanf(url,"source:%[^#]#line%d",urlPath,&line) == 2)
      {
        // Replace link to copied extension
        CString path(urlPath);
        path.Replace("%20"," ");
        if (m_materialsExtPath.CompareNoCase(path) == 0)
        {
          replace_url.Format("source:extension.i7x#line%d",line);
          url = replace_url;
        }
      }
    }
  }

  // Select the panel to show the source in
  int otherPanel = 0;
  if (GetPanel(0)->ContainsTab(from))
    otherPanel = 1;
  Panel* panel = GetPanel(otherPanel);

  // Show the source tab if the highlight is accepted
  if (((TabSource*)panel->GetTab(Panel::Tab_Source))->Highlight(url,highlight))
  {
    panel->SetActiveTab(Panel::Tab_Source);
  }
  else if (ExtensionFrame::StartHighlight(url,highlight,m_settings))
  {
  }
  else
    ::MessageBeep(MB_ICONASTERISK);
}

void ProjectFrame::OnDocLink(const char* url, TabInterface* from)
{
  // Select the tab to show the page in
  Panel::Tabs tab = Panel::Tab_Doc;
  CString docDir;
  docDir.Format("%s\\Inform\\Documentation",(LPCSTR)theApp.GetHomeDir());
  CString extUrlBase(theApp.PathToUrl(docDir));
  if (strncmp(url,extUrlBase,extUrlBase.GetLength()) == 0)
    tab = Panel::Tab_Extensions;

  // Select the panel to show the page in
  int thisPanel = 1;
  if (GetPanel(0)->ContainsTab(from))
    thisPanel = 0;

  // Show the appropriate tab
  if (tab == Panel::Tab_Extensions)
  {
    ((TabExtensions*)GetPanel(thisPanel)->GetTab(Panel::Tab_Extensions))->Show(CString(url));
    GetPanel(thisPanel)->SetActiveTab(Panel::Tab_Extensions);
  }
  else
  {
    ((TabDoc*)GetPanel(thisPanel)->GetTab(Panel::Tab_Doc))->Show(CString(url));
    GetPanel(thisPanel)->SetActiveTab(Panel::Tab_Doc);
  }
}

void ProjectFrame::OnSkeinLink(const char* url, TabInterface* from)
{
  char nodeId[256];
  char exId = 0;
  if (sscanf(url,"skein:%[^?]?case=%c",&nodeId,&exId) == 2)
  {
    // Switch to the appropriate example
    int index = 0;
    for (int i = 0; i < m_examples.GetSize(); i++)
    {
      if (m_examples.GetAt(i).id == exId)
        index = i+1;
    }
    if ((index > 0) && (index < m_exampleList.GetCount()))
    {
      if (index != m_exampleList.GetCurSel())
      {
        m_exampleList.SetCurSel(index);
        OnChangedExample();
      }
    }

    // Show the appropriate node
    Skein::Node* node = m_skein.FindNode(nodeId);
    if (node != NULL)
    {}//XXXXDK Show in testing tab? Select transcript?
  }
}

void ProjectFrame::OnSettingsChange(TabSettings* changed)
{
  m_needCompile = true;

  TabSettings* left = (TabSettings*)GetPanel(0)->GetTab(Panel::Tab_Settings);
  TabSettings* right = (TabSettings*)GetPanel(1)->GetTab(Panel::Tab_Settings);

  if (changed == left)
    right->UpdateFromSettings();
  else if (changed == right)
    left->UpdateFromSettings();
}

bool ProjectFrame::LoadToolBar(void)
{
  CToolBarCtrl& ctrl = m_toolBar.GetToolBarCtrl();

  // Set the button identifiers
  static const UINT buttons[] =
  {
    ID_PLAY_GO,
    ID_PLAY_REPLAY,
    ID_RELEASE_GAME,
    ID_PLAY_TEST,
    ID_FILE_INSTALL_XP
  };
  m_toolBar.SetButtons(buttons,sizeof buttons/sizeof buttons[0]);

  // Set the size of the images
  int w = 32, h = 32;
  m_toolBar.SetSizes(CSize(w+8,h+7),CSize(w,h));

  // Load the images
  HIMAGELIST imgList = ::ImageList_Create(32,32,ILC_COLOR32,0,5);
  ::ImageList_Add(imgList,theApp.GetCachedImage("Toolbar")->GetSafeHandle(),0);
  ctrl.SetImageList(CImageList::FromHandle(imgList));

  imgList = ::ImageList_Create(32,32,ILC_COLOR32,0,5);
  ::ImageList_Add(imgList,theApp.GetCachedImage("Toolbar-disabled")->GetSafeHandle(),0);
  ctrl.SetDisabledImageList(CImageList::FromHandle(imgList));

  // Add selective text for buttons
  ctrl.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
  for (int i = 0; i < ctrl.GetButtonCount(); i++)
  {
    UINT id = m_toolBar.GetItemID(i);
    if (id != ID_SEPARATOR)
    {
      CString btnText, tipText;
      btnText.LoadString(id);
      AfxExtractSubString(tipText,btnText,1,'\n');
      m_toolBar.SetButtonText(i,tipText);
      m_toolBar.SetButtonStyle(i,m_toolBar.GetButtonStyle(i)|BTNS_SHOWTEXT);
    }
  }

  switch (m_projectType)
  {
  case Project_I7:
    ctrl.HideButton(ID_PLAY_TEST);
    ctrl.HideButton(ID_FILE_INSTALL_XP);
    break;
  case Project_I7XP:
    {
      ctrl.HideButton(ID_PLAY_REPLAY);
      ctrl.HideButton(ID_RELEASE_GAME);

      // Create space for the examples list control
      for (int i = 0; i < I7XP_TBAR_SPACER_COUNT; i++)
      {
        TBBUTTON spacer = { -1,0 };
        spacer.iString = -1;
        ctrl.InsertButton(I7XP_TBAR_SPACER_POS,&spacer);
      }

      // Create the examples list control
      m_exampleList.Create(CBS_DROPDOWNLIST|WS_CHILD|WS_VISIBLE,
        CRect(0,0,100,100),&m_toolBar,IDC_EXAMPLE_LIST);
      m_exampleList.SetFont(m_toolBar.GetFont());
      SetExampleListLocation();

      // Set the initial contents and selection for the examples
      m_exampleList.AddString("Test All");
      m_exampleList.SetCurSel(0);
    }
    break;
  default:
    ASSERT(0);
    break;
  }
  return true;
}

bool ProjectFrame::UpdateExampleList(void)
{
  // Run intest to list the examples
  CString cmdLine;
  cmdLine.Format(
    "\"%s\\Compilers\\intest\" -no-history -threads=1"
    " -using -extension \"%s\\Source\\extension.i7x\" -do -catalogue",
    (LPCSTR)theApp.GetAppDir(),(LPCSTR)m_projectDir);
  IntestOutputSink sink;
  int code = theApp.RunCommand(m_projectDir,cmdLine,"intest.exe",sink,true);
  sink.Done();

  if (!theApp.IsValidFrame(this))
    return false;

  if (code == 0)
  {
    // Update the list of all examples
    m_examples.RemoveAll();
    for (int i = 0; i < sink.results.GetSize(); i++)
    {
      char exId = 0;
      CStringW result = sink.results.GetAt(i);
      if (swscanf(result,L"extension Example %C = ",&exId) == 1)
      {
        Example example;
        example.id = exId;
        example.index = i+1;
        example.name = result.Mid(22);
        if (!example.name.IsEmpty())
          m_examples.Add(example);
      }
    }

    // Get the index of the current choice
    int index = m_exampleList.GetCurSel();

    // Remove all but the first entry
    m_exampleList.ShowDropDown(FALSE);
    while (m_exampleList.GetCount() > 1)
      m_exampleList.DeleteString(1);

    // Add the example names
    for (int i = 0; i < m_examples.GetSize(); i++)
      m_exampleList.AddString(m_examples.GetAt(i).name);

    // Set the index of the current choice
    if (m_exampleList.SetCurSel(index) == CB_ERR)
      m_exampleList.SetCurSel(0);

    OnChangedExample();
    return true;
  }
  else
  {
    CString msg;
    msg.Format("Failed to generate list of examples\nIntest returned code %d",code);
    MessageBox(msg,INFORM_TITLE,MB_OK|MB_ICONERROR);
    return false;
  }
}

void ProjectFrame::SetExampleListLocation(void)
{
  CRect r1, r2;
  m_toolBar.GetItemRect(I7XP_TBAR_SPACER_POS,r1);
  m_toolBar.GetItemRect(I7XP_TBAR_SPACER_POS+I7XP_TBAR_SPACER_COUNT-1,r2);

  // Find the height of the examples list control
  int h = 0;
  COMBOBOXINFO boxInfo = { sizeof COMBOBOXINFO,0 };
  if (m_exampleList.GetComboBoxInfo(&boxInfo))
    h = boxInfo.rcItem.bottom+boxInfo.rcItem.top;
  else
  {
    LOGFONT lf;
    m_exampleList.GetFont()->GetLogFont(&lf);
    h = 13+abs(lf.lfHeight);
  }

  // Position and size the examples list control
  m_exampleList.MoveWindow(r1.left+4,(r1.bottom+r1.top-h)/2,r2.right-r1.left-8,
    DPI::getMonitorRect(this).Width()/2);
}

bool ProjectFrame::GetExtensionInfo(CString& path, CStringW& name, CStringW& author)
{
  path = m_projectDir+"\\Source\\extension.i7x";
  CStringW firstLine = ExtensionFrame::ReadExtensionFirstLine(path);
  if (!firstLine.IsEmpty())
  {
    CStringW version;
    return ExtensionFrame::IsValidExtension(firstLine,name,author,version);
  }
  return false;
}

CString ProjectFrame::GetMaterialsFolder(void)
{
  // Get the path to the ".materials" directory
  int projectExt = m_projectDir.Find(GetProjectFileExt());
  if (projectExt == -1)
    return "";
  CString path;
  path.Format("%s.materials",m_projectDir.Left(projectExt));
  return path;
}

bool ProjectFrame::CopyExtensionToMaterials(void)
{
  m_materialsExtPath.Empty();

  CString sourcePath;
  CStringW extName, extAuthor;
  if (!GetExtensionInfo(sourcePath,extName,extAuthor))
    return false;

  CString destPath = GetMaterialsFolder();
  ::CreateDirectory(destPath,NULL);
  destPath.Append("\\Extensions");
  ::CreateDirectory(destPath,NULL);
  destPath.AppendFormat("\\%S",(LPCWSTR)extAuthor);
  ::CreateDirectory(destPath,NULL);
  if (::GetFileAttributes(destPath) == INVALID_FILE_ATTRIBUTES)
    return false;

  destPath.AppendFormat("\\%S.i7x",(LPCWSTR)extName);
  if (::CopyFile(sourcePath,destPath,FALSE))
  {
    m_materialsExtPath = destPath;
    return true;
  }
  return false;
}

ProjectFrame::Example ProjectFrame::GetCurrentExample(void)
{
  int index = m_exampleList.GetCurSel()-1;
  if ((index >= 0) && (index < m_examples.GetSize()))
    return m_examples.GetAt(index);
  return Example();
}

void ProjectFrame::TestCurrentExample(bool testAll)
{
  bool compiled = CompileProject(false,true,true);
  if (!BusyWantStop())
  {
    if (compiled)
    {
      BusyProject busy(this);

      // Run intest to get the list of test commands
      CString cmdLine;
      cmdLine.Format(
        "\"%s\\Compilers\\intest\" -no-history -threads=1"
        " -using -extension \"%s\\Source\\extension.i7x\" -do -script %c",
        (LPCSTR)theApp.GetAppDir(),(LPCSTR)m_projectDir,(LPCSTR)m_exampleCompiled.id);
      IntestOutputSink sink;
      int code = theApp.RunCommand(m_projectDir,cmdLine,"intest.exe",sink,true);
      sink.Done();

      if (code == 0)
      {
        m_skein.Reset(true);
        for (int i = 0; i < sink.results.GetSize(); i++)
          m_skein.NewLine(sink.results.GetAt(i));
        m_skein.Reset(false);
        RunProject();
        m_progress.LongTaskAdvance();
        SendMessage(WM_PROGRESS,50,(LPARAM)"Running example");

        // Add a task to show a report on the test, or run the next test
        PlaySkein next(testAll ? ReportThenRunNextTest : ShowTestReport);
        m_playThreads.push(next);
      }
      else
      {
        CString msg;
        msg.Format("Failed to generate test commands\nIntest returned code %d",code);
        MessageBox(msg,INFORM_TITLE,MB_OK|MB_ICONERROR);
      }
    }
    else if (testAll)
    {
      PlaySkein next(RunNextTest);
      m_playThreads.push(next);
      PostMessage(WM_PLAYNEXTTHREAD);
    }
    else
      GetPanel(ChoosePanel(Panel::Tab_Results))->SetActiveTab(Panel::Tab_Results);
  }
}

ExampleComboBox::ExampleComboBox()
{
  EnableActiveAccessibility();
}

HRESULT ExampleComboBox::get_accName(VARIANT child, BSTR* accName)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  if (child.lVal == CHILDID_SELF)
  {
    CString name("Examples");
    *accName = name.AllocSysString();
    return S_OK;
  }
  return S_FALSE;
}
