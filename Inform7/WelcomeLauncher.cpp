#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "ProjectFrame.h"
#include "RecentProjectList.h"
#include "TextFormat.h"

#include "DpiFunctions.h"
#include "ScaleGfx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RECENT_MAX 8

IMPLEMENT_DYNAMIC(WelcomeLauncherView, FormScrollArea)

WelcomeLauncherView::WelcomeLauncherView() : FormScrollArea(WelcomeLauncherView::IDD)
{
  m_rightGapPerDpi = 0.0;
  m_bottomGapPerDpi = 0.0;
  m_newsTabPerDpi = 0.0;
}

BEGIN_MESSAGE_MAP(WelcomeLauncherView, FormScrollArea)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_WM_SETCURSOR()
  ON_WM_SIZE()
  ON_COMMAND_RANGE(IDC_OPEN_0, IDC_OPEN_7, OnOpenProject)
  ON_COMMAND(IDC_CREATE_PROJECT, OnCreateProject)
  ON_COMMAND(IDC_CREATE_EXTENSION, OnCreateExtProject)
  ON_COMMAND_RANGE(IDC_SAMPLE_ONYX, IDC_SAMPLE_DISENCHANTMENT, OnCopySampleProject)
  ON_COMMAND_RANGE(IDC_ADVICE_NEW, IDC_ADVICE_CREDITS, OnClickedAdvice)
  ON_COMMAND_RANGE(IDC_LINK_IFTF, IDC_LINK_IFDB_SRC, OnClickedLink)
  ON_NOTIFY(TTN_NEEDTEXT, 0, OnToolTipText)
  ON_MESSAGE(WM_CMDLISTCLICKED, OnCmdListClicked)
END_MESSAGE_MAP()

BOOL WelcomeLauncherView::Create(DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  if (!FormScrollArea::Create(dwRequestedStyle,rect,pParentWnd,nID))
  {
    TRACE("Failed to create form\n");
    return FALSE;
  }
  CRect rectTemplate;
  GetWindowRect(rectTemplate);

  // Create the HTML control window
  if (!m_html.Create(NULL,NULL,WS_CHILD,CRect(0,0,0,0),this,1))
  {
    TRACE("Failed to create HTML control\n");
    return FALSE;
  }
  m_html.SetWindowText("Advice");
  m_html.ShowWindow(SW_HIDE);

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
      cmd.SetBackSysColor(COLOR_WINDOW);
      cmd.ShowWindow(SW_SHOW);
      break;
    case IDC_CREATE_PROJECT:
      cmd.SetBackSysColor(COLOR_WINDOW);
      cmd.SetIcon("Icon-Inform",0);
      break;
    case IDC_CREATE_EXTENSION:
      cmd.SetBackSysColor(COLOR_WINDOW);
      cmd.SetIcon("Icon-I7X",0);
      break;
    case IDC_SAMPLE_ONYX:
    case IDC_SAMPLE_DISENCHANTMENT:
      cmd.SetBackSysColor(COLOR_WINDOW);
      cmd.SetIcon("Icon-Inform",0);
      break;
    case IDC_LINK_IFTF:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      cmd.SetIcon("IFTF-Logo",2);
      break;
    case IDC_LINK_IFDB_SRC:
      cmd.SetBackSysColor(COLOR_WINDOW);
      cmd.SetIcon("Icon-New",0);
      break;
    default:
      cmd.SetBackSysColor(COLOR_BTNFACE);
      break;
    }
  }

  // Subclass the news list control
  m_news.SubclassDlgItem(IDC_NEWS,this);
  m_news.SetBackSysColor(COLOR_BTNFACE);

  // Update commands for opening recent projects
  UpdateRecent();

  // Update the list of news items
  UpdateNews();

  // Set the fonts for all controls, and perform any layout required
  SetFonts();
  SetLayout();

  // Get scaling factors for handling a DPI change
  int dpi = DPI::getWindowDPI(this);
  CRect ifdbSrcRect, summonRect;
  GetDlgItem(IDC_LINK_IFDB_SRC)->GetWindowRect(ifdbSrcRect);
  GetDlgItem(IDC_STATIC_SUMMON)->GetWindowRect(summonRect);
  m_rightGapPerDpi = (double)(rectTemplate.right - ifdbSrcRect.right) / (double)dpi;
  m_bottomGapPerDpi = (double)(rectTemplate.bottom - summonRect.bottom) / (double)dpi;

  EnableToolTips();
  return TRUE;
}

