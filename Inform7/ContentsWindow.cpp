#include "stdafx.h"
#include "ContentsWindow.h"
#include "TabSource.h"
#include "Inform.h"
#include "OSLayer.h"

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ContentsPane, CScrollView)

BEGIN_MESSAGE_MAP(ContentsPane, CScrollView)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_WM_VSCROLL()
  ON_MESSAGE(WM_PRINT, OnPrint)
END_MESSAGE_MAP()

ContentsPane::ContentsPane() : m_smallest(SourceLexer::Section)
{
}

void ContentsPane::SetHeadings(const CArray<SourceLexer::Heading>& headings, const SourceHeading& selected)
{
  // Use the new array of items
  m_items.RemoveAll();
  m_items.Copy(headings);

  // Rebuild the tree of items
  m_tree.reset(new Node(NULL,NULL,0,0));
  BuildTree(m_tree.get());

  // Mark the selected node from the previous selection, if any
  SetSelectedNode(selected);

  SetScrollSize();
}

void ContentsPane::UpdateSmallest(Level smallest)
{
  m_smallest = smallest;
  SetScrollSize();
  Invalidate();
}

int ContentsPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Set up the fonts
  LOGFONT fontInfo;
  ::ZeroMemory(&fontInfo,sizeof fontInfo);
  theApp.GetFont().GetLogFont(&fontInfo);
  m_font.CreateFontIndirect(&fontInfo);
  fontInfo.lfWeight = FW_BOLD;
  m_boldFont.CreateFontIndirect(&fontInfo);
  return 0;
}

BOOL ContentsPane::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void ContentsPane::OnLButtonUp(UINT nFlags, CPoint point)
{
  Node* node = NodeAtPoint(point);
  if ((node != NULL) && (node->item != NULL))
  {
    std::auto_ptr<SourceRange> sr(new SourceRange());
    sr->startLine = node->item->line;
    sr->endLine = 0;
    sr->full = ((nFlags & MK_SHIFT) != 0);

    // Get the heading names leading to this node
    Node* nameNode = node;
    while ((nameNode != NULL) && (nameNode->item != NULL))
    {
      if (nameNode->GetLevel() > SourceLexer::Title)
        sr->heading.Add(CStringW(nameNode->item->name));
      nameNode = nameNode->parent;
    }

    // Find the start of the next range from this node
    while (node->parent != NULL)
    {
      Node* nextNode = node->parent->GetNextChild(node);
      if (nextNode != NULL)
      {
        node = nextNode;
        break;
      }
      node = node->parent;
    }
    if ((node != NULL) && (node->item != NULL))
    {
      sr->endLine = node->item->line-1;
      if (sr->endLine < 0)
        sr->endLine = 0;
    }

    GetParentTab()->PostMessage(WM_SOURCERANGE,(WPARAM)sr.release());
  }

  CScrollView::OnLButtonUp(nFlags,point);
}

void ContentsPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
    return;
  if (pScrollBar == NULL)
  {
    if ((nSBCode == SB_THUMBPOSITION) || (nSBCode == SB_THUMBTRACK))
    {
      SCROLLINFO scroll;
      ::ZeroMemory(&scroll,sizeof scroll);
      scroll.cbSize = sizeof scroll;
      GetScrollInfo(SB_VERT,&scroll);
      nPos = scroll.nTrackPos;
    }
    OnScroll(MAKEWORD(0xFF,nSBCode),nPos);
  }
}

LRESULT ContentsPane::OnPrint(WPARAM dc, LPARAM)
{
  Draw(CDC::FromHandle((HDC)dc),-GetDeviceScrollPosition().y);

  // Use default processing to pass the print message to all child windows
  return Default();
}

