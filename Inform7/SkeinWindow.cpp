#include "stdafx.h"
#include "SkeinWindow.h"
#include "Messages.h"
#include "Inform.h"
#include "Dialogs.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SkeinWindow, CScrollView)

BEGIN_MESSAGE_MAP(SkeinWindow, CScrollView)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_VSCROLL()
  ON_WM_HSCROLL()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONUP()
  ON_WM_CONTEXTMENU()
  ON_WM_MOUSEACTIVATE()

  ON_MESSAGE(WM_RENAMENODE, OnRenameNode)
  ON_MESSAGE(WM_LABELNODE, OnLabelNode)
END_MESSAGE_MAP()

SkeinWindow::SkeinWindow() : m_skein(NULL), m_font(NULL)
{
}

int SkeinWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CScrollView::OnCreate(lpCreateStruct) == -1)
    return -1;

  if (m_font == NULL)
  {
    // Store the font to use
    m_font = &(theApp.GetFont());

    // Create a font for labels
    LOGFONT fontInfo;
    ::ZeroMemory(&fontInfo,sizeof fontInfo);
    m_font->GetLogFont(&fontInfo);
    fontInfo.lfWeight = FW_BOLD;
    m_labelFont.CreateFontIndirect(&fontInfo);

    // Work out the size of the font
    m_fontSize = theApp.MeasureFont(m_font);

    // Load bitmaps
    m_bitmaps[BackPlayed] = GetImage("Skein-played",false,true);
    m_bitmaps[BackPlayedDark] = GetImage("Skein-played",true,true);
    m_bitmaps[BackUnplayed] = GetImage("Skein-unplayed",false,true);
    m_bitmaps[BackUnplayedDark] = GetImage("Skein-unplayed",true,true);
    m_bitmaps[BackAnnotate] = GetImage("Skein-annotation",false,true);
    m_bitmaps[DiffersBadge] = GetImage("SkeinDiffersBadge",false,false);
  }

  // Control for editing text
  if (m_edit.Create(ES_CENTER|ES_AUTOHSCROLL|WS_CHILD,this,0) == FALSE)
    return -1;
  m_edit.SetFont(m_font);

  return 0;
}

void SkeinWindow::OnSize(UINT nType, int cx, int cy)
{
  CScrollView::OnSize(nType,cx,cy);

  if (m_skein != NULL)
    Layout();
}

void SkeinWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void SkeinWindow::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
      GetScrollInfo(SB_HORZ,&scroll);
      nPos = scroll.nTrackPos;
    }
    OnScroll(MAKEWORD(nSBCode,0xFF),nPos);
  }
}

BOOL SkeinWindow::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void SkeinWindow::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  Skein::Node* node = NodeAtPoint(point);
  if (node != NULL)
  {
    // Start the skein playing to this node
    GetParentFrame()->SendMessage(WM_PLAYSKEIN,(WPARAM)node);
  }

  CScrollView::OnLButtonDblClk(nFlags,point);
}

void SkeinWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  Skein::Node* node = NodeAtPoint(point);
  if (node != NULL)
  {
    // Is the user clicking on the "differs badge"?
    if ((node->GetDiffers() != Skein::Node::ExpectedSame) && (node->GetExpectedText().IsEmpty() == FALSE))
    {
      CSize badgeSize = m_bitmaps[DiffersBadge]->GetSize();
      CRect badgeRect = m_nodes[node];
      badgeRect.left = badgeRect.right-badgeSize.cx;
      badgeRect.top = badgeRect.bottom-badgeSize.cy;
      if (badgeRect.PtInRect(point))
        GetParentFrame()->SendMessage(WM_SHOWTRANSCRIPT,(WPARAM)node,(LPARAM)GetSafeHwnd());
    }
  }

  CScrollView::OnLButtonUp(nFlags,point);
}