CSize WelcomeLauncherView::GetTotalSize() const
{
  int w = 0;
  int h = 0;

  for (int id = IDC_STATIC_NEWS; id <= IDC_NEWS; id++)
  {
    CRect r;
    GetDlgItem(id)->GetWindowRect(r);
    ScreenToClient(r);

    if (w < r.right)
      w = r.right;
    if (h < r.bottom)
      h = r.bottom;
  }

  int dpi = DPI::getWindowDPI((CWnd*)this);
  w += (int)(m_rightGapPerDpi * dpi);
  h += (int)(m_bottomGapPerDpi * dpi);
  return CSize(w,h);
}

void WelcomeLauncherView::PostNcDestroy()
{
  // Do nothing
}

BOOL WelcomeLauncherView::PreTranslateMessage(MSG* pMsg)
{
  if (DrawScrollArea::PreTranslateMessage(pMsg))
    return TRUE;

  CFrameWnd* pFrameWnd = GetParentFrame();
  while (pFrameWnd != NULL)
  {
    if (pFrameWnd->PreTranslateMessage(pMsg))
      return TRUE;
    pFrameWnd = pFrameWnd->GetParentFrame();
  }
  if (::GetWindow(m_hWnd,GW_CHILD) == NULL)
    return FALSE;

  // Backspace hides the HTML page
  if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_BACK))
    ShowHtml(false);
  
  // Don't process dialog messages if the HTML page is showing
  if (m_html.IsWindowVisible())
    return FALSE;

  return PreTranslateInput(pMsg);
}

HBRUSH WelcomeLauncherView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH brush = FormScrollArea::OnCtlColor(pDC,pWnd,nCtlColor);
  if (nCtlColor == CTLCOLOR_STATIC)
  {
    brush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    DarkMode* dark = DarkMode::GetActive(this);
    
    switch (pWnd->GetDlgCtrlID())
    {
    case IDC_STATIC_OPEN_RECENT:
    case IDC_STATIC_CREATE_NEW:
    case IDC_STATIC_SAMPLE:
      pDC->SetBkColor(dark ?
        dark->GetColour(DarkMode::Back) : ::GetSysColor(COLOR_WINDOW));
      pDC->SetTextColor(dark ?
        dark->GetColour(DarkMode::Fore) : ::GetSysColor(COLOR_BTNTEXT));
      break;
    case IDC_STATIC_NEWS:
    case IDC_STATIC_ADVICE:
    case IDC_STATIC_COMMUNITY:
    case IDC_STATIC_SUMMON:
    case IDC_NEWS:
      pDC->SetBkColor(dark ?
        dark->GetColour(DarkMode::Dark3) : ::GetSysColor(COLOR_BTNFACE));
      pDC->SetTextColor(dark ?
        dark->GetColour(DarkMode::Fore) : ::GetSysColor(COLOR_BTNTEXT));
      break;
    }
  }
  return brush;
}

BOOL WelcomeLauncherView::OnEraseBkgnd(CDC* pDC)
{
  CArray<CRect> regions;
  GetRegions(regions);

  DarkMode* dark = DarkMode::GetActive(this);

  CPen pen(PS_SOLID,0,dark ?
    dark->GetColour(DarkMode::Dark1) : ::GetSysColor(COLOR_BTNSHADOW));
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
    pDC->FillSolidRect(regions.GetAt(1),dark ?
    dark->GetColour(DarkMode::Dark3) : ::GetSysColor(COLOR_BTNFACE));
    pDC->FillSolidRect(regions.GetAt(2),dark ?
    dark->GetColour(DarkMode::Back) : ::GetSysColor(COLOR_WINDOW));
    pDC->FillSolidRect(regions.GetAt(3),dark ?
    dark->GetColour(DarkMode::Dark3) : ::GetSysColor(COLOR_BTNFACE));

    pDC->MoveTo(0,regions.GetAt(2).top);
    pDC->LineTo(width,regions.GetAt(2).top);

    pDC->MoveTo(width/2,regions.GetAt(2).top+(fs.cy/2));
    pDC->LineTo(width/2,regions.GetAt(3).top-(fs.cy/2));

    pDC->MoveTo(0,regions.GetAt(3).top);
    pDC->LineTo(width,regions.GetAt(3).top);

    pDC->MoveTo(width/2,regions.GetAt(3).top+(fs.cy/2));
    pDC->LineTo(width/2,regions.GetAt(4).top-(fs.cy/2));
  }

  pDC->FillSolidRect(regions.GetAt(4),dark ?
    dark->GetColour(DarkMode::Dark3) : ::GetSysColor(COLOR_BTNFACE));

  pDC->MoveTo(0,regions.GetAt(4).top);
  pDC->LineTo(regions.GetAt(4).right,regions.GetAt(4).top);

  oldPen = pDC->SelectObject(oldPen);
  return TRUE;
}

