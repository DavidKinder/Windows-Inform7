// The story tab

#include "stdafx.h"
#include "TabStory.h"
#include "GameWindow.h"
#include "Inform.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabStory, TabBase)

BEGIN_MESSAGE_MAP(TabStory, TabBase)
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
}

void TabStory::MoveTab(CRect& rect)
{
  ASSERT(m_parent);

  if (m_game != NULL)
  {
    m_rect = rect;
    m_parent->MapWindowPoints(m_game->GetParent(),m_rect);

    CSize fontSize;
    int heading, h;
    SizeTab(m_rect,fontSize,heading,h);

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
    m_active = true;
    state.tab = Panel::Tab_Story;

    // Activating the story tab will change the status of the
    // story tab on the other pane, so redraw the entire window
    GetParentFrame()->Invalidate();
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

BOOL TabStory::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}
