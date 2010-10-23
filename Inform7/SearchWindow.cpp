#include "stdafx.h"
#include "SearchWindow.h"
#include "Inform.h"
#include "OSLayer.h"

#include <MultiMon.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SearchWindow, CFrameWnd)

BEGIN_MESSAGE_MAP(SearchWindow, CFrameWnd)
  ON_WM_CLOSE()
  ON_WM_SIZE()
  ON_WM_ACTIVATE()
  ON_NOTIFY(NM_DBLCLK, IDC_SEARCH_RESULTS, OnResultsSelect)
  ON_NOTIFY(NM_RETURN, IDC_SEARCH_RESULTS, OnResultsSelect)
  ON_NOTIFY(NM_CUSTOMDRAW, IDC_SEARCH_RESULTS, OnResultsDraw)
  ON_NOTIFY(LVN_ENDSCROLL, IDC_SEARCH_RESULTS, OnResultsEndScroll)
END_MESSAGE_MAP()

SearchWindow::SearchWindow() : m_shown(false), m_searching(false), m_source(NULL), m_parent(NULL)
{
}

BOOL SearchWindow::Create(CWnd* parent)
{
  m_parent = parent;
  if (CFrameWnd::Create(NULL,"Search results",
    WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME,
    rectDefault,parent,NULL,WS_EX_TOOLWINDOW) == FALSE)
  {
    return FALSE;
  }

  if (m_resultsList.Create(
    WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL|LVS_NOSORTHEADER,
    CRect(0,0,0,0),this,IDC_SEARCH_RESULTS) == FALSE)
  {
    return FALSE;
  }
  m_resultsList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
  m_resultsList.InsertColumn(0,"Context");
  m_resultsList.InsertColumn(1,"Location");

  m_font.CreatePointFont(theApp.GetDialogFontSize(),theApp.GetFontName());
  m_resultsList.SetFont(&m_font);

  theApp.SetIcon(this);
  return TRUE;
}

BOOL SearchWindow::PreTranslateMessage(MSG* pMsg)
{
  if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_ESCAPE))
    PostMessage(WM_CLOSE);
  return CFrameWnd::PreTranslateMessage(pMsg);
}

void SearchWindow::PostNcDestroy()
{
  // Do nothing
}

void SearchWindow::OnClose()
{
  ShowWindow(SW_HIDE);
}

void SearchWindow::OnSize(UINT nType, int cx, int cy)
{
  CFrameWnd::OnSize(nType,cx,cy);

  CRect client;
  GetClientRect(client);

  m_resultsList.MoveWindow(client);
}

void SearchWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
  CFrameWnd::OnActivate(nState,pWndOther,bMinimized);

  if ((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE))
    m_resultsList.SetFocus();
}

void SearchWindow::OnResultsSelect(NMHDR* pNotifyStruct, LRESULT* result)
{
  int item = m_resultsList.GetNextItem(-1,LVNI_SELECTED);
  if ((item >= 0) && (item < (int)m_results.size()))
    m_source->Highlight(m_results[item]);
  *result = 0;
}

