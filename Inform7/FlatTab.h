#pragma once

class FlatTab : public CTabCtrl
{
  DECLARE_DYNAMIC(FlatTab)

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg void OnSelChanging(NMHDR*, LRESULT* pResult);

public:
  FlatTab(bool buttons);

  class TabController
  {
  public:
    virtual bool IsTabEnabled(int tab) = 0;
    virtual COLORREF GetTabColour(int tab) = 0;
  };

  void SetTabController(TabController* controller);
  void SelectNextTab(bool forward);

  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  bool SetActiveTab(int tab);
  bool IsTabEnabled(int tab);
  int NextEnabledTab(int currentTab, bool wrap);
  int PrevEnabledTab(int currentTab, bool wrap);

  TabController* m_controller;

  CFont m_font;
  bool m_buttons;
};
