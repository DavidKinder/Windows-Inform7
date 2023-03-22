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
  afx_msg void OnCancelMode();
  afx_msg void OnCaptureChanged(CWnd* pWnd);
  afx_msg void OnTimer(UINT_PTR nIDEvent);

  CPoint ScreenToWindow(CPoint screenPt);
  void RedrawNonClient(void);
  Slider GetVerticalSlider(void);
  Slider GetHorizontalSlider(void);

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
    TimerInitialClick = 0xD000,
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
  virtual void DrawHorizontal(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture) = 0;
  virtual void DrawCorner(CDC& dc, DarkMode* dark, const CRect& rect) = 0;
};

class DrawChromeScroll : public DrawScroll
{
public:
  void DrawVertical(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);
  void DrawHorizontal(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);
  void DrawCorner(CDC& dc, DarkMode* dark, const CRect& rect);

protected:
  void DrawVerticalArrow(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide, DrawScrollWindow::Part part,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);
  void DrawHorizontalArrow(CDC& dc, DarkMode* dark, bool drag,
    const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide, DrawScrollWindow::Part part,
    const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture);
  COLORREF GetSysColour(DarkMode* dark, int sysColour);
};
