// The extensions tab

#include "stdafx.h"
#include "TabExtensions.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabExtensions, TabBase)

BEGIN_MESSAGE_MAP(TabExtensions, TabBase)
  ON_WM_SIZE()
  ON_MESSAGE(WM_PUBLIBERROR, OnPubLibError)
  ON_MESSAGE(WM_USERNAVIGATE, OnUserNavigate)
END_MESSAGE_MAP()

const char* TabExtensions::m_files[TabExtensions::Number_ExtTabs] =
{
  "\\Inform\\Documentation\\Extensions.html",
  "\\Inform\\Documentation\\ExtIndex.html",
  "http://www.emshort.com/pl/"
};

TabExtensions::TabExtensions() : m_tab(true), m_initialised(false), m_notify(NULL)
{
}

const char* TabExtensions::GetName(void)
{
  return "Extensions";
}

void TabExtensions::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the tab control
  CRect zeroRect(0,0,0,0);
  m_tab.Create(WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,zeroRect,this,0);
  m_tab.SendMessage(TCM_SETMINTABWIDTH,0,8);

  // Add tabs
  m_tab.InsertItem(ExtTab_Home,"?H");
  m_tab.InsertItem(ExtTab_Definitions,"Definitions");
  m_tab.InsertItem(ExtTab_Library,"Public Library");

  // Create the HTML control window
  if (!m_html.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,1))
  {
    TRACE("Failed to create HTML control\n");
  }
  m_html.SetLinkConsumer(this);
}

void TabExtensions::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabExtensions::MakeActive(TabState& state)
{
  if (!m_initialised)
  {
    // Show the extensions index page
    Show(GetUrlForTab(ExtTab_Home));
  }

  // Make the window visible
  ShowWindow(SW_SHOW);
  m_html.SetFocusOnContent();

  // Use and update the given tab state
  if (state.tab == Panel::Tab_Extensions)
    Show(state.url);
  GetTabState(state);
}

BOOL TabExtensions::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_html.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabExtensions::CompileProject(CompileStage stage, int code)
{
  if (stage == RanNaturalInform)
  {
    if (code == 0)
    {
      // Reload the current page, in case it is generated by compilation
      m_html.Refresh();
    }
  }
}

void TabExtensions::PrefsChanged(CRegKey& key)
{
  m_html.Refresh();
}

void TabExtensions::Show(const char* url)
{
  m_html.Navigate(url,true);
  m_initialised = true;
  UpdateActiveTab();
}

void TabExtensions::DownloadedExt(int id)
{
  if (GetActiveTab() == ExtTab_Library)
  {
    CString code;
    code.Format("downloadSucceeded(%d);",id);
    m_html.RunJavaScript(code);
  }
}

void TabExtensions::SourceLink(const char* url)
{
}

void TabExtensions::LibraryLink(const char* url)
{
  CStringArray* libraryUrls = new CStringArray();
  libraryUrls->Add(url);
  GetParentFrame()->PostMessage(WM_EXTDOWNLOAD,(WPARAM)libraryUrls);
}

void TabExtensions::SkeinLink(const char* url)
{
}

bool TabExtensions::DocLink(const char* url)
{
  // Switch to the documentation tab if the URL points to a file in the application
  CString appUrl = theApp.PathToUrl(theApp.GetAppDir());
  if (strncmp(url,appUrl,appUrl.GetLength()) == 0)
  {
    if (m_notify)
      m_notify->OnDocLink(url,this);
    return true;
  }
  else
    return false;
}

void TabExtensions::LinkDone(void)
{
}

void TabExtensions::LinkError(const char* url)
{
  CString plUrl = GetUrlForTab(ExtTab_Library);
  if (strncmp(url,plUrl,plUrl.GetLength()) == 0)
    PostMessage(WM_PUBLIBERROR);
}

void TabExtensions::SetLinkNotify(LinkNotify* notify)
{
  m_notify = notify;
}

BOOL TabExtensions::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  // Pick up a tab change
  if (((LPNMHDR)lParam)->code == TCN_SELCHANGE)
  {
    ExtTabs tab = GetActiveTab();
    if (tab != No_ExtTab)
    {
      Show(GetUrlForTab(tab));

      TabState state;
      GetTabState(state);
      Panel::GetPanel(this)->AddToTabHistory(state);
    }
  }

  return TabBase::OnNotify(wParam, lParam, pResult);
}

void TabExtensions::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_tab.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);

  // Call the base class to resize and get parameters
  CSize fontSize;
  int heading;
  SizeTab(client,fontSize,heading);

  // Get the dimensions of the first and last tab buttons
  CRect firstTabItem, lastTabItem;
  m_tab.GetItemRect(ExtTab_Home,firstTabItem);
  m_tab.GetItemRect(ExtTab_Library,lastTabItem);
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

  m_html.MoveWindow(client,TRUE);
}

LRESULT TabExtensions::OnPubLibError(WPARAM, LPARAM)
{
  Show(theApp.GetAppDir()+"\\Documentation\\sections\\pl404.html");
  return 0;
}

LRESULT TabExtensions::OnUserNavigate(WPARAM, LPARAM)
{
  if (IsWindowVisible())
  {
    UpdateActiveTab();

    TabState state;
    GetTabState(state);
    Panel::GetPanel(this)->AddToTabHistory(state);
  }
  return 0;
}

TabExtensions::ExtTabs TabExtensions::GetActiveTab(void)
{
  return (ExtTabs)m_tab.GetCurSel();
}

void TabExtensions::UpdateActiveTab(void)
{
  CString url = m_html.GetURL();
  int idx = No_ExtTab;
  for (int i = 0; i < sizeof m_files / sizeof m_files[0]; i++)
  {
    CString check(m_files[i]);
    if (url.Find(check) >= 0)
      idx = i;
    check.Replace('\\','/');
    if (url.Find(check) >= 0)
      idx = i;
  }
  if (idx == No_ExtTab)
  {
    if (url.CompareNoCase("inform://Extensions/Extensions.html") == 0)
      idx = ExtTab_Home;
    else if (url.Find("pl404.html") > 0)
      idx = ExtTab_Library;
  }
  if (idx != GetActiveTab())
    m_tab.SetCurSel(idx);
}

void TabExtensions::GetTabState(TabState& state)
{
  state.tab = Panel::Tab_Extensions;
  state.url = m_html.GetURL();
}

CString TabExtensions::GetUrlForTab(ExtTabs tab)
{
  if (tab == ExtTab_Library)
    return m_files[tab];
  return theApp.GetHomeDir()+m_files[tab];
}
