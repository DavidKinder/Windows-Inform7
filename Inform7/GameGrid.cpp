#include "stdafx.h"
#include "Inform.h"
#include "GameGrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(GameGrid, CWnd)

BEGIN_MESSAGE_MAP(GameGrid, CWnd)
  ON_WM_CREATE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

GameGrid::GameGrid(MainWindow* main)
{
  ASSERT(main != NULL);

  m_main = main;
  m_rows = 0;
  m_columns = 0;

  m_x = 0;
  m_y = 0;

  m_charWidth = 0;
  m_charHeight = 0;
  m_topMargin = 0;

  Create(NULL,"",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),main->GetWnd(),0);
}

bool GameGrid::IsKindOf(const CRuntimeClass* rclass) const
{
  return (CWnd::IsKindOf(rclass) != FALSE);
}

BOOL GameGrid::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

HWND GameGrid::GetSafeHwnd(void) const
{
  return CWnd::GetSafeHwnd();
}

void GameGrid::Layout(const CRect& r)
{
  ResizeGrid(r.Height()/m_charHeight,r.Width()/m_charWidth);
  if (r.top == 0)
  {
    m_topMargin = r.Height() % m_charHeight;
    if (m_topMargin > m_charHeight/4)
      m_topMargin = m_charHeight/4;
  }
  m_main->DeferMoveWindow(GetSafeHwnd(),r);
}

void GameGrid::GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r)
{
  w = size * m_charWidth;
  h = size * m_charHeight;
  if ((r.top == 0) && (size > 0))
    h += m_charHeight/4;
}

void GameGrid::AddText(const CStringW& text, bool fromSkein)
{
  if (m_y < m_rows)
  {
    // Set the new text
    for (int i = 0; i < text.GetLength(); i++)
    {
      if (text[i] == '\n')
      {
        m_x = 0;
        if (m_y < m_rows)
          m_y++;
      }
      else if (m_x < m_columns)
      {
        m_grid[m_y].SetChar(m_x,text[i]);
        m_grid[m_y].SetAttributes(m_x,m_current);
        m_x++;
      }
    }
  }
  Invalidate();
}

void GameGrid::ClearText(bool styles, bool reverse)
{
  int rows = m_rows;
  int cols = m_columns;
  ResizeGrid(0,0);
  ResizeGrid(rows,cols);

  if (reverse)
  {
    GridInfo info;
    info.reverse = true;

    for (int i = 0; i < m_rows; i++)
    {
      for (int j = 0; j < m_grid[i].GetLength(); j++)
        m_grid[i].SetAttributes(j,info);
    }
  }

  m_x = 0;
  m_y = 0;

  Invalidate();
}

void GameGrid::SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size)
{
  m_current.bold = bold;
  m_current.italic = italic;
  m_current.reverse = reverse;
}

void GameGrid::SetColours(COLORREF fore, COLORREF back)
{
  m_current.fore = fore;
  m_current.back = back;
}

void GameGrid::SetCursor(int x, int y)
{
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  m_x = x;
  m_y = y;
}

void GameGrid::MoveToEnd(void)
{
}

void GameGrid::Draw(CDibSection* image, int val1, int val2, int width, int height)
{
}

COLORREF GameGrid::GetAlphaColour(void)
{
  return -1;
}

void GameGrid::SetLink(int link)
{
  m_current.link = link;
}

void GameGrid::SetParagraph(Justify justify)
{
}

void GameGrid::SetBackColour(COLORREF colour)
{
}

int GameGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  m_font.CreatePointFont(10*theApp.GetFontPointSize(),theApp.GetFixedFontName());
  SetFont(&m_font);

  // Get the with and height of the font
  CSize fontSize = theApp.MeasureFont(&m_font);
  m_charWidth = fontSize.cx;
  m_charHeight = fontSize.cy;

  ClearText(false,false);
  return 0;
}

