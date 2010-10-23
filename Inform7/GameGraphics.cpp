#include "stdafx.h"
#include "Inform.h"
#include "GameGraphics.h"

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(GameGraphics, CWnd)

BEGIN_MESSAGE_MAP(GameGraphics, CWnd)
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

GameGraphics::GameGraphics(MainWindow* main)
{
  ASSERT(main != NULL);

  m_main = main;
  m_rect = CRect(0,0,0,0);
  m_back = RGB(0xFF,0xFF,0xFF);
  m_bitmap = NULL;

  Create(NULL,"",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),main->GetWnd(),0);
}

GameGraphics::~GameGraphics()
{
  if (m_bitmap)
    delete m_bitmap;
}

bool GameGraphics::IsKindOf(const CRuntimeClass* rclass) const
{
  return (CWnd::IsKindOf(rclass) != FALSE);
}

BOOL GameGraphics::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

HWND GameGraphics::GetSafeHwnd(void) const
{
  return CWnd::GetSafeHwnd();
}

void GameGraphics::Layout(const CRect& r)
{
  m_rect = r;
  m_rect.MoveToXY(0,0);
  m_main->DeferMoveWindow(GetSafeHwnd(),r);

  CDC* dcWnd = GetDC();
  CDC dcMem;
  dcMem.CreateCompatibleDC(dcWnd);

  if ((m_rect.Width() > 0) && (m_rect.Height() > 0))
  {
    CDibSection* bitmap = new CDibSection;
    if (bitmap->CreateBitmap(dcWnd->GetSafeHdc(),m_rect.Width(),m_rect.Height()))
    {
      CBitmap* oldBitmap = CDibSection::SelectDibSection(dcMem,bitmap);
      dcMem.FillSolidRect(m_rect,m_back);

      if (m_bitmap != NULL)
      {
        CDC dcMem2;
        dcMem2.CreateCompatibleDC(dcWnd);

        // Copy old image into new bitmap
        CSize size = m_bitmap->GetSize();
        CBitmap* oldBitmap2 = CDibSection::SelectDibSection(dcMem2,m_bitmap);
        dcMem.BitBlt(0,0,size.cx,size.cy,&dcMem2,0,0,SRCCOPY);
        dcMem2.SelectObject(oldBitmap2);
      }

      dcMem.SelectObject(oldBitmap);
      if (m_bitmap)
        delete m_bitmap;
      m_bitmap = bitmap;
    }
    else
      delete bitmap;
  }
  ReleaseDC(dcWnd);
}

void GameGraphics::GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r)
{
  w = size;
  h = size;
}

void GameGraphics::AddText(const CStringW& text, bool fromSkein)
{
}

void GameGraphics::ClearText(bool styles, bool reverse)
{
}

void GameGraphics::SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size)
{
}

void GameGraphics::SetColours(COLORREF fore, COLORREF back)
{
}

void GameGraphics::SetCursor(int x, int y)
{
}

void GameGraphics::MoveToEnd(void)
{
}

