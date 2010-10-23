#include "stdafx.h"
#include "SplashScreen.h"
#include "Inform.h"
#include "ProjectFrame.h"

extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);

IMPLEMENT_DYNAMIC(SplashScreen, I7BaseDialog)

SplashScreen::SplashScreen(CWnd* pParent) : I7BaseDialog(SplashScreen::IDD,TRUE,pParent)
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
END_MESSAGE_MAP()

void SplashScreen::ShowSplash(void)
{
  // Does the user want the splash screen?
  int last = theApp.GetProfileInt("Start","Open Last Project",0);
  if (last != 0)
  {
    // Open the last project, if possible
    if (::GetFileAttributes(theApp.GetLastProjectDir()) != INVALID_FILE_ATTRIBUTES)
    {
      if (ProjectFrame::StartLastProject())
        return;
    }
  }

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
    "To begin writing, please click one of the three buttons to the right."
    "\n\n"
    "Inform 7 was created by Graham Nelson, with the help of Emily Short and "
    "many others. The Windows front-end was written by David Kinder.");

  // Create a font for the buttons
  LOGFONT fontInfo;
  ::ZeroMemory(&fontInfo,sizeof fontInfo);
  GetFont()->GetLogFont(&fontInfo);
  fontInfo.lfWeight = FW_BOLD;
  m_buttonFont.CreateFontIndirect(&fontInfo);
  m_newProject.SetFont(&m_buttonFont);
  m_reopenLast.SetFont(&m_buttonFont);
  m_openProject.SetFont(&m_buttonFont);

  // Does the last project exist?
  if (::GetFileAttributes(theApp.GetLastProjectDir()) == INVALID_FILE_ATTRIBUTES)
    m_reopenLast.EnableWindow(FALSE);

  if (theApp.GetColourDepth() >= 16)
  {
    CRect client;
    GetClientRect(client);

    // Get the unscaled background
    CDibSection* back = theApp.GetCachedImage("Welcome Background");

    // Create a bitmap for the scaled background
    CDC* dc = GetDesktopWindow()->GetDC();
    m_back.CreateBitmap(dc->GetSafeHdc(),client.Width(),client.Height());
    GetDesktopWindow()->ReleaseDC(dc);

    // Scale and stretch the background
    ScaleGfx(back->GetBits(),back->GetSize().cx,back->GetSize().cy,
      m_back.GetBits(),client.Width(),client.Height());
  }

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
