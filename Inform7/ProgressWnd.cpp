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

void ProgressWnd::ShowProgress(const char* text, int progress)
{
  CString currentText;
  m_text.GetWindowText(currentText);
  if (currentText != text)
    m_text.SetWindowText(text);

  m_progress.SetPos(progress);

  if (!IsWindowVisible())
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
  }
}
