#include "stdafx.h"
#include "SkeinWindow.h"
#include "Inform.h"
#include "TabTesting.h"

#include "Dialogs.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WINNING_LABEL L"***"

IMPLEMENT_DYNCREATE(SkeinWindow, CWnd)

BEGIN_MESSAGE_MAP(SkeinWindow, DrawScrollArea)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_VSCROLL()
  ON_WM_HSCROLL()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_CONTEXTMENU()
  ON_WM_MOUSEACTIVATE()
  ON_WM_MOUSEMOVE()
  ON_WM_CANCELMODE()
  ON_WM_CAPTURECHANGED()
  ON_WM_CHAR()
  ON_WM_TIMER()

  ON_MESSAGE(WM_RENAMENODE, OnRenameNode)
END_MESSAGE_MAP()

SkeinWindow::SkeinWindow() : m_skein(NULL), m_skeinIndex(-1),
  m_mouseOverNode(NULL), m_mouseOverMenu(false), m_mouseMode(MouseNormal), m_clickTime(0), m_dragNode(NULL),
  m_pctAnim(-1), m_showTranscriptAfterAnim(false)
{
  HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(IDC_DRAGCOPY),RT_GROUP_CURSOR);
  ASSERT(hInst != NULL);
  m_arrowDragCopy = ::LoadCursor(hInst,MAKEINTRESOURCE(IDC_DRAGCOPY));
  m_arrowDragMove = ::LoadCursor(hInst,MAKEINTRESOURCE(IDC_DRAGMOVE));
}

SkeinWindow::~SkeinWindow()
{
  if (m_anchorWindow != NULL)
    m_anchorWindow->DestroyWindow();
}

int SkeinWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (DrawScrollArea::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Control for editing text
  if (m_edit.Create(ES_CENTER|ES_AUTOHSCROLL|WS_CHILD,this,0) == FALSE)
    return -1;

  SetFontsBitmaps();
  return 0;
}

void SkeinWindow::OnSize(UINT nType, int cx, int cy)
{
  DrawScrollArea::OnSize(nType,cx,cy);

  if (m_skein != NULL)
    Layout(Skein::LayoutDefault);
}

BOOL SkeinWindow::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

void SkeinWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  // If a drag operation starts, will it be for copying or moving?
  MouseMode dragMode = ((::GetKeyState(VK_CONTROL) & 0x8000) != 0) ? MouseDragCopy : MouseDragMove;

  // Can we allow a drag operation to start?
  bool canDrag = true;
  if (m_mouseOverNode == NULL) // Not over a node
    canDrag = false;
  else if (m_mouseOverMenu) // Over a node's menu button
    canDrag = false;
  else if (m_mouseOverNode->GetParent() == NULL) // Over the root node
    canDrag = false;
  else if (dragMode == MouseDragMove)
  {
    bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
    if (gameRunning && m_skein->InPlayThread(m_mouseOverNode)) // In the playing thread
      canDrag = false;
  }

  if (canDrag)
  {
    // Is the user starting a drag operation? If not, check the mouse button
    // has been released during DragDetect() and re-post the message if needed.
    CPoint scrPoint(point);
    ClientToScreen(&scrPoint);
    if (DragDetect(scrPoint))
    {
      m_dragNode = m_mouseOverNode;
      m_mouseMode = dragMode;

      SetCapture();
      switch (m_mouseMode)
      {
      case MouseDragCopy:
        SetCursor(m_arrowDragCopy);
        break;
      case MouseDragMove:
        SetCursor(m_arrowDragMove);
        break;
      }
    }
    else if ((::GetKeyState(VK_LBUTTON) & 0x8000) == 0)
      PostMessage(WM_LBUTTONUP,nFlags,MAKELPARAM(point.x,point.y));
  }

  DrawScrollArea::OnLButtonDown(nFlags,point);
}

void SkeinWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
  switch (m_mouseMode)
  {
  case MouseNormal:
  case MouseClicked:
    // Handle the 'normal', non-drag case of the mouse button being released
    {
      // Is this a double click?
      bool doubleClick = false;
      if (m_mouseMode == MouseClicked)
        doubleClick = (::GetTickCount() - m_clickTime) < ::GetDoubleClickTime();

      // Put the mouse mode back to normal here in case we return early from this method
      m_mouseMode = MouseNormal;

      Skein::Node* node = NodeAtPoint(point);
      if (node != NULL)
      {
        // Is the user clicking on the context menu button?
        if (!node->IsTestSubItem())
        {
          if (GetMenuButtonRect(m_nodes[node]).PtInRect(point))
          {
            if (!doubleClick)
            {
              CPoint sp(point);
              ClientToScreen(&sp);
              OnContextMenu(this,sp);
            }
            DrawScrollArea::OnLButtonUp(nFlags,point);
            return;
          }
        }

        if (doubleClick)
        {
          // Start the skein playing to this node
          if (!node->IsTestSubItem())
            GetParentFrame()->SendMessage(WM_PLAYSKEIN,(WPARAM)node);
        }
        else
        {
          // Start a timer to expire after the double click time
          SetTimer(1,::GetDoubleClickTime(),NULL);
        }
      }

      // Is the user clicking in the transcript?
      if (m_transcript.IsActive())
      {
        if (m_transcript.LButtonUp(point,m_skein))
          return;
      }

      // If this might be the start of a double click, save the click time and position
      if (!doubleClick)
      {
        m_mouseMode = MouseClicked;
        m_clickTime = ::GetTickCount();
        m_clickPoint = point;
      }
    }
    break;
 
  case MouseDragMove:
  case MouseDragCopy:
    // Handle the case of the mouse being used to drag a skein node
    {
      // Store and reset the mouse mode: releasing the cursor capture will clear
      // the mode by sending a WM_CAPTURECHANGED message.
      MouseMode mode = m_mouseMode;
      m_mouseMode = MouseNormal;

      // Release the cursor
      ::ReleaseCapture();

      CPoint scrPoint(point);
      ClientToScreen(&scrPoint);
      CWnd* dropWnd = CWnd::WindowFromPoint(scrPoint);
      if ((dropWnd != NULL) && dropWnd->IsKindOf(RUNTIME_CLASS(SkeinWindow)))
      {
        CPoint dropPoint(scrPoint);
        dropWnd->ScreenToClient(&dropPoint);
        Skein::Node* dropNode = ((SkeinWindow*)dropWnd)->NodeAtPoint(dropPoint);
        if (dropNode != NULL)
        {
          if (dropNode == m_dragNode)
          {
            // If a node is dropped on itself, treat it as a normal mouse button action
            // by calling this method again, now that the mouse mode has been reset.
            OnLButtonUp(nFlags,point);
            return;
          }
          else if (m_skein->IsValidNode(m_dragNode))
            ((SkeinWindow*)dropWnd)->PerformDrop(this,m_dragNode,dropNode,mode == MouseDragMove);
        }
      }
    }
    break;
  }

  DrawScrollArea::OnLButtonUp(nFlags,point);
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

  // No menu for items created by a "test me" command
  if (node->IsTestSubItem())
    return;

  // Get the context menu
  CMenu popup;
  popup.LoadMenu(IDR_SKEIN);
  CMenu* menu = popup.GetSubMenu(0);

  // Update the state of the context menu items
  if (node->GetParent() == NULL)
  {
    menu->RemoveMenu(ID_SKEIN_INSERT_PREVIOUS,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_DELETE,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_EDIT,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_SET_WINNING,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_DELETE_ALL,MF_BYCOMMAND);
    menu->RemoveMenu(ID_SKEIN_LOCK,MF_BYCOMMAND);
  }
  if (node->GetLabel() == WINNING_LABEL)
    menu->CheckMenuItem(ID_SKEIN_SET_WINNING,MF_BYCOMMAND|MF_CHECKED);
  if (node->GetNumChildren() > 1)
    menu->RemoveMenu(ID_SKEIN_INSERT_NEXT,MF_BYCOMMAND);
  if (node->GetLocked())
    menu->RemoveMenu(ID_SKEIN_LOCK,MF_BYCOMMAND);
  else
    menu->RemoveMenu(ID_SKEIN_UNLOCK,MF_BYCOMMAND);
  if (node->IsTestCommand())
  {
    menu->RemoveMenu(ID_SKEIN_INSERT_NEXT,MF_BYCOMMAND);    
    menu->RemoveMenu(ID_SKEIN_SPLIT_THREAD,MF_BYCOMMAND);    
  }
  RemoveExcessSeparators(menu);
  if (gameRunning && m_skein->InPlayThread(node))
  {
    menu->EnableMenuItem(ID_SKEIN_DELETE,MF_BYCOMMAND|MF_GRAYED);
    menu->EnableMenuItem(ID_SKEIN_DELETE_ALL,MF_BYCOMMAND|MF_GRAYED);
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
    StartEdit(node);
    break;
  case ID_SKEIN_SET_WINNING:
    if (node->GetLabel() == WINNING_LABEL)
      m_skein->SetLabel(node,L"");
    else
    {
      RemoveWinningLabels(m_skein->GetRoot());
      m_skein->SetLabel(node,WINNING_LABEL);
    }
    break;
  case ID_SKEIN_INSERT_PREVIOUS:
    {
      AnimatePrepare();
      Skein::Node* newNode = m_skein->AddNewParent(node);
      Command* cmd = new CommandStartEdit(this,newNode);
      GetParentFrame()->PostMessage(WM_ANIMATESKEIN,0,(LPARAM)cmd);
    }
    break;
  case ID_SKEIN_INSERT_NEXT:
    {
      Skein::Node* newNode = NULL;
      switch (node->GetNumChildren())
      {
      case 0:
        AnimatePrepare();
        newNode = m_skein->AddNew(node);
        break;
      case 1:
        AnimatePrepare();
        newNode = m_skein->AddNewParent(node->GetChild(0));
        break;
      }
      if (newNode != NULL)
      {
        Command* cmd = new CommandStartEdit(this,newNode);
        GetParentFrame()->PostMessage(WM_ANIMATESKEIN,0,(LPARAM)cmd);
      }
    }
    break;
  case ID_SKEIN_SPLIT_THREAD:
    {
      AnimatePrepare();
      Skein::Node* newNode = m_skein->AddNew(node);
      Command* cmd = new CommandStartEdit(this,newNode);
      GetParentFrame()->PostMessage(WM_ANIMATESKEIN,0,(LPARAM)cmd);
    }
    break;
  case ID_SKEIN_DELETE:
    AnimatePrepare();
    m_skein->RemoveSingle(node);
    GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
    break;
  case ID_SKEIN_DELETE_ALL:
    AnimatePrepare();
    m_skein->RemoveAll(node);
    GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
    break;
  case ID_SKEIN_BLESS:
    m_skein->Bless(node,false);
    break;
  case ID_SKEIN_BLESS_ALL:
    m_skein->Bless(node,true);
    break;
  case ID_SKEIN_LOCK:
    m_skein->Lock(node);
    break;
  case ID_SKEIN_UNLOCK:
    m_skein->Unlock(node);
    break;
  case ID_SKEIN_TRIM:
    {
      LPCWSTR head = L"Trim the skein?";
      LPCWSTR msg =
        L"This will remove commands from the skein that have not been locked. "
        L"This action cannot be undone.";
      int btn = 0;
      if (SUCCEEDED(::TaskDialog(GetSafeHwnd(),0,L_INFORM_TITLE,head,msg,TDCBF_YES_BUTTON|TDCBF_NO_BUTTON,TD_WARNING_ICON,&btn)))
      {
        if (btn == IDYES)
        {
          bool gameRunning = (GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0);
          m_skein->Trim(node,gameRunning);
        }
      }
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
  return DrawScrollArea::OnMouseActivate(pDesktopWnd,nHitTest,message);
}

void SkeinWindow::OnMouseMove(UINT nFlags, CPoint point)
{
  Skein::Node* mouseOverNode = NodeAtPoint(point);
  bool mouseOverMenu = false;
  if (mouseOverNode)
  {
    if (mouseOverNode->IsTestSubItem())
      mouseOverNode = NULL;
    else if (GetMenuButtonRect(m_nodes[mouseOverNode]).PtInRect(point))
      mouseOverMenu = true;
  }

  if ((m_mouseOverNode != mouseOverNode) || (m_mouseOverMenu != mouseOverMenu))
  {
    m_mouseOverNode = mouseOverNode;
    m_mouseOverMenu = mouseOverMenu;
    Invalidate();
  }

  if (m_transcript.IsActive())
  {
    if (m_transcript.MouseMove(point))
      Invalidate();
  }

  DrawScrollArea::OnMouseMove(nFlags,point);
}

void SkeinWindow::OnCancelMode()
{
  if ((m_mouseMode == MouseDragMove) || (m_mouseMode == MouseDragCopy))
  {
    m_mouseMode = MouseNormal;
    ::ReleaseCapture();
  }
  DrawScrollArea::OnCancelMode();
}

void SkeinWindow::OnCaptureChanged(CWnd* wnd)
{
  if ((m_mouseMode == MouseDragMove) || (m_mouseMode == MouseDragCopy))
    m_mouseMode = MouseNormal;
  DrawScrollArea::OnCaptureChanged(wnd);
}

void SkeinWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  if (nChar == 27) // Esc
  {
    if ((m_mouseMode == MouseDragMove) || (m_mouseMode == MouseDragCopy))
    {
      m_mouseMode = MouseNormal;
      ::ReleaseCapture();
    }
  }
  DrawScrollArea::OnChar(nChar,nRepCnt,nFlags);
}

