// The results tab

#include "stdafx.h"
#include "TabResults.h"
#include "Inform.h"
#include "Messages.h"
#include "Panel.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROBLEMS_FILE "\\Build\\Problems.html"
#define CBLORB_FILE "\\Build\\StatusCblorb.html"

IMPLEMENT_DYNAMIC(TabResults, TabBase)

BEGIN_MESSAGE_MAP(TabResults, TabBase)
  ON_WM_SIZE()
  ON_MESSAGE(WM_USERNAVIGATE, OnUserNavigate)
  ON_MESSAGE(WM_FINDREPLACECMD, OnFindReplaceCmd)
END_MESSAGE_MAP()

TabResults::TabResults() : m_notify(NULL), m_inform6(NoError)
{
}

const char* TabResults::GetName(void)
{
  return "Results";
}

void TabResults::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);

  // Add tabs
  m_tab.InsertItem(ResTab_Report,"Report");
  m_tab.InsertItem(ResTab_Console,"Console");

  // Create the report HTML control
  if (!m_report.Create(NULL,NULL,WS_CHILD,zeroRect,this,0))
  {
    TRACE("Failed to create report HTML control\n");
  }
  m_report.SetLinkConsumer(this);

  // Create the console edit control
  m_console.Create(this,0);

  // Make the report the initial tab
  Panel::FreezeHistory freeze(Panel::GetPanel(this));
  SetActiveTab(ResTab_Report,false);

  // Set window text for accessibility
  m_report.SetWindowText("Report page");
  m_console.SetAccName("Console page");
}

void TabResults::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabResults::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  SetFocusOnContent();

  // Use and update the given tab state
  if (state.tab == Panel::Tab_Results)
  {
    Panel::FreezeHistory freeze(Panel::GetPanel(this));
    if ((ResultTabs)state.section == ResTab_Report)
      m_report.Navigate(state.url,true);
    SetActiveTab((ResultTabs)state.section,true);
  }
  GetTabState(state);
}

