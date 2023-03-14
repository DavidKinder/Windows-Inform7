#include "stdafx.h"
#include "DrawScrollWindow.h"

#include "DarkMode.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::unique_ptr<DrawScroll> DrawScrollWindow::m_draw;

BEGIN_MESSAGE_MAP(DrawScrollWindow, CWnd)
  ON_WM_NCCALCSIZE()
  ON_WM_NCPAINT()
  ON_WM_NCHITTEST()
  ON_WM_NCMOUSEMOVE()
  ON_WM_NCMOUSELEAVE()
  ON_WM_NCLBUTTONDOWN()
  ON_WM_NCLBUTTONDBLCLK()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONUP()
  ON_WM_CAPTURECHANGED()
  ON_WM_TIMER()
END_MESSAGE_MAP()

void DrawScrollWindow::SetDraw(DrawScroll* draw)
{
  m_draw.reset(draw);
}

BOOL DrawScrollWindow::Create(DWORD style, CWnd* parentWnd, UINT id)
{
  m_v.active = style & WS_VSCROLL;
  m_h.active = style & WS_HSCROLL;
  style &= ~(WS_VSCROLL|WS_HSCROLL);
  return CWnd::Create(NULL,NULL,style,CRect(0,0,0,0),parentWnd,id);
}

int DrawScrollWindow::SetScrollInfo(int bar, LPCSCROLLINFO scrollInfo, BOOL redraw)
{
  BarState* state = NULL;
  switch (bar)
  {
  case SB_VERT:
    state = &m_v;
    break;
  case SB_HORZ:
    state = &m_h;
    break;
  default:
    ASSERT(FALSE);
    return 0;
  }

  if (!state->active)
    return 0;

  if (scrollInfo->fMask & SIF_RANGE)
  {
    ASSERT(scrollInfo->nMax >= scrollInfo->nMin);

    state->min = scrollInfo->nMin;
    state->max = scrollInfo->nMax;
  }
  if (scrollInfo->fMask & SIF_PAGE)
  {
    UINT range = scrollInfo->nMax - scrollInfo->nMin + 1;
    state->page = (scrollInfo->nPage < range) ? scrollInfo->nPage : range;
  }
  if (scrollInfo->fMask & SIF_POS)
    state->SetPos(scrollInfo->nPos);

  state->visible = (state->page < (state->max - state->min + 1));

  UpdateMouseOver();
  if (redraw)
    RedrawNonClient();
  return state->pos;
}

BOOL DrawScrollWindow::GetScrollInfo(int bar, LPSCROLLINFO scrollInfo, UINT mask)
{
  scrollInfo->fMask = mask;

  BarState* state = NULL;
  switch (bar)
  {
  case SB_VERT:
    state = &m_v;
    break;
  case SB_HORZ:
    state = &m_h;
    break;
  default:
    ASSERT(FALSE);
    return FALSE;
  }

  if (!state->active)
    return FALSE;

  if (mask & SIF_RANGE)
  {
    scrollInfo->nMin = state->min;
    scrollInfo->nMax = state->max;
  }
  if (mask & SIF_PAGE)
    scrollInfo->nPage = state->page;
  if (mask & SIF_POS)
    scrollInfo->nPos = state->pos;
  if (mask & SIF_TRACKPOS)
    scrollInfo->nTrackPos = state->trackPos;
  return TRUE;
}

void DrawScrollWindow::OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS* lpncsp)
{
  RECT& r = lpncsp->rgrc[0];
  CRect rw(r);

  // Adjust the size of the non-client area to include the scrollbars
  int dpi = DPI::getWindowDPI(this);
  if (m_v.visible)
  {
    r.right -= DPI::getSystemMetrics(SM_CXVSCROLL,dpi);
    if (r.right < r.left)
      r.right = r.left;
  }
  if (m_h.visible)
  {
    r.bottom -= DPI::getSystemMetrics(SM_CYHSCROLL,dpi);
    if (r.bottom < r.top)
      r.bottom = r.top;
  }

  // Store the scrollbar rectangles
  m_v.rect.SetRect(r.right,r.top,rw.right,r.bottom);
  m_v.rect.OffsetRect(-rw.TopLeft());
  m_h.rect.SetRect(r.left,r.bottom,r.right,rw.bottom);
  m_h.rect.OffsetRect(-rw.TopLeft());
}

