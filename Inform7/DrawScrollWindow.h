#pragma once

#include <memory>

class DrawScroll;

class DrawScrollWindow : public CWnd
{
public:
  static void SetDraw(DrawScroll* draw);

  BOOL Create(DWORD style, CWnd* parentWnd, UINT id);
  void EnableScrollBarCtrl(int bar, BOOL enable = TRUE);

  int SetScrollInfo(int bar, LPCSCROLLINFO scrollInfo, BOOL redraw = TRUE);
  BOOL GetScrollInfo(int bar, LPSCROLLINFO scrollInfo, UINT mask = SIF_ALL);
  int GetScrollLimit(int bar);
  int SetScrollPos(int bar, int pos, BOOL redraw = TRUE);
  int GetScrollPos(int bar);
  void SetScrollRange(int bar, int minPos, int maxPos, BOOL redraw = TRUE);
  void GetScrollRange(int bar, LPINT minPos, LPINT maxPos);

  struct BarState
  {
    bool active = false;
    bool visible = false;
    CRect rect;

    int min = 0;
    int max = 0;
    int page = 0;
    int pos = 0;
    int trackPos = 0;

    bool SetActive(bool newActive);
    bool SetPos(int newPos);
    bool SetTrackPos(int newTrackPos);
  };

  struct Slider
  {
    int btn = 0;
    int pos = 0;
    int len = 0;
  };

  enum Bar
  {
    NoBar,
    BarVertical,
    BarHorizontal
  };

  enum Part
  {
    NoPart,
    PartUpButton,
    PartAboveSlider,
    PartSlider,
    PartBelowSlider,
    PartDownButton
  };

  struct Element
  {
    Bar bar = NoBar;
    Part part = NoPart;

    bool operator==(const Element& element) const;
    bool operator!=(const Element& element) const;
  };

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
  afx_msg void OnNcPaint();
  afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
  afx_msg LRESULT OnNcHitTest(CPoint point);
  afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
  afx_msg void OnNcMouseLeave();
  afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
  afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
  afx_msg void OnMouseMove(UINT nHitTest, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnCaptureChanged(CWnd* pWnd);
  afx_msg void OnTimer(UINT_PTR nIDEvent);

  CPoint ScreenToWindow(CPoint screenPt);
  void RedrawNonClient(void);
  Slider GetVerticalSlider(void);

  BarState m_v, m_h;
  Element m_capture;
  Element m_mouseOver;
  bool m_trackingMouse = false;
  int m_dragSliderOffset = 0;
  int m_dragStartPos = 0;

  Element GetElementAtCursor(void);
  Element GetElementAtPoint(CPoint point);
  void DoElementAction(const Element& element);
  void UpdateMouseOver(void);
  bool IsDraggingSlider(Bar bar);

  enum Timer
  {
    TimerInitialClick = 1,
    TimerMouseCapture
  };

  void StartTimer(Timer timer);
  void StopTimer(void);
  void DoCaptureTimer(void);

  UINT_PTR m_timer = 0;

  static std::unique_ptr<DrawScroll> m_draw;
};

class DarkMode;

class DrawScroll
{
public:
  virtual void DrawVertical(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture) = 0;
};

class DrawChromeScroll : public DrawScroll
{
public:
  void DrawVertical(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);

protected:
  void DrawVerticalArrow(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide, DrawScrollWindow::Part part,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);
  COLORREF GetSysColour(DarkMode* dark, int sysColour);
};

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

  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnHScroll(UINT code, UINT pos, CScrollBar* bar);
  afx_msg void OnVScroll(UINT code, UINT pos, CScrollBar* bar);

  CSize m_total = CSize(0,0);
  CSize m_page = CSize(0,0);
  CSize m_line = CSize(0,0);

  bool m_insideUpdate = false;
};
