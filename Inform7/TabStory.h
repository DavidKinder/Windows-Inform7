#pragma once

#include "TabBase.h"

class GameWindow;

class TabStory : public TabBase
{
  DECLARE_DYNAMIC(TabStory)

public:
  TabStory();

  void SetGame(GameWindow* game);
  bool IsActive(void);

  // Implementation of TabInterface
  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);
  void MakeInactive(void);
  bool IsEnabled(void);
  void PrefsChanged(CRegKey& key);
  void UpdateDPI(const std::map<CWnd*,double>& layout);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  CString GetToolTip(UINT_PTR id);

private:
  void Resize(void);

  CWnd* m_parent;
  GameWindow* m_game;
  CButton m_stop;

  CRect m_rect;
  bool m_active;

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);
  afx_msg void OnPlayStop();
};
