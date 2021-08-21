#include "stdafx.h"
#include "FlatButton.h"
#include "Inform.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FlatButton, CButton)

BEGIN_MESSAGE_MAP(FlatButton, CButton)
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

FlatButton::FlatButton() : m_mouseOver(false)
{
}

BOOL FlatButton::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= BS_PUSHBUTTON|BS_OWNERDRAW;
  return CButton::PreCreateWindow(cs);
}

void FlatButton::DrawItem(LPDRAWITEMSTRUCT dis)
{
  CDC* dcPaint = CDC::FromHandle(dis->hDC);
  CRect rectPaint(dis->rcItem);
  bool disabled = (dis->itemState & ODS_DISABLED) != 0;

  // Create a bitmap to paint into
  CDC dc;
  dc.CreateCompatibleDC(dcPaint);
  CRect rect(0,0,rectPaint.Width(),rectPaint.Height());
  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),rect.Width(),rect.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);

  // Draw the background
  if (dis->itemState & ODS_SELECTED)
    theApp.DrawSelectRect(dc,rect,false);
  else if (m_mouseOver && !disabled)
    theApp.DrawSelectRect(dc,rect,true);
  else
    dc.FillSolidRect(rect,::GetSysColor(COLOR_BTNFACE));

  // Get the button's text
  CString text;
  GetWindowText(text);

  // Draw the contents of the button
  if (text == "?<")
  {
    CRect imageRect(rect);
    if (imageRect.Width() > imageRect.Height())
      imageRect.DeflateRect((imageRect.Width() - imageRect.Height())/2,0);
    int gap = imageRect.Height()/6;
    imageRect.DeflateRect(gap,gap);
    CDibSection* dib = GetImage("Arrow-left",imageRect.Size(),disabled);
    bitmap.AlphaBlend(dib,gap,gap,FALSE);
  }
  else if (text == "?>")
  {
    CRect imageRect(rect);
    if (imageRect.Width() > imageRect.Height())
      imageRect.DeflateRect((imageRect.Width() - imageRect.Height())/2,0);
    int gap = imageRect.Height()/6;
    imageRect.DeflateRect(gap,gap);
    CDibSection* dib = GetImage("Arrow-right",imageRect.Size(),disabled);
    bitmap.AlphaBlend(dib,gap,gap,FALSE);
  }

  // Draw the control from the bitmap
  dcPaint->BitBlt(rectPaint.left,rectPaint.top,rectPaint.Width(),rectPaint.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void FlatButton::OnMouseMove(UINT nFlags, CPoint point)
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

LRESULT FlatButton::OnMouseLeave(WPARAM, LPARAM)
{
  if (m_mouseOver == true)
  {
    m_mouseOver = false;
    Invalidate();
  }
  return Default();
}

CDibSection* FlatButton::GetImage(const char* name, const CSize& size, bool light)
{
  // Is the image in the cache?
  CString scaleName;
  scaleName.Format("%s-scaled-%ddpi",name,DPI::getWindowDPI(this));
  if (light)
    scaleName += "-light";
  CDibSection* dib = theApp.GetCachedImage(scaleName);
  if (dib != NULL)
    return dib;

  // Create the scaled image
  CDibSection* original_dib = theApp.GetCachedImage(name);
  CSize original_size = original_dib->GetSize();
  double scaleX = (double)size.cx / (double)original_size.cx;
  double scaleY = (double)size.cy / (double)original_size.cy;
  dib = theApp.CreateScaledImage(original_dib,scaleX,scaleY);
  if (light)
  {
    int sr, sg, sb, a;
    DWORD src;

    CSize size = dib->GetSize();
    for (int y = 0; y < size.cy; y++)
    {
      for (int x = 0; x < size.cx; x++)
      {
        src = dib->GetPixel(x,y);
        sb = src & 0xFF;
        src >>= 8;
        sg = src & 0xFF;
        src >>= 8;
        sr = src & 0xFF;
        src >>= 8;
        a = src & 0xFF;

        const double lighten = 0.4;
        sr = (int)(0xFF - ((0xFF - sr) * lighten));
        sg = (int)(0xFF - ((0xFF - sg) * lighten));
        sb = (int)(0xFF - ((0xFF - sb) * lighten));

        dib->SetPixel(x,y,(a<<24)|(sr<<16)|(sg<<8)|sb);
      }
    }
  }
  theApp.CacheImage(scaleName,dib);
  return dib;
}
