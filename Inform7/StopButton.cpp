#include "stdafx.h"
#include "StopButton.h"
#include "Inform.h"
#include "Dib.h"
#include "DpiFunctions.h"
#include "ScaleGfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(StopButton, CButton)

BEGIN_MESSAGE_MAP(StopButton, CButton)
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL StopButton::Create(DWORD style, const RECT& rect, CWnd* parent, UINT id)
{
  return CButton::Create("",style,rect,parent,id);
}

CSize StopButton::GetButtonSize(void)
{
  CSize sz = theApp.MeasureFont(this,theApp.GetFont(this,InformApp::FontSystem));
  sz.cx = sz.cy;
  return sz;
}

BOOL StopButton::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= BS_PUSHBUTTON|BS_OWNERDRAW;
  return CButton::PreCreateWindow(cs);
}

void StopButton::DrawItem(LPDRAWITEMSTRUCT dis)
{
  CDC* dcPaint = CDC::FromHandle(dis->hDC);
  CRect rectPaint(dis->rcItem);
  CDibSection* dib = GetImage((dis->itemState & ODS_SELECTED));

  CDC imageDC;
  imageDC.CreateCompatibleDC(dcPaint);
  CBitmap imageBitmap;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(imageDC,dib);
  dcPaint->BitBlt(0,0,rectPaint.Width(),rectPaint.Height(),&imageDC,0,0,SRCCOPY);
  imageDC.SelectObject(oldBitmap);
}

BOOL StopButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  ::SetCursor(::LoadCursor(0,IDC_ARROW));
  return TRUE;
}

CDibSection* StopButton::GetImage(bool dark)
{
  // Is the image in the cache?
  CString imageName;
  imageName.Format(dark ? "Stop-scaled-%ddpi-dark" : "Stop-scaled-%ddpi",DPI::getWindowDPI(this));
  CDibSection* imageDib = theApp.GetCachedImage(imageName);
  if (imageDib != NULL)
    return imageDib;

  // Load the unscaled image
  CDibSection* dib = theApp.GetCachedImage("Stop");
  CSize dibSize = dib->GetSize();

  // Create a scaled image
  CSize imageDibSize = GetButtonSize();
  imageDib = new CDibSection();
  CDC* dc = GetDC();
  imageDib->CreateBitmap(dc->GetSafeHdc(),imageDibSize.cx,imageDibSize.cy);
  ReleaseDC(dc);

  // Scale and stretch the image
  ScaleGfx(dib->GetBits(),dibSize.cx,dibSize.cy,
    imageDib->GetBits(),imageDibSize.cx,imageDibSize.cy);

  // Adjust and alpha blend with the background colour
  if (dark)
    imageDib->Darken(0.7);
  imageDib->AlphaBlend(::GetSysColor(COLOR_BTNFACE));

  theApp.CacheImage(imageName,imageDib);
  return imageDib;
}
