// The results tab

#include "stdafx.h"
#include "TabResults.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROBLEMS_FILE "\\Build\\Problems.html"
#define CBLORB_FILE "\\Build\\StatusCblorb.html"

IMPLEMENT_DYNAMIC(TabResults, TabBase)

BEGIN_MESSAGE_MAP(TabResults, TabBase)
  ON_WM_SIZE()
END_MESSAGE_MAP()

TabResults::TabResults() : m_tab(true), m_problems(NULL), m_notify(NULL), m_inform6(NoError)
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
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);

  // Add tabs
  m_tab.InsertItem(ResTab_Progress,"Progress");
  m_tab.InsertItem(ResTab_Problems,"Problems");

  // Create the progress edit control
  m_progress.Create(this,0);

  // Create the problems HTML control
  m_problems = (ReportHtml*)(RUNTIME_CLASS(ReportHtml)->CreateObject());
  if (!m_problems->Create(NULL,NULL,WS_CHILD,zeroRect,this,0))
  {
    TRACE("Failed to create problems HTML control\n");
  }
  m_problems->SetLinkConsumer(this);

  // Make progress the initial tab
  Panel::FreezeHistory freeze(Panel::GetPanel(this));
  SetActiveTab(ResTab_Progress,false);
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
    if ((ResultTabs)state.section == ResTab_Problems)
      m_problems->Navigate(state.url,true);
    SetActiveTab((ResultTabs)state.section,true);
  }
  GetTabState(state);
}

BOOL TabResults::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  switch (GetActiveTab())
  {
  case ResTab_Progress:
    if (m_progress.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  case ResTab_Problems:
    if (m_problems->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  }

  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabResults::OpenProject(const char* path, bool primary)
{
  m_projectDir = path;

  // Clear previous compile information
  m_progress.ClearText();
  m_problems->Navigate("about:blank",false);
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
    // Clear previous compile information
    m_progress.ClearText();
    m_problems->Navigate("about:blank",false);
    m_inform6 = NoError;

    // Switch to the progress tab
    SetActiveTab(ResTab_Progress,false);
    break;

  case RanNaturalInform:
    // Load in the problem report
    switch (code)
    {
    case 0:
    case 1:
      m_problems->Navigate(m_projectDir+PROBLEMS_FILE,false);
      break;
    case 2:
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\Error2.html",false);
      break;
    case 10:
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\Error10.html",false);
      break;
    case 11:
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\Error11.html",false);
      break;
    default:
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\Error0.html",false);
      break;
    }

    // Switch to the problems tab
    SetActiveTab(ResTab_Problems,false);
    break;

  case RanInform6:
    if (code != 0)
    {
      // Show the I6 error pages
      switch (m_inform6)
      {
      case MemorySetting:
        m_problems->Navigate(theApp.GetAppDir()+
          "\\Documentation\\sections\\ErrorI6MemorySetting.html",false);
        break;
      case ReadableMemory:
        m_problems->Navigate(theApp.GetAppDir()+
          "\\Documentation\\sections\\ErrorI6Readable.html",false);
        break;
      case StoryFileLimit:
        m_problems->Navigate(theApp.GetAppDir()+
          "\\Documentation\\sections\\ErrorI6TooBig.html",false);
        break;
      default:
        m_problems->Navigate(theApp.GetAppDir()+
          "\\Documentation\\sections\\ErrorI6.html",false);
        break;
      }
      SetActiveTab(ResTab_Problems,false);
    }
    break;

  case RanCBlorb:
    switch (code)
    {
    case 0:
    case 1:
      // Show the cBlorb status report
      m_problems->Navigate(m_projectDir+CBLORB_FILE,false);
      SetActiveTab(ResTab_Problems,false);
      break;
    default:
      // Show the generic cBlorb error page
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\ErrorCblorb.html",false);
      SetActiveTab(ResTab_Problems,false);
      break;
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
    m_progress.SetFormat(true);
  else if (strncmp(msg,statistics,strlen(statistics)) == 0)
    m_progress.SetFormat(true);
  else if (strncmp(msg,compiledWith,strlen(compiledWith)) == 0)
    m_progress.SetFormat(false);

  // Add the progress message
  m_progress.AppendText(msg);

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
  m_progress.FontChanged();
  m_problems->Refresh();
}

void TabResults::SetLinkNotify(LinkNotify* notify)
{
  m_notify = notify;
}

int TabResults::GetTabHeight(void)
{
  // Get the height of the row of tab buttons
  CRect tabSize(0,4,100,100);
  CRect tabArea = tabSize;
  m_tab.AdjustRect(FALSE,tabArea);
  return tabArea.top;
}

void TabResults::ShowRuntimeProblem(int problem)
{
  CString runtime;
  runtime.Format("%s\\Documentation\\sections\\RTP_P%d.html",theApp.GetAppDir(),problem);
  m_problems->Navigate(runtime,false);
  SetActiveTab(ResTab_Problems,false);
}

void TabResults::ShowTerpFailed(void)
{
  CString failed;
  failed.Format("%s\\Documentation\\windows\\ErrorTerp.html",theApp.GetAppDir());
  m_problems->Navigate(failed,false);
  SetActiveTab(ResTab_Problems,false);
}

void TabResults::SourceLink(const char* url)
{
  if (m_notify)
    m_notify->OnSourceLink(url,this,theApp.GetColour(InformApp::ColourError));
}

void TabResults::DocLink(const wchar_t* url)
{
  if (m_notify)
    m_notify->OnDocLink(url,this);
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

  if (m_tab.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);

  // Call the base class to resize and get parameters
  CSize fontSize;
  int heading, h;
  SizeTab(CRect(client),fontSize,heading,h);

  // Get the dimensions of the first and last tab buttons
  CRect firstTabItem, lastTabItem;
  m_tab.GetItemRect(ResTab_Progress,firstTabItem);
  m_tab.GetItemRect(ResTab_Problems,lastTabItem);
  int w = lastTabItem.right - firstTabItem.left + 4;

  // Resize the tab control
  CRect tabSize;
  tabSize.left = (client.Width()-w)/2;
  tabSize.right = tabSize.left+w;
  if (tabSize.left < 0)
    tabSize.left = 0;
  if (tabSize.right > client.right)
    tabSize.right = client.right;
  tabSize.top = 2;
  tabSize.bottom = client.Height()-tabSize.top-2;
  m_tab.MoveWindow(tabSize,TRUE);

  // Work out the display area of the tab control
  CRect tabArea = tabSize;
  m_tab.AdjustRect(FALSE,tabArea);
  client.top = tabArea.top+2;

  // Resize the tab page controls
  m_progress.MoveWindow(client,TRUE);
  m_problems->MoveWindow(client,TRUE);
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
    case ResTab_Progress:
      m_progress.ShowWindow(SW_SHOW);
      m_problems->ShowWindow(SW_HIDE);
      break;
    case ResTab_Problems:
      m_progress.ShowWindow(SW_HIDE);
      m_problems->ShowWindow(SW_SHOW);
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
  if ((ResultTabs)state.section == ResTab_Problems)
    state.url = m_problems->GetURL();
}

void TabResults::SetFocusOnContent(void)
{
  switch (GetActiveTab())
  {
  case ResTab_Progress:
    m_progress.SetFocus();
    break;
  case ResTab_Problems:
    m_problems->SetFocusOnContent();
    break;
  }
}