void SkeinWindow::OnTimer(UINT_PTR nIDEvent)
{
  if (nIDEvent == 1)
  {
    KillTimer(1);

    if (m_mouseMode == MouseClicked)
    {
      m_mouseMode = MouseNormal;

      // A single click has happened, so just select the node
      Skein::Node* node = NodeAtPoint(m_clickPoint);
      if (node != NULL)
      {
        AnimatePrepareOnlyThis();
        if (!m_transcript.IsActive())
          m_showTranscriptAfterAnim = true;
        m_transcript.SetEndNode(m_transcript.ContainsNode(node) ?
          NULL : m_skein->GetThreadEnd(node),this);
        Layout(Skein::LayoutReposition);
        GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
        UpdateHelp();
      }
    }
  }

  DrawScrollArea::OnTimer(nIDEvent);
}

LRESULT SkeinWindow::OnRenameNode(WPARAM node, LPARAM line)
{
  Skein::Node* theNode = (Skein::Node*)node;
  if (m_skein->IsValidNode(theNode) == false)
    return 0;

  AnimatePrepare();
  m_skein->SetLine(theNode,(LPWSTR)line);
  m_skein->SortSiblings(theNode);
  GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
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
  CFont* oldFont = dc.SelectObject(theApp.GetFont(this,InformApp::FontDisplay));
  CPoint viewOrigin = pDC->GetViewportOrg();

  // Clear the background
  DarkMode* dark = DarkMode::GetActive(this);
  dc.FillSolidRect(client,dark ?
    dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack));

  if (m_skein->IsActive())
  {
    // Redo the layout if needed
    SkeinLayout(dc,Skein::LayoutDefault);

    // Get relevant state from the project frame
    bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;

    // Is the transcript to be drawn?
    Skein::Node* rootNode = m_skein->GetRoot();
    bool drawTranscript = false;
    if (m_transcript.IsActive())
    {
      drawTranscript = true;
      if (m_showTranscriptAfterAnim)
      {
        if (rootNode->IsAnimated(m_skeinIndex))
          drawTranscript = false;
        else
          m_showTranscriptAfterAnim = false;
      }
    }

    // Draw all nodes
    CSize drawOrigin = viewOrigin+GetLayoutBorder();
    for (int i = 0; i < 2; i++)
    {
      DrawNodeTree(i,rootNode,dc,bitmap,
        client,drawOrigin,CPoint(0,0),dark,gameRunning);
      if ((i == 0) && drawTranscript)
        m_transcript.DrawArrows(dc,drawOrigin,dark,m_skeinIndex);
    }

    // Draw the transcript, if visible
    if (drawTranscript)
      m_transcript.Draw(dc,drawOrigin,bitmap);

    // If the edit window is visible, exclude the area under it to reduce flicker
    if (m_edit.IsWindowVisible())
    {
      CRect editRect;
      m_edit.GetWindowRect(&editRect);
      ScreenToClient(&editRect);
      editRect -= viewOrigin;
      pDC->ExcludeClipRect(editRect);
    }
  }

  // Draw the memory bitmap on the window's device context
  pDC->BitBlt(-viewOrigin.x,-viewOrigin.y,client.Width(),client.Height(),&dc,0,0,SRCCOPY);

  // Restore the original device context settings
  dc.SelectObject(oldFont);
  dc.SelectObject(oldBitmap);
}