void GameGrid::OnPaint()
{
  CPaintDC dc(this);

  CRect client;
  GetClientRect(client);

  // Create a memory device context
  CDC dcMem;
  dcMem.CreateCompatibleDC(&dc);

  // Create a memory bitmap
  CBitmap bitmap;
  if (bitmap.CreateCompatibleBitmap(&dc,client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = dcMem.SelectObject(&bitmap);
  CFont* oldFont = dcMem.GetCurrentFont();

  // Clear the background
  dcMem.FillSolidRect(client,theApp.GetColour(InformApp::ColourBack));

  if (m_columns > 0)
  {
    // Draw each line of text
    for (int i = 0; i < m_rows; i++)
    {
      int len = m_grid[i].GetLength();
      LPCWSTR str = m_grid[i].GetString();

      GridInfo attrs = m_grid[i].GetAttributes(0);
      bool reversed = attrs.reverse;
      int r1 = 0, r2 = 1;
      while (r2 <= len)
      {
        // End of line or attribute change?
        if ((r2 == len) || (m_grid[i].GetAttributes(r2) != attrs))
        {
          // Draw the section of line
          SelectFont(dcMem,i,r1);
          ::ExtTextOutW(dcMem,m_charWidth*r1,m_topMargin+(m_charHeight*i),
            ETO_OPAQUE,NULL,str+r1,r2-r1,NULL);

          // If whole line reversed, print over any remaining space
          if ((r2 == len) && reversed)
          {
            dcMem.ExtTextOut(m_charWidth*r2,m_topMargin+(m_charHeight*i),
              ETO_OPAQUE,NULL,"    ",4,NULL);

            // Fill in the top margin too
            if (i == 0)
              dcMem.FillSolidRect(0,0,client.Width(),m_topMargin,dcMem.GetBkColor());
          }

          // Start a new line section
          r1 = r2;
          r2 = r1+1;
          if (r1 < len)
          {
            attrs = m_grid[i].GetAttributes(r1);
            reversed &= attrs.reverse;
          }
        }
        else
          r2++;
      }
    }
  }

  // Draw the memory bitmap on the window's device context
  dc.BitBlt(0,0,client.Width(),client.Height(),&dcMem,0,0,SRCCOPY);

  // Restore the original device context settings
  dcMem.SelectObject(oldFont);
  dcMem.SelectObject(oldBitmap);
}

void GameGrid::OnLButtonDown(UINT nFlags, CPoint point) 
{
  int x = point.x/m_charWidth;
  int y = (point.y-m_topMargin)/m_charHeight;
  if (y < 0)
    y = 0;
  m_main->GameMouseEvent(this,x,y);

  int link = LinkAtPoint(point);
  if (link != 0)
    m_main->GameLinkEvent(this,link);

  CWnd::OnLButtonDown(nFlags,point);
}

BOOL GameGrid::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  if (theApp.IsWaitCursor())
  {
    theApp.RestoreWaitCursor();
    return TRUE;
  }
  if (nHitTest == HTCLIENT)
  {
    CPoint p(GetCurrentMessage()->pt);
    ScreenToClient(&p);

    if (LinkAtPoint(p) != 0)
    {
      HCURSOR hand = ::LoadCursor(0,IDC_HAND);
      if (hand != 0)
      {
        ::SetCursor(hand);
        return TRUE;
      }
    }
  }
  return CWnd::OnSetCursor(pWnd,nHitTest,message);
}

void GameGrid::SelectFont(CDC& dc, int row, int column)
{
  // Discard any previous font
  dc.SelectObject(&m_font);
  m_currentFont.DeleteObject();

  // Get the attributes of the grid character
  const GridInfo& attributes = m_grid[row].GetAttributes(column);

  // If there are font attributes set, create a suitable font
  if (attributes.bold || attributes.italic)
  {
    LOGFONT fontInfo;
    m_font.GetLogFont(&fontInfo);
    if (attributes.bold)
      fontInfo.lfWeight = FW_BOLD;
    if (attributes.italic)
      fontInfo.lfItalic = TRUE;
    m_currentFont.CreateFontIndirect(&fontInfo);
    dc.SelectObject(&m_currentFont);
  }
  else if (attributes.link != 0)
  {
    LOGFONT fontInfo;
    m_font.GetLogFont(&fontInfo);
    fontInfo.lfUnderline = TRUE;
    m_currentFont.CreateFontIndirect(&fontInfo);
    dc.SelectObject(&m_currentFont);
  }
  else
    dc.SelectObject(&m_font);

  // Set colours
  if (attributes.reverse)
  {
    dc.SetTextColor(attributes.back);
    dc.SetBkColor(attributes.fore);
  }
  else
  {
    dc.SetTextColor(attributes.fore);
    dc.SetBkColor(attributes.back);
  }
  if (attributes.link != 0)
    dc.SetTextColor(theApp.GetColour(InformApp::ColourHyperlink));
}

void GameGrid::ResizeGrid(int rows, int columns)
{
  if (rows < 0)
    rows = 0;
  if (columns < 0)
    columns = 0;

  m_rows = rows;
  m_columns = columns;

  m_grid.SetSize(m_rows);
  for (int i = 0; i < m_rows; i++)
  {
    // If the row is longer, pad with spaces
    m_grid[i].SetLength(m_columns,L' ');
  }
}

int GameGrid::LinkAtPoint(const CPoint& p)
{
  int x = p.x/m_charWidth;
  if ((x < 0) || (x >= m_columns))
    return 0;

  int y = (p.y-m_topMargin)/m_charHeight;
  if ((y < 0) || (y >= m_rows))
    return 0;

  return m_grid[y].GetAttributes(x).link;
}

GameGrid::GridInfo::GridInfo()
{
  bold = false;
  italic = false;
  reverse = false;
  link = 0;

  fore = theApp.GetColour(InformApp::ColourText);
  back = theApp.GetColour(InformApp::ColourBack);
}

bool GameGrid::GridInfo::operator!=(const GridInfo& info) const
{
  if (bold != info.bold)
    return true;
  if (italic != info.italic)
    return true;
  if (reverse != info.reverse)
    return true;
  if (link != info.link)
    return true;
  if (fore != info.fore)
    return true;
  if (back != info.back)
    return true;
  return false;
}

int GameGrid::GridString::GetLength(void) const
{
  return m_string.GetLength();
}

LPCWSTR GameGrid::GridString::GetString(void) const
{
  return (LPCWSTR)m_string;
}

const GameGrid::GridInfo& GameGrid::GridString::GetAttributes(int index) const
{
  return m_attributes[index];
}

void GameGrid::GridString::SetLength(int length, wchar_t character)
{
  int currentLength = GetLength();

  LPWSTR buffer = m_string.GetBufferSetLength(length);
  for (int i = currentLength; i < length; i++)
    buffer[i] = character;
  m_string.ReleaseBuffer(length);

  m_attributes.SetSize(length);
}

void GameGrid::GridString::SetChar(int index, wchar_t character)
{
  m_string.SetAt(index,character);
}

void GameGrid::GridString::SetAttributes(int index, const GridInfo& attributes)
{
  m_attributes[index] = attributes;
}
