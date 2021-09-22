#include "stdafx.h"
#include "WelcomeLauncher.h"
#include "Inform.h"
#include "ProjectFrame.h"
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
  ON_WM_ERASEBKGND()
  ON_WM_CTLCOLOR()
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

void WelcomeLauncher::ShowLauncher(void)
{
  // Show the window
  DoModal();
}

BOOL WelcomeLauncher::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  theApp.SetIcon(this);

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

  SetBackBitmap();
  return TRUE;
}

BOOL WelcomeLauncher::OnEraseBkgnd(CDC* pDC)
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

HBRUSH WelcomeLauncher::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  pDC->SetBkMode(TRANSPARENT);
  return (HBRUSH)::GetStockObject(NULL_BRUSH);
}

LRESULT WelcomeLauncher::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  Default();
  SetBackBitmap();
  return 0;
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
