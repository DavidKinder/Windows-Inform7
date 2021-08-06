#include "stdafx.h"
#include "ButtonTab.h"
#include "Inform.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ButtonTab, CTabCtrl)

BEGIN_MESSAGE_MAP(ButtonTab, CTabCtrl)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
END_MESSAGE_MAP()

int ButtonTab::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
    return -1;
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
  return 0;
}

BOOL ButtonTab::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void ButtonTab::OnPaint()
{
  CRect client;
  GetClientRect(client);
  CRect below(client);
  AdjustRect(FALSE,below);
  client.bottom = below.top;

  CPaintDC dcPaint(this);
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);

  CDibSection bitmap;
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  dc.FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));

  CPen linePen(PS_SOLID,0,::GetSysColor(COLOR_BTNSHADOW));
  CFont* oldFont = dc.SelectObject(GetFont());
  CPen* oldPen = dc.SelectObject(&linePen);
  dc.SetBkMode(TRANSPARENT);

  int sel = GetCurSel();
  for (int i = 0; i < GetItemCount(); i++)
  {
    TCITEM item;
    ::ZeroMemory(&item,sizeof item);
    item.mask = TCIF_TEXT;

    CString text;
    item.pszText = text.GetBufferSetLength(256);
    item.cchTextMax = 256;
    GetItem(i,&item);
    text.ReleaseBuffer();

    CRect itemRect;
    GetItemRect(i,itemRect);
    itemRect.top = client.top;
    itemRect.bottom = client.bottom;

    if (i == sel)
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
      dc.StretchBlt(itemRect.left,itemRect.top,itemRect.Width(),itemRect.Height(),
        &selectDC,0,0,bitmapInfo.bmWidth,bitmapInfo.bmHeight,SRCCOPY);
      selectDC.SelectObject(oldBitmap);
    }
    else
    {
      if (i == 0)
      {
        int gap = itemRect.Height()/6;
        dc.MoveTo(itemRect.left,itemRect.top+gap);
        dc.LineTo(itemRect.left,itemRect.bottom-gap);
      }
      if ((i != sel-1) && (i != GetItemCount()-1))
      {
        int gap = itemRect.Height()/6;
        dc.MoveTo(itemRect.right,itemRect.top+gap);
        dc.LineTo(itemRect.right,itemRect.bottom-gap);
      }
    }

    if (text == "?H")
    {
      if (itemRect.Width() > itemRect.Height())
        itemRect.DeflateRect((itemRect.Width() - itemRect.Height())/2,0);
      itemRect.DeflateRect(itemRect.Height()/6,itemRect.Height()/6);
      CDibSection* dib = GetImage("Home",itemRect.Size());
      bitmap.AlphaBlend(dib,itemRect.left,itemRect.top,FALSE);
    }
    else
    {
      dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
      dc.DrawText(text,itemRect,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
    }
  }

  dc.SelectObject(oldFont);
  dc.SelectObject(oldPen);

  dcPaint.BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

void ButtonTab::UpdateDPI(void)
{
  SetFont(theApp.GetFont(this,InformApp::FontPanel));
}

CDibSection* ButtonTab::GetImage(const char* name, const CSize& size)
{
  // Is the image in the cache?
  CString scaleName;
  scaleName.Format("%s-scaled-%ddpi",name,DPI::getWindowDPI(this));
  CDibSection* dib = theApp.GetCachedImage(scaleName);
  if (dib != NULL)
    return dib;

  // Create the scaled image
  CDibSection* original_dib = theApp.GetCachedImage(name);
  CSize original_size = original_dib->GetSize();
  double scaleX = (double)size.cx / (double)original_size.cx;
  double scaleY = (double)size.cy / (double)original_size.cy;
  dib = theApp.CreateScaledImage(original_dib,scaleX,scaleY);
  theApp.CacheImage(scaleName,dib);
  return dib;
}
