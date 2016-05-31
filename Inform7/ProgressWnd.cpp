#include "stdafx.h"
#include "ProgressWnd.h"
#include "Inform.h"

#include <MultiMon.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ProgressWnd, CWnd)

BEGIN_MESSAGE_MAP(ProgressWnd, CWnd)
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

ProgressWnd::ProgressWnd() : m_longStep(0), m_longStepTotal(0)
{
}

BOOL ProgressWnd::Create(CWnd* parentWnd, DWORD style)
{
  if (!CWnd::Create(NULL,"",WS_CHILD|WS_CLIPCHILDREN|style,CRect(0,0,0,0),parentWnd,0))
    return FALSE;
  if (!m_text.Create("",WS_CHILD|WS_VISIBLE|SS_CENTER,CRect(0,0,0,0),this))
    return FALSE;
  m_text.SetFont(theApp.GetFont(InformApp::FontSystem));
  if (!m_progress.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,IDC_PROGRESS))
    return FALSE;
  return TRUE;
}

BOOL ProgressWnd::OnEraseBkgnd(CDC* dc)
{
  CRect r;
  GetClientRect(r);
  dc->DrawEdge(r,EDGE_RAISED,BF_ADJUST|BF_RECT);
  dc->FillSolidRect(r,::GetSysColor(COLOR_BTNFACE));
  return TRUE;
}

void ProgressWnd::ToFront()
{
  if (IsWindowVisible())
    SetWindowPos(&CWnd::wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
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
    // Get the width of the monitor
    int monWidth = 0;
    MONITORINFO monInfo;
    ::ZeroMemory(&monInfo,sizeof monInfo);
    monInfo.cbSize = sizeof monInfo;
    HMONITOR mon = ::MonitorFromWindow(GetSafeHwnd(),MONITOR_DEFAULTTOPRIMARY);
    if (::GetMonitorInfo(mon,&monInfo))
      monWidth = monInfo.rcWork.right - monInfo.rcWork.left;
    else
      monWidth = ::GetSystemMetrics(SM_CXSCREEN);

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
    CSize fs = theApp.MeasureFont(theApp.GetFont(InformApp::FontSystem));
    int height = fs.cy * 11/2;

    SetWindowPos(&CWnd::wndTop,
      parentRect.left+((parentRect.Width()-width)/2),
      parentRect.top+((parentRect.Height()-height)/2),
      width,height,SWP_SHOWWINDOW);
    m_text.MoveWindow(fs.cx*2,fs.cy,width-(fs.cx*4),fs.cy*3/2,TRUE);
    m_progress.MoveWindow(fs.cx*2,fs.cy*3,width-(fs.cx*4),fs.cy*3/2,TRUE);
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
      AfxGetApp()->EndWaitCursor();
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
