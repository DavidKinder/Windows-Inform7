#include "stdafx.h"
#include "SplashScreen.h"
#include "Inform.h"
#include "ProjectFrame.h"
#include "DpiFunctions.h"
#include "ScaleGfx.h"

IMPLEMENT_DYNAMIC(SplashScreen, I7BaseDialog)

SplashScreen::SplashScreen(CWnd* pParent) : I7BaseDialog(SplashScreen::IDD,pParent), m_original(NULL)
{
}

void SplashScreen::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_NEW_PROJECT, m_newProject);
  DDX_Control(pDX, IDC_REOPEN_LAST, m_reopenLast);
  DDX_Control(pDX, IDC_OPEN_PROJECT, m_openProject);
}

BEGIN_MESSAGE_MAP(SplashScreen, I7BaseDialog)
  ON_WM_ERASEBKGND()
  ON_WM_CTLCOLOR()
  ON_BN_CLICKED(IDC_NEW_PROJECT, OnNewProject)
  ON_BN_CLICKED(IDC_REOPEN_LAST, OnReopenLast)
  ON_BN_CLICKED(IDC_OPEN_PROJECT, OnOpenProject)
  ON_WM_SHOWWINDOW()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

void SplashScreen::ShowSplash(void)
{
  // Show the splash screen
  DoModal();
}

BOOL SplashScreen::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

  m_intro.SubclassDlgItem(IDC_INTRO,this);
  m_intro.SetWindowText(
    "Welcome to Inform 7, a design system for interactive fiction based on "
    "natural language."
    "\n\n"
    "Inform 7 was created by Graham Nelson, with the help of Emily Short and "
    "many others. The Windows front-end was written by David Kinder.");

  // Does the last project exist?
  if (::GetFileAttributes(theApp.GetLastProjectDir()) == INVALID_FILE_ATTRIBUTES)
    m_reopenLast.EnableWindow(FALSE);

  // Get the unscaled background
  m_original = theApp.GetCachedImage("Welcome Background@4x");
  ASSERT(m_original != NULL);
  CSize originalSize = m_original->GetSize();

  // Adjust the dialog to the same aspect ratio as the background
  CRect client;
  GetClientRect(client);
  int heightAdjust =
    ((client.Width() * originalSize.cy) / originalSize.cx) - client.Height();
  CRect windowRect;
  GetWindowRect(windowRect);
  windowRect.bottom += heightAdjust;
  MoveWindow(windowRect,FALSE);

  // Adjust the button positions
  CWnd* btns[3];
  btns[0] = &m_newProject;
  btns[1] = &m_reopenLast;
  btns[2] = &m_openProject;
  for (int i = 0; i < (sizeof btns / sizeof btns[0]); i++)
  {
    CRect btnRect;
    btns[i]->GetWindowRect(btnRect);
    ScreenToClient(btnRect);
    btnRect.top += heightAdjust;
    btnRect.bottom += heightAdjust;
    btns[i]->MoveWindow(btnRect,FALSE);
  }

  SetButtonFont();
  SetBackBitmap();
  return TRUE;
}

BOOL SplashScreen::OnEraseBkgnd(CDC* pDC)
{
  CRect client;
  GetClientRect(client);

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

  return TRUE;
}

HBRUSH SplashScreen::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  pDC->SetBkMode(TRANSPARENT);
  return (HBRUSH)::GetStockObject(NULL_BRUSH);
}

void SplashScreen::OnShowWindow(BOOL bShow, UINT nStatus)
{
  I7BaseDialog::OnShowWindow(bShow,nStatus);

  if (bShow)
  {
    // Make the "Open Project" button the default
    m_newProject.SetButtonStyle(m_newProject.GetButtonStyle() & ~BS_DEFPUSHBUTTON);
    SetDefID(IDC_OPEN_PROJECT);
    m_openProject.SetFocus();
  }
}

LRESULT SplashScreen::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  Default();

  SetButtonFont();
  SetBackBitmap();
  return 0;
}

void SplashScreen::OnNewProject()
{
  if (ProjectFrame::StartNewProject(theApp.GetLastProjectDir(),this))
    EndDialog(IDOK);
}

void SplashScreen::OnReopenLast()
{
  if (ProjectFrame::StartLastProject())
    EndDialog(IDOK);
}

void SplashScreen::OnOpenProject()
{
  if (ProjectFrame::StartExistingProject(theApp.GetLastProjectDir(),this))
    EndDialog(IDOK);
}

void SplashScreen::SetButtonFont(void)
{
  if (m_newProject.GetSafeHwnd() != 0)
  {
    m_buttonFont.DeleteObject();

    LOGFONT fontInfo;
    ::ZeroMemory(&fontInfo,sizeof fontInfo);
    GetFont()->GetLogFont(&fontInfo);
    fontInfo.lfWeight = FW_BOLD;
    m_buttonFont.CreateFontIndirect(&fontInfo);
    m_newProject.SetFont(&m_buttonFont);
    m_reopenLast.SetFont(&m_buttonFont);
    m_openProject.SetFont(&m_buttonFont);
  }
}

void SplashScreen::SetBackBitmap(void)
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
