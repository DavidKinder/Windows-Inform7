#include "stdafx.h"
#include "SourceWindow.h"
#include "Inform.h"

#include "Dib.h"

#include "Platform.h"
#include "Scintilla.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SourceWindow, CWnd)

BEGIN_MESSAGE_MAP(SourceWindow, DrawScrollWindow)
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
  m_windowType = NoBorder;
  m_back = 0;
  m_tearTop = false;
  m_tearBottom = false;
  m_arrowTop.SetRectEmpty();
  m_arrowBottom.SetRectEmpty();
  m_imageTop = NULL;
  m_imageBottom = NULL;
}

void SourceWindow::Create(CWnd* parent, ProjectType projectType, WindowType windowType)
{
  // Get the background colour
  switch (projectType)
  {
  case Project_I7:
    m_back = theApp.GetColour(InformApp::ColourBack);
    break;
  case Project_I7XP:
    m_back = theApp.GetColour(InformApp::ColourI7XP);
    break;
  default:
    ASSERT(0);
    break;
  }

  m_windowType = windowType;

  DWORD style = WS_CHILD|WS_CLIPCHILDREN|WS_VSCROLL;
  if (m_windowType == SingleLine)
    style |= WS_BORDER;
  DrawScrollWindow::Create(style,parent,0);

  // Create the edit control and make this window in charge of the scroll bar
  m_edit.Create(this,1,m_back,(projectType == Project_I7XP));
}

SourceEdit& SourceWindow::GetEdit(void)
{
  return m_edit;
}

void SourceWindow::LoadSettings(SourceSettings& set)
{
  DarkMode* dark = DarkMode::GetActive(this);
  m_back = dark ? dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack);
  ProjectType projectType = (ProjectType)GetParentFrame()->SendMessage(WM_PROJECTTYPE);

  DWORD enabled = 1;
  set.GetDWord("Syntax Colouring",enabled);
  if (enabled)
  {
    DWORD colour = 0;
    switch (projectType)
    {
    case Project_I7:
      if (set.GetDWord("Source Paper Colour",colour))
        m_back = (COLORREF)colour;
      else
        m_back = theApp.GetColour(InformApp::ColourBack);
      break;
    case Project_I7XP:
      if (set.GetDWord("Ext Paper Colour",colour))
        m_back = (COLORREF)colour;
      else
        m_back = theApp.GetColour(InformApp::ColourI7XP);
      break;
    }
  }

  m_edit.LoadSettings(set,m_back);
  GetImages(projectType);
}

void SourceWindow::PrefsChanged(void)
{
  m_edit.PrefsChanged();
  Resize();
  Invalidate();
}

void SourceWindow::SetDarkMode(DarkMode* dark)
{
  m_edit.SetDarkMode(dark);
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

void SourceWindow::GetTears(bool& top, bool& bottom)
{
  top = m_tearTop;
  bottom = m_tearBottom;
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

  m_edit.Select(range,centre);
}

int SourceWindow::GetCurrentLine(void)
{
  CHARRANGE lines = m_edit.GetRangeLines(m_edit.GetSelect());
  return lines.cpMin;
}

BOOL SourceWindow::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void SourceWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  DrawScrollWindow::OnLButtonDown(nFlags,point);

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
  DrawScrollWindow::OnSize(nType,cx,cy);
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
  ssi->nPos = SetScrollInfo(ssi->nBar,(LPCSCROLLINFO)ssi->lpsi,ssi->bRedraw);
  *result = 1;
}

// Handle scroll bar call from the edit control
void SourceWindow::OnEditGetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result)
{
  SCNXGetScrollInfo* gsi = (SCNXGetScrollInfo*)pNotifyStruct;
  *result = GetScrollInfo(gsi->nBar,(LPSCROLLINFO)gsi->lpsi) ? 1 : 0;
}

void SourceWindow::Resize(void)
{
  CRect client;
  GetClientRect(client);

  // Adjust for margins
  CSize fontSize = theApp.MeasureFont(this,theApp.GetFont(this,InformApp::FontDisplay));
  if (m_tearTop)
    client.top += m_imageTop->GetSize().cy;
  else if (m_windowType == SingleLine)
    client.top += fontSize.cy/8;
  else
    client.top += fontSize.cy/4;
  if (m_tearBottom)
    client.bottom -= m_imageBottom->GetSize().cy;
  else if (m_windowType == SingleLine)
    client.bottom -= fontSize.cy/8;
  else
    client.bottom -= fontSize.cy/4;

  if (m_windowType == Border)
    client.left += 1;
  else if (m_windowType != SingleLine)
  {
    // Make sure that an integral number of lines are visible
    int lineHeight = m_edit.GetLineHeight();
    client.bottom -= client.Height() % lineHeight;
  }

  m_edit.MoveWindow(client,IsWindowVisible());
}