void ContentsPane::OnDraw(CDC* pDC)
{
  // Get the dimensions of the window
  CRect client;
  GetClientRect(client);
  CPoint origin = pDC->GetViewportOrg();

  // Create a memory device context
  CDC dc;
  dc.CreateCompatibleDC(pDC);

  // Create a memory bitmap
  CDibSection bitmap;
  if (bitmap.CreateBitmap(pDC->GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);

  // Draw into the memory bitmap, then copy it to the window's device context
  Draw(&dc,origin.y);
  pDC->BitBlt(-origin.x,-origin.y,client.Width(),client.Height(),&dc,0,0,SRCCOPY);

  // Restore the original device context settings
  dc.SelectObject(oldBitmap);
}

void ContentsPane::PostNcDestroy()
{
  // Do nothing
}

void ContentsPane::Draw(CDC* dc, int origin_y)
{
  // Get the dimensions of the window
  CRect client;
  GetClientRect(client);

  // Select the default font
  CFont* oldFont = dc->SelectObject(&m_font);
  CSize fs = theApp.MeasureFont(&m_font);
  dc->SetTextColor(theApp.GetColour(InformApp::ColourText));
  dc->SetBkMode(TRANSPARENT);

  // Select a pen for line drawing
  CPen pen(PS_SOLID,0,theApp.GetColour(InformApp::ColourBorder));
  CPen* oldPen = dc->SelectObject(&pen);

  // Clear the background
  dc->FillSolidRect(client,theApp.GetColour(InformApp::ColourContents));

  // Clear out any previous selection rectanges
  m_tree->ZeroRects();

  if (m_tree->GetCount(m_smallest,false) == 0)
  {
    CString msg;
    if (m_tree->GetCount(SourceLexer::Section,false) == 0)
    {
      msg =
        "Larger Inform projects are usually divided up with headings like 'Chapter 2 - Into the Forest'. "
        "This page automatically displays those headings as a Table of Contents, but since there are no "
        "headings in this project yet, there is nothing to see.";
    }
    else
    {
      msg =
        "No headings are visible at this level. Drag the slider below to the right to make the headings "
        "in the source text visible.";
    }
    CRect textRect(client);
    textRect.DeflateRect(fs.cx*2,fs.cy);
    dc->DrawText(msg,textRect,DT_LEFT|DT_TOP|DT_WORDBREAK);
  }
  else
  {
    bool title = false;
    int h = 0;
    DrawNode(dc,m_tree.get(),title,h,origin_y);
  }

  // Restore the original device context settings
  dc->SelectObject(oldPen);
  dc->SelectObject(oldFont);
}

void ContentsPane::DrawNode(CDC* dc, Node* node, bool& title, int& h, int origin_y)
{
  // Choose a font for the item, or use the default
  if ((node->GetLevel() == SourceLexer::Title) || (node->selected == Node::NodeSelected))
    dc->SelectObject(&m_boldFont);
  else
    dc->SelectObject(&m_font);

  // Draw the item
  if (node->item != NULL)
  {
    if (node->GetLevel() == SourceLexer::Title)
      title = true;

    if ((node->GetLevel() == SourceLexer::Title) || (node->GetLevel() <= m_smallest))
    {
      const CStringW& name = node->item->name;
      CSize fs = theApp.MeasureFont(&m_font);
      int x = fs.cx+((node->indent-(title ? 1 : 0))*fs.cy);
      int y = origin_y+(((h++)*4+1)*fs.cy/3);

      CRect client;
      GetClientRect(client);
      CRect textRect(x,y,client.Width()-fs.cx,y+fs.cy);

      // If selected, draw the highlighting
      if (node->selected == Node::NodeSelected)
        DrawSelectBack(dc,node,textRect);

      // Is the text rectangle visible?
      CRect intersect;
      if (intersect.IntersectRect(textRect,client))
      {
        // Draw the heading
        theOS.DrawText(dc,name,name.GetLength(),textRect,DT_WORD_ELLIPSIS);
        if (node->GetLevel() != SourceLexer::Title)
        {
          COLORREF back = theApp.GetColour(InformApp::ColourContents);
          if (node->selected == Node::NodeSelected)
            back = theApp.GetColour(InformApp::ColourContentsSelect);
          else if (node->selected == Node::NodeBelowSelection)
            back = theApp.GetColour(InformApp::ColourContentsBelow);

          // Draw a circle to the left of the heading
          CDC dcFrom;
          dcFrom.CreateCompatibleDC(dc);
          CBitmap* fromBitmap = CDibSection::SelectDibSection(dcFrom,GetCircle(back));
          dc->BitBlt(x-fs.cy,y,fs.cy,fs.cy,&dcFrom,0,0,SRCCOPY);
          dcFrom.SelectObject(fromBitmap);
        }

        // Store a selection rectangle
        SIZE textSize;
        ::GetTextExtentPoint32W(dc->GetSafeHdc(),name,name.GetLength(),&textSize);
        if (textSize.cx < textRect.Width())
          textRect.right = textRect.left+textSize.cx;
        textRect.InflateRect(fs.cy,fs.cy/3);
        node->selectRect1 = textRect;
      }

      if ((node->GetLevel() != SourceLexer::Title) && (node->selected != Node::NodeSelected))
      {
        int count = node->GetCount(m_smallest,true);
        if (count > 1)
        {
          // Draw a vertical line to mark out child nodes
          int y1 = origin_y+((h*4+1)*fs.cy/3);
          int y2 = origin_y+((h-1+count)*4*fs.cy/3);
          dc->MoveTo(x-(fs.cy/2),y1);
          dc->LineTo(x-(fs.cy/2),y2);

          // Store a second selection rectangle
          node->selectRect2 = CRect(x-fs.cy,y1,x,y2);
        }
      }
    }
  }

  // Draw child nodes
  for (int i = 0; i < node->children.GetSize(); i++)
    DrawNode(dc,node->children.GetAt(i),title,h,origin_y);
}

void ContentsPane::DrawSelectBack(CDC* dc, Node* node, const CRect& textRect)
{
  CSize fs = theApp.MeasureFont(&m_font);
  CRect backRect(textRect);
  backRect.left -= fs.cy;
  backRect.InflateRect(fs.cx/3,fs.cy/6);

  CPen pen(PS_SOLID,0,theApp.GetColour(InformApp::ColourContentsSelect));
  CPen* oldPen = dc->SelectObject(&pen);

  int count = node->GetCount(m_smallest,true);
  if (count > 1)
  {
    CBrush brush1(theApp.GetColour(InformApp::ColourContentsBelow));
    CBrush* oldBrush = dc->SelectObject(&brush1);

    int y = backRect.bottom;
    backRect.bottom += ((count-1)*(fs.cy*4/3)+1);
    dc->RoundRect(backRect,CPoint(fs.cx,fs.cx));

    dc->MoveTo(backRect.left,y);
    dc->LineTo(backRect.right,y);

    CBrush brush2(theApp.GetColour(InformApp::ColourContentsSelect));
    dc->SelectObject(&brush2);
    CPoint fillPoint((backRect.left+backRect.right)/2,y-1);
    if (!dc->PtVisible(fillPoint))
      fillPoint.y = backRect.top+1;
    dc->FloodFill(fillPoint.x,fillPoint.y,theApp.GetColour(InformApp::ColourContentsSelect));

    dc->SelectObject(oldBrush);
  }
  else
  {
    CBrush brush(theApp.GetColour(InformApp::ColourContentsSelect));
    CBrush* oldBrush = dc->SelectObject(&brush);

    dc->RoundRect(backRect,CPoint(fs.cx,fs.cx));
    dc->SelectObject(oldBrush);
  }

  dc->SelectObject(oldPen);
}

void ContentsPane::BuildTree(Node* parent)
{
  ASSERT(parent != NULL);

  for (int i = 0; i < m_items.GetSize(); i++)
  {
    const Item* item = &(m_items.GetAt(i));
    if (item->level != SourceLexer::Root)
    {
      // Check if this item's level is higher than the parent's level
      while (parent->GetLevel() >= item->level)
      {
        // If so, go to the parent's parent
        if (parent->parent == NULL)
          break;
        parent = parent->parent;
      }

      // Add a new node to the parent node and make the new node the parent
      Node* node = new Node(item,parent,i+1,parent->indent+1);
      parent = node;
    }
  }
}

ContentsPane::Node* ContentsPane::NodeAtPoint(const CPoint& point, Node* node)
{
  // If the node is null, use the root of the tree
  if (node == NULL)
    node = m_tree.get();

  // Is the point in a selection rectangle of the node?
  if (node->selectRect1.Width() > 0)
  {
    if (node->selectRect1.PtInRect(point))
      return node;
  }
  if (node->selectRect2.Width() > 0)
  {
    if (node->selectRect2.PtInRect(point))
      return node;
  }

  // Test all child nodes
  for (int i = 0; i < node->children.GetSize(); i++)
  {
    Node* fromChild = NodeAtPoint(point,node->children.GetAt(i));
    if (fromChild != NULL)
      return fromChild;
  }
  return NULL;
}

CWnd* ContentsPane::GetParentTab(CWnd* wnd)
{
  if (wnd == NULL)
    wnd = this;
  while (wnd != NULL)
  {
    if (wnd->IsKindOf(RUNTIME_CLASS(TabSource)))
      return wnd;
    wnd = wnd->GetParent();
  }
  return NULL;
}

void ContentsPane::SetScrollSize(void)
{
  if (m_tree.get() == NULL)
    return;

  // Set the size of the contents pane to be big enough
  if (m_tree->GetCount(m_smallest,false) == 0)
    SetScrollSizes(MM_TEXT,CSize(1,1));
  else
  {
    CRect client;
    GetClientRect(client);
    CSize fs = theApp.MeasureFont(&m_font);
    int count = m_tree->GetCount(m_smallest,true);
    SetScrollSizes(MM_TEXT,CSize(1,((count*4)+1)*fs.cy/3),client.Size());
  }
}

CDibSection* ContentsPane::GetCircle(COLORREF back)
{
  // Is the image in the cache?
  CString circleName;
  circleName.Format("Contents-circle-scaled-%x",(int)back);
  CDibSection* circleDib = theApp.GetCachedImage(circleName);
  if (circleDib != NULL)
    return circleDib;

  // Load the unscaled image
  CDibSection* dib = theApp.GetCachedImage("Contents-circle");
  CSize dibSize = dib->GetSize();

  // Work out the scaled image size
  CSize circleDibSize = theApp.MeasureFont(&(theApp.GetFont()));
  circleDibSize.cx = circleDibSize.cy;

  // Create a scaled image
  circleDib = new CDibSection();
  CDC* dc = AfxGetMainWnd()->GetDC();
  circleDib->CreateBitmap(dc->GetSafeHdc(),circleDibSize.cx,circleDibSize.cy);
  AfxGetMainWnd()->ReleaseDC(dc);

  // Scale and stretch the image
  ScaleGfx(dib->GetBits(),dibSize.cx,dibSize.cy,
    circleDib->GetBits(),circleDibSize.cx,circleDibSize.cy);

  // Alpha blend with the background colour
  circleDib->AlphaBlend(back);

  theApp.CacheImage(circleName,circleDib);
  return circleDib;
}

void ContentsPane::SetSelectedNode(const SourceHeading& selected)
{
  if (selected.GetSize() == 0)
    return;

  // Find the node to search under
  Node* node = m_tree.get();
  if (node->children.GetCount() != 1)
  {
    ASSERT(FALSE);
    return;
  }
  node = node->children.GetAt(0);
  if (node->GetLevel() != SourceLexer::Title)
  {
    ASSERT(FALSE);
    return;
  }

  int selIndex = (int)selected.GetSize()-1;
  while (selIndex >= 0)
  {
    Node* newNode = NULL;
    for (int i = 0; i < (int)node->children.GetCount(); i++)
    {
      if (node->children.GetAt(i)->item->name == selected.GetAt(selIndex))
        newNode = node->children.GetAt(i);
    }
    if (newNode == NULL)
      return;

    if (selIndex == 0)
    {
      newNode->SelectNode(Node::NodeSelected);
      return;
    }
    selIndex--;
    node = newNode;
  }
}

ContentsPane::Node::Node(const Item* item_, Node* parent_, UINT id_, int indent_)
{
  parent = parent_;
  if (parent != NULL)
    parent->children.Add(this);

  item = item_;
  id = id_;
  indent = indent_;
  selected = NodeNotSelected;

  CRect zero(0,0,0,0);
  selectRect1 = zero;
  selectRect2 = zero;
}

ContentsPane::Node::~Node()
{
  for (int i = 0; i < children.GetSize(); i++)
    delete children.GetAt(i);
}

ContentsPane::Level ContentsPane::Node::GetLevel(void)
{
  if (item == NULL)
    return SourceLexer::Root;
  return item->level;
}

int ContentsPane::Node::GetCount(Level smallest, bool title)
{
  int count = 0;
  switch (GetLevel())
  {
  case SourceLexer::Root:
    count = 0;
    break;
  case SourceLexer::Title:
    count = title ? 1 : 0;
    break;
  default:
    if (GetLevel() > smallest)
      return 0;
    count = 1;
    break;
  }

  for (int i = 0; i < children.GetSize(); i++)
    count += children.GetAt(i)->GetCount(smallest,title);
  return count;
}

void ContentsPane::Node::ZeroRects(void)
{
  CRect zero(0,0,0,0);
  selectRect1 = zero;
  selectRect2 = zero;
  for (int i = 0; i < children.GetSize(); i++)
    children.GetAt(i)->ZeroRects();
}

ContentsPane::Node* ContentsPane::Node::GetNextChild(Node* child)
{
  for (int i = 0; i < children.GetSize()-1; i++)
  {
    if (children.GetAt(i) == child)
      return children.GetAt(i+1);
  }
  return NULL;
}

void ContentsPane::Node::SelectNode(NodeSelection sel)
{
  selected = sel;

  if (sel == NodeSelected)
    sel = NodeBelowSelection;
  for (int i = 0; i < children.GetSize(); i++)
    children.GetAt(i)->SelectNode(sel);
}

IMPLEMENT_DYNAMIC(ContentsWindow, CWnd)

BEGIN_MESSAGE_MAP(ContentsWindow, CWnd)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_PRINT, OnPrint)
END_MESSAGE_MAP()

