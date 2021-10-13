#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "DpiFunctions.h"
#include "Inform.h"
#include "ProjectFrame.h"
#include "RecentProjectList.h"
#include "ScaleGfx.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//XXXXDK
// Crash in libcef on start, open previous project, close??
// Review text in html pages
// DPI change
// Keyboard
// Accessibility
// Ahead-of-time creation of libcef control?

IMPLEMENT_DYNAMIC(WelcomeLauncherView, CFormView)

WelcomeLauncherView::WelcomeLauncherView() : CFormView(WelcomeLauncherView::IDD)
{
}

BEGIN_MESSAGE_MAP(WelcomeLauncherView, CFormView)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_COMMAND_RANGE(IDC_OPEN_0, IDC_OPEN_9, OnOpenProject)
  ON_COMMAND(IDC_CREATE_PROJECT, OnCreateProject)
  ON_COMMAND(IDC_CREATE_EXTENSION, OnCreateExtProject)
  ON_COMMAND_RANGE(IDC_SAMPLE_ONYX, IDC_SAMPLE_DISENCHANTMENT, OnCopySampleProject)
  ON_COMMAND_RANGE(IDC_ADVICE_NEW, IDC_ADVICE_CREDITS, OnClickedAdvice)
  ON_COMMAND_RANGE(IDC_LINK_INFORM7, IDC_LINK_IFDB_SRC, OnClickedLink)
END_MESSAGE_MAP()

BOOL WelcomeLauncherView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
  DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
  // Start of code copied from CFormView::Create() in MFC sources to enable a call to our CreateDlg()
  CREATESTRUCT cs = { 0 };
  cs.style = dwRequestedStyle;
  if (!PreCreateWindow(cs))
    return FALSE;

  if (!CreateDlg(m_lpszTemplateName,pParentWnd))
    return FALSE;
  ModifyStyle(WS_BORDER|WS_CAPTION,cs.style & (WS_BORDER|WS_CAPTION));
  ModifyStyleEx(WS_EX_CLIENTEDGE,cs.dwExStyle & WS_EX_CLIENTEDGE);
  SetDlgCtrlID(nID);

  CRect rectTemplate;
  GetWindowRect(rectTemplate);
  SetScrollSizes(MM_TEXT,rectTemplate.Size());

  if (!ExecuteDlgInit(m_lpszTemplateName))
    return FALSE;

  SetWindowPos(NULL,rect.left,rect.top,rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE);
  ShowWindow(SW_NORMAL);
  // End of code copied from CFormView::Create() in MFC sources

  // Create the HTML control window
  if (!m_html.Create(NULL,NULL,WS_CHILD,CRect(0,0,0,0),this,1))
  {
    TRACE("Failed to create HTML control\n");
    return FALSE;
  }
  m_html.SetWindowText("Advice");

  // Create and use larger fonts
  LOGFONT lf;
  GetFont()->GetLogFont(&lf);
  LONG fontHeight = lf.lfHeight;
  lf.lfWeight = FW_NORMAL;
  lf.lfHeight = (LONG)(fontHeight*1.15);
  m_bigFont.DeleteObject();
  m_bigFont.CreateFontIndirect(&lf);
  lf.lfWeight = FW_BOLD;
  lf.lfHeight = (LONG)(fontHeight*1.3);
  m_titleFont.DeleteObject();
  m_titleFont.CreateFontIndirect(&lf);
  for (int id = IDC_STATIC_OPEN_RECENT; id <= IDC_STATIC_COMMUNITY; id++)
    GetDlgItem(id)->SetFont(&m_titleFont);

  // Subclass command buttons
  ASSERT((sizeof m_cmds / sizeof m_cmds[0]) == (IDC_SAMPLE_DISENCHANTMENT - IDC_ADVICE_NEW + 1));
  for (int id = IDC_ADVICE_NEW; id <= IDC_SAMPLE_DISENCHANTMENT; id++)
  {
    CommandButton& cmd = m_cmds[id - IDC_ADVICE_NEW];
    cmd.SubclassDlgItem(id,this);

    switch (id)
    {
    case IDC_OPEN_0:
    case IDC_OPEN_1:
    case IDC_OPEN_2:
    case IDC_OPEN_3:
    case IDC_OPEN_4:
    case IDC_OPEN_5:
    case IDC_OPEN_6:
    case IDC_OPEN_7:
    case IDC_OPEN_8:
    case IDC_OPEN_9:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetFont(&m_bigFont);
      cmd.ShowWindow(SW_SHOW);
      break;
    case IDC_CREATE_PROJECT:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetFont(&m_bigFont);
      cmd.SetIcon("Icon-Inform");
      break;
    case IDC_CREATE_EXTENSION:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetFont(&m_bigFont);
      cmd.SetIcon("Icon-I7X");
      break;
    case IDC_SAMPLE_ONYX:
    case IDC_SAMPLE_DISENCHANTMENT:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetFont(&m_bigFont);
      cmd.SetIcon("Icon-Inform");
      cmd.SetTabStop(theApp.MeasureText(&cmd,"Browse Inform projects ").cx);
      break;
    case IDC_LINK_IFDB_SRC:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetFont(&m_bigFont);
      cmd.SetIcon("Icon-New");
      break;
    default:
      cmd.SetBackSysColor(COLOR_WINDOW);
      break;
    }
  }

  // Update commands for opening recent projects
  RecentProjectList* recent = theApp.GetRecentProjectList();
  recent->RemoveInvalid();
  int idx = 0;
  while (idx < recent->GetSize())
  {
    CString display;
    recent->AppendDisplayName(idx,display);
    if (display.IsEmpty())
      break;

    CommandButton& cmd = m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW];
    cmd.SetWindowText(display);
    cmd.SetIcon((ProjectFrame::TypeFromDir((*recent)[idx]) == Project_I7XP) ? "Icon-I7X" : "Icon-Inform");
    ++idx;
  }
  {
    CommandButton& cmd = m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW];
    cmd.SetWindowText("Open...");
    cmd.SetIcon("Icon-Folder");
    idx++;
  }
  for (; idx < 10; ++idx)
    m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW].ShowWindow(SW_HIDE);

  return TRUE;
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL WelcomeLauncherView::CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
  LPCDLGTEMPLATE lpDialogTemplate = NULL;
  HGLOBAL hDialogTemplate = NULL;
  HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName,RT_DIALOG);
  HRSRC hResource = ::FindResource(hInst,lpszTemplateName,RT_DIALOG);
  hDialogTemplate = LoadResource(hInst,hResource);
  if (hDialogTemplate != NULL)
    lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
  ASSERT(lpDialogTemplate != NULL);

  BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate,pParentWnd,hInst);
  UnlockResource(hDialogTemplate);
  FreeResource(hDialogTemplate);
  return bSuccess;
}

INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL WelcomeLauncherView::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd, HINSTANCE hInst)
{
  if (!hInst)
    hInst = AfxGetResourceHandle();

  HGLOBAL hTemplate = NULL;
  HWND hWnd = NULL;

  TRY
  {
    CDialogTemplate dlgTemp(lpDialogTemplate);
    SetFont(dlgTemp);
    hTemplate = dlgTemp.Detach();
    lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst,lpDialogTemplate,pParentWnd->GetSafeHwnd(),AfxDlgProc);
  }
  CATCH_ALL(e)
  {
    e->Delete();
    m_nModalResult = -1;
  }
  END_CATCH_ALL

  if (!AfxUnhookWindowCreate())
    PostNcDestroy();

  if ((hWnd != NULL) && !(m_nFlags & WF_CONTINUEMODAL))
  {
    ::DestroyWindow(hWnd);
    hWnd = NULL;
  }

  if (hTemplate != NULL)
  {
    GlobalUnlock(hTemplate);
    GlobalFree(hTemplate);
  }

  if (hWnd == NULL)
    return FALSE;
  return TRUE;
}

void WelcomeLauncherView::SetFont(CDialogTemplate& dlgTemplate)
{
  NONCLIENTMETRICS ncm;
  ::ZeroMemory(&ncm,sizeof ncm);
  ncm.cbSize = sizeof ncm;
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof ncm,&ncm,0);

  dlgTemplate.SetFont(ncm.lfMessageFont.lfFaceName,9);
}

void WelcomeLauncherView::PostNcDestroy()
{
  // Do nothing
}

HBRUSH WelcomeLauncherView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH brush = CFormView::OnCtlColor(pDC,pWnd,nCtlColor);
  if (nCtlColor == CTLCOLOR_STATIC)
  {
    brush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    switch (pWnd->GetDlgCtrlID())
    {
    case IDC_STATIC_OPEN_RECENT:
    case IDC_STATIC_CREATE_NEW:
    case IDC_STATIC_SAMPLE:
    case IDC_STATIC_SUMMON:
      pDC->SetBkColor(::GetSysColor(COLOR_BTNFACE));
      break;
    case IDC_STATIC_ADVICE:
    case IDC_STATIC_COMMUNITY:
      pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
      break;
    }
  }
  return brush;
}

