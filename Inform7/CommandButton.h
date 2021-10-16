#pragma once

#include "Dib.h"

class CommandButton : public CButton
{
  DECLARE_DYNAMIC(CommandButton)

public:
  CommandButton();

  void SetBackSysColor(int index);
  void SetTabStop(int tab);
  void SetIcon(const char* name);
  void UpdateDPI(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnPaint();

  void GetScaledIcon(void);

  int m_backIndex;
  int m_tabStop;
  CString m_iconName;
  CDibSection m_icon;

  bool m_mouseOver;
};