BOOL TabResults::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  switch (GetActiveTab())
  {
  case ResTab_Report:
    if (m_report.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  case ResTab_Console:
    if (m_console.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  }
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabResults::OpenProject(const char* path, bool primary)
{
  m_projectDir = path;

  // Clear previous compile information
  CString blankPath = theApp.GetAppDir()+"\\Documentation\\windows\\Blank.html";
  m_report.Navigate(blankPath,false);
  m_console.ClearText();
}

bool TabResults::SaveProject(const char* path, bool primary)
{
  m_projectDir = path;
  return true;
}

void TabResults::CompileProject(CompileStage stage, int code)
{
  switch (stage)
  {
  case CompileStart:
    {
      // Clear previous compile information
      CString blankPath = theApp.GetAppDir()+"\\Documentation\\windows\\Blank.html";
      m_report.Navigate(blankPath,false);
      m_console.ClearText();
      m_inform6 = NoError;
    }
    break;

  case RanInform7:
    // Load in the report
    switch (code)
    {
    case 0:
    case 1:
      m_report.Navigate(TextFormat::AnsiToUTF8(
        m_projectDir+PROBLEMS_FILE),false);
      break;
    case 2:
      m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
        "\\Documentation\\Error2.html"),false);
      break;
    case 10:
      m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
        "\\Documentation\\Error10.html"),false);
      break;
    case 11:
      m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
        "\\Documentation\\Error11.html"),false);
      break;
    default:
      m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
        "\\Documentation\\Error0.html"),false);
      break;
    }

    // Switch to the report tab
    SetActiveTab(ResTab_Report,false);
    break;

  case RanInform6:
    if (code != 0)
    {
      // Show the I6 error pages
      switch (m_inform6)
      {
      case MemorySetting:
        m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
          "\\Documentation\\ErrorI6MemorySetting.html"),false);
        break;
      case ReadableMemory:
        m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
          "\\Documentation\\ErrorI6Readable.html"),false);
        break;
      case StoryFileLimit:
        m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
          "\\Documentation\\ErrorI6TooBig.html"),false);
        break;
      default:
        m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
          "\\Documentation\\ErrorI6.html"),false);
        break;
      }
      SetActiveTab(ResTab_Report,false);
    }
    break;

  case RanInblorb:
    // Show the inblorb status report if created, otherwise show the generic error page
    {
      CString statusPath = m_projectDir+CBLORB_FILE;
      if (GetParentFrame()->SendMessage(WM_ISBUILDFILE,(WPARAM)(LPCSTR)statusPath))
        m_report.Navigate(TextFormat::AnsiToUTF8(statusPath),false);
      else
      {
        m_report.Navigate(TextFormat::AnsiToUTF8(theApp.GetAppDir()+
          "\\Documentation\\ErrorCblorb.html"),false);
      }
    }
    SetActiveTab(ResTab_Report,false);
    break;

  case RanIntestSource:
    if (code != 0)
      SetActiveTab(ResTab_Console,false);
    break;

  case RanIntestReport:
    // Show the test report
    if (code > 0)
    {
      CString reportPath;
      reportPath.Format("%s\\Build\\Inform-Report-%d.html",(LPCSTR)m_projectDir,code);
      m_report.Navigate(TextFormat::AnsiToUTF8(reportPath),false);
    }
    else
      m_report.Navigate(TextFormat::AnsiToUTF8(m_projectDir+PROBLEMS_FILE),false);
    SetActiveTab(ResTab_Report,false);
    break;

  case RanInstallExtension:
    // Show the extension installation report
    if (code == 0)
    {
      CString reportPath;
      reportPath.Format("%s\\Build\\Install.html",(LPCSTR)m_projectDir);
      m_report.Navigate(TextFormat::AnsiToUTF8(reportPath),false);
      SetActiveTab(ResTab_Report,false);
    }
    else
      SetActiveTab(ResTab_Console,false);
    break;

  case TestStart:
    m_console.ClearText();
    SetActiveTab(ResTab_Console,false);
    break;

  case RanIntestTest:
    // Show the test report
    if (code == 0)
    {
      CString reportPath;
      reportPath.Format("%s\\Build\\Test.html",(LPCSTR)m_projectDir);
      m_report.Navigate(TextFormat::AnsiToUTF8(reportPath),false);
      SetActiveTab(ResTab_Report,false);
    }
    break;
  }
}

void TabResults::Progress(const char* msg)
{
  // Check for Inform 6 printing a memory map or statistics
  const char* dynamic = "Dynamic +---------------------+";
  const char* statistics = "In:  1 source code files";
  const char* compiledWith = "Compiled with ";
  if (strncmp(msg,dynamic,strlen(dynamic)) == 0)
    m_console.SetFormat(true);
  else if (strncmp(msg,statistics,strlen(statistics)) == 0)
    m_console.SetFormat(true);
  else if (strncmp(msg,compiledWith,strlen(compiledWith)) == 0)
    m_console.SetFormat(false);

  // Add the progress message
  m_console.AppendText(msg);

  const char* errorMsg = NULL;
  const char* errors[] = { "Fatal error:", "Error:" };

  // Check for an Inform 6 error
  for (int i = 0; i < sizeof errors/sizeof errors[0]; i++)
  {
    const char* pos = strstr(msg,errors[i]);
    if (pos != NULL)
    {
      errorMsg = pos+strlen(errors[i]);
      while (*errorMsg == ' ')
        errorMsg++;
      break;
    }
  }

  if (errorMsg != NULL)
  {
    char buffer[256];
    if (sscanf(errorMsg,"The memory setting %[^)] has been exceeded.",buffer) == 1)
      m_inform6 = MemorySetting;
    else if (sscanf(errorMsg,"This program has overflowed the maximum readable-memory size of the %s format.",buffer) == 1)
      m_inform6 = ReadableMemory;
    else if (sscanf(errorMsg,"The story file exceeds %s limit",buffer) == 1)
      m_inform6 = StoryFileLimit;
  }
}

void TabResults::PrefsChanged(CRegKey& key)
{
  m_report.Refresh();
  m_console.FontChanged();
}

void TabResults::UpdateDPI(const std::map<CWnd*,double>& layout)
{
  TabBase::UpdateDPI(layout);
  m_console.PrefsChanged();
  Resize();
}

