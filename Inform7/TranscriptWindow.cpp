#include "stdafx.h"
#include "TranscriptWindow.h"
#include "Inform.h"
#include "OSLayer.h"
#include "Messages.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TranscriptWindow, CView)

BEGIN_MESSAGE_MAP(TranscriptWindow, CView)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_ERASEBKGND()
  ON_WM_VSCROLL()
  ON_WM_MOUSEACTIVATE()
  ON_WM_MOUSEWHEEL()
  ON_WM_MOUSEMOVE()
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()

  ON_MESSAGE(WM_SETEXPECTED, OnSetExpected)
END_MESSAGE_MAP()

TranscriptWindow::TranscriptWindow() : m_mouseOver(false)
{
  m_skein = NULL;
  m_skeinPlayed = NULL;
  m_skeinSelected = NULL;
  m_skeinEndThread = NULL;
}

BOOL TranscriptWindow::PreCreateWindow(CREATESTRUCT& cs)
{
  // Show the vertical scrollbar
  cs.style |= WS_VSCROLL;
  return CView::PreCreateWindow(cs);
}

void TranscriptWindow::PostNcDestroy()
{
  // Do nothing
}

int TranscriptWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CView::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Create the font for buttons
  m_btnFont.CreatePointFont(theApp.GetDialogFontSize(),theApp.GetFontName());

  // Turn the vertical scrollbar off
  EnableScrollBar(SB_VERT,ESB_DISABLE_BOTH);

  // Control for editing text
  if (m_edit.Create(ES_MULTILINE|WS_CHILD,this,0) == FALSE)
    return -1;
  m_edit.SetMargins(theApp.MeasureFont(m_edit.GetFont()).cx/4);

  return 0;
}

BOOL TranscriptWindow::OnEraseBkgnd(CDC* pDC)
{
  // Don't erase the background
  return TRUE;
}