void WelcomeLauncherView::OnToolTipText(NMHDR* hdr, LRESULT* result)
{
  TOOLTIPTEXT* ttt = (TOOLTIPTEXT*)hdr;

  UINT_PTR id = hdr->idFrom;
  if (ttt->uFlags & TTF_IDISHWND)
    id = (UINT)(WORD)::GetDlgCtrlID((HWND)id);

  CString tipText = GetToolTip(id);
  static char tipTextBuffer[256];
  lstrcpyn(tipTextBuffer,tipText,sizeof tipTextBuffer / sizeof tipTextBuffer[0]);
  ttt->lpszText = tipTextBuffer;

  ::SetWindowPos(hdr->hwndFrom,HWND_TOP,0,0,0,0,
    SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);

  CWnd* wnd = CWnd::FromHandle(hdr->hwndFrom);
  if (wnd && wnd->IsKindOf(RUNTIME_CLASS(CToolTipCtrl)))
    DarkMode::Set((CToolTipCtrl*)wnd,DarkMode::GetActive(this));

  *result = 0;
}

void WelcomeLauncherView::OnLButtonUp(UINT nFlags, CPoint point)
{
  FormScrollArea::OnLButtonUp(nFlags,point);

  CArray<CRect> regions;
  GetRegions(regions);
  if (regions.GetAt(0).PtInRect(point))
  {
    if (m_html.IsWindowVisible())
      ShowHtml(false);
  }
}

BOOL WelcomeLauncherView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  UINT_PTR id = pWnd->GetDlgCtrlID();
  CString url = GetUrl(id,(id == IDC_NEWS) ? m_news.GetHotIndex() : -1);
  if (!url.IsEmpty())
  {
    HCURSOR hand = ::LoadCursor(0,IDC_HAND);
    if (hand != 0)
    {
      ::SetCursor(hand);
      return TRUE;
    }
  }
  return FormScrollArea::OnSetCursor(pWnd,nHitTest,message);
}

void WelcomeLauncherView::OnSize(UINT nType, int cx, int cy)
{
  FormScrollArea::OnSize(nType,cx,cy);

  // Turn scrollbars off
  EnableScrollBarCtrl(SB_BOTH,FALSE);
}

void WelcomeLauncherView::OnOpenProject(UINT nID)
{
  int i = nID - IDC_OPEN_0;

  CString dir;
  if (i < m_recentProjects.GetSize())
    dir = m_recentProjects[i];
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
  CString toDir = theApp.PickDirectory("Choose where to copy the sample project",
    "Location of sample project:","Copy Sample Project",lastDir,this);
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
      htmlRect.bottom = regions.GetAt(3).bottom;
      m_html.MoveWindow(htmlRect);

      ShowHtml(true);
      m_html.Navigate(TextFormat::AnsiToUTF8(
        theApp.GetAppDir()+"\\Documentation\\advice\\"+pages[i]),true);
    }
  }
}

void WelcomeLauncherView::OnClickedLink(UINT nID)
{
  if (HIWORD(GetCurrentMessage()->wParam) == BN_CLICKED)
  {
    CString url = GetUrl(nID,-1);
    if (!url.IsEmpty())
      ::ShellExecute(0,NULL,url,NULL,NULL,SW_SHOWNORMAL);
  }
}

LRESULT WelcomeLauncherView::OnCmdListClicked(WPARAM id, LPARAM index)
{
  CString url = GetUrl(id,(int)index);
  if (!url.IsEmpty())
    ::ShellExecute(0,NULL,url,NULL,NULL,SW_SHOWNORMAL);
  return 0;
}

void WelcomeLauncherView::UpdateDPI(void)
{
  SetFonts();
  SetLayout();
  for (CommandButton& cmd : m_cmds)
    cmd.UpdateDPI();
}

