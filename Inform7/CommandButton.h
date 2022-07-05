#pragma once

#include "Dib.h"

class CommandButton : public CButton
{
  DECLARE_DYNAMIC(CommandButton)

public:
  void SetBackSysColor(int index);
  void SetTabStop(int tab);
  void SetIcon(const char* name, int left);
  void UpdateDPI(void);
  void ChangeWidthForIcon(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnPaint();

  void GetScaledIcon(void);

  int m_backIndex = 0;
  int m_tabStop = 0;
  CString m_iconName;
  CDibSection m_icon;
  int m_iconLeft = 0;

  bool m_mouseOver = false;
};

class CommandListBox : public CListBox
{
  DECLARE_DYNAMIC(CommandListBox)

  void SetBackSysColor(int index);
  void SetTabStop(int tab);

  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);

  int m_backIndex = 0;
  int m_tabStop = 0;

  bool m_mouseOver = false;
  int m_hotIndex = -1;
  bool m_hotSelect = false;
};
