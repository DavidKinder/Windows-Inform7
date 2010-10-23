#include "stdafx.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#include "TabDoc.h"
#include "TabErrors.h"
#include "TabGame.h"
#include "TabIndex.h"
#include "TabSettings.h"
#include "TabSkein.h"
#include "TabSource.h"
#include "TabTranscript.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(Panel, CWnd)

BEGIN_MESSAGE_MAP(Panel, CWnd)
  ON_WM_CREATE()
  ON_WM_SIZE()
END_MESSAGE_MAP()

Panel::Panel() : m_tab(false)
{
  m_tabs[Tab_Source] = new TabSource;
  m_tabs[Tab_Errors] = new TabErrors;
  m_tabs[Tab_Index] = new TabIndex;
  m_tabs[Tab_Skein] = new TabSkein;
  m_tabs[Tab_Transcript] = new TabTranscript;
  m_tabs[Tab_Game] = new TabGame;
  m_tabs[Tab_Doc] = new TabDoc;
  m_tabs[Tab_Settings] = new TabSettings;

  m_tabHistoryPos = 0;
  m_tabHistoryFrozen = false;

  m_tab.SetTabController(this);
}

Panel::~Panel()
{
  for (int i = 0; i < Number_Tabs; i++)
    delete m_tabs[i];
}

BOOL Panel::PreCreateWindow(CREATESTRUCT& cs) 
{
  if (!CWnd::PreCreateWindow(cs))
    return FALSE;

  cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
    ::LoadCursor(NULL,IDC_ARROW));
  return TRUE;
}

void Panel::PostNcDestroy()
{
  delete this;
}

int Panel::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) != 0)
    return -1;

  // Create a tab control
  if (!m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,CRect(0,0,0,0),this,0))
    return -1;

  // Add the individual tabs
  for (int i = 0; i < Number_Tabs; i++)
    m_tab.InsertItem(i,m_tabs[i]->GetName());

  // Create the tab windows
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->CreateTab(&m_tab);

  return 0;
}

void Panel::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType, cx, cy);

  CRect client;
  GetClientRect(client);
  m_tab.MoveWindow(client,TRUE);

  // Update the tabs
  CRect tabSize = GetTabSize();
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->MoveTab(tabSize);
}