void SkeinWindow::OnContextMenu(CWnd* pWnd, CPoint point)
{
  // No menu if currently editing
  if (m_edit.IsWindowVisible())
    return;

  // No menu if running the game
  bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
  bool gameWaiting = GetParentFrame()->SendMessage(WM_GAMEWAITING) != 0;
  if (gameRunning && !gameWaiting)
    return;

  // Find the node under the mouse, if any
  CPoint cp(point);
  ScreenToClient(&cp);
  Skein::Node* node = NodeAtPoint(cp);
  if (node == NULL)
    return;

  // Get the context menu
  CMenu popup;
  popup.LoadMenu(IDR_SKEIN);
  CMenu* menu = popup.GetSubMenu(0);

  // Update the state of the context menu items
  if (node->GetParent() == NULL)
  {
    menu->RemoveMenu(ID_SKEIN_EDIT,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_ADD_LABEL,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_EDIT_LABEL,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_INSERT_KNOT,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_DELETE,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_DELETE_BELOW,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_DELETE_THREAD,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_LOCK,MF_BYCOMMAND|MF_GRAYED);
    menu->RemoveMenu(ID_SKEIN_UNLOCK,MF_BYCOMMAND|MF_GRAYED);
    menu->RemoveMenu(ID_SKEIN_LOCK_THREAD,MF_BYCOMMAND|MF_GRAYED);
    menu->RemoveMenu(ID_SKEIN_UNLOCK_THREAD,MF_BYCOMMAND|MF_GRAYED);
  }
  else
  {
    if (gameRunning && m_skein->InCurrentThread(node))
    {
      menu->EnableMenuItem(ID_SKEIN_DELETE,MF_BYCOMMAND|MF_GRAYED);
      menu->EnableMenuItem(ID_SKEIN_DELETE_BELOW,MF_BYCOMMAND|MF_GRAYED);
    }
    if (gameRunning && m_skein->InCurrentThread(m_skein->GetThreadTop(node)))
      menu->EnableMenuItem(ID_SKEIN_DELETE_THREAD,MF_BYCOMMAND|MF_GRAYED);
    if (node->GetLabel().IsEmpty())
      menu->RemoveMenu(ID_SKEIN_EDIT_LABEL,MF_BYCOMMAND);
    else
      menu->RemoveMenu(ID_SKEIN_ADD_LABEL,MF_BYCOMMAND);
  }
  if (node->GetTemporary())
  {
    menu->RemoveMenu(ID_SKEIN_UNLOCK,MF_BYCOMMAND|MF_GRAYED);
    menu->RemoveMenu(ID_SKEIN_UNLOCK_THREAD,MF_BYCOMMAND|MF_GRAYED);
  }
  else
  {
    menu->RemoveMenu(ID_SKEIN_LOCK,MF_BYCOMMAND|MF_GRAYED);
    menu->RemoveMenu(ID_SKEIN_LOCK_THREAD,MF_BYCOMMAND|MF_GRAYED);
  }

  // Show the context menu
  int cmd = menu->TrackPopupMenuEx(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD,
    point.x,point.y,GetParentFrame(),NULL);

  // Act on the context menu choice
  switch (cmd)
  {
  case ID_SKEIN_PLAY:
    GetParentFrame()->SendMessage(WM_PLAYSKEIN,(WPARAM)node);
    break;
  case ID_SKEIN_EDIT:
    StartEdit(node,false);
    break;
  case ID_SKEIN_ADD_LABEL:
  case ID_SKEIN_EDIT_LABEL:
    // Make sure that the label background is visible
    Invalidate();
    StartEdit(node,true);
    break;
  case ID_SKEIN_TRANSCRIPT:
    GetParentFrame()->SendMessage(WM_SHOWTRANSCRIPT,(WPARAM)node,(LPARAM)GetSafeHwnd());
    break;
  case ID_SKEIN_LOCK:
    m_skein->Lock(node);
    break;
  case ID_SKEIN_UNLOCK:
    m_skein->Unlock(node);
    break;
  case ID_SKEIN_LOCK_THREAD:
    m_skein->Lock(m_skein->GetThreadBottom(node));
    break;
  case ID_SKEIN_UNLOCK_THREAD:
    m_skein->Unlock(m_skein->GetThreadTop(node));
    break;
  case ID_SKEIN_NEW_THREAD:
    {
      Skein::Node* newNode = m_skein->AddNew(node);

      // Force a repaint so that the new node is drawn and recorded
      Invalidate();
      UpdateWindow();
      StartEdit(newNode,false);
    }
    break;
  case ID_SKEIN_INSERT_KNOT:
    {
      Skein::Node* newNode = m_skein->AddNewParent(node);
      Invalidate();
      UpdateWindow();
      StartEdit(newNode,false);
    }
    break;
  case ID_SKEIN_DELETE:
    if (CanRemove(node))
      m_skein->RemoveSingle(node);
    break;
  case ID_SKEIN_DELETE_BELOW:
    if (CanRemove(node))
      m_skein->RemoveAll(node);
    break;
  case ID_SKEIN_DELETE_THREAD:
    {
      Skein::Node* topNode = m_skein->GetThreadTop(node);
      if (CanRemove(topNode))
        m_skein->RemoveAll(topNode);
    }
    break;
  case ID_SKEIN_SAVE_TRANSCRIPT:
    {
      SimpleFileDialog dialog(FALSE,"txt",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||",this);
      dialog.m_ofn.lpstrTitle = "Save the transcript up to this knot";
      if (dialog.DoModal() == IDOK)
        m_skein->SaveTranscript(node,dialog.GetPathName());
    }
    break;
  case 0:
    // Make sure this window still has the focus
    SetFocus();
    break;
  }
}

int SkeinWindow::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
  // If the edit window is visible and the user clicks outside it, end editing
  if (m_edit.IsWindowVisible())
  {
    CPoint point(::GetMessagePos());
    ScreenToClient(&point);

    if (ChildWindowFromPoint(point) != &m_edit)
      m_edit.EndEdit();
  }
  return CScrollView::OnMouseActivate(pDesktopWnd,nHitTest,message);
}