void DrawScrollWindow::OnNcPaint()
{
  ASSERT(m_draw);

  // Get the window and client rectangles, in the window co-ordinate space
  CRect rw, rc;
  GetWindowRect(rw);
  ScreenToClient(rw);
  GetClientRect(rc);
  rc.OffsetRect(-rw.TopLeft());
  rw.OffsetRect(-rw.TopLeft());

  // Make the clipping region the window rectangle, excluding the client rectangle
  CWindowDC wdc(this);
  wdc.ExcludeClipRect(rc);
  wdc.IntersectClipRect(rw);

  // Create a buffered DC to paint into
  HDC hdc = 0;
  HANDLE pb = ::BeginBufferedPaint(wdc.GetSafeHdc(),rw,BPBF_COMPATIBLEBITMAP,NULL,&hdc);

  // Draw the scrollbars
  if (pb != 0)
  {
    CDC* dc = CDC::FromHandle(hdc);
    DarkMode* dark = DarkMode::GetActive(this);

    if (m_v.visible)
      m_draw->DrawVertical(*dc,dark,IsDraggingSlider(BarVertical),m_v,GetVerticalSlider(),m_mouseOver,m_capture);
    if (m_h.visible)
    {
      //TODO: horizontal
    }
    if (m_v.visible && m_h.visible)
    {
      //TODO: horizontal and vertical
    }

    ::EndBufferedPaint(pb,TRUE);
  }
}

LRESULT DrawScrollWindow::OnNcHitTest(CPoint point)
{
  // Convert the point to window co-ordinates
  point = ScreenToWindow(point);

  if (m_v.visible && m_v.rect.PtInRect(point))
    return HTVSCROLL;
  if (m_h.visible && m_h.rect.PtInRect(point))
    return HTHSCROLL;
  return Default();
}

void DrawScrollWindow::OnNcMouseMove(UINT nHitTest, CPoint point)
{
  Element element = GetElementAtPoint(point);
  if (element != m_mouseOver)
  {
    m_mouseOver = element;
    RedrawNonClient();
  }

  if (!m_trackingMouse)
  {
    TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), 0 };
    tme.dwFlags = TME_LEAVE|TME_NONCLIENT;
    tme.hwndTrack = GetSafeHwnd();
    if (::TrackMouseEvent(&tme))
      m_trackingMouse = true;
  }
}

void DrawScrollWindow::OnNcMouseLeave()
{
  m_trackingMouse = false;
  UpdateMouseOver();
  RedrawNonClient();
}

void DrawScrollWindow::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
  Element element = GetElementAtPoint(point);
  if (element.part != NoPart)
  {
    SetCapture();
    m_capture = element;

    if (element.part == PartSlider)
    {
      CPoint wndPoint = ScreenToWindow(point);
      switch (m_capture.bar)
      {
      case BarVertical:
        {
          Slider slide = GetVerticalSlider();
          m_dragSliderOffset = wndPoint.y - (m_v.rect.top + slide.btn + slide.pos);
          m_dragStartPos = m_v.pos;
        }
        break;
      case BarHorizontal:
        //TODO: horizontal
        break;
      default:
        ASSERT(FALSE);
        break;
      }
    }
    else
    {
      DoElementAction(element);
      StartTimer(TimerInitialClick);
    }
  }
  else
    Default();
}

void DrawScrollWindow::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
  // Treat a double click as two single clicks
  OnNcLButtonDown(nHitTest,point);
}