void TranscriptWindow::OnDraw(CDC* pDC)
{
  CRect clientRect;
  GetClientRect(clientRect);

  CDC dc;
  dc.CreateCompatibleDC(pDC);

  // Create an off-screen bitmap for drawing
  CDibSection bitmap;
  if (bitmap.CreateBitmap(pDC->GetSafeHdc(),clientRect.Width(),clientRect.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);

  dc.SetBkMode(TRANSPARENT);
  dc.FillSolidRect(clientRect,theApp.GetColour(InformApp::ColourBack));
  CFont* oldFont = dc.SelectObject(&(theApp.GetFont()));
  CPen linePen(PS_SOLID,1,theApp.GetColour(InformApp::ColourBorder));
  CPen* oldPen = dc.SelectObject(&linePen);

  int y = 0;
  int yinput = m_layout.fontSize.cy+(2*m_layout.margin.cy);

  // If the transcript is taller than the window, work out the drawing origin
  if (GetHeight() > clientRect.Height())
  {
    SCROLLINFO scroll;
    ::ZeroMemory(&scroll,sizeof scroll);
    scroll.cbSize = sizeof scroll;
    GetScrollInfo(SB_VERT,&scroll);
    y -= scroll.nPos;
  }

  // Clear the map of visible buttons and expected texts
  m_buttons.clear();
  m_expecteds.clear();

  // Loop over the transcript nodes
  std::deque<NodeLayout>::const_iterator nodeIt;
  for (nodeIt = m_layout.nodes.begin(); nodeIt != m_layout.nodes.end(); ++nodeIt)
  {
    const NodeLayout& nl = *nodeIt;

    // Is this node visible?
    CRect nodeRect(0,y,1,y+m_layout.fontSize.cy+nl.height+(4*m_layout.margin.cy)+1);
    CRect intersection;
    if (intersection.IntersectRect(nodeRect,clientRect) == FALSE)
    {
      // Not visible, so increase the y position and try the next node
      y += yinput+nl.height+(2*m_layout.margin.cy);
      continue;
    }

    // Get the text associated with the node
    const CStringW& transcript = nl.node->GetTranscriptText();
    const CStringW& expected = nl.node->GetExpectedText();
    const CStringW& line = nl.node->GetLine();

    // Fill in the background of the input line
    dc.FillSolidRect(0,y+1,clientRect.Width(),yinput,
      theApp.GetColour(InformApp::ColourTransInput));

    // Fill in the background of the transcript text
    COLORREF back;
    if (transcript.IsEmpty())
      back = theApp.GetColour(InformApp::ColourTransUnset);
    else
    {
      back = theApp.GetColour(nl.node->GetChanged() ?
        InformApp::ColourTransDiffers : InformApp::ColourTransSame);
    }
    CRect backRect(0,y+yinput,m_layout.columnWidth,
      y+m_layout.fontSize.cy+nl.height+(4*m_layout.margin.cy));
    dc.FillSolidRect(backRect,back);

    // Fill in the background of the expected text
    if (expected.IsEmpty())
      back = theApp.GetColour(InformApp::ColourTransUnset);
    else
    {
      switch (nl.node->GetDiffers())
      {
      case Skein::Node::ExpectedSame:
        back = theApp.GetColour(InformApp::ColourTransSame);
        break;
      case Skein::Node::ExpectedNearlySame:
        back = RGB(255,255,127);
        break;
      case Skein::Node::ExpectedDifferent:
        back = theApp.GetColour(InformApp::ColourTransDiffers);
        break;
      default:
        ASSERT(FALSE);
        back = theApp.GetColour(InformApp::ColourTransDiffers);
        break;
      }
    }
    backRect.OffsetRect(m_layout.columnWidth+1,0);
    dc.FillSolidRect(backRect,Brighter(back));

    // If this is the first node in the transcript, draw a horizontal line at the top
    if (nl.node == m_skein->GetRoot())
    {
      dc.MoveTo(0,y);
      dc.LineTo(clientRect.Width(),y);
    }

    // Draw a horizontal line below the node's input
    dc.MoveTo(0,y+yinput);
    dc.LineTo(clientRect.Width(),y+yinput);

    // Draw a final horizontal line below the text boxes
    dc.MoveTo(0,y+yinput+nl.height+(2*m_layout.margin.cy));
    dc.LineTo(clientRect.Width(),y+yinput+nl.height+(2*m_layout.margin.cy));

    // Draw a dividing line between the two text boxes
    dc.MoveTo(m_layout.columnWidth,y+yinput);
    dc.LineTo(m_layout.columnWidth,y+yinput+nl.height+(2*m_layout.margin.cy));

    // If this is the last played knot in the skein, draw a yellow border around it
    if (nl.node == m_skeinPlayed)
    {
      CRect backRect(0,y,clientRect.Width(),
        y+m_layout.fontSize.cy+nl.height+(4*m_layout.margin.cy));
      DrawInsideRect(dc,backRect,CSize(m_layout.margin.cx*3/4,m_layout.margin.cy*7/8),
        theApp.GetColour(InformApp::ColourTransPlayed));
    }

    // If this is the knot selected in the skein, draw a blue border around it
    if (nl.node == m_skeinSelected)
    {
      CRect backRect(0,y,clientRect.Width(),
        y+m_layout.fontSize.cy+nl.height+(4*m_layout.margin.cy));
      DrawInsideRect(dc,backRect,CSize(m_layout.margin.cx*3/8,m_layout.margin.cy*7/16),
        theApp.GetColour(InformApp::ColourTransSelect));
    }

    // Draw the buttons at the end of the input line, and store their positions
    int btnHeight = m_layout.fontSize.cy+m_layout.margin.cy;
    CRect btnRect(
      CPoint(clientRect.Width()-(m_layout.margin.cy/2),y+(m_layout.margin.cy/2)),
      CSize(0,btnHeight));
    btnRect = DrawButton(dc,btnRect,false,"Show knot",Button(nl.node,ButtonShow),true);
    btnRect.right = btnRect.left-(m_layout.margin.cy/2);
    DrawButton(dc,btnRect,false,"Play to here",Button(nl.node,ButtonPlay),true);

    // Write the node's input
    dc.SetTextColor(theApp.GetColour(InformApp::ColourText));
    CRect textRect(m_layout.margin.cx,y+m_layout.margin.cy,
      btnRect.left-m_layout.margin.cx,y+yinput);
    theOS.DrawText(&dc,line,line.GetLength(),textRect,DT_SINGLELINE|DT_NOPREFIX|DT_END_ELLIPSIS);

    // Draw the 'bless' button, and store its position
    y += yinput;
    btnRect.right = m_layout.columnWidth;
    btnRect.top = y+(((nl.height+(2*m_layout.margin.cy))-btnHeight)/2);
    btnRect.bottom = btnRect.top+btnHeight;
    DrawButton(dc,btnRect,true,"Bless",Button(nl.node,ButtonBless),nl.node->CanBless());

    // Write the text in the text boxes
    textRect = CRect(m_layout.margin.cx,y+m_layout.margin.cy,
      m_layout.columnWidth-m_layout.centreMargin,y+m_layout.margin.cy+nl.height);
    DrawText(dc,textRect,transcript,nl.node->GetTranscriptDiffs());
    textRect.OffsetRect(m_layout.columnWidth+m_layout.centreMargin-m_layout.margin.cx,0);
    DrawText(dc,textRect,expected,nl.node->GetExpectedDiffs());
    textRect.InflateRect(theApp.MeasureFont(m_edit.GetFont()).cx/4,0);
    m_expecteds.push_back(std::make_pair(backRect,Expected(nl.node,textRect)));

    // Advance the y position
    y += nl.height+(2*m_layout.margin.cy);
  }

  dc.SelectObject(oldPen);
  dc.SelectObject(oldFont);

  // If the edit window is visible, exclude the area under it to reduce flicker
  if (m_edit.IsWindowVisible())
  {
    CRect editRect;
    m_edit.GetWindowRect(&editRect);
    ScreenToClient(&editRect);
    pDC->ExcludeClipRect(editRect);
  }

  // Copy to the on-screen device context
  pDC->BitBlt(0,0,clientRect.Width(),clientRect.Height(),&dc,0,0,SRCCOPY);
  dc.SelectObject(oldBitmap);
}

BOOL TranscriptWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CWnd* focusWnd = GetFocus();
  if (focusWnd == &m_edit)
  {
    if (m_edit.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }
  return CView::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TranscriptWindow::OnSize(UINT nType, int cx, int cy)
{
  CView::OnSize(nType,cx,cy);

  if (m_skein != NULL)
    Layout();
}

void TranscriptWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  if (pScrollBar == NULL)
  {
    if ((nSBCode == SB_THUMBPOSITION) || (nSBCode == SB_THUMBTRACK))
    {
      // Get the current scrollbar position
      SCROLLINFO scroll;
      ::ZeroMemory(&scroll,sizeof scroll);
      scroll.cbSize = sizeof scroll;
      GetScrollInfo(SB_VERT,&scroll);
      nPos = scroll.nTrackPos;
    }
    Scroll(nSBCode,nPos,1);
  }
  CView::OnVScroll(nSBCode,nPos,pScrollBar);
}

int TranscriptWindow::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
  // If the edit window is visible and the user clicks outside it, end editing
  if (m_edit.IsWindowVisible())
  {
    CPoint point(::GetMessagePos());
    ScreenToClient(&point);

    if (ChildWindowFromPoint(point) != &m_edit)
      m_edit.EndEdit();
  }
  return CView::OnMouseActivate(pDesktopWnd,nHitTest,message);
}

