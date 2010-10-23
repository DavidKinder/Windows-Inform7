// The game tab

#include "stdafx.h"
#include "TabGame.h"
#include "GameWindow.h"
#include "Inform.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabGame, TabBase)

BEGIN_MESSAGE_MAP(TabGame, TabBase)
END_MESSAGE_MAP()

TabGame::TabGame() : m_parent(NULL), m_game(NULL), m_active(false)
{
}

void TabGame::SetGame(GameWindow* game)
{
  m_game = game;
}

bool TabGame::IsActive(void)
{
  return m_active;
}

const char* TabGame::GetName(void)
{
  return "Game";
}

void TabGame::CreateTab(CWnd* parent)
{
  m_parent = parent;
  Create(parent);
}

void TabGame::MoveTab(CRect& rect)
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

void TabGame::MakeActive(TabState& state)
{
  if (m_game != NULL)
  {
    m_game->ShowWindow(m_rect,this);
    ShowWindow(SW_SHOW);
    m_active = true;
    state.tab = Panel::Tab_Game;

    // Activating the game tab will change the status of the
    // game tab on the other pane, so redraw the entire window
    GetParentFrame()->Invalidate();
  }
}

void TabGame::MakeInactive(void)
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

bool TabGame::IsEnabled(void)
{
  if (m_game != NULL)
  {
    if (m_game->CanShow(this))
      return true;
  }
  return false;
}

BOOL TabGame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}
