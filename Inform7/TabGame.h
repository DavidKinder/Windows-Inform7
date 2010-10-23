#pragma once

#include "TabBase.h"

class GameWindow;

class TabGame : public TabBase
{
  DECLARE_DYNAMIC(TabGame)

public:
  TabGame();

  void SetGame(GameWindow* game);
  bool IsActive(void);

  // Implementation of TabInterface
  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);
  void MakeInactive(void);
  bool IsEnabled(void);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

private:
  CWnd* m_parent;
  GameWindow* m_game;

  CRect m_rect;
  bool m_active;

protected:
  DECLARE_MESSAGE_MAP()
};
