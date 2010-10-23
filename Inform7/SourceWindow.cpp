#include "stdafx.h"
#include "SourceWindow.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SourceWindow, CWnd)

BEGIN_MESSAGE_MAP(SourceWindow, CWnd)
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_VSCROLL()
  ON_NOTIFY(SCNX_SETSCROLLINFO, 1, OnEditSetScrollInfo)
  ON_NOTIFY(SCNX_GETSCROLLINFO, 1, OnEditGetScrollInfo)
  ON_MESSAGE(WM_PRINT, OnPrint)
END_MESSAGE_MAP()

SourceWindow::SourceWindow()
{
  m_tearTop = false;
  m_tearBottom = false;
  m_arrowTop.SetRectEmpty();
  m_arrowBottom.SetRectEmpty();
  m_imageTop = NULL;
  m_imageBottom = NULL;
}

void SourceWindow::Create(CWnd* parent)
{
  // Get the top and bottom margin images
  m_imageTop = theApp.GetCachedImage("Torn-top");
  m_imageBottom = theApp.GetCachedImage("Torn-bottom");

  CWnd::Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_VSCROLL,CRect(0,0,0,0),parent,0);

  // Create the edit control and make this window in charge of the scroll bar
  m_edit.Create(this,1);
}

SourceEdit& SourceWindow::GetEdit(void)
{
  return m_edit;
}

const SourceHeading& SourceWindow::GetHeading(void)
{
  return m_heading;
}

void SourceWindow::GetAllHeadings(CArray<SourceLexer::Heading>& headings)
{
  return m_edit.GetAllHeadings(headings);
}

void SourceWindow::ShowBetween(int startLine, int endLine, const SourceHeading* heading)
{
  m_edit.ShowBetween(startLine,endLine);
  if (heading != NULL)
    m_heading.Copy(*heading);
  else
    m_heading.RemoveAll();

  bool resize = false;
  if (m_tearTop != (startLine > 0))
  {
    m_tearTop = (startLine > 0);
    resize = true;
  }
  if (m_tearBottom != (endLine > 0))
  {
    m_tearBottom = (endLine > 0);
    resize = true;
  }

  if (resize)
  {
    Resize();
    Invalidate();
  }
}

void SourceWindow::Highlight(int line, COLORREF colour, bool centre)
{
  // If the line is not currently visible, show the whole of the source
  if (!m_edit.IsLineShown(line))
    ShowBetween(0,0,NULL);

  m_edit.Highlight(line,colour,centre);
}

void SourceWindow::Highlight(CHARRANGE range, bool centre)
{
  CHARRANGE lines = m_edit.GetRangeLines(range);

  // If the range is not currently visible, show the whole of the source
  if (!m_edit.IsLineShown(lines.cpMin) || !m_edit.IsLineShown(lines.cpMax))
    ShowBetween(0,0,NULL);

  m_edit.Highlight(range,centre);
}

BOOL SourceWindow::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void SourceWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  CWnd::OnLButtonDown(nFlags,point);

  // Check for a click on an arrow
  CPoint cursor = GetCurrentMessage()->pt;
  ScreenToClient(&cursor);
  if (m_arrowTop.PtInRect(cursor))
    GetParent()->PostMessage(WM_NEXTRANGE,1);
  else if (m_arrowBottom.PtInRect(cursor))
    GetParent()->PostMessage(WM_NEXTRANGE,0);
}

void SourceWindow::OnPaint()
{
  Draw(CPaintDC(this));
}

LRESULT SourceWindow::OnPrint(WPARAM dc, LPARAM)
{
  Draw(*(CDC::FromHandle((HDC)dc)));

  // Use default processing to pass the print message to all child windows
  return Default();
}

void SourceWindow::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType,cx,cy);
  if (m_edit.GetSafeHwnd() != 0)
    Resize();
}

void SourceWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  // Send the scroll bar message to the edit control
  const MSG* msg = GetCurrentMessage();
  m_edit.SendMessage(WM_VSCROLL,msg->wParam,msg->lParam);
}