void GameGraphics::Draw(CDibSection* image, int val1, int val2, int width, int height)
{
  if (m_bitmap != NULL)
  {
    CSize bmpSize = m_bitmap->GetSize();
    CSize imgSize = image->GetSize();

    // Apply scaling, if needed
    bool scale = false;
    if ((width >= 0) && (width != imgSize.cx))
    {
      imgSize.cx = width;
      scale = true;
    }
    if ((height >= 0) && (height != imgSize.cy))
    {
      imgSize.cy = height;
      scale = true;
    }

    // Work out clipping if the image is only partially in the bitmap
    int x1 = 0, x2 = imgSize.cx, y1 = 0, y2 = imgSize.cy;
    if (val1 < 0)
      x1 = val1 * -1;
    if (val2 < 0)
      y1 = val2 * -1;
    if (val1 + x2 > bmpSize.cx)
      x2 = bmpSize.cx - val1;
    if (val2 + y2 > bmpSize.cy)
      y2 = bmpSize.cy - val2;

    // If the graphic is being scaled, copy and stretch the graphic
    // into a temporary DIBSection, otherwise just use the bits from
    // the graphic.
    CDibSection scaledBitmap;
    DWORD* bits = NULL;
    if (scale)
    {
      // Create a temporary bitmap
      CDC* dcWnd = GetDC();
      scaledBitmap.CreateBitmap(dcWnd->GetSafeHdc(),imgSize.cx,imgSize.cy);
      ReleaseDC(dcWnd);

      // Scale the image into the temporary bitmap
      bits = scaledBitmap.GetBits();
      ScaleGfx((COLORREF*)image->GetBits(),image->GetSize().cx,image->GetSize().cy,
        bits,imgSize.cx,imgSize.cy);
    }
    else
      bits = image->GetBits();

    // Alpha blend each pixel of the graphic into the graphics window
    ::GdiFlush();
    for (int y = y1; y < y2; y++)
    {
      for (int x = x1; x < x2; x++)
      {
        // Split the source colour into red, green, blue and alpha
        DWORD srcColour = CDibSection::GetPixel(bits,imgSize.cx,x,y);
        int sb = srcColour & 0xFF;
        srcColour >>= 8;
        int sg = srcColour & 0xFF;
        srcColour >>= 8;
        int sr = srcColour & 0xFF;
        srcColour >>= 8;
        int a = srcColour & 0xFF;

        // Split the destination colour into red, green, blue and alpha
        DWORD destColour = m_bitmap->GetPixel(x+val1,y+val2);
        int db = destColour & 0xFF;
        destColour >>= 8;
        int dg = destColour & 0xFF;
        destColour >>= 8;
        int dr = destColour & 0xFF;

        // Perform alpha blending
        if (a == 0)
        {
        }
        else if (a == 0xFF)
        {
          dr = sr;
          dg = sg;
          db = sb;
        }
        else
        {
          // Rescale from 0..255 to 0..256
          a += a>>7;
          dr += (a * (sr - dr) >> 8);
          dg += (a * (sg - dg) >> 8);
          db += (a * (sb - db) >> 8);
        }
        m_bitmap->SetPixel(x+val1,y+val2,(dr<<16)|(dg<<8)|db);
      }
    }
  }
  Invalidate();
}

COLORREF GameGraphics::GetAlphaColour(void)
{
  return -1;
}

void GameGraphics::SetLink(int link)
{
}

void GameGraphics::SetParagraph(Justify justify)
{
}

void GameGraphics::SetBackColour(COLORREF colour)
{
  m_back = colour;
}

void GameGraphics::FillRect(const CRect& r, COLORREF colour)
{
  if (m_bitmap != NULL)
  {
    CDC* dcWnd = GetDC();
    CDC dcMem;
    dcMem.CreateCompatibleDC(dcWnd);

    CBitmap* oldBitmap = CDibSection::SelectDibSection(dcMem,m_bitmap);
    dcMem.FillSolidRect(r,colour);
    dcMem.SelectObject(oldBitmap);

    ReleaseDC(dcWnd);
  }
  Invalidate();
}

void GameGraphics::OnPaint()
{
  CPaintDC dc(this);

  if (m_bitmap != NULL)
  {
    CDC* dcWnd = GetDC();
    CDC dcMem;
    dcMem.CreateCompatibleDC(dcWnd);

    CSize size = m_bitmap->GetSize();
    CBitmap* oldBitmap = CDibSection::SelectDibSection(dcMem,m_bitmap);
    dc.BitBlt(0,0,size.cx,size.cy,&dcMem,0,0,SRCCOPY);
    dcMem.SelectObject(oldBitmap);

    ReleaseDC(dcWnd);
  }
  else
    dc.FillSolidRect(m_rect,m_back);
}

void GameGraphics::OnLButtonDown(UINT nFlags, CPoint point) 
{
  m_main->GameMouseEvent(this,point.x,point.y);
  CWnd::OnLButtonDown(nFlags,point);
}