#define ANIMATE_TIME 200
#define ANIMATE_STEPS 20

ContentsWindow::ContentsWindow()
{
  m_animation = NoAnim;
  m_animateStep = 0;
}

void ContentsWindow::SetHeadings(const CArray<SourceLexer::Heading>& headings, const SourceHeading& selected)
{
  m_contents.SetHeadings(headings,selected);
}

void ContentsWindow::SetFocus(void)
{
  m_contents.SetFocus();
}

void ContentsWindow::SlideIn(CWnd* source)
{
  // Only do anything if not visible
  if (IsWindowVisible())
    return;

  // Show the contents and hide the source
  ShowWindow(SW_SHOW);
  source->ShowWindow(SW_HIDE);

  // Show the animation
  AnimateSlide(AnimSlideIn,source);
  Invalidate();
}

void ContentsWindow::SlideOut(CWnd* source)
{
  // Only do anything if visible
  if (!IsWindowVisible())
    return;

  // Show the animation
  AnimateSlide(AnimSlideOut,source);

  // Show the source and hide the contents
  ShowWindow(SW_HIDE);
  source->ShowWindow(SW_SHOW);
  source->Invalidate();
}

void ContentsWindow::LoadSettings(CRegKey& key)
{
  DWORD pos;
  if (key.QueryDWORDValue("Contents Depth",pos) == ERROR_SUCCESS)
  {
    m_depth.SetPos(pos);
    UpdateSmallest();
  }
}