void WelcomeLauncherView::UpdateRecent(void)
{
  bool show = (m_html.IsWindowVisible() == FALSE);

  RecentProjectList* recent = theApp.GetRecentProjectList();
  recent->RemoveInvalid();

  int idx = 0;
  m_recentProjects.RemoveAll();
  while ((idx < recent->GetSize()) && (idx < RECENT_MAX-1))
  {
    CString display;
    recent->AppendDisplayName(idx,display);
    if (display.IsEmpty())
      break;

    m_recentProjects.Add((*recent)[idx]);
    CommandButton& cmd = m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW];
    if (show)
      cmd.ShowWindow(SW_SHOW);
    cmd.SetWindowText(display);
    cmd.SetIcon((ProjectFrame::TypeFromDir((*recent)[idx]) == Project_I7XP) ? "Icon-I7X" : "Icon-Inform",0);
    ++idx;
  }
  {
    CommandButton& cmd = m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW];
    if (show)
      cmd.ShowWindow(SW_SHOW);
    cmd.SetWindowText("Open...");
    cmd.SetIcon("Icon-Folder",0);
    idx++;
  }
  if (show)
  {
    for (; idx < RECENT_MAX; ++idx)
      m_cmds[IDC_OPEN_0 + idx - IDC_ADVICE_NEW].ShowWindow(SW_HIDE);
  }
}

// Structure representing a date in the IFTF news file
struct NewsDate
{
  NewsDate(LPCSTR str)
  {
    // Parse a date as yyyy-mm-dd
    if (str[0] != '\0')
      sscanf(str,"%4d-%2d-%2d",&year,&month,&day);
  }

  bool IsEmpty(void) const
  {
    return (year == 0) || (month == 0) || (day == 0);
  }

  // Format the month as a short string in the user's locale, e.g. in English,
  // January is formatted as "Jan".
  CString FormatMonth() const
  {
    SYSTEMTIME sysTime = { 0 };
    sysTime.wYear = year;
    sysTime.wMonth = month;
    sysTime.wDay = day;

    char output[32] = { 0 };
    if (::GetDateFormat(LOCALE_USER_DEFAULT,0,&sysTime,"MMM",output,sizeof output) > 0)
      return output;
    return "";
  }

  // Format a date according to the LOCALE_IDATE order
  CString Format(DWORD order) const
  {
    CString output;
    switch (order)
    {
    case 0: // month day year
    default:
      output.Format("%s %d %d",FormatMonth(),day,year);
      break;
    case 1: // day month year
      output.Format("%d %s %d",day,FormatMonth(),year);
      break;
    case 2: // year month day
      output.Format("%d %s %d",year,FormatMonth(),day);
      break;
    }
    return output;
  }

  int year = 0;
  int month = 0;
  int day = 0;
};

// Format a date range according to the LOCALE_IDATE order
CString FormatDateRange(const NewsDate& start, const NewsDate& end, DWORD order)
{
  CString output;
  if (start.year != end.year)
  {
    // If the range spans multiple years, format as "start date - end date".
    output.Format("%s - %s",(LPCSTR)start.Format(order),(LPCSTR)end.Format(order));
  }
  else if (start.month != end.month)
  {
    // If the range spans multiple months, format the range with a single year but the
    // day-month range ordered according to the LOCALE_IDATE order.
    switch (order)
    {
    case 0:
    default: // range(month day) year
      output.Format("%s %d - %s %d %d",start.FormatMonth(),start.day,end.FormatMonth(),end.day,start.year);
      break;
    case 1: // range(day month) year
      output.Format("%d %s - %d %s %d",start.day,start.FormatMonth(),end.day,end.FormatMonth(),start.year);
      break;
    case 2: // year range(month day)
      output.Format("%d %s %d - %s %d",start.year,start.FormatMonth(),start.day,end.FormatMonth(),end.day);
      break;
    }
  }
  else
  {
    // If the range is within a month, format the range with a single year and month
    // and a day range, ordered according to the LOCALE_IDATE order.
    switch (order)
    {
    case 0:
    default: // month range(day) year
      output.Format("%s %d-%d %d",start.FormatMonth(),start.day,end.day,start.year);
      break;
    case 1: // range(day) month year
      output.Format("%d-%d %s %d",start.day,end.day,start.FormatMonth(),start.year);
      break;
    case 2: // year month range(day)
      output.Format("%d %s %d-%d",start.year,start.FormatMonth(),start.day,end.day);
      break;
    }
  }
  return output;
}