LRESULT SkeinWindow::OnRenameNode(WPARAM node, LPARAM line)
{
  Skein::Node* theNode = (Skein::Node*)node;
  if (m_skein->IsValidNode(theNode) == false)
    return 0;

  m_skein->SetLine(theNode,(LPWSTR)line);
  return 0;
}

LRESULT SkeinWindow::OnLabelNode(WPARAM node, LPARAM line)
{
  Skein::Node* theNode = (Skein::Node*)node;
  if (m_skein->IsValidNode(theNode) == false)
    return 0;

  m_skein->SetLabel(theNode,(LPWSTR)line);

  // Lock the thread containing the affected node
  m_skein->Lock(m_skein->GetThreadBottom(theNode));
  return 0;
}

void SkeinWindow::OnDraw(CDC* pDC)
{
  // Clear out any previous node positions
  m_nodes.clear();

  // Get the dimensions of the window
  CRect client;
  GetClientRect(client);

  // Create a memory device context
  CDC dc;
  dc.CreateCompatibleDC(pDC);

  // Create a memory bitmap
  CDibSection bitmap;
  if (bitmap.CreateBitmap(pDC->GetSafeHdc(),client.Width(),client.Height()) == FALSE)
    return;
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&bitmap);
  CFont* oldFont = dc.SelectObject(m_font);

  // Clear the background
  dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourBack));

  // Redo the layout if needed
  m_skein->Layout(dc,&m_labelFont,m_fontSize.cx*10);

  // Work out the position of the centre of the root node
  CPoint origin = pDC->GetViewportOrg();
  CPoint rootCentre(origin);
  rootCentre.y += (int)(m_fontSize.cy*2.2);

  // If there is no horizontal scrollbar, centre the root node
  BOOL horiz, vert;
  CheckScrollBars(horiz,vert);
  if (horiz)
    rootCentre.x += GetTotalSize().cx/2;
  else
    rootCentre.x += client.Width()/2;

  // Get the end node of the transcript
  Skein::Node* transcriptEnd = (Skein::Node*)
    GetParentFrame()->SendMessage(WM_TRANSCRIPTEND);

  // Draw all nodes
  DrawNodeTree(m_skein->GetRoot(),transcriptEnd,dc,bitmap,client,
    CPoint(0,0),rootCentre,m_fontSize.cy*5);

  // If the edit window is visible, exclude the area under it to reduce flicker
  if (m_edit.IsWindowVisible())
  {
    CRect editRect;
    m_edit.GetWindowRect(&editRect);
    ScreenToClient(&editRect);
    editRect -= pDC->GetViewportOrg();
    pDC->ExcludeClipRect(editRect);
  }

  // Draw the memory bitmap on the window's device context
  pDC->BitBlt(-origin.x,-origin.y,client.Width(),client.Height(),&dc,0,0,SRCCOPY);

  // Restore the original device context settings
  dc.SelectObject(oldFont);
  dc.SelectObject(oldBitmap);
}

void SkeinWindow::PostNcDestroy()
{
  // Do nothing
}

void SkeinWindow::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_skein->AddListener(this);
  Layout();
}

void SkeinWindow::Layout(void)
{
  CRect client;
  GetClientRect(client);
  SetScrollSizes(MM_TEXT,GetLayoutSize(),client.Size());
}