void DrawScrollWindow::OnMouseMove(UINT nHitTest, CPoint point)
{
  ClientToScreen(&point);
  point = ScreenToWindow(point);
  UpdateMouseOver();

  const int DRAG_SLIDER_LIMIT = 128;
  if (IsDraggingSlider(BarVertical))
  {
    int trackPos;
    if ((point.x > m_v.rect.left - DRAG_SLIDER_LIMIT) && (point.x < m_v.rect.right + DRAG_SLIDER_LIMIT))
    {
      Slider slide = GetVerticalSlider();
      int range = m_v.max - m_v.min + 1;
      int height = m_v.rect.Height() - (2*slide.btn);
      trackPos = MulDiv(point.y - m_dragSliderOffset,range - m_v.page,height - slide.len);
    }
    else
      trackPos = m_dragStartPos;

    if (m_v.SetTrackPos(trackPos))
    {
      RedrawNonClient();
      PostMessage(WM_VSCROLL,MAKEWPARAM(SB_THUMBTRACK,m_v.trackPos));
    }
  }
  else if (IsDraggingSlider(BarHorizontal))
  {
    //TODO: horizontal
  }

  Default();
}

void DrawScrollWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (GetCapture() == this)
    ReleaseCapture();
  Default();
}

void DrawScrollWindow::OnCaptureChanged(CWnd* pWnd)
{
  if (IsDraggingSlider(BarVertical))
  {
    PostMessage(WM_VSCROLL,MAKEWPARAM(SB_THUMBPOSITION,m_v.trackPos));
    PostMessage(WM_VSCROLL,SB_ENDSCROLL);
  }
  else if (IsDraggingSlider(BarHorizontal))
  {
    //TODO: horizontal
  }

  m_capture = Element();
  UpdateMouseOver();
  StopTimer();
  RedrawNonClient();
  Default();
}