BOOL WelcomeLauncherView::OnEraseBkgnd(CDC* pDC)
{
  CArray<CRect> regions;
  GetRegions(regions);

  CPen pen(PS_SOLID,0,::GetSysColor(COLOR_BTNSHADOW));
  CPen* oldPen = pDC->SelectObject(&pen);

  int width = regions.GetAt(0).right;
  CSize fs = theApp.MeasureFont(this,GetFont());

  if (m_banner.GetSafeHandle() != 0)
  {
    // Create a memory device context
    CDC dc;
    dc.CreateCompatibleDC(pDC);

    // Select the banner into it
    CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&m_banner);

    // Draw the banner
    pDC->BitBlt(0,0,width,m_banner.GetSize().cy,&dc,0,0,SRCCOPY);

    // Restore the original device context settings
    dc.SelectObject(oldBitmap);
  }
  pDC->MoveTo(0,regions.GetAt(0).bottom-1);
  pDC->LineTo(width,regions.GetAt(0).bottom-1);

  if (m_html.IsWindowVisible() == FALSE)
  {
    pDC->FillSolidRect(regions.GetAt(1),::GetSysColor(COLOR_BTNFACE));
    pDC->FillSolidRect(regions.GetAt(2),::GetSysColor(COLOR_WINDOW));

    pDC->MoveTo(0,regions.GetAt(2).top);
    pDC->LineTo(width,regions.GetAt(2).top);

    pDC->MoveTo(width/2,regions.GetAt(1).top+(fs.cy/2));
    pDC->LineTo(width/2,regions.GetAt(2).top-(fs.cy/2));

    pDC->MoveTo(width/2,regions.GetAt(2).top+(fs.cy/2));
    pDC->LineTo(width/2,regions.GetAt(3).top-(fs.cy/2));
  }

  pDC->FillSolidRect(regions.GetAt(3),::GetSysColor(COLOR_BTNFACE));
  pDC->MoveTo(0,regions.GetAt(3).top);
  pDC->LineTo(regions.GetAt(3).right,regions.GetAt(3).top);

  oldPen = pDC->SelectObject(oldPen);
  return TRUE;
}

void WelcomeLauncherView::OnLButtonUp(UINT nFlags, CPoint point)
{
  CFormView::OnLButtonUp(nFlags, point);

  CArray<CRect> regions;
  GetRegions(regions);
  if (regions.GetAt(0).PtInRect(point))
  {
    if (m_html.IsWindowVisible())
      ShowHtml(false);
  }
}

void WelcomeLauncherView::OnOpenProject(UINT nID)
{
  CString dir = (*(theApp.GetRecentProjectList()))[nID - IDC_OPEN_0];
  if (dir.IsEmpty())
    ProjectFrame::StartExistingProject(theApp.GetLastProjectDir(),this);
  else
    ProjectFrame::StartNamedProject(dir);
}

void WelcomeLauncherView::OnCreateProject()
{
  ProjectFrame::StartNewProject(theApp.GetLastProjectDir(),this);
}

void WelcomeLauncherView::OnCreateExtProject()
{
  ProjectFrame::StartNewExtProject(theApp.GetLastProjectDir(),this,NULL);
}

void WelcomeLauncherView::OnCopySampleProject(UINT nID)
{
  // Find the parent of the last project directory
  CString lastDir = theApp.GetLastProjectDir();
  int i = lastDir.ReverseFind('\\');
  if (i != -1)
    lastDir.Truncate(i);
  else
    lastDir.Empty();

  // Get the destination directory
  CString toDir = theApp.PickDirectory("Choose a directory to save into",lastDir,this);
  if (toDir.IsEmpty())
    return;

  // Convert the destination directory to a shell item
  CComPtr<IShellItem> toItem;
  if (FAILED(::SHCreateItemFromParsingName(CStringW(toDir),NULL,__uuidof(IShellItem),(void**)&toItem)))
    return;

  // Create a file operation to perform the copy
  CComPtr<IFileOperation> fileOper;
  if (FAILED(fileOper.CoCreateInstance(CLSID_FileOperation)))
    return;
  fileOper->SetOperationFlags(FOF_NO_UI);

  // Add the shell items to copy to the operation
  CString fromDir = theApp.GetAppDir();
  fromDir.Append("\\Samples");
  CString resultDir(toDir);
  switch (nID)
  {
  case IDC_SAMPLE_ONYX:
    {
      CComPtr<IShellItem> projectItem;
      if (FAILED(::SHCreateItemFromParsingName(CStringW(fromDir)+L"\\Onyx.inform",NULL,__uuidof(IShellItem),(void**)&projectItem)))
        return;
      if (FAILED(fileOper->CopyItem(projectItem,toItem,NULL,NULL)))
        return;

      resultDir.Append("\\Onyx.inform");
    }
    break;
  case IDC_SAMPLE_DISENCHANTMENT:
    {
      CComPtr<IShellItem> projectItem;
      if (FAILED(::SHCreateItemFromParsingName(CStringW(fromDir)+L"\\Disenchantment Bay.inform",NULL,__uuidof(IShellItem),(void**)&projectItem)))
        return;
      if (FAILED(fileOper->CopyItem(projectItem,toItem,NULL,NULL)))
        return;

      CComPtr<IShellItem> materialsItem;
      if (FAILED(::SHCreateItemFromParsingName(CStringW(fromDir)+L"\\Disenchantment Bay.materials",NULL,__uuidof(IShellItem),(void**)&materialsItem)))
        return;
      if (FAILED(fileOper->CopyItem(materialsItem,toItem,NULL,NULL)))
        return;

      resultDir.Append("\\Disenchantment Bay.inform");
    }
    break;
  default:
    ASSERT(FALSE);
    return;
  }

  // Copy the sample project
  if (FAILED(fileOper->PerformOperations()))
    return;

  // Open the copied sample project
  ProjectFrame::StartNamedProject(resultDir);
}