void TabResults::SetDarkMode(DarkMode* dark)
{
  TabBase::SetDarkMode(dark);
  m_console.SetDarkMode(dark);
}

void TabResults::SetLinkNotify(LinkNotify* notify)
{
  m_notify = notify;
}

void TabResults::ShowPage(LPCSTR path)
{
  m_report.Navigate(TextFormat::AnsiToUTF8(path),false);
  SetActiveTab(ResTab_Report,false);
}

void TabResults::SourceLink(const char* url)
{
  if (m_notify)
    m_notify->OnSourceLink(url,this,theApp.GetColour(InformApp::ColourError));
}

void TabResults::LibraryLink(const char* url)
{
}

void TabResults::SkeinLink(const char* url)
{
  if (m_notify)
    m_notify->OnSkeinLink(url,this);
}

bool TabResults::DocLink(const char* url)
{
  if (m_notify)
    return m_notify->OnDocLink(url,this);
  return true;
}

void TabResults::LinkDone(void)
{
}

void TabResults::LinkError(const char* url)
{
}

BOOL TabResults::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
    SetActiveTab(GetActiveTab(),true);

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabResults::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);
  Resize();
}

LRESULT TabResults::OnUserNavigate(WPARAM, LPARAM)
{
  if (IsWindowVisible())
  {
    TabState state;
    GetTabState(state);
    Panel::GetPanel(this)->AddToTabHistory(state);
  }
  return 0;
}

LRESULT TabResults::OnFindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  return m_report.OnFindReplaceCmd(wParam,lParam);
}

void TabResults::Resize(void)
{
  if (m_tab.GetSafeHwnd() != 0)
  {
    CRect client;
    GetClientRect(client);

    // Call the base class to resize and get parameters
    CSize fontSize;
    int heading;
    SizeTab(CRect(client),fontSize,heading);

    // Get the dimensions of the first and last tab buttons
    CRect firstTabItem = m_tab.GetItemRect(ResTab_Report);
    CRect lastTabItem = m_tab.GetItemRect(ResTab_Console);
    int w = lastTabItem.right - firstTabItem.left;

    // Resize the tab control
    CRect tabSize;
    tabSize.right = client.Width();
    tabSize.left = tabSize.right-w;
    if (tabSize.left < 0)
      tabSize.left = 0;
    if (tabSize.right > client.right)
      tabSize.right = client.right;
    tabSize.top = 0;
    tabSize.bottom = heading;
    m_tab.MoveWindow(tabSize,TRUE);

    // Resize the tab page controls
    client.top = heading;
    m_report.MoveWindow(client,TRUE);
    m_console.MoveWindow(client,TRUE);
  }
}

TabResults::ResultTabs TabResults::GetActiveTab(void)
{
  return (ResultTabs)m_tab.GetCurSel();
}

void TabResults::SetActiveTab(ResultTabs tab, bool focus)
{
  if (tab != No_ResTab)
  {
    // Set the tab control
    if (GetActiveTab() != tab)
      m_tab.SetCurSel(tab);

    // Show the appropriate control
    switch (tab)
    {
    case ResTab_Report:
      m_console.ShowWindow(SW_HIDE);
      m_report.ShowWindow(SW_SHOW);
      break;
    case ResTab_Console:
      m_console.ShowWindow(SW_SHOW);
      m_report.ShowWindow(SW_HIDE);
      break;
    }

    if (focus)
      SetFocusOnContent();

    if (IsWindowVisible())
    {
      TabState state;
      GetTabState(state);
      Panel::GetPanel(this)->AddToTabHistory(state);
    }
  }
}

void TabResults::GetTabState(TabState& state)
{
  state.tab = Panel::Tab_Results;
  state.section = GetActiveTab();
  if ((ResultTabs)state.section == ResTab_Report)
    state.url = m_report.GetURL();
}

void TabResults::SetFocusOnContent(void)
{
  switch (GetActiveTab())
  {
  case ResTab_Report:
    m_report.SetFocus();
    break;
  case ResTab_Console:
    m_console.SetFocus();
    break;
  }
}
