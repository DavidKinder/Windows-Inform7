#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "Inform.h"
#include "TextFormat.h"
#include "DpiFunctions.h"
#include "ScaleGfx.h"

//XXXXDK DPI,keyboard,accessibility

IMPLEMENT_DYNAMIC(WelcomeLauncher, I7BaseDialog)

WelcomeLauncher::WelcomeLauncher(CWnd* pParent) : I7BaseDialog(WelcomeLauncher::IDD,pParent), m_original(NULL)
{
}

void WelcomeLauncher::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(WelcomeLauncher, I7BaseDialog)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONUP()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_COMMAND_RANGE(IDC_ADVICE_NEW, IDC_ADVICE_CREDITS, OnClickedAdvice)
  ON_COMMAND_RANGE(IDC_LINK_INFORM7, IDC_LINK_IFDB_SRC, OnClickedLink)
END_MESSAGE_MAP()

void WelcomeLauncher::ShowLauncher(void)
{
  // Show the window
  DoModal();
  theApp.m_pMainWnd = NULL;//XXXXDK
}

BOOL WelcomeLauncher::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  // If there is no main window, make this dialog it
  if (AfxGetMainWnd() == NULL)
    theApp.m_pMainWnd = this;

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
  for (int id = IDC_LINK_IFDB_SRC; id <= IDC_LINK_IFDB_SRC; id++)
    GetDlgItem(id)->SetFont(&m_bigFont);
  for (int id = IDC_STATIC_OPEN_RECENT; id <= IDC_STATIC_COMMUNITY; id++)
    GetDlgItem(id)->SetFont(&m_titleFont);

  // Subclass links
  for (int id = IDC_ADVICE_NEW; id <= IDC_LINK_IFDB_SRC; id++)
  {
    CommandButton& link = m_links[id - IDC_ADVICE_NEW];
    link.SubclassDlgItem(id,this);
    link.SetBackSysColor((id >= IDC_LINK_IFDB_SRC) ? COLOR_BTNFACE : COLOR_WINDOW);
  }

  // Get the unscaled banner
  m_original = theApp.GetCachedImage("Welcome Banner");
  ASSERT(m_original != NULL);

  // Create the scaled banner
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
  SetBannerBitmap();
  return 0;
}

LRESULT WelcomeLauncher::OnKickIdle(WPARAM, LPARAM)
{
  ReportHtml::DoWebBrowserWork();
  return 0;
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
  if (m_original)
  {
    // Create a bitmap for the scaled banner
    CArray<CRect> regions;
    GetRegions(regions);
    CRect scaled = regions.GetAt(0);
    CDC* dc = GetDC();
    m_banner.DeleteBitmap();
    m_banner.CreateBitmap(dc->GetSafeHdc(),scaled.Width(),scaled.Height());
    ReleaseDC(dc);

    // Scale and stretch the background
    CSize originalSize = m_original->GetSize();
    ScaleGfx(m_original->GetBits(),originalSize.cx,originalSize.cy,
      m_banner.GetBits(),scaled.Width(),scaled.Height());
  }
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
  for (int id = IDC_STATIC_OPEN_RECENT; id <= IDC_LINK_IFDB_SRC; id++)
  {
    if (id != IDC_STATIC_SUMMON)
      GetDlgItem(id)->ShowWindow(show ? SW_HIDE : SW_SHOW);
  }
  m_html.ShowWindow(show ? SW_SHOW :SW_HIDE);
}