BOOL Panel::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  switch (((LPNMHDR)lParam)->code)
  {
  case TCN_SELCHANGE:
    // Pick up a tab change
    SetActiveTab(GetActiveTab());
    break;
  }

  return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL Panel::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Let the active tab process the command first
  Tabs tab = GetActiveTab();
  if (tab != No_Tab)
  {
    if (GetTab(tab)->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }

  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

// Get the rectangle for a tab window
CRect Panel::GetTabSize(void)
{
  CRect client;
  GetClientRect(client);

  // Work out the display area of the tab control
  CRect tabArea = client;
  m_tab.AdjustRect(FALSE,tabArea);

  // Have the tab overlay all but the tab pane
  client.top = tabArea.top;
  return client;
}

TabInterface* Panel::GetTab(Tabs tab)
{
  return m_tabs[tab];
}

Panel::Tabs Panel::GetActiveTab(void)
{
  return (Tabs)m_tab.GetCurSel();
}

void Panel::SetActiveTab(Tabs tab)
{
  if (tab != No_Tab)
  {
    // Set the tab control
    if (GetActiveTab() != tab)
      m_tab.SetCurSel(tab);

    // Show the appropriate panel
    TabState state;
    for (int i = 0; i < Number_Tabs; i++)
    {
      if (i == tab)
        m_tabs[i]->MakeActive(state);
      else
        m_tabs[i]->MakeInactive();
    }

    // Save the tab state in the history
    AddToTabHistory(state);
  }
}

bool Panel::ContainsTab(TabInterface* tab)
{
  for (int i = 0; i < Number_Tabs; i++)
  {
    if (m_tabs[i] == tab)
      return true;
  }
  return false;
}

void Panel::OpenProject(const char* path, bool primary)
{
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->OpenProject(path,primary);
}

bool Panel::SaveProject(const char* path, bool primary)
{
  bool saved = true;
  for (int i = 0; i < Number_Tabs; i++)
  {
    if (m_tabs[i]->SaveProject(path,primary) == false)
      saved = false;
  }
  return saved;
}

void Panel::CompileProject(TabInterface::CompileStage stage, int code)
{
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->CompileProject(stage,code);
}

bool Panel::IsProjectEdited(void)
{
  bool edited = false;
  for (int i = 0; i < Number_Tabs; i++)
  {
    if (m_tabs[i]->IsProjectEdited())
      edited = true;
  }
  return edited;
}

void Panel::Progress(const char* msg)
{
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->Progress(msg);
}

void Panel::LoadSettings(CRegKey& key)
{
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->LoadSettings(key);
}

void Panel::SaveSettings(CRegKey& key)
{
  for (int i = 0; i < Number_Tabs; i++)
    m_tabs[i]->SaveSettings(key);
}

bool Panel::IsTabEnabled(int tab)
{
  return m_tabs[tab]->IsEnabled();
}

COLORREF Panel::GetTabColour(int tab)
{
  return m_tabs[tab]->GetTabColour();
}

bool Panel::CanTabNavigate(bool forward)
{
  TabState state = GetTabNavigate(forward);
  if (state.tab != No_Tab)
    return m_tabs[state.tab]->IsEnabled();
  return false;
}

const char* Panel::TabNavigateName(bool forward)
{
  TabState state = GetTabNavigate(forward);
  if (state.tab != No_Tab)
    return m_tabs[state.tab]->GetName();
  return NULL;
}

void Panel::TabNavigate(bool forward)
{
  TabState state = GetTabNavigate(forward);
  if (state.tab != No_Tab)
  {
    // Navigate forward or backward
    if (forward)
      m_tabHistoryPos--;
    else
      m_tabHistoryPos++;

    // Set the tab control
    if (GetActiveTab() != state.tab)
      m_tab.SetCurSel(state.tab);

    // Show the appropriate panel
    for (int i = 0; i < Number_Tabs; i++)
    {
      if (i == state.tab)
        m_tabs[i]->MakeActive(state);
      else
        m_tabs[i]->MakeInactive();
    }
  }
}

TabState Panel::GetTabNavigate(bool forward)
{
  TabState state;
  if (forward)
  {
    if (m_tabHistoryPos > 0)
      state = m_tabHistory[m_tabHistory.GetSize()-m_tabHistoryPos];
  }
  else
  {
    if (m_tabHistoryPos < m_tabHistory.GetSize()-1)
      state = m_tabHistory[m_tabHistory.GetSize()-m_tabHistoryPos-2];
  }
  return state;
}

void Panel::AddToTabHistory(TabState state)
{
  if (m_tabHistoryFrozen)
    return;
  if (state.tab == No_Tab)
    return;

  // Is this a new tab state?
  if (m_tabHistoryPos < m_tabHistory.GetSize())
  {
    if (m_tabHistory[m_tabHistory.GetSize()-m_tabHistoryPos-1] == state)
      return;
  }

  // Truncate the existing tab history and add the new state
  m_tabHistory.SetSize(m_tabHistory.GetSize()-m_tabHistoryPos);
  m_tabHistory.Add(state);
  m_tabHistoryPos = 0;

  // Discard the start of the history if it has grown too large
  if (m_tabHistory.GetSize() > 150)
    m_tabHistory.RemoveAt(0,50);
}

Panel* Panel::GetPanel(CWnd* wnd)
{
  while (wnd != NULL)
  {
    if (wnd->IsKindOf(RUNTIME_CLASS(Panel)))
      return (Panel*)wnd;
    wnd = wnd->GetParent();
  }
  return NULL;
}

Panel::FreezeHistory::FreezeHistory(Panel* panel) : m_panel(panel)
{
  m_panel->m_tabHistoryFrozen = true;
}

Panel::FreezeHistory::~FreezeHistory()
{
  m_panel->m_tabHistoryFrozen = false;
}