void WelcomeLauncherView::UpdateNews(void)
{
  m_news.ResetContent();
  m_newsUrls.clear();

  // Get the enumeration giving the order for elements of dates, e.g. day-month-year
  DWORD dateOrder = 0;
  GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE|LOCALE_RETURN_NUMBER,(LPSTR)&dateOrder,sizeof dateOrder);

  bool empty = true;
  int widestDate = 0;
  CStdioFile newsFile;
  if (newsFile.Open(theApp.GetHomeDir()+"\\Inform\\cal.txt",CFile::modeRead|CFile::typeText))
  {
    CString newsLine;
    while (newsFile.ReadString(newsLine))
    {
      // Parse the news file
      int tab1 = newsLine.Find('\t');
      if (tab1 < 0)
        continue;
      int tab2 = newsLine.Find('\t',tab1+1);
      if (tab2 < 0)
        continue;
      int tab3 = newsLine.Find('\t',tab2+1);
      if (tab3 < 0)
        continue;
      NewsDate newsStart(newsLine.Left(tab1));
      NewsDate newsEnd(newsLine.Mid(tab1+1,tab2-tab1-1));
      CString newsTitle = newsLine.Mid(tab2+1,tab3-tab2-1);
      m_newsUrls.push_back((LPCSTR)newsLine.Mid(tab3+1));

      CString newsItem;
      if (newsEnd.IsEmpty())
        newsItem = newsStart.Format(dateOrder);
      else
        newsItem = FormatDateRange(newsStart,newsEnd,dateOrder);

      int dw = theApp.MeasureText(&m_news,newsItem).cx;
      if (dw > widestDate)
        widestDate = dw;

      newsItem.AppendChar('\t');
      newsItem.Append(newsTitle);
      m_news.AddString(newsItem);
      empty = false;
    }
    newsFile.Close();
  }

  if (empty)
    m_news.AddString("There is no news available");

  CSize fs = theApp.MeasureFont(&m_news,m_news.GetFont());
  int ts = widestDate + (fs.cx*4);
  m_news.SetTabStop(ts);
  m_newsTabPerDpi = (double)ts / (double)DPI::getWindowDPI(this);
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

INT_PTR WelcomeLauncherView::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
  INT_PTR result = FormScrollArea::OnToolHitTest(point,pTI);
  if (pTI)
    pTI->uFlags &= ~TTF_CENTERTIP;
  return result;
}

void WelcomeLauncherView::SetDarkMode(DarkMode* dark)
{
  m_news.SetDarkMode(dark);
}

void WelcomeLauncherView::GetRegions(CArray<CRect>& regions)
{
  CRect client;
  GetClientRect(client);
  CSize fs = theApp.MeasureFont(this,GetFont());

  CRect labelRect;
  GetDlgItem(IDC_STATIC_NEWS)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  CRect regionRect(client);
  regionRect.bottom = labelRect.top - (fs.cy/2);
  regions.Add(regionRect);

  GetDlgItem(IDC_STATIC_OPEN_RECENT)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  regionRect.top = regionRect.bottom;
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

void WelcomeLauncherView::SetFonts(void)
{
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

  for (int id = IDC_STATIC_NEWS; id <= IDC_STATIC_COMMUNITY; id++)
    GetDlgItem(id)->SetFont(&m_titleFont);
  for (int id = IDC_ADVICE_NEW; id <= IDC_SAMPLE_DISENCHANTMENT; id++)
  {
    CommandButton& cmd = m_cmds[id - IDC_ADVICE_NEW];

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
    case IDC_CREATE_PROJECT:
    case IDC_CREATE_EXTENSION:
    case IDC_LINK_IFDB_SRC:
      cmd.SetFont(&m_bigFont);
      break;
    case IDC_SAMPLE_ONYX:
    case IDC_SAMPLE_DISENCHANTMENT:
      cmd.SetFont(&m_bigFont);
      cmd.SetTabStop(theApp.MeasureText(&cmd,"Browse Inform projects ").cx);
      break;
    }
  }
}

