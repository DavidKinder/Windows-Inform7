#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "DpiFunctions.h"
#include "Inform.h"
#include "ProjectFrame.h"
#include "RecentProjectList.h"
#include "ScaleGfx.h"
#include "TextFormat.h"

//XXXXDK modeless,DPI,keyboard,accessibility
// review text in html pages
// ahead-of-time creation of libcef control?

IMPLEMENT_DYNAMIC(WelcomeLauncher, I7BaseDialog)

WelcomeLauncher::WelcomeLauncher(CWnd* pParent) : I7BaseDialog(WelcomeLauncher::IDD,pParent)
{
  m_start = false;
}

BEGIN_MESSAGE_MAP(WelcomeLauncher, I7BaseDialog)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_COMMAND_RANGE(IDC_OPEN_0, IDC_OPEN_9, OnOpenProject)
  ON_COMMAND(IDC_CREATE_PROJECT, OnCreateProject)
  ON_COMMAND(IDC_CREATE_EXTENSION, OnCreateExtProject)
  ON_COMMAND_RANGE(IDC_SAMPLE_ONYX, IDC_SAMPLE_DISENCHANTMENT, OnCopySampleProject)
  ON_COMMAND_RANGE(IDC_ADVICE_NEW, IDC_ADVICE_CREDITS, OnClickedAdvice)
  ON_COMMAND_RANGE(IDC_LINK_INFORM7, IDC_LINK_IFDB_SRC, OnClickedLink)
END_MESSAGE_MAP()

void WelcomeLauncher::ShowStartLauncher(void)
{
  m_start = true;
  DoModal();
}

void WelcomeLauncher::CloseLauncher(void)
{
  if (m_start)
    EndDialog(IDOK);
  else
    ASSERT(FALSE);
}

BOOL WelcomeLauncher::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  // Create the HTML control window
  if (!m_html.Create(NULL,NULL,WS_CHILD,CRect(0,0,0,0),this,1))
  {
    TRACE("Failed to create HTML control\n");
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

  // Create the scaled banner bitmap
  SetBannerBitmap();
  return TRUE;
}

HBRUSH WelcomeLauncher::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH brush = I7BaseDialog::OnCtlColor(pDC,pWnd,nCtlColor);
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

BOOL WelcomeLauncher::OnEraseBkgnd(CDC* pDC)
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

void WelcomeLauncher::OnLButtonUp(UINT nFlags, CPoint point)
{
  I7BaseDialog::OnLButtonUp(nFlags, point);

  CArray<CRect> regions;
  GetRegions(regions);
  if (regions.GetAt(0).PtInRect(point))
  {
    if (m_html.IsWindowVisible())
      ShowHtml(false);
  }
}

LRESULT WelcomeLauncher::OnDpiChanged(WPARAM, LPARAM)
{
  Default();
  if (m_html.GetSafeHwnd() != 0)
  {
    SetBannerBitmap();
  }
  return 0;
}

LRESULT WelcomeLauncher::OnKickIdle(WPARAM, LPARAM)
{
  if (m_start)
    ReportHtml::DoWebBrowserWork(false);
  return 0;
}

void WelcomeLauncher::OnOpenProject(UINT nID)
{
  CString dir = (*(theApp.GetRecentProjectList()))[nID - IDC_OPEN_0];
  if (dir.IsEmpty())
  {
    if (ProjectFrame::StartExistingProject(theApp.GetLastProjectDir(),this))
      CloseLauncher();
  }
  else
  {
    if (ProjectFrame::StartNamedProject(dir))
      CloseLauncher();
  }
}

void WelcomeLauncher::OnCreateProject()
{
  if (ProjectFrame::StartNewProject(theApp.GetLastProjectDir(),this))
    CloseLauncher();
}

void WelcomeLauncher::OnCreateExtProject()
{
  if (ProjectFrame::StartNewExtProject(theApp.GetLastProjectDir(),this,NULL))
    CloseLauncher();
}

void WelcomeLauncher::OnCopySampleProject(UINT nID)
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
  if (ProjectFrame::StartNamedProject(resultDir))
    CloseLauncher();
}

void WelcomeLauncher::OnClickedAdvice(UINT nID)
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

void WelcomeLauncher::OnClickedLink(UINT nID)
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

void WelcomeLauncher::SetBannerBitmap(void)
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

void WelcomeLauncher::GetRegions(CArray<CRect>& regions)
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

void WelcomeLauncher::ShowHtml(bool show)
{
  for (int id = IDC_STATIC_OPEN_RECENT; id <= IDC_SAMPLE_DISENCHANTMENT; id++)
  {
    if (id != IDC_STATIC_SUMMON)
      GetDlgItem(id)->ShowWindow(show ? SW_HIDE : SW_SHOW);
  }
  m_html.ShowWindow(show ? SW_SHOW :SW_HIDE);
}
