#pragma once

#include <string>
#include <vector>

class PanelTab : public CWnd
{
  DECLARE_DYNAMIC(PanelTab)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);

public:
  PanelTab();
  bool UpdateOrientation(CRegKey& key);

  CFont* GetFont(void);

  int GetItemCount(void) const;
  CString GetItem(int item) const;
  CRect GetItemRect(int item);
  void InsertItem(int item, LPCSTR name);

  int GetCurSel(void) const;
  void SetCurSel(int item);

  class TabController
  {
  public:
    virtual bool IsTabEnabled(int tab) = 0;
  };

  void SetTabController(TabController* controller);
  CSize GetTabHeaderSize(void);

  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  bool SetActiveTab(int tab);
  bool IsTabEnabled(int tab);

  bool m_vertical;
  std::vector<CString> m_items;
  int m_currentItem;

  TabController* m_controller;

  int m_mouseOverItem;
  bool m_mouseTrack;
};