void WelcomeLauncherView::SetLayout(void)
{
  m_cmds[IDC_LINK_IFTF - IDC_ADVICE_NEW].ChangeWidthForIcon();

  m_news.SetTabStop((int)(m_newsTabPerDpi * DPI::getWindowDPI(this)));
  m_news.SetItemHeight(0,(int)(1.2 * theApp.MeasureFont(this,GetFont()).cy));

  // Resize the news list. As this will always be an integral number of elements high,
  // if we don't resize it here it will keep getting shorter on every DPI change.
  CArray<CRect> regions;
  GetRegions(regions);
  CRect newsRect;
  m_news.GetWindowRect(newsRect);
  ScreenToClient(newsRect);
  newsRect.bottom = regions[1].bottom-1;
  m_news.MoveWindow(newsRect,TRUE);
}

void WelcomeLauncherView::ShowHtml(bool show)
{
  for (int id = IDC_STATIC_NEWS; id <= IDC_NEWS; id++)
  {
    if ((id >= m_recentProjects.GetSize() + IDC_OPEN_0) && (id < RECENT_MAX + IDC_OPEN_0))
      continue;
    if (id == IDC_STATIC_SUMMON)
      continue;
    GetDlgItem(id)->ShowWindow(show ? SW_HIDE : SW_SHOW);
  }
  m_html.ShowWindow(show ? SW_SHOW : SW_HIDE);
}

CString WelcomeLauncherView::GetToolTip(UINT_PTR id)
{
  if (id == IDC_NEWS)
    return GetUrl(id,m_news.GetHotIndex());
  return GetUrl(id,-1);
}

CString WelcomeLauncherView::GetUrl(UINT_PTR id, int index)
{
  if (id == IDC_NEWS)
  {
    if ((index >= 0) && (index < (int)m_newsUrls.size()))
      return m_newsUrls[index].c_str();
    return "";
  }

  static char* urls[] =
  {
    "https://iftechfoundation.org/",
    "http://www.inform7.com",
    "https://ifdb.org/search?sortby=ratu&newSortBy.x=0&newSortBy.y=0&searchfor=system%3AInform+7",
    "https://www.intfiction.org/forum/viewforum.php?f=7",
    "https://ifwiki.org/index.php/Main_Page",
    "https://planet-if.com/",
    "https://ifcomp.org",
    "https://github.com/ganelson/inform",
    "https://ifdb.org/search?sortby=new&newSortBy.x=0&newSortBy.y=0&searchfor=tag%3A+i7+source+available"
  };

  UINT_PTR i = id - IDC_LINK_IFTF;
  if ((i >= 0) && (i < (sizeof urls / sizeof urls[0])))
    return urls[i];

  return "";
}