BOOL SkeinWindow::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= WS_CLIPCHILDREN;
  return DrawScrollArea::PreCreateWindow(cs);
}

void SkeinWindow::SetSkein(Skein* skein, int idx)
{
  m_skein = skein;
  m_skeinIndex = idx;
  m_skein->AddListener(this);
  m_transcript.SetEndNode(NULL,this);
  Layout(Skein::LayoutDefault);
}

void SkeinWindow::Layout(Skein::LayoutMode mode)
{
  CRect client;
  GetClientRect(client);
  SetScrollSizes(MM_TEXT,GetLayoutSize(mode),client.Size());
}

void SkeinWindow::PrefsChanged(void)
{
  m_rootFont.DeleteObject();
  m_testMeFont.DeleteObject();
  SetFontsBitmaps();

  Layout(Skein::LayoutRecalculate);
  Invalidate();
}

void SkeinWindow::SkeinLayout(CDC& dc, Skein::LayoutMode mode)
{
  if (m_skein->IsActive())
  {
    CSize spacing = GetLayoutSpacing();
    m_skein->Layout(dc,m_skeinIndex,mode,spacing,m_transcript);
  }
}

void SkeinWindow::SkeinChanged(Skein::Change change)
{
  if (GetSafeHwnd() == 0)
    return;

  switch (change)
  {
  case Skein::TreeChanged:
    if (m_transcript.IsActive())
    {
      m_transcript.ValidateNodes(m_skein,this);
      Layout(Skein::LayoutReposition);
    }
    else
      Layout(Skein::LayoutDefault);
    Invalidate();
    break;
  case Skein::NodeTextChanged:
    Layout(Skein::LayoutDefault);
    Invalidate();
    break;
  case Skein::NodeTranscriptChanged:
    if (m_transcript.IsActive())
      Layout(Skein::LayoutReposition);
    Invalidate();
    break;
  case Skein::PlayedChanged:
  case Skein::LockChanged:
    Invalidate();
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  UpdateHelp();
}

void SkeinWindow::SkeinEdited(bool edited)
{
  if (GetSafeHwnd() != 0)
    GetParentFrame()->SendMessage(WM_PROJECTEDITED,0);
}

void SkeinWindow::SkeinShowNode(Skein::Node* node, bool select)
{
  if (GetSafeHwnd() == 0)
    return;

  if (!NodeFullyVisible(node))
  {
    // Work out the position of the node
    CPoint origin = GetLayoutBorder();
    int x = origin.x + node->GetX(m_skeinIndex);
    int y = origin.y + node->GetY(m_skeinIndex);

    // Centre the node
    CRect client;
    GetClientRect(client);
    x -= client.Width()/2;
    y -= client.Height()/2;
    if (x < 0)
      x = 0;
    if (y < 0)
      y = 0;

    // Only change the co-ordinates if there are scrollbars
    if (!m_h.visible)
      x = 0;
    if (!m_v.visible)
      y = 0;

    ScrollToDevicePosition(CPoint(x,y));
  }

  if (select)
  {
    AnimatePrepareOnlyThis();
    if (!m_transcript.IsActive())
      m_showTranscriptAfterAnim = true;
    m_transcript.SetEndNode(m_skein->GetThreadEnd(node),this);
    Layout(Skein::LayoutReposition);
    GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
    UpdateHelp();
  }
}

void SkeinWindow::SkeinNodesShown(
  bool& unselected, bool& selected, bool& active, bool& differs, int& count)
{
  unselected = false;
  selected = false;
  active = false;
  differs = false;
  count = 0;

  if (m_skein->IsActive())
  {
    bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
    SkeinNodesShown(m_skein->GetRoot(),gameRunning,unselected,selected,active,differs,count);
  }
}

void SkeinWindow::UpdateHelp(void)
{
  // Update the help to match what is shown
  CWnd* wnd = this;
  while (wnd != NULL)
  {
    if (wnd->IsKindOf(RUNTIME_CLASS(TabTesting)))
      ((TabTesting*)wnd)->PostMessage(WM_UPDATEHELP);
    wnd = wnd->GetParent();
  }
}

void SkeinWindow::RemoveWinningLabels(Skein::Node* node)
{
  if (node->GetLabel() == WINNING_LABEL)
    m_skein->SetLabel(node,L"");
  for (int i = 0; i < node->GetNumChildren(); i++)
    RemoveWinningLabels(node->GetChild(i));
}

void SkeinWindow::PerformDrop(SkeinWindow* fromWnd, Skein::Node* dragNode, Skein::Node* dropNode, bool move)
{
  // Don't do anything if a node is dropped on its parent
  if (dragNode->GetParent() == dropNode)
    return;

  // Don't do anything if a node is dropped on one of its descendents
  if (dragNode->FindAncestor(dropNode) != NULL)
    return;

  // Don't do anything if the target node has test sub-item children
  if (dropNode->GetNumTestSubChildren() > 0)
    return;

  // Get ready to animate the skein, if needed
  AnimatePrepare();

  // Move or copy, as appropriate
  bool done = true;
  if (move)
    done = m_skein->MoveNode(dragNode,dropNode,fromWnd->m_skein);
  else
    m_skein->CopyNode(dragNode,dropNode);

  // Animate the skein, if needed
  if (done)
    GetParentFrame()->PostMessage(WM_ANIMATESKEIN);
}

void SkeinWindow::TranscriptShown(bool& transcript, bool& anyTick, bool& anyCross)
{
  transcript = m_transcript.IsActive();
  m_transcript.Shown(anyTick,anyCross);
}

void SkeinWindow::AnimatePrepare()
{
  if (m_skein->IsActive())
    m_skein->GetRoot()->AnimatePrepare(-1);
}

void SkeinWindow::AnimatePrepareOnlyThis()
{
  if (m_skein->IsActive())
    m_skein->GetRoot()->AnimatePrepare(m_skeinIndex);
}

void SkeinWindow::Animate(int pct)
{
  m_pctAnim = pct;
  Invalidate();
  UpdateWindow();
}

bool SkeinWindow::IsTranscriptActive(void)
{
  return m_transcript.IsActive();
}

void SkeinWindow::SaveTranscript(const char* path)
{
  m_transcript.SaveTranscript(path);
}

CSize SkeinWindow::GetWheelScrollDistance(CSize sizeDistance, BOOL bHorz, BOOL bVert)
{
  CSize sizeRet;
  if (bHorz)
    sizeRet.cx = sizeDistance.cx / 2;
  else
    sizeRet.cx = 0;
  if (bVert)
    sizeRet.cy = sizeDistance.cy / 2;
  else
    sizeRet.cy = 0;
  return sizeRet;
}

CSize SkeinWindow::GetLayoutSize(Skein::LayoutMode mode)
{
  CSize size(0,0);
  if (m_skein->IsActive())
  {
    // Redo the layout if needed
    CDC* dc = GetDC();
    CFont* font = dc->SelectObject(theApp.GetFont(this,InformApp::FontDisplay));
    SkeinLayout(*dc,mode);
    dc->SelectObject(font);
    ReleaseDC(dc);

    // Get the size of the tree
    size = m_skein->GetTreeExtent(m_skeinIndex);
    if (m_transcript.IsActive())
    {
      // Is the transcript to the right of the skein?
      CPoint transOrigin = m_transcript.GetOrigin();
      int transcriptRight = transOrigin.x + m_transcript.GetWidth();
      if (transcriptRight > size.cx)
        size.cx = transcriptRight;

      // Is the transcript taller than the skein?
      int transcriptBottom = transOrigin.y + m_transcript.GetHeight();
      if (transcriptBottom > size.cy)
        size.cy = transcriptBottom;
    }

    // Add border space to all sides
    CSize border = GetLayoutBorder();
    size += border;
    size += border;
  }
  return size;
}

CSize SkeinWindow::GetLayoutSpacing(void)
{
  return CSize(m_fontSize.cx*6,m_fontSize.cy*2);
}

CSize SkeinWindow::GetLayoutBorder(void)
{
  return CSize(m_fontSize.cx*5,(int)(m_fontSize.cy*1.6));
}

void SkeinWindow::SetFontsBitmaps(void)
{
  // Set the edit control's font
  CFont* font = theApp.GetFont(this,InformApp::FontDisplay);
  m_edit.SetFont(font);

  // Create fonts
  LOGFONT fontInfo;
  ::ZeroMemory(&fontInfo,sizeof fontInfo);
  font->GetLogFont(&fontInfo);
  fontInfo.lfWeight = FW_BOLD;
  m_rootFont.CreateFontIndirect(&fontInfo);
  fontInfo.lfWeight = FW_NORMAL;
  fontInfo.lfItalic = TRUE;
  m_testMeFont.CreateFontIndirect(&fontInfo);

  // Work out the size of the font
  m_fontSize = theApp.MeasureFont(this,font);

  // Load bitmaps
  m_bitmaps[BackActive] = GetImage("Skein-active");
  m_bitmaps[BackUnselected] = GetImage("Skein-unselected");
  m_bitmaps[BackSelected] = GetImage("Skein-selected");
  m_bitmaps[MenuActive] = GetImage("Skein-active-menu");
  m_bitmaps[MenuUnselected] = GetImage("Skein-unselected-menu");
  m_bitmaps[MenuSelected] = GetImage("Skein-selected-menu");
  m_bitmaps[MenuOver] = GetImage("Skein-over-menu");
  m_bitmaps[DiffersBadge] = GetImage("SkeinDiffersBadge");
  m_bitmaps[StarBadge] = GetImage("SkeinStarBadge");
  m_bitmaps[BlessButton] = GetImage("Trans-tick-off");
  m_bitmaps[BlessButtonOver] = GetImage("Trans-tick");
  m_bitmaps[CurseButton] = GetImage("Trans-cross-off");
  m_bitmaps[CurseButtonOver] = GetImage("Trans-cross");

  // Set the transcript's fonts and bitmaps
  m_transcript.SetFontsBitmaps(this,m_bitmaps);
}

void SkeinWindow::DrawNodeTree(int phase, Skein::Node* node, CDC& dc, CDibSection& bitmap,
  const CRect& client, const CPoint& origin, const CPoint& parent, DarkMode* dark, bool gameRunning)
{
  CSize nodePos = node->GetAnimatePos(m_skeinIndex,m_pctAnim);
  CPoint nodeCentre(origin.x + nodePos.cx,origin.y + nodePos.cy);

  switch (phase)
  {
  case 0:
    // Draw a line connecting the node to its parent
    if (node->GetParent() != NULL)
    {
      DrawNodeLine(dc,bitmap,client,parent,nodeCentre,
        dark ? dark->GetColour(DarkMode::Fore) : theApp.GetColour(InformApp::ColourSkeinLine),
        dark ? dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack),
        node->GetLocked());
    }
    break;
  case 1:
    // Draw the node
    DrawNode(node,dc,bitmap,client,nodeCentre,m_transcript.ContainsNode(node),gameRunning);
    break;
  }

  // Draw all the node's children
  for (int i = 0; i < node->GetNumChildren(); i++)
    DrawNodeTree(phase,node->GetChild(i),dc,bitmap,client,origin,nodeCentre,dark,gameRunning);
}