void ContentsWindow::SaveSettings(CRegKey& key)
{
  key.SetDWORDValue("Contents Depth",m_depth.GetPos());
}

int ContentsWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Create the scrolling contents pane
  m_contents.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,0);
  m_contents.UpdateSmallest(SourceLexer::Part);

  // Create the depth slider
  m_depth.Create(WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS,CRect(0,0,0,0),this,0);
  m_depth.SetRange(0,4);
  m_depth.SetPos(2);
  return 0;
}

BOOL ContentsWindow::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void ContentsWindow::OnPaint()
{
  CPaintDC dcPaint(this);

  if (m_animation == NoAnim)
  {
    // Exclude the area under the depth slider
    CRect depthRect;
    m_depth.GetWindowRect(&depthRect);
    ScreenToClient(&depthRect);
    dcPaint.ExcludeClipRect(depthRect);

    Draw(&dcPaint);
  }
  else
  {
    // Work out which animation frame to draw
    int step = (m_animation == AnimSlideIn) ? m_animateStep : ANIMATE_STEPS-m_animateStep-1;
    CSize sz = m_contentsImage.GetSize();
    int x = (sz.cx * step) / ANIMATE_STEPS;

    // Draw the frame of the animation
    CDC dc;
    dc.CreateCompatibleDC(&dcPaint);
    CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&m_contentsImage);
    dcPaint.BitBlt(0,0,x,sz.cy,&dc,sz.cx-x,0,SRCCOPY);
    CDibSection::SelectDibSection(dc,&m_sourceImage);
    dcPaint.BitBlt(x,0,sz.cx-x,sz.cy,&dc,0,0,SRCCOPY);
    dc.SelectObject(oldBitmap);
  }
}