void SkeinWindow::SkeinChanged(Skein::Change change)
{
  if (GetSafeHwnd() == 0)
    return;

  switch (change)
  {
  case Skein::TreeChanged:
  case Skein::NodeTextChanged:
    Layout();
    Invalidate();
    break;
  case Skein::ThreadChanged:
  case Skein::NodeColourChanged:
  case Skein::LockChanged:
  case Skein::TranscriptThreadChanged:
    Invalidate();
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

void SkeinWindow::SkeinEdited(bool edited)
{
  if (GetSafeHwnd() != 0)
    GetParentFrame()->SendMessage(WM_PROJECTEDITED);
}

void SkeinWindow::SkeinShowNode(Skein::Node* node, Skein::Show why)
{
  if (GetSafeHwnd() == 0)
    return;

  switch (why)
  {
  case Skein::JustShow:
  case Skein::ShowSelect:
  case Skein::ShowNewLine:
    {
      // Work out the position of the node
      int x = (GetTotalSize().cx/2)+node->GetX();
      int y = m_fontSize.cy*((5*node->GetDepth())-1);
      if (y < 0)
        y = 0;

      // Centre the node horizontally
      CRect client;
      GetClientRect(client);
      x -= client.Width()/2;

      // Only change the co-ordinates if there are scrollbars
      BOOL horiz, vert;
      CheckScrollBars(horiz,vert);
      if (horiz == FALSE)
        x = 0;
      if (vert == FALSE)
        y = 0;

      ScrollToPosition(CPoint(x,y));
    }
    break;
  case Skein::ShowNewTranscript:
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

CSize SkeinWindow::GetLayoutSize(void)
{
  CDC* dc = AfxGetMainWnd()->GetDC();
  CFont* font = dc->SelectObject(m_font);

  CSize size;

  // Redo the layout if needed
  m_skein->Layout(*dc,&m_labelFont,m_fontSize.cx*10);

  // The width is the width of all nodes below the root
  size.cx = m_skein->GetRoot()->GetTreeWidth(*dc,&m_labelFont,m_fontSize.cx*10)+
    (m_fontSize.cx*10);

  // The height comes from the maximum tree depth
  size.cy = m_skein->GetRoot()->GetMaxDepth()*m_fontSize.cy*5;

  dc->SelectObject(font);
  AfxGetMainWnd()->ReleaseDC(dc);

  return size;
}

void SkeinWindow::DrawNodeTree(Skein::Node* node, Skein::Node* transcriptEnd, CDC& dc,
  CDibSection& bitmap, const CRect& client, const CPoint& parentCentre,
  const CPoint& siblingCentre, int spacing)
{
  // Draw the node
  CPoint nodeCentre(siblingCentre.x+node->GetX(),siblingCentre.y);
  DrawNode(node,dc,bitmap,client,nodeCentre);

  // Draw a line connecting the node to its parent
  if (node->GetParent() != NULL)
  {
    COLORREF colour = theApp.GetColour(
      node->GetTemporary() ? InformApp::ColourUnlocked : InformApp::ColourLocked);
    DrawNodeLine(dc,bitmap,client,parentCentre,nodeCentre,colour,
      m_skein->InThread(node,transcriptEnd),ShowLabel(node),node->GetTemporary());
  }

  // Draw all the node's children
  CPoint childSiblingCentre(siblingCentre.x,siblingCentre.y+spacing);
  for (int i = 0; i < node->GetNumChildren(); i++)
  {
    DrawNodeTree(node->GetChild(i),transcriptEnd,dc,bitmap,client,
      nodeCentre,childSiblingCentre,spacing);
  }
}

void SkeinWindow::DrawNode(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CRect& client,
  const CPoint& centre)
{
  // Store the current device context properties
  UINT align = dc.GetTextAlign();
  int mode = dc.GetBkMode();

  // Set the device context properties
  dc.SetTextAlign(TA_TOP|TA_LEFT);
  dc.SetBkMode(TRANSPARENT);

  // Get the text associated with the node
  LPCWSTR line = node->GetLine();
  LPCWSTR label = node->GetLabel();
  int width = node->GetLineWidth(dc,&m_labelFont);

  // Check if this node is visible before drawing
  CRect nodeArea(centre,CSize(width+m_fontSize.cx*10,m_fontSize.cy*3));
  nodeArea.OffsetRect(nodeArea.Width()/-2,nodeArea.Height()/-2);
  CRect intersect;
  if (intersect.IntersectRect(client,nodeArea))
  {
    CDibSection* back = m_bitmaps[BackUnplayed];

    bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
    Skein::Node* playNode = gameRunning ? m_skein->GetPlayed() : m_skein->GetCurrent();
    while (playNode != NULL)
    {
      if (playNode == node)
      {
        back = m_bitmaps[BackPlayed];
        break;
      }
      playNode = playNode->GetParent();
    }

    if (node->GetExpectedText().IsEmpty())
    {
      if (back == m_bitmaps[BackPlayed])
        back = m_bitmaps[BackPlayedDark];
      else if (back == m_bitmaps[BackUnplayed])
        back = m_bitmaps[BackUnplayedDark];
    }

    // Draw the node's background
    DrawNodeBack(node,dc,bitmap,centre,width,back);

    // Write out the node's line
    int textWidth = node->GetLineTextWidth();
    ::ExtTextOutW(dc.GetSafeHdc(),centre.x-(textWidth/2),centre.y-(m_fontSize.cy/2),0,
      NULL,line,(UINT)wcslen(line),NULL);

    // Write out the node's label, if any
    if (ShowLabel(node))
    {
      CFont* oldFont = dc.SelectObject(&m_labelFont);
      SIZE size;
      ::GetTextExtentPoint32W(dc.GetSafeHdc(),label,(UINT)wcslen(label),&size);
      ::ExtTextOutW(dc.GetSafeHdc(),centre.x-(size.cx/2),centre.y-(int)(2.1*m_fontSize.cy),
        0,NULL,label,(UINT)wcslen(label),NULL);
      dc.SelectObject(oldFont);
    }
  }

  // Reset the device context properties
  dc.SetTextAlign(align);
  dc.SetBkMode(mode);
}

void SkeinWindow::DrawNodeBack(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CPoint& centre,
  int width, CDibSection* back)
{
  // Create a device context for the source bitmap
  CDC dcFrom;
  dcFrom.CreateCompatibleDC(&dc);
  CBitmap* fromBitmap = CDibSection::SelectDibSection(dcFrom,back);

  int y = centre.y-(back->GetSize().cy/2)+(int)(0.12*m_fontSize.cy);
  int edgeWidth = m_fontSize.cx*4;

  // Draw the rounded edges of the background
  dc.BitBlt(centre.x-(width/2)-edgeWidth,y,edgeWidth,back->GetSize().cy,
    &dcFrom,0,0,SRCCOPY);
  dc.BitBlt(centre.x+(width/2),y,edgeWidth,back->GetSize().cy,
    &dcFrom,back->GetSize().cx-edgeWidth,0,SRCCOPY);

  // Draw the rest of the background
  {
    int x = centre.x-(width/2);
    while (x < centre.x+(width/2))
    {
      int w = back->GetSize().cx-(2*edgeWidth);
      if (x+w > centre.x+(width/2))
        w = centre.x+(width/2)-x;
      dc.BitBlt(x,y,w,back->GetSize().cy,&dcFrom,edgeWidth,0,SRCCOPY);
      x += w;
    }
  }

  // Draw the "differs badge", if needed
  if ((node->GetDiffers() != Skein::Node::ExpectedSame) && (node->GetExpectedText().IsEmpty() == FALSE))
  {
    CDibSection* badge = m_bitmaps[DiffersBadge];
    CSize badgeSize = badge->GetSize();
    bitmap.AlphaBlend(badge,
      centre.x+(width/2)+edgeWidth-badgeSize.cx,y+back->GetSize().cy-badgeSize.cy);
  }

  // Draw the label background, if needed
  if (ShowLabel(node))
  {
    CDibSection::SelectDibSection(dcFrom,m_bitmaps[BackAnnotate]);
    int labelWidth = node->GetLabelTextWidth();
    int labelHeight = (int)(0.9*back->GetSize().cy);
    int labelY = y - (int)(1.8*m_fontSize.cy);

    dc.BitBlt(centre.x-(labelWidth/2)-edgeWidth,labelY,edgeWidth,labelHeight,
      &dcFrom,0,0,SRCCOPY);
    dc.BitBlt(centre.x+(labelWidth/2),labelY,edgeWidth,labelHeight,
      &dcFrom,back->GetSize().cx-edgeWidth,0,SRCCOPY);

    int x = centre.x-(labelWidth/2);
    while (x < centre.x+(labelWidth/2))
    {
      int w = back->GetSize().cx-(2*edgeWidth);
      if (x+w > centre.x+(labelWidth/2))
        w = centre.x+(labelWidth/2)-x;
      dc.BitBlt(x,labelY,w,labelHeight,&dcFrom,edgeWidth,0,SRCCOPY);
      x += w;
    }
  }

  dcFrom.SelectObject(fromBitmap);

  // Store the node's size and position
  m_nodes[node] = CRect(
    CPoint(centre.x-(width/2)-edgeWidth,y),
    CSize(width+(2*edgeWidth),back->GetSize().cy));
}

void SkeinWindow::DrawNodeLine(CDC& dc, CDibSection& bitmap, const CRect& client,
  const CPoint& from, const CPoint& to, COLORREF fore, bool bold, bool label, bool dashed)
{
  int p1x = from.x;
  int p1y = from.y+(int)(1.2*m_fontSize.cy);
  int p2x = to.x;
  int p2y = to.y-(int)(1.2*m_fontSize.cy);
  if (label)
    p2y -= (int)(1.5*m_fontSize.cy);

  // Check if we need to draw the line
  CRect lineArea(p1x,p1y,p2x+1,p2y);
  lineArea.NormalizeRect();
  if (lineArea.Width() == 0)
    lineArea.right++;
  if (lineArea.Height() == 0)
    lineArea.bottom++;
  CRect intersect;
  if (intersect.IntersectRect(client,lineArea) == FALSE)
    return;

  // Special case for a vertical line
  if (p1x == p2x)
  {
    COLORREF back = theApp.GetColour(InformApp::ColourBack);
    dc.SetBkColor(back);

    CPen pen1;
    pen1.CreatePen(dashed ? PS_DOT : PS_SOLID,0,fore);
    CPen* oldPen = dc.SelectObject(&pen1);
    dc.MoveTo(p1x,p1y);
    dc.LineTo(p2x,p2y);
    if (bold)
    {
      dc.MoveTo(p1x-1,p1y);
      dc.LineTo(p2x-1,p2y);
      dc.MoveTo(p1x+1,p1y);
      dc.LineTo(p2x+1,p2y);
    }

    CPen pen2;
    pen2.CreatePen(dashed ? PS_DOT : PS_SOLID,0,LinePixelColour(0.66,fore,back));
    dc.SelectObject(&pen2);
    if (bold)
    {
      dc.MoveTo(p1x-2,p1y);
      dc.LineTo(p2x-2,p2y);
      dc.MoveTo(p1x+2,p1y);
      dc.LineTo(p2x+2,p2y);
    }
    else
    {
      dc.MoveTo(p1x-1,p1y);
      dc.LineTo(p2x-1,p2y);
      dc.MoveTo(p1x+1,p1y);
      dc.LineTo(p2x+1,p2y);
    }

    dc.SelectObject(oldPen);
    return;
  }

  if (p2x < p1x)
  {
    int t;
    t = p2x;
    p2x = p1x;
    p1x = t;
    t = p2y;
    p2y = p1y;
    p1y = t;
  }

  int incMajor = 0, incMinor = 0;
  int d = 0, x = 0, y = 0, two_v_dx, two_v_dy;
  double invDenom, two_dx_invDenom, two_dy_invDenom;
  int count = 0;

  // Draw an anti-aliased line
  int dx = p2x - p1x;
  int dy = p2y - p1y;
  if ((dy >= 0) && (dy <= dx))
  {
    d = 2*dy-dx;
    incMajor = 2*dy;
    incMinor = 2*(dy-dx);

    two_v_dx = 0;
    invDenom = 1.0/(2.0*sqrt((double)((dx*dx)+(dy*dy))));
    two_dx_invDenom = 2.0*dx*invDenom;

    x = p1x;
    y = p1y;

    DrawLinePixel(dc,bitmap,x,y,0.0,fore);
    DrawLinePixel(dc,bitmap,x,y+1,(two_dx_invDenom/1.5),fore);
    DrawLinePixel(dc,bitmap,x,y-1,(two_dx_invDenom/1.5),fore);

    while (x < p2x)
    {
      if (d <= 0)
      {
        two_v_dx = d+dx;
        d += incMajor;
        x++;
      }
      else
      {
        two_v_dx = d-dx;
        d += incMinor;
        x++;
        y++;
      }
      if (!dashed || (count > 3))
      {
        DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dx*invDenom)/1.5),fore);
        DrawLinePixel(dc,bitmap,x,y+1,fabs((two_dx_invDenom-(two_v_dx*invDenom))/1.5),fore);
        DrawLinePixel(dc,bitmap,x,y-1,fabs((two_dx_invDenom+(two_v_dx*invDenom))/1.5),fore);
      }
      count = (count > 6) ? 0 : count+1;
    }
  }

  if ((dy < 0) && (-dy <= dx))
  {
    d = 2*dy+dx;
    incMajor = 2*dy;
    incMinor = 2*(dy+dx);

    two_v_dx = 0;
    invDenom = 1.0/(2.0*sqrt((double)((dx*dx)+(dy*dy))));
    two_dx_invDenom = 2.0*dx*invDenom;

    x = p1x;
    y = p1y;

    DrawLinePixel(dc,bitmap,x,y,0.0,fore);
    DrawLinePixel(dc,bitmap,x,y+1,(two_dx_invDenom/1.5),fore);
    DrawLinePixel(dc,bitmap,x,y-1,(two_dx_invDenom/1.5),fore);

    while (x < p2x)
    {
      if (d <= 0)
      {
        two_v_dx = d+dx;
        d += incMinor;
        x++;
        y--;
      }
      else
      {
        two_v_dx = d-dx;
        d += incMajor;
        x++;
      }
      if (!dashed || (count > 3))
      {
        DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dx*invDenom)/1.5),fore);
        DrawLinePixel(dc,bitmap,x,y+1,fabs((two_dx_invDenom-(two_v_dx*invDenom))/1.5),fore);
        DrawLinePixel(dc,bitmap,x,y-1,fabs((two_dx_invDenom+(two_v_dx*invDenom))/1.5),fore);
      }
      count = (count > 6) ? 0 : count+1;
    }
  }

  if ((dy > 0) && (dy > dx))
  {
    d = 2*dx-dy;
    incMajor = 2*dx;
    incMinor = 2*(dx-dy);

    two_v_dy = 0;
    invDenom = 1.0/(2.0*sqrt((double)((dx*dx)+(dy*dy))));
    two_dy_invDenom = 2.0*dy*invDenom;

    x = p1x;
    y = p1y;

    DrawLinePixel(dc,bitmap,x,y,0.0,fore);
    DrawLinePixel(dc,bitmap,x+1,y,(two_dy_invDenom/1.5),fore);
    DrawLinePixel(dc,bitmap,x-1,y,(two_dy_invDenom/1.5),fore);

    while (y < p2y)
    {
      if (d <= 0)
      {
        two_v_dy = d + dy;
        d += incMajor;
        y++;
      }
      else
      {
        two_v_dy = d - dy;
        d += incMinor;
        y++;
        x++;
      }
      if (!dashed || (count > 3))
      {
        DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dy*invDenom)/1.5),fore);
        DrawLinePixel(dc,bitmap,x+1,y,fabs((two_dy_invDenom-(two_v_dy*invDenom))/1.5),fore);
        DrawLinePixel(dc,bitmap,x-1,y,fabs((two_dy_invDenom+(two_v_dy*invDenom))/1.5),fore);
      }
      count = (count > 6) ? 0 : count+1;
    }
  }

  if ((dy < 0) && (-dy > dx))
  {
    d = 2*dx+dy;
    incMajor = 2*dx;
    incMinor = 2*(dx+dy);

    two_v_dy = 0;
    invDenom = 1.0/(2.0*sqrt((double)((dx*dx)+(dy*dy))));
    two_dy_invDenom = 2.0*fabs((double)dy)*invDenom;

    x = p1x;
    y = p1y;

    DrawLinePixel(dc,bitmap,x,y,0.0,fore);
    DrawLinePixel(dc,bitmap,x+1,y,(two_dy_invDenom/1.5),fore);
    DrawLinePixel(dc,bitmap,x-1,y,(two_dy_invDenom/1.5),fore);

    while (y > p2y)
    {
      if (d <= 0)
      {
        two_v_dy = d-dy;
        d += incMajor;
        y--;
      }
      else
      {
        two_v_dy = d+dy;
        d += incMinor;
        y--;
        x++;
      }
      if (!dashed || (count > 3))
      {
        DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dy*invDenom)/1.5),fore);
        DrawLinePixel(dc,bitmap,x+1,y,fabs((two_dy_invDenom-(two_v_dy*invDenom))/1.5),fore);
        DrawLinePixel(dc,bitmap,x-1,y,fabs((two_dy_invDenom+(two_v_dy*invDenom))/1.5),fore);
      }
      count = (count > 6) ? 0 : count+1;
    }
  }

  if (bold)
  {
    DrawNodeLine(dc,bitmap,client,from+CSize(1,0),to+CSize(1,0),fore,false,label,dashed);
    DrawNodeLine(dc,bitmap,client,from+CSize(0,1),to+CSize(0,1),fore,false,label,dashed);
    DrawNodeLine(dc,bitmap,client,from+CSize(1,1),to+CSize(1,1),fore,false,label,dashed);
  }
}