void SkeinWindow::DrawNode(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CRect& client,
  const CPoint& centre, bool selected, bool gameRunning)
{
  // Store the current device context properties
  UINT align = dc.GetTextAlign();
  int mode = dc.GetBkMode();

  // Set the device context properties
  dc.SetTextAlign(TA_TOP|TA_LEFT);
  dc.SetBkMode(TRANSPARENT);

  // Get the text associated with the node
  LPCWSTR line = node->GetLine();
  int width = node->CalcLineWidth(dc,m_skeinIndex);

  // Check if this node is visible before drawing
  CSize spacing = GetLayoutSpacing();
  CRect nodeArea(centre,CSize(width+spacing.cx,spacing.cy));
  nodeArea.OffsetRect(nodeArea.Width()/-2,nodeArea.Height()/-2);
  CRect intersect;
  if (intersect.IntersectRect(client,nodeArea))
  {
    // Draw the node's background
    DrawNodeBack(node,bitmap,centre,width,m_bitmaps[GetNodeBack(node,selected,gameRunning)]);

    // Set the default text colour
    dc.SetTextColor(theApp.GetColour(InformApp::ColourBack));

    // Change the font, if needed
    CFont* oldFont = NULL;
    int textWidth = node->GetLineTextWidth(m_skeinIndex);
    if (node == m_skein->GetRoot())
    {
      oldFont = dc.SelectObject(&m_rootFont);
      CSize textSize;
      if (::GetTextExtentPoint32W(dc.GetSafeHdc(),line,(UINT)wcslen(line),&textSize))
        textWidth = textSize.cx;
    }
    else if (node->IsTestSubItem())
    {
      oldFont = dc.SelectObject(&m_testMeFont);
      CSize textSize;
      if (::GetTextExtentPoint32W(dc.GetSafeHdc(),line,(UINT)wcslen(line),&textSize))
        textWidth = textSize.cx;
      dc.SetTextColor(theApp.GetColour(InformApp::ColourTestMe));
    }

    // Write out the node's line
    ::ExtTextOutW(dc.GetSafeHdc(),centre.x-(textWidth/2),centre.y-(m_fontSize.cy/2),0,
      NULL,line,(UINT)wcslen(line),NULL);

    // Put the old font back
    if (oldFont != NULL)
      dc.SelectObject(oldFont);
  }

  // Reset the device context properties
  dc.SetTextAlign(align);
  dc.SetBkMode(mode);
}