void ContentsWindow::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType,cx,cy);

  CSize fs = theApp.MeasureFont(&(theApp.GetFont()));
  m_contents.MoveWindow(0,0,cx,cy-fs.cy*2);
  m_depth.MoveWindow(cx/3,(cy-fs.cy*2)+4,cx/3,fs.cy*3/2);
}

void ContentsWindow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  CWnd::OnHScroll(nSBCode,nPos,pScrollBar);
  UpdateSmallest();
  Invalidate();
}

LRESULT ContentsWindow::OnPrint(WPARAM dc, LPARAM)
{
  Draw(CDC::FromHandle((HDC)dc));

  // Use default processing to pass the print message to all child windows
  return Default();
}

void ContentsWindow::Draw(CDC* dc)
{
  CRect client, contents;
  GetClientRect(client);
  m_contents.GetClientRect(contents);
  CSize fs = theApp.MeasureFont(&(theApp.GetFont()));

  // Only draw the space under the contents pane
  client.top = contents.Height();
  dc->FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));
  dc->DrawEdge(client,EDGE_ETCHED,BF_TOP);

  // Draw the label for the control
  CFont* oldFont = dc->SelectObject(&(theApp.GetFont()));
  dc->SetTextColor(theApp.GetColour(InformApp::ColourText));
  dc->SetBkMode(TRANSPARENT);
  CRect labelRect(0,client.top+4,(client.Width()/3)-fs.cx,client.top+(fs.cy*3/2));
  dc->DrawText("Heading depth",13,labelRect,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);

  // Draw the label for the selection in the control
  const char* labels[] =
    { "Volumes only", "Volumes and books", "Parts and higher", "Chapters and higher", "All headings" };
  ASSERT((m_depth.GetPos() >= 0) && (m_depth.GetPos() <= 4));
  const char* label = labels[m_depth.GetPos()];
  labelRect.left = (client.Width()*2/3)+fs.cx;
  labelRect.right = client.Width();
  dc->DrawText(label,(int)strlen(label),labelRect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);

  dc->SelectObject(oldFont);
}

