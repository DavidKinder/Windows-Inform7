// The index tab

#include "stdafx.h"
#include "TabIndex.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const char* TabIndex::m_files[TabIndex::Number_IdxTabs] =
{
  "\\Index\\Welcome.html",
  "\\Index\\Contents.html",
  "\\Index\\Actions.html",
  "\\Index\\Kinds.html",
  "\\Index\\Phrasebook.html",
  "\\Index\\Rules.html",
  "\\Index\\Scenes.html",
  "\\Index\\World.html",
};

IMPLEMENT_DYNAMIC(TabIndex, TabBase)

BEGIN_MESSAGE_MAP(TabIndex, TabBase)
  ON_WM_SIZE()
  ON_MESSAGE(WM_USERNAVIGATE, OnUserNavigate)
END_MESSAGE_MAP()

TabIndex::TabIndex() : m_tab(true), m_index(NULL), m_notify(NULL)
{
}

const char* TabIndex::GetName(void)
{
  return "Index";
}

void TabIndex::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);
  m_tab.SendMessage(TCM_SETMINTABWIDTH,0,8);

  // Add tabs
  m_tab.InsertItem(IdxTab_Home,"?H");
  m_tab.InsertItem(IdxTab_Contents,"Contents");
  m_tab.InsertItem(IdxTab_Actions,"Actions");
  m_tab.InsertItem(IdxTab_Kinds,"Kinds");
  m_tab.InsertItem(IdxTab_Phrasebook,"Phrasebook");
  m_tab.InsertItem(IdxTab_Rules,"Rules");
  m_tab.InsertItem(IdxTab_Scenes,"Scenes");
  m_tab.InsertItem(IdxTab_World,"World");

  // Create the index HTML control
  m_index = (ReportHtml*)(RUNTIME_CLASS(ReportHtml)->CreateObject());
  if (!m_index->Create(NULL,NULL,WS_CHILD|WS_VISIBLE,zeroRect,this,0))
  {
    TRACE("Failed to create index HTML control\n");
  }
  m_index->SetLinkConsumer(this);

  // Make contents the initial tab
  Panel::FreezeHistory freeze(Panel::GetPanel(this));
  SetActiveTab(IdxTab_Home,false);
}

void TabIndex::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabIndex::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  m_index->SetFocusOnContent();

  if (state.tab == Panel::Tab_Index)
  {
    Panel::FreezeHistory freeze(Panel::GetPanel(this));
    SetActiveTab((IndexTabs)state.section,true);
    m_index->Navigate(state.url,true);
  }
  GetTabState(state);
}

BOOL TabIndex::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  if (m_index->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabIndex::OpenProject(const char* path, bool primary)
{
  m_projectDir = path;
  SetActiveTab(GetActiveTab(),false);
}

bool TabIndex::SaveProject(const char* path, bool primary)
{
  m_projectDir = path;
  return true;
}

void TabIndex::CompileProject(CompileStage stage, int code)
{
  if (stage == RanNaturalInform)
  {
    if (code == 0)
    {
      // Reload the current index pages
      SetActiveTab(GetActiveTab(),false);
    }
  }
}

void TabIndex::PrefsChanged(CRegKey& key)
{
  m_index->Refresh();
}

void TabIndex::SetLinkNotify(LinkNotify* notify)
{
  m_notify = notify;
}

void TabIndex::ShowIndex(int index)
{
  SetActiveTab((IndexTabs)(IdxTab_Home+index),true);
}

void TabIndex::SourceLink(const char* url)
{
  if (m_notify)
    m_notify->OnSourceLink(url,this,theApp.GetColour(InformApp::ColourHighlight));
}

void TabIndex::LibraryLink(const char* url)
{
}

void TabIndex::SkeinLink(const char* url)
{
}

bool TabIndex::DocLink(const wchar_t* url)
{
  if (m_notify)
    m_notify->OnDocLink(url,this);
  return true;
}

bool TabIndex::LinkError(const char* url)
{
  return false;
}

BOOL TabIndex::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
    SetActiveTab(GetActiveTab(),true);

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabIndex::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_tab.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);

  // Call the base class to resize and get parameters
  CSize fontSize;
  int heading;
  SizeTab(CRect(client),fontSize,heading);

  // Get the dimensions of the first and last tab buttons
  CRect firstTabItem, lastTabItem;
  m_tab.GetItemRect(IdxTab_Home,firstTabItem);
  m_tab.GetItemRect(IdxTab_World,lastTabItem);
  int w = lastTabItem.right - firstTabItem.left + 4;

  // Resize the tab control
  CRect tabSize;
  tabSize.right = client.Width();
  tabSize.left = tabSize.right-w;
  if (tabSize.left < 0)
    tabSize.left = 0;
  tabSize.top = 0;
  tabSize.bottom = client.Height()-tabSize.top;
  m_tab.MoveWindow(tabSize,TRUE);

  // Work out the display area of the tab control
  CRect tabArea = tabSize;
  m_tab.AdjustRect(FALSE,tabArea);
  client.top = tabArea.top;

  // Resize the index control
  m_index->MoveWindow(client,TRUE);
}

LRESULT TabIndex::OnUserNavigate(WPARAM, LPARAM)
{
  if (IsWindowVisible())
  {
    // Has the user switched to a different section of the index?
    CString url = m_index->GetURL();
    int idx = No_IdxTab;
    for (int i = 0; i < sizeof m_files / sizeof m_files[0]; i++)
    {
      CString check(m_files[i]);
      if (url.Find(check) > 0)
        idx = i;
      check.Replace('\\','/');
      if (url.Find(check) > 0)
        idx = i;
    }
    if ((idx != No_IdxTab) && (idx != GetActiveTab()))
      m_tab.SetCurSel(idx);

    TabState state;
    GetTabState(state);
    Panel::GetPanel(this)->AddToTabHistory(state);
  }
  return 0;
}

TabIndex::IndexTabs TabIndex::GetActiveTab(void)
{
  return (IndexTabs)m_tab.GetCurSel();
}

void TabIndex::SetActiveTab(IndexTabs tab, bool focus)
{
  if (tab != No_IdxTab)
  {
    // Set the tab control
    if (GetActiveTab() != tab)
      m_tab.SetCurSel(tab);

    // Get the index file to show
    CString htmlFile(m_projectDir);
    htmlFile += m_files[tab];

    // If the index file exists, show it
    if (::GetFileAttributes(htmlFile) != INVALID_FILE_ATTRIBUTES)
      m_index->Navigate(htmlFile,focus);
    else
      m_index->Navigate("about:blank",false);

    if (focus)
      m_index->SetFocusOnContent();

    if (IsWindowVisible())
    {
      TabState state;
      GetTabState(state);
      Panel::GetPanel(this)->AddToTabHistory(state);
    }
  }
}

void TabIndex::GetTabState(TabState& state)
{
  state.tab = Panel::Tab_Index;
  state.section = GetActiveTab();
  state.url = m_index->GetURL();
}
