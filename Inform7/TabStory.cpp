// The story tab

#include "stdafx.h"
#include "TabStory.h"
#include "GameWindow.h"
#include "Inform.h"
#include "Messages.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabStory, TabBase)

BEGIN_MESSAGE_MAP(TabStory, TabBase)
  ON_WM_SIZE()
  ON_COMMAND(ID_PLAY_STOP, OnPlayStop)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

TabStory::TabStory() : m_parent(NULL), m_game(NULL), m_active(false)
{
}

void TabStory::SetGame(GameWindow* game)
{
  m_game = game;
}

bool TabStory::IsActive(void)
{
  return m_active;
}

const char* TabStory::GetName(void)
{
  return "Story";
}

void TabStory::CreateTab(CWnd* parent)
{
  m_parent = parent;
  Create(parent);
  m_stop.Create("Stop",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_PLAY_STOP);
  m_stop.SetFont(theApp.GetFont(InformApp::FontPanel));
}

void TabStory::MoveTab(CRect& rect)
{
  ASSERT(m_parent);

  if (m_game != NULL)
  {
    m_rect = rect;
    m_parent->MapWindowPoints(m_game->GetParent(),m_rect);

    CSize fontSize;
    int heading;
    SizeTab(m_rect,fontSize,heading);

    if (m_active)
      m_game->MoveWindow(m_rect,TRUE);
    MoveWindow(rect,TRUE);
  }
}

void TabStory::MakeActive(TabState& state)
{
  if (m_game != NULL)
  {
    m_game->ShowWindow(m_rect,this);
    ShowWindow(SW_SHOW);
    GetParentFrame()->SendMessage(WM_STORYACTIVE);
    m_active = true;
    state.tab = Panel::Tab_Story;
  }
}

void TabStory::MakeInactive(void)
{
  if (m_game != NULL)
  {
    if (m_active)
    {
      m_game->HideWindow(this);
      ShowWindow(SW_HIDE);
      m_active = false;
    }
    GetParentFrame()->Invalidate();
  }
}

bool TabStory::IsEnabled(void)
{
  if (m_game != NULL)
  {
    if (m_game->CanShow(this))
      return true;
  }
  return false;
}

CString TabStory::GetToolTip(UINT_PTR id)
{
  if (id == ID_PLAY_STOP)
    return "Stop the story that is currently running";
  return TabBase::GetToolTip(id);
}

BOOL TabStory::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabStory::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_stop.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);
  CSize fontSize;
  int heading;
  SizeTab(client,fontSize,heading);

  int sw = theApp.MeasureText(&m_stop).cx+(fontSize.cx*3);
  int gapx = (fontSize.cx/4);
  int gapy = (fontSize.cx/4);
  int x = client.Width()-sw-gapx;
  m_stop.MoveWindow(x,gapy,sw,heading-(2*gapy),TRUE);
}

LRESULT TabStory::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    if (m_game != NULL)
      m_stop.EnableWindow(m_game->IsRunning() ? TRUE : FALSE);
    else
      m_stop.EnableWindow(FALSE);
  }
  return TabBase::OnIdleUpdateCmdUI(wParam,lParam);
}

void TabStory::OnPlayStop()
{
  if (m_game != NULL)
    m_game->StopInterpreter(false);
}