void DrawScrollWindow::OnTimer(UINT_PTR nIDEvent)
{
  switch (nIDEvent)
  {
  case TimerInitialClick:
    StopTimer();
    StartTimer(TimerMouseCapture);
    DoCaptureTimer();
    break;
  case TimerMouseCapture:
    DoCaptureTimer();
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

CPoint DrawScrollWindow::ScreenToWindow(CPoint screenPt)
{
  CRect rw;
  GetWindowRect(rw);
  return screenPt - rw.TopLeft();
}

void DrawScrollWindow::RedrawNonClient(void)
{
  SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED|SWP_DRAWFRAME);
}

DrawScrollWindow::Slider DrawScrollWindow::GetVerticalSlider(void)
{
  Slider slide;
  slide.btn = DPI::getSystemMetrics(SM_CYVSCROLL,DPI::getWindowDPI(this));

  int range = m_v.max - m_v.min + 1;
  int height = m_v.rect.Height() - (2*slide.btn);
  slide.len = MulDiv(m_v.page,height,range);
  if (slide.len < 8)
    slide.len = 8;
  int pos = IsDraggingSlider(BarVertical) ? m_v.trackPos : m_v.pos;
  slide.pos = MulDiv(pos,height - slide.len,range - m_v.page);
  if (slide.pos < 0)
    slide.pos = 0;
  return slide;
}

DrawScrollWindow::Element DrawScrollWindow::GetElementAtCursor(void)
{
  CPoint point;
  ::GetCursorPos(&point);
  return GetElementAtPoint(point);
}

DrawScrollWindow::Element DrawScrollWindow::GetElementAtPoint(CPoint point)
{
  // Convert the point from screen to window co-ordinates
  point = ScreenToWindow(point);

  // Determine the scrollbar element under the cursor, if any
  Element element;
  if (m_v.visible && m_v.rect.PtInRect(point))
  {
    element.bar = BarVertical;

    int btnSize = DPI::getSystemMetrics(SM_CYVSCROLL,DPI::getWindowDPI(this));
    int pos = point.y - m_v.rect.top;
    Slider slide = GetVerticalSlider();
    if (pos < btnSize)
      element.part = PartUpButton;
    else if (pos < btnSize + slide.pos)
      element.part = PartAboveSlider;
    else if (pos < btnSize + slide.pos + slide.len)
      element.part = PartSlider;
    else if (pos < m_v.rect.bottom - btnSize)
      element.part = PartBelowSlider;
    else
      element.part = PartDownButton;
  }
  else if (m_h.visible && m_h.rect.PtInRect(point))
  {
    element.bar = BarHorizontal;
    //TODO: horizontal
  }
  return element;
}

void DrawScrollWindow::DoElementAction(const Element& element)
{
  BarState* state = NULL;
  switch (element.bar)
  {
  case BarVertical:
    state = &m_v;
    break;
  case BarHorizontal:
    state = &m_h;
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  WPARAM wp;
  bool changed = false;

  switch (element.part)
  {
  case PartUpButton:
    changed = state->SetPos(state->pos-1);
    wp = SB_LINEUP;
    break;
  case PartAboveSlider:
    changed = state->SetPos(state->pos - state->page);
    wp = SB_PAGEUP;
    break;
  case PartBelowSlider:
    changed = state->SetPos(state->pos + state->page);
    wp = SB_PAGEDOWN;
    break;
  case PartDownButton:
    changed = state->SetPos(state->pos+1);
    wp = SB_LINEDOWN;
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  if (changed)
  {
    UpdateMouseOver();
    RedrawNonClient();
    switch (element.bar)
    {
    case BarVertical:
      PostMessage(WM_VSCROLL,wp);
      break;
    case BarHorizontal:
      PostMessage(WM_HSCROLL,wp);
      break;
    }
  }
}

void DrawScrollWindow::UpdateMouseOver(void)
{
  Element element = GetElementAtCursor();
  if (element != m_mouseOver)
    m_mouseOver = element;
}

bool DrawScrollWindow::IsDraggingSlider(Bar bar)
{
  return ((m_capture.bar == bar) && (m_capture.part == PartSlider));
}

void DrawScrollWindow::StartTimer(Timer timer)
{
  StopTimer();
  switch (timer)
  {
  case TimerInitialClick:
    m_timer = SetTimer(timer,500,NULL);
    break;
  case TimerMouseCapture:
    m_timer = SetTimer(timer,50,NULL);
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

void DrawScrollWindow::StopTimer(void)
{
  if (m_timer != 0)
  {
    KillTimer(m_timer);
    m_timer = 0;
  }
}

void DrawScrollWindow::DoCaptureTimer(void)
{
  Element element = GetElementAtCursor();
  switch (m_capture.part)
  {
  case PartUpButton:
  case PartAboveSlider:
  case PartBelowSlider:
  case PartDownButton:
    // Only do something if the cursor is over the element that started this action
    if (element == m_capture)
      DoElementAction(m_capture);
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

bool DrawScrollWindow::BarState::SetPos(int newPos)
{
  int maxPos = max - page + 1;
  if (newPos < min)
    newPos = min;
  else if (newPos > maxPos)
    newPos = maxPos;

  if (pos != newPos)
  {
    pos = newPos;
    return true;
  }
  return false;
}

bool DrawScrollWindow::BarState::SetTrackPos(int newTrackPos)
{
  int maxPos = max - page + 1;
  if (newTrackPos < min)
    newTrackPos = min;
  else if (newTrackPos > maxPos)
    newTrackPos = maxPos;

  if (trackPos != newTrackPos)
  {
    trackPos = newTrackPos;
    return true;
  }
  return false;
}

bool DrawScrollWindow::Element::operator==(const Element& element) const
{
  return ((bar == element.bar) && (part == element.part));
}

bool DrawScrollWindow::Element::operator!=(const Element& element) const
{
  return ((bar != element.bar) || (part != element.part));
}

void DrawChromeScroll::DrawVertical(CDC& dc, DarkMode* dark, bool drag,
  const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide,
  const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture)
{
  // Draw the background
  dc.FillSolidRect(state.rect,GetSysColour(dark,COLOR_BTNFACE));

  // Draw the arrows
  DrawVerticalArrow(dc,dark,drag,state,slide,DrawScrollWindow::PartUpButton,hot,capture);
  DrawVerticalArrow(dc,dark,drag,state,slide,DrawScrollWindow::PartDownButton,hot,capture);

  // Draw the slider bar
  bool sliderHot = (hot.bar == DrawScrollWindow::BarVertical) && (hot.part == DrawScrollWindow::PartSlider);
  COLORREF fore = GetSysColour(dark,drag || sliderHot ? COLOR_BTNSHADOW : COLOR_SCROLLBAR);
  dc.FillSolidRect(state.rect.left+1,state.rect.top+slide.btn+slide.pos,state.rect.Width()-2,slide.len,fore);
}

void DrawChromeScroll::DrawVerticalArrow(CDC& dc, DarkMode* dark, bool drag,
  const DrawScrollWindow::BarState& state, const DrawScrollWindow::Slider& slide, DrawScrollWindow::Part part,
  const DrawScrollWindow::Element& hot, const DrawScrollWindow::Element& capture)
{
  CRect r;
  int dir = 0;
  bool atEnd = false;

  switch (part)
  {
  case DrawScrollWindow::PartUpButton:
    r = CRect(state.rect.left,state.rect.top,state.rect.right,state.rect.top+slide.btn);
    dir = +1;
    atEnd = ((drag ? state.trackPos : state.pos) <= 0);
    break;
  case DrawScrollWindow::PartDownButton:
    r = CRect(state.rect.left,state.rect.bottom-slide.btn,state.rect.right,state.rect.bottom);
    dir = -1;
    atEnd = ((drag ? state.trackPos : state.pos) >= state.max - state.page + 1);
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  COLORREF back = GetSysColour(dark,COLOR_BTNFACE);
  COLORREF fore = GetSysColour(dark,COLOR_BTNTEXT);

  if (atEnd)
    fore = GetSysColour(dark,COLOR_BTNSHADOW);
  else if ((capture.bar == DrawScrollWindow::BarVertical) && (capture.part == part))
  {
    back = GetSysColour(dark,COLOR_BTNSHADOW);
    fore = GetSysColour(dark,COLOR_WINDOW);
  }
  else if ((hot.bar == DrawScrollWindow::BarVertical) && (hot.part == part))
  {
    back = GetSysColour(dark,COLOR_SCROLLBAR);
    fore = GetSysColour(dark,COLOR_BTNTEXT);
  }

  dc.FillSolidRect(r,back);

  CPen pen;
  pen.CreatePen(PS_SOLID,1,fore);
  CPen* oldPen = dc.SelectObject(&pen);

  CPoint c = r.CenterPoint();
  int y = c.y-(dir*(r.Height()/16));
  for (int i = 0; i < r.Height()/4; i++)
  {
    dc.MoveTo(c.x-i,y+(dir*i));
    dc.LineTo(c.x+i+1,y+(dir*i));
  }

  dc.SelectObject(oldPen);
}

COLORREF DrawChromeScroll::GetSysColour(DarkMode* dark, int sysColour)
{
  if (dark)
  {
    switch (sysColour)
    {
    case COLOR_BTNFACE:
      return dark->GetColour(DarkMode::Dark3);
    case COLOR_BTNSHADOW:
      return dark->GetColour(DarkMode::Dark1);
    case COLOR_BTNTEXT:
      return dark->GetColour(DarkMode::Fore);
    case COLOR_SCROLLBAR:
      return RGB(0x68,0x68,0x68);
    case COLOR_WINDOW:
      return dark->GetColour(DarkMode::Darkest);
    default:
      ASSERT(FALSE);
      break;
    }
  }

  return ::GetSysColor(sysColour);
}