void WelcomeLauncherView::OnClickedAdvice(UINT nID)
{
  if (HIWORD(GetCurrentMessage()->wParam) == BN_CLICKED)
  {
    static char* pages[] =
    {
      "AdviceNewToInform.html",
      "AdviceUpgrading.html",
      "AdviceKeyboardShortcuts.html",
      "AdviceMaterialsFolder.html",
      "AdviceCredits.html"
    };

    int i = nID - IDC_ADVICE_NEW;
    if ((i >= 0) && (i < (sizeof pages / sizeof pages[0])))
    {
      CArray<CRect> regions;
      GetRegions(regions);
      CRect htmlRect(regions.GetAt(1));
      htmlRect.bottom = regions.GetAt(2).bottom;
      m_html.MoveWindow(htmlRect);

      ShowHtml(true);
      m_html.Navigate(TextFormat::AnsiToUTF8(
        theApp.GetAppDir()+"\\Documentation\\windows\\"+pages[i]),true);
    }
  }
}

void WelcomeLauncherView::OnClickedLink(UINT nID)
{
  if (HIWORD(GetCurrentMessage()->wParam) == BN_CLICKED)
  {
    static char* urls[] =
    {
      "http://www.inform7.com",
      "http://ifdb.org/search?sortby=ratu&newSortBy.x=0&newSortBy.y=0&searchfor=system%3AInform+7",
      "http://www.intfiction.org/forum",
      "http://www.intfiction.org/forum/viewforum.php?f=7",
      "http://ifwiki.org/index.php/Main_Page",
      "http://planet-if.com/",
      "http://ifcomp.org",
      "http://ifdb.org/search?sortby=new&newSortBy.x=0&newSortBy.y=0&searchfor=tag%3A+i7+source+available"
    };

    int i = nID - IDC_LINK_INFORM7;
    if ((i >= 0) && (i < (sizeof urls / sizeof urls[0])))
      ::ShellExecute(0,NULL,urls[i],NULL,NULL,SW_SHOWNORMAL);
  }
}

void WelcomeLauncherView::SetBannerBitmap(void)
{
  CDibSection* original = theApp.GetCachedImage("Welcome Banner");
  ASSERT(original != NULL);

  // Create a bitmap for the scaled banner
  CArray<CRect> regions;
  GetRegions(regions);
  CRect scaled = regions.GetAt(0);
  CDC* dc = GetDC();
  m_banner.DeleteBitmap();
  m_banner.CreateBitmap(dc->GetSafeHdc(),scaled.Width(),scaled.Height());
  ReleaseDC(dc);

  // Scale and stretch the background
  CSize originalSize = original->GetSize();
  ScaleGfx(original->GetBits(),originalSize.cx,originalSize.cy,
    m_banner.GetBits(),scaled.Width(),scaled.Height());
}

