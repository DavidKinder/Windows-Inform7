#include "stdafx.h"
#include "ArrowButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ArrowButton, CButton)

BEGIN_MESSAGE_MAP(ArrowButton, CButton)
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

ArrowButton::ArrowButton(ArrowStyle style) : m_style(style), m_mouseOver(false)
{
}

BOOL ArrowButton::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= BS_PUSHBUTTON;
  return CButton::PreCreateWindow(cs);
}

void ArrowButton::DrawControl(CDC* dc, CRect rect, UINT ownerDrawState)
{
  int offset = 0;

  // Get the button's text
  CString text;
  GetWindowText(text);
  TEXTMETRIC metrics;
  dc->GetTextMetrics(&metrics);

  if (::IsAppThemed())
  {
    // Open the button theme
    HTHEME theme = ::OpenThemeData(GetSafeHwnd(),L"Button");
    if (theme)
    {
      // Work out the button state
      UINT state = 0;
      if (ownerDrawState & ODS_SELECTED)
        state = PBS_PRESSED;
      else if (ownerDrawState & ODS_DISABLED)
        state = PBS_DISABLED;
      else if (m_mouseOver)
        state = PBS_HOT;
      else
        state = PBS_NORMAL;

      // Get the background size
      ::GetThemeBackgroundContentRect(theme,dc->GetSafeHdc(),BP_PUSHBUTTON,state,rect,NULL);

      // Draw the button's text
      CRect textRect(rect);
      textRect.left += metrics.tmAveCharWidth;
      CStringW textW(text);
      ::DrawThemeText(theme,dc->GetSafeHdc(),BP_PUSHBUTTON,state,textW,textW.GetLength(),
        DT_LEFT|DT_VCENTER|DT_SINGLELINE,DTT_GRAYED,textRect);

      // Draw the arrow
      DrawArrow(dc,rect.right-15+offset,(rect.bottom/2)+offset,
        (ownerDrawState & ODS_DISABLED) ? ::GetSysColor(COLOR_BTNSHADOW) : RGB(0,0,0),-1);

      // Draw the focus rectangle
      if ((ownerDrawState & ODS_FOCUS) && !(ownerDrawState & ODS_NOFOCUSRECT))
        dc->DrawFocusRect(rect);

      // Close the button theme
      ::CloseThemeData(theme);
    }
  }
  else
  {
    // Work out the button state
    UINT state = DFCS_BUTTONPUSH|DFCS_ADJUSTRECT;
    if (ownerDrawState & ODS_DISABLED)
      state |= DFCS_INACTIVE;
    if (ownerDrawState & ODS_SELECTED)
      state |= DFCS_PUSHED;

    // Draw the control's frame
    dc->DrawFrameControl(rect,DFC_BUTTON,state);

    // Shift co-ordinates if the button is down
    if (ownerDrawState & ODS_SELECTED)
      offset = 1;

    // Draw the button's text
    CRect textRect(rect);
    textRect.left += offset+metrics.tmAveCharWidth;
    textRect.top += offset-1+((textRect.Height()-metrics.tmHeight+1)/2);
    dc->DrawState(textRect.TopLeft(),textRect.Size(),text,
      (ownerDrawState & ODS_DISABLED) ? DSS_DISABLED : DSS_NORMAL,TRUE,0,(HBRUSH)0);

    // Draw the arrow
    DrawArrow(dc,rect.right-15+offset,(rect.bottom/2)+offset,
      (ownerDrawState & ODS_DISABLED) ? ::GetSysColor(COLOR_BTNSHADOW) : RGB(0,0,0),
      (ownerDrawState & ODS_DISABLED) ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : -1);

    // Draw the focus rectangle
    if ((ownerDrawState & ODS_FOCUS) && !(ownerDrawState & ODS_NOFOCUSRECT))
    {
      rect.DeflateRect(1,1);
      dc->DrawFocusRect(rect);
    }
  }
}

void ArrowButton::DrawArrow(CDC* dc, int x, int y, COLORREF colour, COLORREF highlight)
{
  // Create and select a pen
  CPen pen;
  pen.CreatePen(PS_SOLID,1,colour);
  CPen* oldPen = dc->SelectObject(&pen);

  int dir = 1;
  switch (m_style)
  {
  case DownLow:
    break;
  case DownCentre:
    y -= 1;
    break;
  case UpCentre:
    dir = -1;
    y += 3;
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  // Draw the arrow
  dc->MoveTo(x+9,y);
  dc->LineTo(x+0,y);
  dc->MoveTo(x+8,y+dir);
  dc->LineTo(x+1,y+dir);
  dc->MoveTo(x+7,y+(2*dir));
  dc->LineTo(x+2,y+(2*dir));
  dc->MoveTo(x+6,y+(3*dir));
  dc->LineTo(x+3,y+(3*dir));

  if (highlight != -1)
  {
    dc->SetPixel(x+9,y,highlight);
    dc->SetPixel(x+8,y+dir,highlight);
    dc->SetPixel(x+7,y+(2*dir),highlight);
    dc->SetPixel(x+6,y+(3*dir),highlight);
    dc->SetPixel(x+5,y+(4*dir),highlight);
  }
  else
    dc->SetPixel(x+5,y+(4*dir),colour);

  // Reset the device context
  dc->SelectObject(oldPen);
}

void ArrowButton::OnMouseMove(UINT nFlags, CPoint point)
{
  if (m_mouseOver == false)
  {
    m_mouseOver = true;
    Invalidate();

    // Listen for the mouse leaving this control
    TRACKMOUSEEVENT tme;
    ::ZeroMemory(&tme,sizeof tme);
    tme.cbSize = sizeof tme;
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = GetSafeHwnd();
    ::TrackMouseEvent(&tme);
  }
  CButton::OnMouseMove(nFlags,point);
}

LRESULT ArrowButton::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOver == true)
  {
    m_mouseOver = false;
    Invalidate();
  }
  return Default();
}

void ArrowButton::OnCustomDraw(NMHDR* nmhdr, LRESULT* result)
{
  NMCUSTOMDRAW* nmcd = (NMCUSTOMDRAW*)nmhdr;
  *result = CDRF_DODEFAULT;

  switch (nmcd->dwDrawStage)
  {
  case CDDS_PREPAINT:
    {
      UINT state = 0;
      if (nmcd->uItemState & CDIS_SELECTED)
        state |= ODS_SELECTED;
      if (nmcd->uItemState & CDIS_DISABLED)
        state |= ODS_DISABLED;
      if (nmcd->uItemState & CDIS_FOCUS)
        state |= ODS_FOCUS;
      if (SendMessage(WM_QUERYUISTATE) & UISF_HIDEFOCUS)
        state |= ODS_NOFOCUSRECT;

      // Draw the control without drawing the background
      DrawControl(CDC::FromHandle(nmcd->hdc),nmcd->rc,state);
      *result = CDRF_SKIPDEFAULT;
    }
    break;
  }
}
