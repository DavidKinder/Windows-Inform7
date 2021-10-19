#include "stdafx.h"
#include "CommandButton.h"
#include "Inform.h"
#include "Dib.h"
#include "ScaleGfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CommandButton, CButton)

BEGIN_MESSAGE_MAP(CommandButton, CButton)
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_PAINT()
END_MESSAGE_MAP()

CommandButton::CommandButton() : m_backIndex(0), m_tabStop(0), m_mouseOver(false)
{
}

void CommandButton::SetBackSysColor(int index)
{
  m_backIndex = index;
}

void CommandButton::SetTabStop(int tab)
{
  m_tabStop = tab;
}

void CommandButton::SetIcon(const char* name)
{
  m_iconName = name;
}

void CommandButton::UpdateDPI(void)
{
  m_icon.DeleteBitmap();
}

void CommandButton::OnMouseMove(UINT nFlags, CPoint point)
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

LRESULT CommandButton::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOver == true)
  {
    m_mouseOver = false;
    Invalidate();
  }
  return Default();
}

static COLORREF Darken(COLORREF colour, double factor)
{
  BYTE r = GetRValue(colour);
  BYTE g = GetGValue(colour);
  BYTE b = GetBValue(colour);
  r = (BYTE)(r * factor);
  g = (BYTE)(g * factor);
  b = (BYTE)(b * factor);
  return RGB(r,g,b);
}

void CommandButton::OnPaint()
{
  CRect client;
  GetClientRect(client);

  CPaintDC dcPaint(this);
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);
  dc.SetBkMode(TRANSPARENT);

  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  CFont* oldFont = dc.SelectObject(GetFont());

  COLORREF backColour = ::GetSysColor(m_backIndex);
  if (GetState() & BST_PUSHED)
    backColour = Darken(backColour,0.9);
  dc.FillSolidRect(client,backColour);

  if (m_mouseOver)
  {
    CPen border;
    border.CreatePen(PS_SOLID,1,Darken(backColour,0.8));
    CPen* oldPen = dc.SelectObject(&border);
    dc.MoveTo(0,0);
    dc.LineTo(client.right-1,0);
    dc.LineTo(client.right-1,client.bottom-1);
    dc.LineTo(0,client.bottom-1);
    dc.LineTo(0,0);
    dc.SelectObject(oldPen);
  }

  CRect textRect(client);
  textRect.left = theApp.MeasureFont(this,GetFont()).cx;

  GetScaledIcon();
  if (m_icon.GetSafeHandle() != 0)
  {
    textRect.left = (textRect.left/2) + m_icon.GetSize().cx;
    bitmap.AlphaBlend(&m_icon,0,2,false);
  }

  dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
  CString caption;
  GetWindowText(caption);
  int tab = caption.Find('\t');
  if (tab > 0)
  {
    dc.DrawText(caption.Left(tab),textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
    textRect.left += m_tabStop;
    dc.DrawText(caption.Mid(tab+1),textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
  }
  else
    dc.DrawText(caption,textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

  if (CWnd::GetFocus() == this)
  {
    if ((SendMessage(WM_QUERYUISTATE) & UISF_HIDEFOCUS) == 0)
      dc.DrawFocusRect(client);
  }

  dc.SelectObject(oldFont);
  dcPaint.BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void CommandButton::GetScaledIcon(void)
{
  if (m_icon.GetSafeHandle() != 0)
    return;
  if (m_iconName.IsEmpty())
    return;

  CDibSection* original = theApp.GetCachedImage(m_iconName);
  ASSERT(original != NULL);
  CSize originalSize = original->GetSize();

  CRect client;
  GetClientRect(client);
  int h = client.Height()-4;
  double factor = h / originalSize.cy;
  int w = (int)(originalSize.cx * ((double)h / (double)originalSize.cy));

  // Create a bitmap for the scaled icon
  CDC* dc = GetDC();
  m_icon.CreateBitmap(dc->GetSafeHdc(),w,h);
  ReleaseDC(dc);

  // Scale and stretch the background
  ScaleGfx(original->GetBits(),originalSize.cx,originalSize.cy,m_icon.GetBits(),w,h);
}
