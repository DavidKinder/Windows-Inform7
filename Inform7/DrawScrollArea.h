#pragma once

#include "DrawScrollWindow.h"

class DrawScrollMouseAnchorWnd;

// Replacement for CScrollView
class DrawScrollArea : public DrawScrollWindow
{
public:
  void SetScrollSizes(int mapMode, SIZE sizeTotal,
    const SIZE& sizePage = CScrollView::sizeDefault, const SIZE& sizeLine = CScrollView::sizeDefault);

  CPoint GetDeviceScrollPosition();
  void ScrollToDevicePosition(POINT point);

protected:
  void UpdateBars();
  BOOL GetTrueClientSize(CSize& size, CSize& sizeBars);
  void GetScrollBarSizes(CSize& sizeBars);
  void GetScrollBarState(CSize sizeClient, CSize& needBars, CSize& sizeRange, CPoint& moveTo, BOOL insideClient);

  virtual void OnPrepareDC(CDC* dc);
  virtual void OnDraw(CDC* dc) = 0;
  virtual BOOL OnScroll(UINT scrollCode, UINT pos, BOOL doScroll = TRUE);
  virtual BOOL OnScrollBy(CSize extent, BOOL doScroll = TRUE);
  virtual CSize GetWheelScrollDistance(CSize distance, BOOL horz, BOOL vert);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar* bar);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* bar);
  afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint point);
  afx_msg LRESULT OnMButtonDown(WPARAM wp, LPARAM lp);

  CSize m_total = CSize(0,0);
  CSize m_page = CSize(0,0);
  CSize m_line = CSize(0,0);

  bool m_insideUpdate = false;

  DrawScrollMouseAnchorWnd* m_anchorWindow = NULL;
  friend class DrawScrollMouseAnchorWnd;
};

class DrawScrollMouseAnchorWnd : public CWnd
{
public:
  DrawScrollMouseAnchorWnd(CPoint anchor);

  BOOL Create(DrawScrollArea* parent);
  void SetBitmap(UINT id);

  virtual BOOL PreTranslateMessage(MSG* msg);

  afx_msg void OnTimer(UINT_PTR id);
  
  DECLARE_MESSAGE_MAP()

private:
  CSize m_size;
  CRect m_drag;
  CPoint m_anchor;
  bool m_quitTracking;
  UINT m_anchorID;
  HCURSOR m_anchorCursor;
};