// Handle scroll bar call from the edit control
void SourceWindow::OnEditSetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result)
{
  SCNXSetScrollInfo* ssi = (SCNXSetScrollInfo*)pNotifyStruct;
  ssi->nPos = ::SetScrollInfo(GetSafeHwnd(),ssi->nBar,(LPCSCROLLINFO)ssi->lpsi,ssi->bRedraw);
  *result = 1;
}

// Handle scroll bar call from the edit control
void SourceWindow::OnEditGetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result)
{
  SCNXGetScrollInfo* gsi = (SCNXGetScrollInfo*)pNotifyStruct;
  GetScrollInfo(gsi->nBar,(LPSCROLLINFO)gsi->lpsi);
  *result = 1;
}

void SourceWindow::Resize(void)
{
  CRect client;
  GetClientRect(client);

  // Adjust for margins
  CSize fontSize = theApp.MeasureFont(&(theApp.GetFont()));
  if (m_tearTop)
    client.top += m_imageTop->GetSize().cy;
  else
    client.top += fontSize.cy/4;
  if (m_tearBottom)
    client.bottom -= m_imageBottom->GetSize().cy;
  else
    client.bottom -= fontSize.cy/4;

  // Make sure that an integral number of lines are visible
  int lineHeight = m_edit.GetLineHeight();
  client.bottom -= client.Height() % lineHeight;

  m_edit.MoveWindow(client,IsWindowVisible());
}

void SourceWindow::Draw(CDC& dc)
{
  CRect client;
  GetClientRect(client);
  CSize fontSize = theApp.MeasureFont(&(theApp.GetFont()));
  int fh = fontSize.cy/4;
  COLORREF back = theApp.GetColour(InformApp::ColourBack);

  m_arrowTop.SetRectEmpty();
  m_arrowBottom.SetRectEmpty();

  if (m_tearTop)
    m_arrowTop = PaintEdge(dc,0,client.Width(),m_imageTop,true);
  else
    dc.FillSolidRect(0,0,client.Width(),fh,back);

  int y = 0;
  if (m_tearBottom)
  {
    y = client.Height()-m_imageBottom->GetSize().cy;
    m_arrowBottom = PaintEdge(dc,y,client.Width(),m_imageBottom,false);
  }
  else
  {
    y = client.Height()-fh;
    dc.FillSolidRect(0,y,client.Width(),fh,back);
  }

  CRect editRect;
  m_edit.GetWindowRect(editRect);
  ScreenToClient(editRect);
  if (y > editRect.bottom)
    dc.FillSolidRect(0,editRect.bottom,client.Width(),y-editRect.bottom,back);
}

CRect SourceWindow::PaintEdge(CDC& dcPaint, int y, int w, CDibSection* image, bool top)
{
  // Create a bitmap to draw into
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);
  CDibSection bitmap;
  CSize sz = image->GetSize();
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),w,sz.cy) == FALSE)
    return CRect(0,0,0,0);
  bitmap.FillSolid(::GetSysColor(COLOR_BTNFACE));

  // Draw the torn edge
  int x = 0;
  while (x < w)
  {
    bitmap.AlphaBlend(image,x,0);
    x += sz.cx;
  }

  // Draw the arrow button
  CDibSection* btn = theApp.GetCachedImage("TearArrow");
  CRect btnRect(CPoint(w/2,sz.cy/2),CSize(0,0));
  CSize btnSize(btn->GetSize());
  btnRect.InflateRect(btnSize.cx/2,btnSize.cy/2);
  bitmap.AlphaBlend(btn,btnRect.left,btnRect.top,!top);

  // Copy the bitmap to the device context
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  dcPaint.BitBlt(0,y,w,sz.cy,&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);

  // Return a rectangle around the arrow button
  btnRect.InflateRect(btnSize.cx/2,btnSize.cy/2);
  btnRect.OffsetRect(0,y);
  return btnRect;
}