BOOL TranscriptWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  // Only respond to the mouse wheel for scrolling
  if ((nFlags & (MK_SHIFT | MK_CONTROL)) == 0)
  {
    // Work out how many lines to scroll
    UINT scrollLines = 0;
    if (::SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&scrollLines,0) == 0)
      scrollLines = 3;

    int lines = ::MulDiv(-zDelta,scrollLines,WHEEL_DELTA);
    Scroll(SB_LINEDOWN,0,lines);
  }
  return CView::OnMouseWheel(nFlags,zDelta,pt);
}

void TranscriptWindow::OnMouseMove(UINT nFlags, CPoint point)
{
  // Redraw the window so that buttons are updated
  Invalidate();

  // Listen for the mouse leaving this window
  if (m_mouseOver == false)
  {
    m_mouseOver = true;

    TRACKMOUSEEVENT tme;
    ::ZeroMemory(&tme,sizeof tme);
    tme.cbSize = sizeof tme;
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = GetSafeHwnd();
    ::_TrackMouseEvent(&tme);
  }

  CView::OnMouseMove(nFlags,point);
}

LRESULT TranscriptWindow::OnMouseLeave(WPARAM, LPARAM)
{
  // Redraw the window to show buttons correctly
  Invalidate();

  m_mouseOver = false;
  return 0;
}

void TranscriptWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  std::vector<std::pair<CRect,Expected> >::const_iterator it;
  for (it = m_expecteds.begin(); it != m_expecteds.end(); ++it)
  {
    if (it->first.PtInRect(point))
    {
      CPoint scrPoint(point);
      ClientToScreen(&scrPoint);

      m_edit.StartEdit(it->second.node,it->second.editRect,scrPoint);
      break;
    }
  }
  CView::OnLButtonDblClk(nFlags,point);
}

void TranscriptWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  std::vector<std::pair<CRect,Button> >::const_iterator it;
  for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    // If the mouse is over a button, select it
    if (it->first.PtInRect(point))
    {
      // Capture mouse input while the button is down
      SetCapture();

      // Redraw the button
      m_buttonDown = it->second;
      Invalidate();
      break;
    }
  }
  CView::OnLButtonDown(nFlags,point);
}

void TranscriptWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  std::vector<std::pair<CRect,Button> >::const_iterator it;
  for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    // If the mouse is over a button ...
    if (it->first.PtInRect(point))
    {
      // ...and it matches the previously pressed button ...
      if (it->second == m_buttonDown)
      {
        // ... then perform the action
        switch (m_buttonDown.action)
        {
        case ButtonBless:
          m_skein->Bless(m_buttonDown.node,false);
          m_skein->Lock(m_buttonDown.node);
          break;
        case ButtonPlay:
          GetParentFrame()->SendMessage(WM_PLAYSKEIN,(WPARAM)m_buttonDown.node);
          break;
        case ButtonShow:
          GetParentFrame()->SendMessage(WM_SHOWSKEIN,(WPARAM)m_buttonDown.node,
            (LPARAM)GetSafeHwnd());
          break;
        }
      }
      break;
    }
  }

  // Release the mouse
  ReleaseCapture();

  // Remove any button selection
  m_buttonDown = Button();
  Invalidate();

  CView::OnLButtonUp(nFlags,point);
}

LRESULT TranscriptWindow::OnSetExpected(WPARAM node, LPARAM text)
{
  Skein::Node* theNode = (Skein::Node*)node;
  if (m_skein->IsValidNode(theNode) == false)
    return 0;

  m_skein->SetExpectedText(theNode,(LPWSTR)text);
  return 0;
}

void TranscriptWindow::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_skein->AddListener(this);

  GetSkeinNodes();
  Layout();
}

void TranscriptWindow::GetSkeinNodes(void)
{
  bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
  m_skeinPlayed = gameRunning ? m_skein->GetPlayed() : m_skein->GetCurrent();
  m_skeinSelected = NULL;
  m_skeinEndThread = m_skein->GetThreadBottom(m_skein->GetCurrent());
}