void SkeinWindow::DrawNodeBack(Skein::Node* node, CDibSection& bitmap, const CPoint& centre,
  int width, CDibSection* back)
{
  int y = centre.y-(back->GetSize().cy/2)+(int)(0.12*m_fontSize.cy);
  int edgeWidth = (m_fontSize.cx*7)/2;

  // Part of the width is taken up with the rounded edges
  width -= m_fontSize.cx*2;
  if (width < 0)
    width = 0;

  // Draw the rounded edges of the background
  bitmap.AlphaBlend(back,0,0,edgeWidth,back->GetSize().cy,
    centre.x-(width/2)-edgeWidth,y,FALSE);
  bitmap.AlphaBlend(back,back->GetSize().cx-edgeWidth,0,edgeWidth,back->GetSize().cy,
    centre.x+(width/2),y,FALSE);

  // Draw the rest of the background
  {
    int x = centre.x-(width/2);
    while (x < centre.x+(width/2))
    {
      int w = back->GetSize().cx-(2*edgeWidth);
      ASSERT(w > 0);
      if (x+w > centre.x+(width/2))
        w = centre.x+(width/2)-x;
      bitmap.AlphaBlend(back,edgeWidth,0,w,back->GetSize().cy,x,y,FALSE);
      x += w;
    }
  }

  CRect nodeRect(
    CPoint(centre.x-(width/2)-edgeWidth,y),
    CSize(width+(2*edgeWidth),back->GetSize().cy));

  // Draw badges, if needed
  if (node->GetDiffers() && (node->GetExpectedText().IsEmpty() == FALSE))
  {
    CSize badgeSize = m_bitmaps[DiffersBadge]->GetSize();
    CRect badgeRect(CPoint(nodeRect.left,nodeRect.top-(badgeSize.cy/8)),badgeSize);
    bitmap.AlphaBlend(m_bitmaps[DiffersBadge],badgeRect.left,badgeRect.top);
  }
  if (node->GetLabel() == WINNING_LABEL)
  {
    CSize badgeSize = m_bitmaps[StarBadge]->GetSize();
    CRect badgeRect(CPoint(nodeRect.left,nodeRect.bottom-badgeSize.cy),badgeSize);
    bitmap.AlphaBlend(m_bitmaps[StarBadge],badgeRect.left,badgeRect.top);
  }

  // Draw the context menu button, if needed
  if (node == m_mouseOverNode)
  {
    CDibSection* menu = NULL;
    if (back == m_bitmaps[BackActive])
      menu = m_bitmaps[MenuActive];
    else if (back == m_bitmaps[BackUnselected])
      menu = m_bitmaps[MenuUnselected];
    else if (back == m_bitmaps[BackSelected])
      menu = m_bitmaps[MenuSelected];
    if (menu != NULL)
    {
      CRect menuRect = GetMenuButtonRect(nodeRect,menu);
      bitmap.AlphaBlend(menu,menuRect.left,menuRect.top);
      if (m_mouseOverMenu)
        bitmap.AlphaBlend(m_bitmaps[MenuOver],menuRect.left,menuRect.top);
    }
  }

  // Store the node's size and position
  m_nodes[node] = nodeRect;
}

