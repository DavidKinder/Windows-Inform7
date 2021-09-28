#pragma once

class CommandButton : public CButton
{
  DECLARE_DYNAMIC(CommandButton)

public:
  CommandButton();

  void SetBackSysColor(int index);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnPaint();

  int m_backIndex;
  bool m_mouseOver;
};
