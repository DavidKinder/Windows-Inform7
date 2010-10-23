// The errors tab

#include "stdafx.h"
#include "TabErrors.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PROBLEMS_FILE "\\Build\\Problems.html"
#define CBLORB_FILE "\\Build\\StatusCblorb.html"

IMPLEMENT_DYNAMIC(TabErrors, TabBase)

BEGIN_MESSAGE_MAP(TabErrors, TabBase)
  ON_WM_SIZE()
END_MESSAGE_MAP()

TabErrors::TabErrors() : m_tab(true), m_problems(NULL), m_notify(NULL), m_inform6(NoError)
{
}

const char* TabErrors::GetName(void)
{
  return "Errors";
}

void TabErrors::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);

  // Add tabs
  m_tab.InsertItem(ErrTab_Progress,"Progress");
  m_tab.InsertItem(ErrTab_Problems,"Problems");

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
  SetActiveTab(ErrTab_Progress,false);
}

void TabErrors::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabErrors::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  SetFocusOnContent();

  // Use and update the given tab state
  if (state.tab == Panel::Tab_Errors)
  {
    Panel::FreezeHistory freeze(Panel::GetPanel(this));
    if ((ErrorTabs)state.section == ErrTab_Problems)
      m_problems->Navigate(state.url,true);
    SetActiveTab((ErrorTabs)state.section,true);
  }
  GetTabState(state);
}

BOOL TabErrors::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  switch (GetActiveTab())
  {
  case ErrTab_Progress:
    if (m_progress.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  case ErrTab_Problems:
    if (m_problems->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
    break;
  }

  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabErrors::OpenProject(const char* path, bool primary)
{
  m_projectDir = path;

  // Clear previous compile information
  m_progress.ClearText();
  m_problems->Navigate("about:blank",false);
}

bool TabErrors::SaveProject(const char* path, bool primary)
{
  m_projectDir = path;
  return true;
}

void TabErrors::CompileProject(CompileStage stage, int code)
{
  switch (stage)
  {
  case CompileStart:
    // Clear previous compile information
    m_progress.ClearText();
    m_problems->Navigate("about:blank",false);
    m_inform6 = NoError;

    // Switch to the progress tab
    SetActiveTab(ErrTab_Progress,false);
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
    SetActiveTab(ErrTab_Problems,false);
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
      SetActiveTab(ErrTab_Problems,false);
    }
    break;

  case RanCBlorb:
    switch (code)
    {
    case 0:
    case 1:
      // Show the cBlorb status report
      m_problems->Navigate(m_projectDir+CBLORB_FILE,false);
      SetActiveTab(ErrTab_Problems,false);
      break;
    default:
      // Show the generic cBlorb error page
      m_problems->Navigate(theApp.GetAppDir()+
        "\\Documentation\\sections\\ErrorCblorb.html",false);
      SetActiveTab(ErrTab_Problems,false);
      break;
    }
    break;
  }
}

void TabErrors::Progress(const char* msg)
{
  // Check for an Inform 6 memory map
  const char* dynamic = "Dynamic +---------------------+";
  const char* compiledWith = "Compiled with ";
  if (strncmp(msg,dynamic,strlen(dynamic)) == 0)
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

void TabErrors::SetLinkNotify(LinkNotify* notify)
{
  m_notify = notify;
}

int TabErrors::GetTabHeight(void)
{
  // Get the height of the row of tab buttons
  CRect tabSize(0,4,100,100);
  CRect tabArea = tabSize;
  m_tab.AdjustRect(FALSE,tabArea);
  return tabArea.top;
}

void TabErrors::ShowRuntimeProblem(int problem)
{
  CString runtime;
  runtime.Format("%s\\Documentation\\sections\\RTP_P%d.html",theApp.GetAppDir(),problem);
  m_problems->Navigate(runtime,false);
  SetActiveTab(ErrTab_Problems,false);
}

void TabErrors::ShowTerpFailed(void)
{
  CString failed;
  failed.Format("%s\\Documentation\\windows\\ErrorTerp.html",theApp.GetAppDir());
  m_problems->Navigate(failed,false);
  SetActiveTab(ErrTab_Problems,false);
}

void TabErrors::SourceLink(const char* url)
{
  if (m_notify)
    m_notify->OnSourceLink(url,this,theApp.GetColour(InformApp::ColourError));
}

void TabErrors::DocLink(const wchar_t* url)
{
  if (m_notify)
    m_notify->OnDocLink(url,this);
}

BOOL TabErrors::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
    SetActiveTab(GetActiveTab(),true);

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabErrors::OnSize(UINT nType, int cx, int cy)
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
  m_tab.GetItemRect(ErrTab_Progress,firstTabItem);
  m_tab.GetItemRect(ErrTab_Problems,lastTabItem);
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

TabErrors::ErrorTabs TabErrors::GetActiveTab(void)
{
  return (ErrorTabs)m_tab.GetCurSel();
}

void TabErrors::SetActiveTab(ErrorTabs tab, bool focus)
{
  if (tab != No_ErrTab)
  {
    // Set the tab control
    if (GetActiveTab() != tab)
      m_tab.SetCurSel(tab);

    // Show the appropriate control
    switch (tab)
    {
    case ErrTab_Progress:
      m_progress.ShowWindow(SW_SHOW);
      m_problems->ShowWindow(SW_HIDE);
      break;
    case ErrTab_Problems:
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

void TabErrors::GetTabState(TabState& state)
{
  state.tab = Panel::Tab_Errors;
  state.section = GetActiveTab();
  if ((ErrorTabs)state.section == ErrTab_Problems)
    state.url = m_problems->GetURL();
}

void TabErrors::SetFocusOnContent(void)
{
  switch (GetActiveTab())
  {
  case ErrTab_Progress:
    m_progress.SetFocus();
    break;
  case ErrTab_Problems:
    m_problems->SetFocusOnContent();
    break;
  }
}
