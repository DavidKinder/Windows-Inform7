#include "stdafx.h"
#include "FlatButton.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FlatButton, CButton)

BEGIN_MESSAGE_MAP(FlatButton, CButton)
END_MESSAGE_MAP()

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
  {
    // Get the bitmap to indicate a selected button
    CBitmap selectBitmap;
    selectBitmap.LoadBitmap(IDR_FLAT_SELECT);
    CDC selectDC;
    selectDC.CreateCompatibleDC(&dc);
    CBitmap* oldBitmap = selectDC.SelectObject(&selectBitmap);

    // Get the bitmap's dimensions
    BITMAP bitmapInfo;
    selectBitmap.GetBitmap(&bitmapInfo);

    // Stretch the bitmap into the selected item's background
    dc.StretchBlt(0,0,rect.Width(),rect.Height(),
      &selectDC,0,0,bitmapInfo.bmWidth,bitmapInfo.bmHeight,SRCCOPY);
    selectDC.SelectObject(oldBitmap);
  }
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

  // Draw the separator
  CPen shadowPen(PS_SOLID,0,::GetSysColor(COLOR_BTNSHADOW));
  dc.SelectObject(shadowPen);
  int gap = rect.Height()/6;
  dc.MoveTo(rect.right-1,gap);
  dc.LineTo(rect.right-1,rect.bottom-gap);

  // Draw the control from the bitmap
  dcPaint->BitBlt(rectPaint.left,rectPaint.top,rectPaint.Width(),rectPaint.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

CDibSection* FlatButton::GetImage(const char* name, const CSize& size, bool light)
{
  // Is the image in the cache?
  CString scaleName;
  scaleName.Format("%s-scaled",name);
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
