#pragma once

class ArrowButton : public CButton
{
  DECLARE_DYNAMIC(ArrowButton)

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void DrawItem(LPDRAWITEMSTRUCT dis);

  void DrawControl(CDC* dc, CRect rect, UINT ownerDrawState, bool back);
  void DrawArrow(CDC* dc, int x, int y, COLORREF colour, COLORREF highlight);

public:
  enum ArrowStyle
  {
    DownLow,
    DownCentre,
    UpCentre
  };

  ArrowButton(ArrowStyle style);

  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnCustomDraw(NMHDR*, LRESULT*);

protected:
  ArrowStyle m_style;
  bool m_mouseOver;
  bool m_customDraw;
};