void TranscriptWindow::Layout(void)
{
  CRect clientRect;
  GetClientRect(clientRect);

  m_layout.clientSize = clientRect.Size();
  m_layout.columnWidth = clientRect.Width()/2;

  m_layout.font = &(theApp.GetFont());
  m_layout.fontSize = theApp.MeasureFont(m_layout.font);
  m_layout.margin = CSize(m_layout.fontSize.cx,m_layout.fontSize.cy/3);
  m_layout.centreMargin = m_layout.margin.cx*4;

  CDC* dc = GetDC();

  m_layout.nodes.clear();
  Skein::Node* node = m_skeinEndThread;
  while (node != NULL)
  {
    NodeLayout nl;
    nl.node = node;

    // Get the text associated with the node
    const CStringW& transcript = nl.node->GetTranscriptText();
    const CStringW& expected = nl.node->GetExpectedText();

    // Measure the height of the expected text
    CRect textRect;
    textRect.SetRectEmpty();
    textRect.right = m_layout.columnWidth-m_layout.margin.cx-m_layout.centreMargin;
    SizeText(*dc,textRect,expected);
    nl.height = textRect.Height();

    // Measure the height of the transcript text
    textRect.SetRectEmpty();
    textRect.right = m_layout.columnWidth-m_layout.margin.cx-m_layout.centreMargin;
    SizeText(*dc,textRect,transcript);
    if (textRect.Height() > nl.height)
      nl.height = textRect.Height();

    // Use the tallest for the height of the node in the transcript
    if (m_layout.fontSize.cy > nl.height)
      nl.height = m_layout.fontSize.cy;

    m_layout.nodes.push_front(nl);
    node = node->GetParent();
  }

  ReleaseDC(dc);

  // Compare the height of the transcript with the height of the window
  int height = GetHeight();
  if (height > clientRect.Height())
  {
    // The transcript is taller than the window, so turn the scrollbar on
    EnableScrollBar(SB_VERT,ESB_ENABLE_BOTH);

    // Get the current scrollbar settings
    SCROLLINFO scroll;
    ::ZeroMemory(&scroll,sizeof scroll);
    scroll.cbSize = sizeof scroll;
    GetScrollInfo(SB_VERT,&scroll);

    // Change the maximum position and the size of the scrollbar
    scroll.nMin = 0;
    scroll.nMax = height-1;
    scroll.nPage = clientRect.Height();
    SetScrollInfo(SB_VERT,&scroll);
  }
  else
  {
    // The transcript is shorter than the window, so turn the scrollbar off
    EnableScrollBar(SB_VERT,ESB_DISABLE_BOTH);

    SCROLLINFO scroll;
    ::ZeroMemory(&scroll,sizeof scroll);
    scroll.cbSize = sizeof scroll;
    SetScrollInfo(SB_VERT,&scroll);
  }
}

void TranscriptWindow::BlessAll(void)
{
  m_skein->Bless(m_skeinEndThread,true);
  m_skein->Lock(m_skeinEndThread);
}