void SourceWindow::Draw(CDC& dc)
{
  CRect client;
  GetClientRect(client);
  CSize fontSize = theApp.MeasureFont(this,theApp.GetFont(this,InformApp::FontDisplay));
  int fh = fontSize.cy/4;
  DarkMode* dark = DarkMode::GetActive(this);

  m_arrowTop.SetRectEmpty();
  m_arrowBottom.SetRectEmpty();

  if (m_tearTop)
    m_arrowTop = PaintEdge(dc,0,client.Width(),m_imageTop,dark,true);
  else
    dc.FillSolidRect(0,0,client.Width(),fh,m_back);

  int y = 0;
  if (m_tearBottom)
  {
    y = client.Height()-m_imageBottom->GetSize().cy;
    m_arrowBottom = PaintEdge(dc,y,client.Width(),m_imageBottom,dark,false);
  }
  else
  {
    y = client.Height()-fh;
    dc.FillSolidRect(0,y,client.Width(),fh,m_back);
  }

  CRect editRect;
  m_edit.GetWindowRect(editRect);
  ScreenToClient(editRect);
  if (y > editRect.bottom)
    dc.FillSolidRect(0,editRect.bottom,client.Width(),y-editRect.bottom,m_back);

  if (m_windowType == Border)
  {
    COLORREF lineColour = dark ? dark->GetColour(DarkMode::Dark2) : ::GetSysColor(COLOR_BTNSHADOW);
    if (!dark)
    {
      // Get the colour for the lines around groups
      HTHEME theme = 0;
      if (::IsAppThemed())
      {
        HTHEME theme = ::OpenThemeData(GetSafeHwnd(),L"Button");
        if (theme != 0)
        {
          COLORREF themeColour = 0;
          if (SUCCEEDED(::GetThemeColor(theme,BP_GROUPBOX,GBS_NORMAL,TMT_EDGEFILLCOLOR,&themeColour)))
          {
            if (themeColour != 0)
              lineColour = themeColour;
          }
          ::CloseThemeData(theme);
        }
      }
    }

    CPen linePen(PS_SOLID,0,lineColour);
    dc.SelectObject(linePen);
    dc.MoveTo(client.Width()-1,0);
    dc.LineTo(0,0);
    dc.LineTo(0,client.Height()-1);
    dc.LineTo(client.Width()-1,client.Height()-1);
  }
}

CRect SourceWindow::PaintEdge(CDC& dcPaint, int y, int w, CDibSection* image, DarkMode* dark, bool top)
{
  // Create a bitmap to draw into
  CDC dc;
  dc.CreateCompatibleDC(&dcPaint);
  CDibSection bitmap;
  CSize sz = image->GetSize();
  if (bitmap.CreateBitmap(dc.GetSafeHdc(),w,sz.cy) == FALSE)
    return CRect(0,0,0,0);
  bitmap.FillSolid(dark ?
    dark->GetColour(DarkMode::Darkest) : ::GetSysColor(COLOR_BTNFACE));

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

void SourceWindow::GetImages(ProjectType projectType)
{
  CString suffix;
  if (projectType == Project_I7XP)
    suffix = "-i7xp";

  // Create the top and bottom margin images, if not already cached
  m_imageTop = theApp.GetCachedImage("Torn-top-blend"+suffix);
  if (m_imageTop == NULL)
    m_imageTop = CreateTornImage("Torn-top","Torn-top-blend"+suffix);
  m_imageBottom = theApp.GetCachedImage("Torn-bottom-blend"+suffix);
  if (m_imageBottom == NULL)
    m_imageBottom = CreateTornImage("Torn-bottom","Torn-bottom-blend"+suffix);
}

CDibSection* SourceWindow::CreateTornImage(const char* inputImage, const char* outputName)
{
  CDibSection* input = theApp.GetCachedImage(inputImage);
  CSize inputSize = input->GetSize();

  CDibSection* output = new CDibSection();
  CDC* dc = AfxGetMainWnd()->GetDC();
  BOOL created = output->CreateBitmap(dc->GetSafeHdc(),inputSize.cx,inputSize.cy);
  ASSERT(created);
  AfxGetMainWnd()->ReleaseDC(dc);

  int br = GetRValue(m_back);
  int bg = GetGValue(m_back);
  int bb = GetBValue(m_back);

  int r, g, b, a;
  DWORD src;
  for (int y = 0; y < inputSize.cy; y++)
  {
    for (int x = 0; x < inputSize.cx; x++)
    {
      src = input->GetPixel(x,y);
      b = src & 0xFF;
      src >>= 8;
      g = src & 0xFF;
      src >>= 8;
      r = src & 0xFF;
      src >>= 8;
      a = src & 0xFF;

      r = (r * br) >> 8;
      g = (g * bg) >> 8;
      b = (b * bb) >> 8;
      output->SetPixel(x,y,(a<<24)|(r<<16)|(g<<8)|b);
    }
  }

  theApp.CacheImage(outputName,output);
  return output;
}
