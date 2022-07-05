#include "stdafx.h"
#include "CommandButton.h"
#include "Dib.h"
#include "Inform.h"
#include "Messages.h"
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

void CommandButton::SetBackSysColor(int index)
{
  m_backIndex = index;
}

void CommandButton::SetTabStop(int tab)
{
  m_tabStop = tab;
}

void CommandButton::SetIcon(const char* name, int left)
{
  m_iconName = name;
  m_iconLeft = left;
  m_icon.DeleteBitmap();
}

void CommandButton::ChangeWidthForIcon(void)
{
  GetScaledIcon();
  if (m_icon.GetSafeHandle() != 0)
  {
    // Change the width of the button to fit the icon, assumes no text
    CRect r;
    GetWindowRect(r);
    SetWindowPos(NULL,0,0,(m_iconLeft*2) + m_icon.GetSize().cx,r.Height(),
      SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREPOSITION|SWP_NOZORDER);
  }
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
    textRect.left = (textRect.left/2) + m_iconLeft + m_icon.GetSize().cx;
    bitmap.AlphaBlend(&m_icon,m_iconLeft,2,false);
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

IMPLEMENT_DYNAMIC(CommandListBox, CListBox)

BEGIN_MESSAGE_MAP(CommandListBox, CListBox)
  ON_WM_ERASEBKGND()
  ON_WM_KEYUP()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

void CommandListBox::SetBackSysColor(int index)
{
  m_backIndex = index;
}

void CommandListBox::SetTabStop(int tab)
{
  m_tabStop = tab;
}

void CommandListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  RECT& rect = lpDrawItemStruct->rcItem;
  COLORREF backColour = ::GetSysColor(m_backIndex);

  if (lpDrawItemStruct->itemID == -1)
  {
    CRect r;
    GetClientRect(r);
    CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
    dc->FillSolidRect(r,backColour);
    return;
  }

  HDC hdc = 0;
  HANDLE pb = ::BeginBufferedPaint(lpDrawItemStruct->hDC,&rect,BPBF_COMPATIBLEBITMAP,NULL,&hdc);
  if (pb == 0)
    return;
  CDC* dc = CDC::FromHandle(hdc);
  CFont* oldFont = dc->SelectObject(GetFont());

  if (m_hotSelect && (m_hotIndex >= 0) && (lpDrawItemStruct->itemID == m_hotIndex))
    backColour = Darken(backColour,0.9);
  dc->FillSolidRect(&rect,backColour);
  dc->SetTextColor(::GetSysColor(COLOR_BTNTEXT));

  CString text;
  GetText(lpDrawItemStruct->itemID,text);

  CRect textRect(rect);
  textRect.left += theApp.MeasureFont(this,GetFont()).cx;

  int tab = text.Find('\t');
  if (tab > 0)
  {
    dc->DrawText(text.Left(tab),textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER);
    textRect.left = m_tabStop;
    dc->DrawText(text.Mid(tab+1),textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER);
  }
  else
    dc->DrawText(text,textRect,DT_SINGLELINE|DT_LEFT|DT_VCENTER);

  if ((m_hotIndex >= 0) && (lpDrawItemStruct->itemID == m_hotIndex))
  {
    CPen border;
    border.CreatePen(PS_SOLID,1,Darken(backColour,0.8));
    CPen* oldPen = dc->SelectObject(&border);
    dc->MoveTo(rect.left,rect.top);
    dc->LineTo(rect.right-1,rect.top);
    dc->LineTo(rect.right-1,rect.bottom-1);
    dc->LineTo(rect.left,rect.bottom-1);
    dc->LineTo(rect.left,rect.top);
    dc->SelectObject(oldPen);
  }

  if (CWnd::GetFocus() == this)
  {
    if (lpDrawItemStruct->itemState & ODS_FOCUS)
    {
      if ((SendMessage(WM_QUERYUISTATE) & UISF_HIDEFOCUS) == 0)
        dc->DrawFocusRect(&rect);
    }
  }

  dc->SelectObject(oldFont);
  ::EndBufferedPaint(pb,TRUE);
}

BOOL CommandListBox::OnEraseBkgnd(CDC* pDC)
{
  CRect r;
  GetClientRect(r);
  pDC->FillSolidRect(r,::GetSysColor(m_backIndex));
  return TRUE;
}

void CommandListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  CListBox::OnKeyUp(nChar,nRepCnt,nFlags);

  if ((nChar == VK_RETURN) || (nChar == VK_SPACE))
  {
    int index = GetCaretIndex();
    if (index >= 0)
      GetParent()->PostMessage(WM_CMDLISTCLICKED,GetDlgCtrlID(),index);
  }
}

void CommandListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
  CListBox::OnLButtonUp(nFlags,point);

  m_hotSelect = true;
  Invalidate();
}

void CommandListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
  CListBox::OnLButtonUp(nFlags,point);

  BOOL outside = TRUE;
  int index = ItemFromPoint(point,outside);
  if ((index >= 0) && !outside)
    GetParent()->PostMessage(WM_CMDLISTCLICKED,GetDlgCtrlID(),index);

  m_hotSelect = false;
  Invalidate();
}

void CommandListBox::OnMouseMove(UINT nFlags, CPoint point)
{
  if (m_mouseOver)
  {
    // Has the highlighted item changed?
    BOOL outside = TRUE;
    int hot = ItemFromPoint(point,outside);
    if (outside)
      hot = -1;

    if (m_hotIndex != hot)
    {
      m_hotIndex = hot;
      Invalidate();
    }
  }
  else
  {
    m_mouseOver = true;
    BOOL outside = TRUE;
    int hot = ItemFromPoint(point,outside);
    if (!outside)
      m_hotIndex = hot;
    Invalidate();

    // Listen for the mouse leaving this control
    TRACKMOUSEEVENT tme;
    ::ZeroMemory(&tme,sizeof tme);
    tme.cbSize = sizeof tme;
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = GetSafeHwnd();
    ::TrackMouseEvent(&tme);
  }
  CListBox::OnMouseMove(nFlags,point);
}

LRESULT CommandListBox::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOver == true)
  {
    m_mouseOver = false;
    m_hotIndex = -1;
    Invalidate();
  }
  return Default();
}