void SkeinWindow::DrawLinePixel(CDC& dc, CDibSection& bitmap, int x, int y, double i,
  COLORREF fore)
{
  CSize size = bitmap.GetSize();
  if ((x < 0) || (y < 0) || (x >= size.cx) || (y >= size.cy))
    return;

  COLORREF back = bitmap.GetPixelColour(x,y);
  int r = (int)(((1.0-i)*GetRValue(fore))+(i*GetRValue(back)));
  int g = (int)(((1.0-i)*GetGValue(fore))+(i*GetGValue(back)));
  int b = (int)(((1.0-i)*GetBValue(fore))+(i*GetBValue(back)));
  bitmap.SetPixel(x,y,(r<<16)|(g<<8)|b);
}

COLORREF SkeinWindow::LinePixelColour(double i, COLORREF fore, COLORREF back)
{
  int r = (int)(((1.0-i)*GetRValue(fore))+(i*GetRValue(back)));
  int g = (int)(((1.0-i)*GetGValue(fore))+(i*GetGValue(back)));
  int b = (int)(((1.0-i)*GetBValue(fore))+(i*GetBValue(back)));
  return RGB(r,g,b);
}

CDibSection* SkeinWindow::GetImage(const char* name, bool dark, bool blend)
{
  // Is the image in the cache?
  CString skeinName;
  skeinName.Format("%s-%s",name,dark ? "scaled-dark" : "scaled");
  CDibSection* dib = theApp.GetCachedImage(skeinName);
  if (dib != NULL)
    return dib;

  // Create the scaled image
  dib = theApp.CreateScaledImage(theApp.GetCachedImage(name),
    m_fontSize.cx*(1.4/7.0),m_fontSize.cy*(1.4/17.0));

  // Darken and alpha blend with the background colour
  if (dark)
    dib->Darken(0.7);
  if (blend)
    dib->AlphaBlend(theApp.GetColour(InformApp::ColourBack));

  theApp.CacheImage(skeinName,dib);
  return dib;
}

