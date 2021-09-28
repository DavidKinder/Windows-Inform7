#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "Inform.h"
#include "TextFormat.h"
#include "DpiFunctions.h"
#include "ScaleGfx.h"

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
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
  ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
  ON_COMMAND_RANGE(IDC_ADVICE_NEW, IDC_ADVICE_CREDITS, OnClickedAdvice)
  ON_COMMAND_RANGE(IDC_LINK_INFORM7, IDC_LINK_IFCOMP, OnClickedLink)
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

  // Create an use a larger bold font
  LOGFONT titleFont;
  GetFont()->GetLogFont(&titleFont);
  titleFont.lfWeight = FW_BOLD;
  titleFont.lfHeight = (LONG)(titleFont.lfHeight*1.2);
  m_titleFont.DeleteObject();
  m_titleFont.CreateFontIndirect(&titleFont);
  const UINT staticIds[] = { IDC_STATIC_OPEN_RECENT,IDC_STATIC_ADVICE,IDC_STATIC_CREATE_NEW,IDC_STATIC_SAMPLE,IDC_STATIC_COMMUNITY };
  for (int i = 0; i < sizeof staticIds / sizeof staticIds[0]; i++)
    GetDlgItem(staticIds[i])->SetFont(&m_titleFont);

  // Subclass links
  ASSERT(IDC_ADVICE_NEW + (sizeof m_links / sizeof m_links[0]) == IDC_LINK_IFCOMP + 1);
  for (int i = 0; i < sizeof m_links / sizeof m_links[0]; i++)
  {
    m_links[i].SubclassDlgItem(IDC_ADVICE_NEW+i,this);
    m_links[i].SetBackSysColor(COLOR_WINDOW);
  }

  // Get the unscaled background
  m_original = theApp.GetCachedImage("Welcome Background@4x");
  ASSERT(m_original != NULL);
  CSize originalSize = m_original->GetSize();
/*
  // Adjust the dialog to the same aspect ratio as the background
  CRect client;
  GetClientRect(client);
  int heightAdjust =
    ((client.Width() * originalSize.cy) / originalSize.cx) - client.Height();
  CRect windowRect;
  GetWindowRect(windowRect);
  windowRect.bottom += heightAdjust;
  MoveWindow(windowRect,FALSE);
*/
  SetBackBitmap();
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

  pDC->FillSolidRect(regions.GetAt(0),RGB(255,0,0));
  pDC->FillSolidRect(regions.GetAt(3),::GetSysColor(COLOR_BTNFACE));

  if (m_html.IsWindowVisible() == FALSE)
  {
    pDC->FillSolidRect(regions.GetAt(1),::GetSysColor(COLOR_BTNFACE));
    pDC->FillSolidRect(regions.GetAt(2),::GetSysColor(COLOR_WINDOW));
  }

/*
  if (m_back.GetSafeHandle() != 0)
  {
    // Create a memory device context
    CDC dc;
    dc.CreateCompatibleDC(pDC);

    // Select the background into it
    CBitmap* oldBitmap = CDibSection::SelectDibSection(dc,&m_back);

    // Draw the background
    pDC->BitBlt(0,0,client.Width(),client.Height(),&dc,0,0,SRCCOPY);

    // Restore the original device context settings
    dc.SelectObject(oldBitmap);
  }
  else
    pDC->FillSolidRect(client,::GetSysColor(COLOR_BTNFACE));
*/

  return TRUE;
}

LRESULT WelcomeLauncher::OnDpiChanged(WPARAM, LPARAM)
{
  Default();
  SetBackBitmap();
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
      "http://ifcomp.org"
    };

    int i = nID - IDC_LINK_INFORM7;
    if ((i >= 0) && (i < (sizeof urls / sizeof urls[0])))
      ::ShellExecute(0,NULL,urls[i],NULL,NULL,SW_SHOWNORMAL);
  }
}

void WelcomeLauncher::SetBackBitmap(void)
{
  if (m_original)
  {
    // Create a bitmap for the scaled background
    CRect client;
    GetClientRect(client);
    CDC* dc = GetDC();
    m_back.DeleteBitmap();
    m_back.CreateBitmap(dc->GetSafeHdc(),client.Width(),client.Height());
    ReleaseDC(dc);

    // Scale and stretch the background
    CSize originalSize = m_original->GetSize();
    ScaleGfx(m_original->GetBits(),originalSize.cx,originalSize.cy,
      m_back.GetBits(),client.Width(),client.Height());
  }
}

void WelcomeLauncher::GetRegions(CArray<CRect>& regions)
{
  CRect client;
  GetClientRect(client);
  double scale = DPI::getWindowDPI(this) / 96.0;

  CRect labelRect;
  GetDlgItem(IDC_STATIC_OPEN_RECENT)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  CRect regionRect(client);
  regionRect.bottom = labelRect.top - (int)(8*scale);
  regions.Add(regionRect);

  GetDlgItem(IDC_STATIC_ADVICE)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = labelRect.top - (int)(8*scale);
  regions.Add(regionRect);

  GetDlgItem(IDC_STATIC_SUMMON)->GetWindowRect(labelRect);
  ScreenToClient(labelRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = labelRect.top - (int)(3*scale);
  regions.Add(regionRect);

  regionRect.top = regionRect.bottom;
  regionRect.bottom = client.bottom;
  regions.Add(regionRect);
}

void WelcomeLauncher::ShowHtml(bool show)
{
  for (int id = IDC_STATIC_OPEN_RECENT; id < IDC_LINK_IFCOMP; id++)
  {
    if (id != IDC_STATIC_SUMMON)
      GetDlgItem(id)->ShowWindow(show ? SW_HIDE : SW_SHOW);
  }
  m_html.ShowWindow(show ? SW_SHOW :SW_HIDE);
}
