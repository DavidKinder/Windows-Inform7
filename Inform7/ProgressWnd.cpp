#include "stdafx.h"
#include "ProgressWnd.h"
#include "Inform.h"

#include "DarkMode.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ProgressWnd, CWnd)

BEGIN_MESSAGE_MAP(ProgressWnd, CWnd)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_BN_CLICKED(IDC_STOP, OnStopClicked)
END_MESSAGE_MAP()

ProgressWnd::ProgressWnd() : m_longStep(0), m_longStepTotal(0), m_wantStop(false)
{
}

BOOL ProgressWnd::Create(CWnd* parentWnd, DWORD style)
{
  if (!CWnd::Create(NULL,"",WS_CHILD|WS_CLIPCHILDREN|style,CRect(0,0,0,0),parentWnd,0))
    return FALSE;
  if (!m_text.Create("",WS_CHILD|WS_VISIBLE|SS_CENTER,CRect(0,0,0,0),this))
    return FALSE;
  m_text.SetFont(theApp.GetFont(this,InformApp::FontSystem));
  if (!m_stop.Create(WS_CHILD,CRect(0,0,0,0),this,IDC_STOP))
    return FALSE;
  if (!m_progress.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,IDC_PROGRESS))
    return FALSE;
  return TRUE;
}

void ProgressWnd::UpdateDPI(void)
{
  m_text.SetFont(theApp.GetFont(this,InformApp::FontSystem));
  if (IsWindowVisible())
  {
    Resize();
    Invalidate();
  }
}

void ProgressWnd::SetDarkMode(DarkMode* dark)
{
  LPCWSTR theme = dark ? L"" : NULL;
  ::SetWindowTheme(m_progress.GetSafeHwnd(),theme,theme);
}

HBRUSH ProgressWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH brush = CWnd::OnCtlColor(pDC,pWnd,nCtlColor);
  if (nCtlColor == CTLCOLOR_STATIC)
  {
    DarkMode* dark = DarkMode::GetActive(this);
    if (dark)
    {
      brush = dark->GetBrush(DarkMode::Darkest);
      pDC->SetBkColor(dark->GetColour(DarkMode::Darkest));
      pDC->SetTextColor(dark->GetColour(DarkMode::Fore));
    }
  }
  return brush;
}

BOOL ProgressWnd::OnEraseBkgnd(CDC* dc)
{
  CRect r;
  GetClientRect(r);
  DarkMode* dark = DarkMode::GetActive(this);
  dc->FillSolidRect(r,dark ?
    dark->GetColour(DarkMode::Darkest) : ::GetSysColor(COLOR_BTNFACE));

  CBrush border;
  border.CreateSolidBrush(dark ?
    dark->GetColour(DarkMode::Dark2) : ::GetSysColor(COLOR_BTNSHADOW));
  dc->FrameRect(r,&border);
  return TRUE;
}

void ProgressWnd::OnStopClicked()
{
  m_wantStop = true;
}

void ProgressWnd::ToFront()
{
  if (IsWindowVisible())
    SetWindowPos(&CWnd::wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

int ProgressWnd::GetProgress()
{
  return m_progress.GetPos();
}

void ProgressWnd::TaskProgress(const char* text, int progress)
{
  CString newText;
  if (m_long.IsEmpty())
    newText = text;
  else
    newText.Format("%s: %s",(LPCSTR)m_long,text);

  CString currentText;
  m_text.GetWindowText(currentText);
  if (currentText != newText)
    m_text.SetWindowText(newText);

  if (m_longStepTotal > 0)
  {
    double step = m_longStep;
    double total = m_longStepTotal;
    m_progress.SetPos((int)((progress/total)+(100.0*step/total)));
  }
  else
    m_progress.SetPos(progress);

  if (IsWindowVisible())
    SetWindowPos(&CWnd::wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  else
  {
    Resize();
    AfxGetApp()->BeginWaitCursor();
  }
}

void ProgressWnd::TaskDone()
{
  if (m_long.IsEmpty())
  {
    if (IsWindowVisible())
    {
      ShowWindow(SW_HIDE);
      m_stop.ShowWindow(SW_HIDE);
      AfxGetApp()->EndWaitCursor();
      m_wantStop = false;
    }
  }
}

void ProgressWnd::LongTaskProgress(const char* text, int step, int stepTotal)
{
  m_long = text;
  m_longStep = step;
  m_longStepTotal = stepTotal;
}

void ProgressWnd::LongTaskAdvance()
{
  if (m_longStepTotal > 0)
    m_longStep++;
}

void ProgressWnd::LongTaskDone()
{
  m_long.Empty();
  m_longStep = 0;
  m_longStepTotal = 0;
  TaskDone();
}

void ProgressWnd::ShowStop()
{
  m_stop.ShowWindow(SW_SHOW);
}

bool ProgressWnd::WantStop()
{
  if (IsWindowVisible())
  {
    theApp.RunMessagePump();
    return m_wantStop;
  }
  else
    return false;
}

void ProgressWnd::Resize(void)
{
  // Get the width of the monitor
  int monWidth = DPI::getMonitorWorkRect(this).Width();

  // Get the width of the parent window
  CWnd* parentWnd = GetParentFrame();
  CRect parentRect;
  parentWnd->GetWindowRect(parentRect);
  parentWnd->ScreenToClient(parentRect);
  int parentWidth = parentRect.Width();

  // Work out the size of the progress window
  int width = parentWidth * 3/4;
  if (width > monWidth / 3)
    width = monWidth / 3;
  CSize fs = theApp.MeasureFont(this,theApp.GetFont(this,InformApp::FontSystem));
  int height = fs.cy * 11/2;

  // Resize the progress window and its controls
  SetWindowPos(&CWnd::wndTop,
    parentRect.left+((parentRect.Width()-width)/2),
    parentRect.top+((parentRect.Height()-height)/2),
    width,height,SWP_SHOWWINDOW);
  CSize ssz = m_stop.GetButtonSize();
  m_text.MoveWindow((fs.cx*2)+ssz.cx,fs.cy,width-(fs.cx*4)-(ssz.cx*2),fs.cy*3/2,TRUE);
  m_stop.MoveWindow(width-(fs.cx*2)-ssz.cx,fs.cy,ssz.cx,ssz.cy,TRUE);
  m_progress.MoveWindow(fs.cx*2,fs.cy*3,width-(fs.cx*4),fs.cy*3/2,TRUE);
}