Skein::Node* SkeinWindow::NodeAtPoint(const CPoint& point)
{
  std::map<Skein::Node*,CRect>::const_iterator it;
  for (it = m_nodes.begin(); it != m_nodes.end(); ++it)
  {
    if (it->second.PtInRect(point))
      return it->first;
  }
  return NULL;
}

bool SkeinWindow::ShowLabel(Skein::Node* node)
{
  if (node->HasLabel())
    return true;
  if (m_edit.IsWindowVisible() && m_edit.EditingLabel(node))
    return true;
  return false;
}

void SkeinWindow::StartEdit(Skein::Node* node, bool label)
{
  std::map<Skein::Node*,CRect>::const_iterator it = m_nodes.find(node);
  if (it != m_nodes.end())
  {
    CDibSection* back = m_bitmaps[BackUnplayed];
    CRect nodeRect = it->second;

    if (label)
    {
      nodeRect.InflateRect((node->GetLabelTextWidth()-nodeRect.Width())/2,0);
      nodeRect.InflateRect(m_fontSize.cx,0);
      nodeRect.top += (back->GetSize().cy/2);
      nodeRect.top -= (int)(0.12*m_fontSize.cy);
      nodeRect.top -= (int)(2.1*m_fontSize.cy);
      nodeRect.bottom = nodeRect.top + m_fontSize.cy+1;
      m_edit.SetFont(&m_labelFont);
    }
    else
    {
      nodeRect.DeflateRect(m_fontSize.cx*3,0);
      nodeRect.top += (back->GetSize().cy/2);
      nodeRect.top -= (int)(0.12*m_fontSize.cy);
      nodeRect.top -= (int)(0.5*m_fontSize.cy);
      nodeRect.bottom = nodeRect.top + m_fontSize.cy+1;
      m_edit.SetFont(m_font);
    }

    m_edit.StartEdit(node,nodeRect,label);
  }
}

bool SkeinWindow::CanRemove(Skein::Node* node)
{
  if (node->GetTemporary())
    return true;
  if (MessageBox(
    "This knot has been locked to preserve it. Do you\n"
    "really want to delete it? (This cannot be undone.)",
    INFORM_TITLE,MB_ICONWARNING|MB_YESNO) == IDYES)
  {
    return true;
  }
  return false;
}