void WelcomeLauncherView::GetRegions(CArray<CRect>& regions)
{
  CRect client;
  GetClientRect(client);
  CSize fs = theApp.MeasureFont(this,GetFont());

  CRect labelRect;
  GetDlgItem(IDC_STATIC_OPEN_RECENT)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  CRect regionRect(client);
  regionRect.bottom = labelRect.top - (fs.cy/2);
  regions.Add(regionRect);

  GetDlgItem(IDC_STATIC_ADVICE)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = labelRect.top - (fs.cy/2);
  regions.Add(regionRect);

  GetDlgItem(IDC_STATIC_SUMMON)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = labelRect.top - (fs.cy/6);
  regions.Add(regionRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = client.bottom;
  regions.Add(regionRect);
}

void WelcomeLauncherView::ShowHtml(bool show)
{
  for (int id = IDC_STATIC_OPEN_RECENT; id <= IDC_SAMPLE_DISENCHANTMENT; id++)
  {
    if (id != IDC_STATIC_SUMMON)
      GetDlgItem(id)->ShowWindow(show ? SW_HIDE : SW_SHOW);
  }
  m_html.ShowWindow(show ? SW_SHOW :SW_HIDE);
}

IMPLEMENT_DYNAMIC(WelcomeLauncherFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(WelcomeLauncherFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_CLOSE()
END_MESSAGE_MAP()

WelcomeLauncherFrame::WelcomeLauncherFrame()
{
}

void WelcomeLauncherFrame::ShowLauncher()
{
  // If the active frame is the launcher, close it
  CFrameWnd* activeFrame = theApp.GetActiveFrame();
  if (activeFrame != NULL)
  {
    if (activeFrame->IsKindOf(RUNTIME_CLASS(WelcomeLauncherFrame)))
    {
      activeFrame->PostMessage(WM_CLOSE);
      return;
    }
  }

  // If the launcher is already open, bring it to the front
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  for (int i = 0; i < frames.GetSize(); i++)
  {
    if (frames[i]->IsKindOf(RUNTIME_CLASS(WelcomeLauncherFrame)))
    {
      frames[i]->ActivateFrame();
      return;
    }
  }

  // Open a new launcher
  WelcomeLauncherFrame* frame = new WelcomeLauncherFrame;
  theApp.NewFrame(frame);
  frame->LoadFrame(IDR_LAUNCHFRAME,WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,NULL,NULL);
  frame->Resize(true);
  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();
}

BOOL WelcomeLauncherFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
  const RECT& rect, CWnd* pParentWnd, LPCTSTR lpszMenuName, DWORD dwExStyle, CCreateContext* pContext)
{
  m_strTitle = lpszWindowName;
  if (!CreateEx(dwExStyle,lpszClassName,lpszWindowName,dwStyle,
    rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,pParentWnd->GetSafeHwnd(),0,pContext))
  {
    return FALSE;
  }
  return TRUE;
}

BOOL WelcomeLauncherFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  if (!CFrameWnd::PreCreateWindow(cs))
    return FALSE;

  cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  cs.lpszClass = AfxRegisterWndClass(0);
  return TRUE;
}

BOOL WelcomeLauncherFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  CPushRoutingFrame push(this);

  // First try the view
  if (m_view.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through frame
  if (CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;

  // Then pump through application
  if (AfxGetApp()->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return FALSE;
}

int WelcomeLauncherFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Create the welcome launcher view
  if (!m_view.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,AFX_IDW_PANE_FIRST,NULL))
  {
    TRACE("Failed to create welcome launcher view\n");
    return -1;
  }
  m_view.SetFocus();

  // Set the application icon
  theApp.SetIcon(this);
  return 0;
}

void WelcomeLauncherFrame::OnDestroy()
{
  ReportHtml::RemoveContext(this);
  CFrameWnd::OnDestroy();
}

void WelcomeLauncherFrame::OnClose()
{
  CArray<CFrameWnd*> frames;
  theApp.GetWindowFrames(frames);
  if (frames.GetSize() == 1)
    theApp.WriteOpenProjectsOnExit();

  theApp.FrameClosing(this);
  CFrameWnd::OnClose();
}

void WelcomeLauncherFrame::Resize(bool centre)
{
  // Make the frame fit the view
  CSize viewSize = m_view.GetTotalSize();
  CRect clientRect, frameRect;
  GetClientRect(clientRect);
  GetWindowRect(frameRect);
  int ncw = frameRect.Width() - clientRect.Width();
  int nch = frameRect.Height() - clientRect.Height();
  frameRect.right = frameRect.left + viewSize.cx + ncw;
  frameRect.bottom = frameRect.top + viewSize.cy + nch;

  if (centre)
  {
    // Center in the monitor
    CRect workRect = DPI::getMonitorWorkRect(this);
    frameRect.MoveToXY((workRect.Width()-frameRect.Width())/2,(workRect.Height()-frameRect.Height())/2);
  }

  MoveWindow(frameRect);

  // Create the scaled banner bitmap
  m_view.SetBannerBitmap();
}