void ContentsWindow::UpdateSmallest(void)
{
  ContentsPane::Level smallest = SourceLexer::Section;
  switch (m_depth.GetPos())
  {
  case 0:
    smallest = SourceLexer::Volume;
    break;
  case 1:
    smallest = SourceLexer::Book;
    break;
  case 2:
    smallest = SourceLexer::Part;
    break;
  case 3:
    smallest = SourceLexer::Chapter;
    break;
  case 4:
    smallest = SourceLexer::Section;
    break;
  default:
    ASSERT(FALSE);
    break;
  }
  m_contents.UpdateSmallest(smallest);
}

void ContentsWindow::AnimateSlide(Animation anim, CWnd* source)
{
  // Don't animate if using Terminal Services
  if (::GetSystemMetrics(SM_REMOTESESSION) != 0)
    return;

  // Prepare for animation
  PrintWindow(m_contentsImage,this,theApp.GetColour(InformApp::ColourContents));
  PrintWindow(m_sourceImage,source,theApp.GetColour(InformApp::ColourBack));
  m_contents.ShowWindow(SW_HIDE);
  m_depth.ShowWindow(SW_HIDE);
  m_animation = anim;

  // Show the animation
  for (int i = 0; i < ANIMATE_STEPS; i++)
  {
    m_animateStep = i;
    Invalidate();
    UpdateWindow();
    ::Sleep(ANIMATE_TIME / ANIMATE_STEPS);
  }

  // Update after animation
  m_contents.ShowWindow(SW_SHOW);
  m_depth.ShowWindow(SW_SHOW);
  m_animation = NoAnim;
}

void ContentsWindow::PrintWindow(CDibSection& dib, CWnd* wnd, COLORREF back)
{
  // Create a device context
  CDC* dcWnd = wnd->GetDC();
  CDC dc;
  dc.CreateCompatibleDC(dcWnd);
  wnd->ReleaseDC(dcWnd);

  // Create a bitmap
  CRect client;
  wnd->GetClientRect(client);
  dib.DeleteBitmap();
  dib.CreateBitmap(dc,client.Width(),client.Height());

  // Print the window into the bitmap
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&dib);
  dc.FillSolidRect(client,back);
  wnd->SendMessage(WM_PRINT,(WPARAM)dc.GetSafeHdc(),PRF_CHILDREN|PRF_CLIENT|PRF_NONCLIENT);
  dc.SelectObject(oldBitmap);
}