IMPLEMENT_DYNAMIC(WelcomeLauncherFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(WelcomeLauncherFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_CLOSE()
  ON_WM_SETTINGCHANGE()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_DARKMODE_ACTIVE, OnDarkModeActive)
END_MESSAGE_MAP()

WelcomeLauncherFrame::WelcomeLauncherFrame()
{
  m_dark = NULL;
}

WelcomeLauncherFrame::~WelcomeLauncherFrame()
{
  if (m_dark)
  {
    delete m_dark;
    m_dark = NULL;
  }
}

void WelcomeLauncherFrame::DownloadNews(void)
{
  CRegKey registryKey;
  if (registryKey.Create(HKEY_CURRENT_USER,REGISTRY_INFORM "\\Welcome") == ERROR_SUCCESS)
  {
    SYSTEMTIME sysTime;
    ::GetLocalTime(&sysTime);
    DWORD today = (sysTime.wYear * 10000) + (sysTime.wMonth * 100) + sysTime.wDay;

    // Only download once a day
    DWORD last = 0;
    if (registryKey.QueryDWORDValue("Last News Download",last) == ERROR_SUCCESS)
    {
      if (today <= last)
      {
        // Does the file exist?
        if (::GetFileAttributes(theApp.GetHomeDir()+"\\Inform\\cal.txt") != INVALID_FILE_ATTRIBUTES)
          return;
      }
    }
    registryKey.SetDWORDValue("Last News Download",today);
  }

  AfxBeginThread(DownloadThread,AfxGetApp());
}

// Implementation of IBindStatusCallback used to wait for the downloading of
// news from the IFTF to complete
class NewsDownload : public IBindStatusCallback
{
public:
  bool Done(void)
  {
    return m_done;
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj)
  {
    if (ppvObj == NULL)
      return E_INVALIDARG;

    if ((riid == IID_IUnknown) || (riid == IID_IBindStatusCallback))
    {
      *ppvObj = (LPVOID)this;
      AddRef();
      return S_OK;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef(void)
  {
    return 1;
  }

  ULONG STDMETHODCALLTYPE Release(void)
  {
    return 1;
  }

  HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD, IBinding*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE GetPriority(LONG*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnLowResource(DWORD)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnProgress(ULONG, ULONG, ULONG statusCode, LPCWSTR)
  {
    if (statusCode == BINDSTATUS_ENDDOWNLOADDATA)
      m_done = true;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT, LPCWSTR)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD* grfBINDF, BINDINFO*)
  {
    if (grfBINDF)
    {
      // Always download, never use cache
      *grfBINDF |= BINDF_GETNEWESTVERSION;
      *grfBINDF &= ~BINDF_GETFROMCACHE_IF_NET_FAIL;
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*)
  {
    return E_NOTIMPL;
  }

  HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID, IUnknown*)
  {
    return E_NOTIMPL;
  }

private:
  bool m_done;
};

UINT WelcomeLauncherFrame::DownloadThread(LPVOID app)
{
  // Discard any cached version of the news file
  LPCSTR url = "https://iftechfoundation.org/calendar/cal.txt";
  ::DeleteUrlCacheEntry(url);

  // Download the news file
  CString downPath = theApp.GetHomeDir()+"\\Inform\\cal.txt";
  NewsDownload download;
  if (SUCCEEDED(::URLDownloadToFile(NULL,url,downPath,0,&download)))
  {
    if (download.Done())
      ((CWinThread*)app)->PostThreadMessage(WM_NEWSDOWNLOAD,0,0);
  }

  return 0;
}

void WelcomeLauncherFrame::ShowLauncher(void)
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
      WelcomeLauncherFrame* frame = (WelcomeLauncherFrame*)frames[i];
      frame->m_view.UpdateRecent();
      frame->Invalidate();
      frame->ActivateFrame();
      return;
    }
  }

  // Open a new launcher
  WelcomeLauncherFrame* frame = new WelcomeLauncherFrame;
  theApp.NewFrame(frame);
  frame->LoadFrame(IDR_LAUNCHFRAME,WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,NULL,NULL);
  frame->Resize(true);

  BOOL cues;
  if (::SystemParametersInfo(SPI_GETKEYBOARDCUES,0,&cues,0) == 0)
    cues = TRUE;
  frame->SendMessage(WM_CHANGEUISTATE,MAKEWPARAM(cues ? UIS_CLEAR : UIS_SET,UISF_HIDEFOCUS));

  frame->ShowWindow(SW_SHOW);
  frame->UpdateWindow();
}

void WelcomeLauncherFrame::UpdateNews(void)
{
  m_view.UpdateNews();
}

void WelcomeLauncherFrame::SetDarkMode(DarkMode* dark)
{
  if (m_dark)
    delete m_dark;
  m_dark = dark;

  DarkMode::Set(this,dark);
  m_view.SetDarkMode(dark);
}

void WelcomeLauncherFrame::SendChanged(InformApp::Changed changed, int value)
{
  if (changed == InformApp::LightDarkMode)
    SetDarkMode(DarkMode::GetEnabled(REGISTRY_INFORM));
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
  if (!m_view.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,AFX_IDW_PANE_FIRST))
  {
    TRACE("Failed to create welcome launcher view\n");
    return -1;
  }
  m_view.SetFocus();

  // Set the application icon
  theApp.SetIcon(this);

  // Turn on dark mode, if needed
  SetDarkMode(DarkMode::GetEnabled(REGISTRY_INFORM));

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

void WelcomeLauncherFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
  CFrameWnd::OnSettingChange(uFlags,lpszSection);

  if ((m_dark != NULL) != DarkMode::IsEnabled(REGISTRY_INFORM))
    theApp.SendAllFrames(InformApp::LightDarkMode,0);
}

LRESULT WelcomeLauncherFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  MoveWindow((LPRECT)lparam,TRUE);
  m_view.UpdateDPI();
  ReportHtml::UpdateWebBrowserPreferences(this);
  Resize(false);
  Invalidate();
  return 0;
}

LRESULT WelcomeLauncherFrame::OnDarkModeActive(WPARAM, LPARAM)
{
  return (LRESULT)m_dark;
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