void SkeinWindow::DrawNodeLine(CDC& dc, CDibSection& bitmap, const CRect& client,
  const CPoint& from, const CPoint& to, COLORREF fore, COLORREF back, bool bold)
{
  int p1x = from.x;
  int p1y = from.y;
  int p2x = to.x;
  int p2y = to.y-(int)(0.7*m_fontSize.cy);

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
    dc.SetBkColor(back);

    CPen pen1;
    pen1.CreatePen(PS_SOLID,0,fore);
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
    pen2.CreatePen(PS_SOLID,0,LinePixelColour(0.66,fore,back));
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

      DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dx*invDenom)/1.5),fore);
      DrawLinePixel(dc,bitmap,x,y+1,fabs((two_dx_invDenom-(two_v_dx*invDenom))/1.5),fore);
      DrawLinePixel(dc,bitmap,x,y-1,fabs((two_dx_invDenom+(two_v_dx*invDenom))/1.5),fore);
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

      DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dx*invDenom)/1.5),fore);
      DrawLinePixel(dc,bitmap,x,y+1,fabs((two_dx_invDenom-(two_v_dx*invDenom))/1.5),fore);
      DrawLinePixel(dc,bitmap,x,y-1,fabs((two_dx_invDenom+(two_v_dx*invDenom))/1.5),fore);
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

      DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dy*invDenom)/1.5),fore);
      DrawLinePixel(dc,bitmap,x+1,y,fabs((two_dy_invDenom-(two_v_dy*invDenom))/1.5),fore);
      DrawLinePixel(dc,bitmap,x-1,y,fabs((two_dy_invDenom+(two_v_dy*invDenom))/1.5),fore);
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

      DrawLinePixel(dc,bitmap,x,y,fabs((two_v_dy*invDenom)/1.5),fore);
      DrawLinePixel(dc,bitmap,x+1,y,fabs((two_dy_invDenom-(two_v_dy*invDenom))/1.5),fore);
      DrawLinePixel(dc,bitmap,x-1,y,fabs((two_dy_invDenom+(two_v_dy*invDenom))/1.5),fore);
      count = (count > 6) ? 0 : count+1;
    }
  }

  if (bold)
  {
    DrawNodeLine(dc,bitmap,client,from+CSize(1,0),to+CSize(1,0),fore,back,false);
    DrawNodeLine(dc,bitmap,client,from+CSize(0,1),to+CSize(0,1),fore,back,false);
    DrawNodeLine(dc,bitmap,client,from+CSize(1,1),to+CSize(1,1),fore,back,false);
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

CDibSection* SkeinWindow::GetImage(const char* name)
{
  // Is the image in the cache?
  CString scaledName;
  scaledName.Format("%s-scaled-%ddpi",name,DPI::getWindowDPI(this));
  CDibSection* dib = theApp.GetCachedImage(scaledName);
  if (dib != NULL)
    return dib;

  // Create the scaled image
  CString unscaledName;
  unscaledName.Format("Skein\\%s",name);
  double scale = m_fontSize.cx*0.15;
  dib = theApp.CreateScaledImage(theApp.GetCachedImage(unscaledName),scale,scale);
  theApp.CacheImage(scaledName,dib);
  return dib;
}

CRect SkeinWindow::GetMenuButtonRect(const CRect& nodeRect, CDibSection* menu)
{
  if (menu == NULL)
    menu = m_bitmaps[MenuUnselected];

  CSize sz = menu->GetSize();
  return CRect(CPoint(nodeRect.right-sz.cx+(sz.cx/4),nodeRect.top-(sz.cy/8)),sz);
}

void SkeinWindow::RemoveExcessSeparators(CMenu* menu)
{
  bool allow = true;
  int i = 0;
  while (i < (int)menu->GetMenuItemCount())
  {
    bool removed = false;

    if (menu->GetMenuItemID(i) == 0)
    {
      if (!allow)
      {
        menu->RemoveMenu(i,MF_BYPOSITION);
        removed = true;
      }
      allow = false;
    }
    else
      allow = true;

    if (!removed)
      i++;
  }

  // Remove any final separator
  int count = (int)menu->GetMenuItemCount();
  if (count > 0)
  {
    if (menu->GetMenuItemID(count-1) == 0)
      menu->RemoveMenu(count-1,MF_BYPOSITION);
  }
}

Skein::Node* SkeinWindow::NodeAtPoint(const CPoint& point)
{
  std::map<Skein::Node*,CRect>::const_iterator it;
  for (it = m_nodes.begin(); it != m_nodes.end(); ++it)
  {
    CRect r = it->second;
    r.InflateRect(m_fontSize.cx/2,m_fontSize.cy/4);
    if (r.PtInRect(point))
      return it->first;
  }
  return NULL;
}

bool SkeinWindow::NodeFullyVisible(Skein::Node* node)
{
  std::map<Skein::Node*,CRect>::const_iterator it = m_nodes.find(node);
  if (it == m_nodes.end())
    return false;

  CRect client;
  GetClientRect(client);

  if ((it->second.left < client.left) || (it->second.right > client.right))
    return false;
  if ((it->second.top < client.top) || (it->second.bottom > client.bottom))
    return false;
  return true;
}

void SkeinWindow::StartEdit(Skein::Node* node)
{
  std::map<Skein::Node*,CRect>::const_iterator it = m_nodes.find(node);
  if (it != m_nodes.end())
  {
    CDibSection* back = m_bitmaps[BackUnselected];

    CRect nodeRect = it->second;
    nodeRect.DeflateRect(m_fontSize.cx*2,0);
    nodeRect.top += (back->GetSize().cy/2);
    nodeRect.top -= (int)(0.12*m_fontSize.cy);
    nodeRect.top -= (int)(0.5*m_fontSize.cy);
    nodeRect.bottom = nodeRect.top + m_fontSize.cy+1;

    m_edit.SetFont(theApp.GetFont(this,InformApp::FontDisplay));
    m_edit.StartEdit(node,nodeRect);
  }
}

SkeinWindow::NodeBitmap SkeinWindow::GetNodeBack(Skein::Node* node, bool selected, bool gameRunning)
{
  NodeBitmap back = selected ? BackSelected : BackUnselected;

  Skein::Node* playNode = gameRunning ? m_skein->GetPlayed() : NULL;
  while (playNode != NULL)
  {
    if (playNode == node)
    {
      back = BackActive;
      break;
    }
    playNode = playNode->GetParent();
  }

  return back;
}

void SkeinWindow::SkeinNodesShown(Skein::Node* node, bool gameRunning,
  bool& unselected, bool& selected, bool& active, bool& differs, int& count)
{
  switch (GetNodeBack(node,m_transcript.ContainsNode(node),gameRunning))
  {
  case BackActive:
    active = true;
    break;
  case BackUnselected:
    unselected = true;
    break;
  case BackSelected:
    selected = true;
    break;
  default:
    ASSERT(FALSE);
    break;
  }

  if (node->GetDiffers() && (node->GetExpectedText().IsEmpty() == FALSE))
    differs = true;
  count++;

  for (int i = 0; i < node->GetNumChildren(); i++)
    SkeinNodesShown(node->GetChild(i),gameRunning,unselected,selected,active,differs,count);
}

SkeinWindow::CommandStartEdit::CommandStartEdit(SkeinWindow* wnd, Skein::Node* node)
  : m_wnd(wnd), m_node(node)
{
}

void SkeinWindow::CommandStartEdit::Run(void)
{
  m_wnd->StartEdit(m_node);
}