void SearchWindow::OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
  // Default to letting Windows draw the control
  *result = CDRF_DODEFAULT;

  // Work out where we are in the drawing process
  NMLVCUSTOMDRAW* custom = (NMLVCUSTOMDRAW*)pNotifyStruct;
  switch (custom->nmcd.dwDrawStage)
  {
  case CDDS_PREPAINT:
    // Tell us when an item is drawn
    *result = CDRF_NOTIFYITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT:
    // Tell us when a sub-item is drawn
    *result = CDRF_NOTIFYSUBITEMDRAW;
    break;
  case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
    // Override painting of the first column
    if (custom->iSubItem == 0)
    {
      // Get the item text
      int item = (int)custom->nmcd.dwItemSpec;
      CStringW text = (LPCWSTR)m_resultsList.GetItemData(item);

      // Get if the item is selected
      bool selected = false;
      if (GetFocus() == &m_resultsList)
      {
        if (item == m_resultsList.GetNextItem(-1,LVNI_SELECTED))
          selected = true;
      }

      // Get the bounding rectangle for drawing the text
      CRect rect;
      m_resultsList.GetSubItemRect(item,custom->iSubItem,LVIR_LABEL,rect);

      // Set up the device context
      CDC* dc = CDC::FromHandle(custom->nmcd.hdc);
      dc->SetTextColor(::GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
      dc->SetBkMode(TRANSPARENT);

      // Draw the background
      rect.bottom--;
      dc->FillSolidRect(rect,::GetSysColor(selected ? COLOR_HIGHLIGHT : COLOR_WINDOW));
      rect.bottom++;

      // Create a bold font
      LOGFONT logFont;
      m_resultsList.GetFont()->GetLogFont(&logFont);
      logFont.lfWeight = FW_BOLD;
      CFont boldFont;
      boldFont.CreateFontIndirect(&logFont);

      // Get position information on the text
      CRect textRect = rect;
      textRect.DeflateRect(2,0);
      int high1 = m_results[item].inContext.cpMin;
      int high2 = m_results[item].inContext.cpMax;

      // Draw the text
      DrawText(dc,text.GetString(),high1,textRect,DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
      CFont* oldFont = dc->SelectObject(&boldFont);
      DrawText(dc,text.GetString()+high1,high2-high1,textRect,DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
      dc->SelectObject(oldFont);
      DrawText(dc,text.GetString()+high2,text.GetLength()-high2,textRect,
        DT_VCENTER|DT_SINGLELINE|DT_WORD_ELLIPSIS|DT_NOPREFIX);

      // Draw the focus rectangle
      if (selected)
      {
        CRgn clip;
        ::GetClipRgn(dc->GetSafeHdc(),clip);
        dc->IntersectClipRect(rect);
        rect.right += 8;
        rect.bottom += 8;
        dc->DrawFocusRect(rect);
        dc->SelectClipRgn(&clip);
      }

      *result = CDRF_SKIPDEFAULT;
    }
    break;
  }
}

void SearchWindow::OnResultsEndScroll(NMHDR* pNotifyStruct, LRESULT* result)
{
  // Force a redraw to work around Microsoft KB813791
  m_resultsList.Invalidate();
}

void SearchWindow::Search(Source* source, LPCWSTR text, CRect& windowRect)
{
  if (m_searching)
    return;

  // Do the search
  m_searching = true;
  std::vector<Result> results;
  source->Search(text,results);

  // Only save the results once the search is complete
  m_source = source;
  m_results.swap(results);
  m_searching = false;

  // Indicate if there were no results, and stop if the window isn't open
  if (m_results.empty())
  {
    ShowWindow(SW_HIDE);

    CStringW msg;
    msg.Format(L"Nothing was found when searching for '%s'",text);
    theOS.MessageBox(m_parent,msg,L"Inform",MB_ICONWARNING|MB_OK);
    return;
  }

  // If the window hasn't been shown before, size and position it
  if (m_shown == false)
  {
    MoveWindow(windowRect,FALSE);

    m_resultsList.SetColumnWidth(0,windowRect.Width()/2);
    m_resultsList.SetColumnWidth(1,LVSCW_AUTOSIZE_USEHEADER);

    // Make the window show an exact number of entries in the list
    CRect listRect;
    m_resultsList.GetClientRect(listRect);
    CSize size = m_resultsList.ApproximateViewRect(
      CSize(-1,-1),m_resultsList.GetCountPerPage());
    windowRect.bottom += size.cy-listRect.Height();
    MoveWindow(windowRect);
  }

  // Move the window so that it isn't over the source
  CRect currentRect, intersection;
  GetWindowRect(currentRect);
  CRect sourceRect = source->WindowRect();
  if (intersection.IntersectRect(currentRect,sourceRect))
  {
    MONITORINFO monInfo;
    ::ZeroMemory(&monInfo,sizeof monInfo);
    monInfo.cbSize = sizeof monInfo;

    HMONITOR mon = ::MonitorFromWindow(m_parent->GetSafeHwnd(),MONITOR_DEFAULTTOPRIMARY);
    if (::GetMonitorInfo(mon,&monInfo))
    {
      CRect screenRect(monInfo.rcWork);

      // Can it be moved to the right or left?
      if (sourceRect.right+currentRect.Width() < screenRect.right)
      {
        currentRect.MoveToX(sourceRect.right);
        MoveWindow(currentRect);
      }
      else if (sourceRect.left-currentRect.Width() >= screenRect.left)
      {
        currentRect.MoveToX(sourceRect.left-currentRect.Width());
        MoveWindow(currentRect);
      }
    }
  }

  // Update the results
  m_resultsList.DeleteAllItems();
  for (int i = 0; i < (int)m_results.size(); i++)
  {
    m_resultsList.InsertItem(i,"");
    m_resultsList.SetItemData(i,(DWORD_PTR)m_results[i].context.c_str());
    m_resultsList.SetItemText(i,1,m_results[i].sourceLocation.c_str());
  }

  // Set the window title
  CString title;
  title.Format("Search results in %s",(LPCTSTR)source->Description());
  AfxSetWindowText(GetSafeHwnd(),title);

  // Show the window
  ShowWindow(SW_SHOW);
  m_resultsList.SetFocus();
  m_shown = true;
}

void SearchWindow::DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format)
{
  if (length > 0)
  {
    theOS.DrawText(dc,text,length,rect,format|DT_SINGLELINE);

    CRect measure(rect);
    theOS.DrawText(dc,text,length,measure,format|DT_SINGLELINE|DT_CALCRECT);
    rect.left += measure.Width();
  }
}

SearchWindow::Result::Result()
{
  inContext.cpMin = 0;
  inContext.cpMax = 0;
  inSource.cpMin = 0;
  inSource.cpMax = 0;
}