void TranscriptWindow::SkeinChanged(Skein::Change change)
{
  if (GetSafeHwnd() == 0)
    return;

  switch (change)
  {
  case Skein::TreeChanged:
  case Skein::ThreadChanged:
    GetSkeinNodes();
    Layout();
    Invalidate();
    break;
  case Skein::NodeColourChanged:
    Layout();
    Invalidate();
    break;
  case Skein::NodeTextChanged:
    Invalidate();
    break;
  case Skein::LockChanged:
  case Skein::TranscriptThreadChanged:
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

void TranscriptWindow::SkeinEdited(bool edited)
{
}

void TranscriptWindow::SkeinShowNode(Skein::Node* node, Skein::Show why)
{
  if (GetSafeHwnd() == 0)
    return;

  switch (why)
  {
  case Skein::ShowSelect:
    m_skeinSelected = node; // Intentional fall-through
  case Skein::JustShow:
    if (ScrollToNode(node) == false)
    {
      m_skeinEndThread = m_skein->GetThreadBottom(node);
      Layout();
      ScrollToNode(node);
    }
    Invalidate();
    break;
  case Skein::ShowNewLine:
    break;
  case Skein::ShowNewTranscript:
    ScrollToNode(node);
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

Skein::Node* TranscriptWindow::GetEndNode(void)
{
  return m_skeinEndThread;
}

Skein::Node* TranscriptWindow::FindRelevantNode(FindAction action, bool next, bool selected)
{
  // Get all the nodes in the transcript or skein
  CArray<Skein::Node*,Skein::Node*> nodes;
  if (action == SkeinDifferent)
  {
    ASSERT(next);
    m_skein->GetAllNodes(nodes);
  }
  else
  {
    Skein::Node* node = m_skeinEndThread;
    while (node != NULL)
    {
      if (next)
        nodes.InsertAt(0,node);
      else
        nodes.Add(node);
      node = node->GetParent();
    }
  }
  if (next)
    nodes.InsertAt(0,(Skein::Node*)NULL);

  // Return the next relevant node after the selected node (which may be NULL)
  bool afterSelected = false;
  for (int i = 0; i < nodes.GetSize(); i++)
  {
    if (afterSelected == false)
    {
      if (nodes.GetAt(i) == (selected ? m_skeinSelected : NULL))
        afterSelected = true;
    }
    else
    {
      switch (action)
      {
      case TranscriptDifferent:
      case SkeinDifferent:
        if (nodes.GetAt(i)->GetDiffers() != Skein::Node::ExpectedSame)
          return nodes.GetAt(i);
        break;
      case TranscriptChanged:
        if (nodes.GetAt(i)->GetChanged())
          return nodes.GetAt(i);
        break;
      default:
        ASSERT(FALSE);
        break;
      }
    }
  }

  // If nothing found, then if searching the whole skein, start from the beginning
  if ((action == SkeinDifferent) && selected)
    return FindRelevantNode(action,next,false);
  return NULL;
}

bool TranscriptWindow::ScrollToNode(Skein::Node* node)
{
  int y = 0;
  std::deque<NodeLayout>::const_iterator nodeIt;
  for (nodeIt = m_layout.nodes.begin(); nodeIt != m_layout.nodes.end(); ++nodeIt)
  {
    if (nodeIt->node == node)
    {
      // Having found the node, scroll the transcript so that the node is at the top
      Scroll(SB_THUMBPOSITION,y,0);
      return true;
    }

    y += m_layout.fontSize.cy+(2*m_layout.margin.cy);
    y += nodeIt->height+(2*m_layout.margin.cy);
  }
  return false;
}

void TranscriptWindow::Scroll(UINT code, int pos, int lines)
{
  CRect clientRect;
  GetClientRect(clientRect);

  // Only scroll if the transcript is taller than the window
  int height = GetHeight();
  if (height <= clientRect.Height())
    return;

  // Get the current scrollbar position
  SCROLLINFO scroll;
  ::ZeroMemory(&scroll,sizeof scroll);
  scroll.cbSize = sizeof scroll;
  GetScrollInfo(SB_VERT,&scroll);
  int y = scroll.nPos;
  int y1 = y;

  // Work out the new scrollbar position
  switch (code)
  {
  case SB_TOP:
    y = scroll.nMin;
    break;
  case SB_BOTTOM:
    y = scroll.nMax;
    break;
  case SB_PAGEUP:
    y -= clientRect.Height()*lines;
    break;
  case SB_PAGEDOWN:
    y += clientRect.Height()*lines;
    break;
  case SB_LINEUP:
    y -= m_layout.fontSize.cy*lines;
    break;
  case SB_LINEDOWN:
    y += m_layout.fontSize.cy*lines;
    break;
  case SB_THUMBPOSITION:
  case SB_THUMBTRACK:
    y = pos;
    break;
  }

  // Make sure that the new position is valid
  if (y < scroll.nMin)
    y = scroll.nMin;
  if (y > scroll.nMax)
    y = scroll.nMax;

  // Set the new position
  scroll.fMask = SIF_POS;
  scroll.nPos = y;
  SetScrollInfo(SB_VERT,&scroll);

  // Get it again, as the system may have not set the
  // scrollbar to the exact values 
  GetScrollInfo(SB_VERT,&scroll);

  // If the scrollbar position has changed from what it was
  // initially, scroll and redraw the transcript window
  if (scroll.nPos != y1)
  {
    Invalidate();
    UpdateWindow();
  }
}

int TranscriptWindow::GetHeight(void)
{
  int height = 0;
  std::deque<NodeLayout>::const_iterator nodeIt;
  for (nodeIt = m_layout.nodes.begin(); nodeIt != m_layout.nodes.end(); ++nodeIt)
  {
    height += m_layout.fontSize.cy+(2*m_layout.margin.cy);
    height += nodeIt->height+(2*m_layout.margin.cy);
  }
  return height;
}

static BYTE ToRGB1(double rm1, double rm2, double rh)
{
  if (rh > 360.0)
    rh -= 360.0;
  else if (rh < 0.0)
    rh += 360.0;
  if (rh < 60.0)
    rm1 = rm1+(rm2-rm1)*rh/60.0;
  else if (rh < 180.0)
    rm1 = rm2;
  else if (rh < 240.0)
    rm1 = rm1+(rm2-rm1)*(240.0-rh)/60.0;
  return (BYTE)(rm1*255.0);
}

COLORREF TranscriptWindow::Brighter(COLORREF colour)
{
  // Convert from RGB to HSV
  BYTE r = GetRValue(colour);
  BYTE g = GetGValue(colour);
  BYTE b = GetBValue(colour);
  BYTE minval = min(r,min(g,b));
  BYTE maxval = max(r,max(g,b));
  double mdiff = maxval-minval;
  double msum = maxval+minval;
  double luminance = msum/510.0;
  double saturation = 0.0;
  double hue = 0.0;
  if (maxval != minval)
  {
    double rnorm = (maxval-r)/mdiff;      
    double gnorm = (maxval-g)/mdiff;
    double bnorm = (maxval-b)/mdiff;   
    saturation = (luminance <= 0.5) ? (mdiff/msum) : (mdiff/(510.0-msum));
    if (maxval == r)
      hue = 60.0*(6.0+bnorm-gnorm);
    if (maxval == g)
      hue = 60.0*(2.0+rnorm-bnorm);
    if (maxval == b)
      hue = 60.0*(4.0+gnorm-rnorm);
    if (hue > 360.0)
      hue = hue-360.0;
  }

  // Decrease the luminance
  luminance *= 0.95;

  // Convert back to RGB
  if (saturation == 0.0)
    r = g = b = (BYTE)(luminance*255.0);
  else
  {
    double rm1, rm2;
    if (luminance <= 0.5)
      rm2 = luminance+luminance*saturation;  
    else
      rm2 = luminance+saturation-luminance*saturation;
    rm1 = 2.0*luminance-rm2;   
    r = ToRGB1(rm1,rm2,hue+120.0);   
    g = ToRGB1(rm1,rm2,hue);
    b = ToRGB1(rm1,rm2,hue-120.0);
  }
  return RGB(r,g,b);
}

CRect TranscriptWindow::DrawButton(
  CDC& dc, CRect& rect, bool centre, const char* text, Button button, bool enable)
{
  // Select the font and measure the text in it
  CFont* oldFont = dc.SelectObject(&m_btnFont);
  CSize size = dc.GetTextExtent(text);

  // Adjust the width of the button
  if (centre)
  {
    rect.left = rect.right-(size.cx/2)-m_layout.margin.cx;
    rect.right = rect.left+size.cx+m_layout.margin.cx*2;
  }
  else
    rect.left = rect.right-size.cx-m_layout.margin.cx*2;

  // Determine the button's state
  CPoint mousePoint = GetCurrentMessage()->pt;
  ScreenToClient(&mousePoint);
  bool over = (rect.PtInRect(mousePoint) != 0);
  bool down = (m_buttonDown == button);

  if (theOS.IsAppThemed())
  {
    // Open the button theme
    HTHEME theme = theOS.OpenThemeData(this,L"Button");
    if (theme)
    {
      UINT state = PBS_NORMAL;
      if (!enable)
        state = PBS_DISABLED;
      else if (over && down)
        state = PBS_PRESSED;
      else if (down)
        state = PBS_HOT;
      else if (over && (m_buttonDown == Button()))
        state = PBS_HOT;

      // Draw the themed control frame
      theOS.DrawThemeBackground(theme,&dc,BP_PUSHBUTTON,state,rect);

      // Get the background size
      CRect backRect(rect);
      theOS.GetThemeBackgroundContentRect(theme,&dc,BP_PUSHBUTTON,state,backRect);

      // Draw the themed button text
      CStringW textW(text);
      theOS.DrawThemeText(theme,&dc,BP_PUSHBUTTON,state,textW,
        DT_CENTER|DT_VCENTER|DT_SINGLELINE,DTT_GRAYED,backRect);

      // Close the button theme
      theOS.CloseThemeData(theme);
    }
  }
  else
  {
    UINT state = DFCS_BUTTONPUSH;
    if (!enable)
      state |= DFCS_INACTIVE;
    else if (over && down)
      state |= DFCS_PUSHED;

    // Draw the control background
    dc.DrawFrameControl(rect,DFC_BUTTON,state);

    // Work out the bounding rectangle for the button text
    CRect textRect(rect);
    textRect.DeflateRect((rect.Width()-size.cx)/2,(rect.Height()-size.cy)/2);
    if (state & DFCS_PUSHED)
    {
      textRect.top++;
      textRect.left++;
    }

    // Draw the button text
    dc.DrawState(textRect.TopLeft(),textRect.Size(),text,
      (state & DFCS_INACTIVE) ? DSS_DISABLED : DSS_NORMAL,TRUE,0,(HBRUSH)0);
  }

  // Select the previous font
  dc.SelectObject(oldFont);

  // Store the button rectangle
  if (enable)
    m_buttons.push_back(std::make_pair(rect,button));

  // Return the button rectangle
  return rect;
}

void TranscriptWindow::SizeText(CDC& dc, CRect& rect, const CStringW& text)
{
  // Replace the text
  m_draw.SetText(text);

  // Work out the height of the text
  m_draw.SizeText(dc,rect);
}

void TranscriptWindow::DrawText(
  CDC& dc, CRect& rect, const CStringW& text, const Diff::DiffResults& diffs)
{
  // Replace the text
  m_draw.SetText(text);

  // Get a range for the whole of the text in the control
  CComPtr<ITextRange> range;
  m_draw.Range(0,0,&range);
  range->MoveEnd(tomStory,1,NULL);

  // Set the up the default font with the correct colour and no underlining
  CComPtr<ITextFont> font;
  range->GetFont(&font);
  font->SetForeColor(theApp.GetColour(InformApp::ColourText));
  font->SetUnderline(tomFalse);

  // Set up a font for the underlining
  CComPtr<ITextFont> underFont;
  font->GetDuplicate(&underFont);
  underFont->SetUnderline(tomSingle);

  // Underline differences
  Diff::DiffResults::const_iterator it;
  for (it = diffs.begin(); it != diffs.end(); ++it)
  {
    CComPtr<ITextRange> range;
    m_draw.Range(it->start,it->start+it->length,&range);
    range->SetFont(underFont);
  }

  // Draw the result
  m_draw.DrawText(dc,rect);
}

void TranscriptWindow::DrawInsideRect(CDC& dc, const CRect& rect, const CSize& size, COLORREF colour)
{
  dc.FillSolidRect(rect.left,rect.top+1,rect.Width(),size.cy,colour);
  dc.FillSolidRect(rect.left,rect.top+1,size.cx,rect.Height()-1,colour);
  dc.FillSolidRect(rect.right-size.cx,rect.top+1,size.cx,rect.Height()-1,colour);
  dc.FillSolidRect(rect.left,rect.bottom-size.cy,rect.Width(),size.cy,colour);
}
