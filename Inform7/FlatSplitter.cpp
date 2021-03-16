#include "stdafx.h"
#include "FlatSplitter.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(FlatSplitter, CSplitterWnd)

BEGIN_MESSAGE_MAP(FlatSplitter, CSplitterWnd)
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

FlatSplitter::FlatSplitter(bool border)
{
  m_cxSplitter = m_cySplitter = 5;
  m_cxBorderShare = m_cyBorderShare = 0;
  m_cxSplitterGap = m_cySplitterGap = 5;
  m_cxBorder = m_cyBorder = (border ? 1 : 0);
}

void FlatSplitter::SetRows(int rows)
{
  if (m_nRows != rows)
  {
    m_nRows = rows;
    RecalcLayout();
  }
}

double FlatSplitter::GetColumnFraction(int col)
{
  CRect client;
  GetClientRect(client);
  int size, minSize;
  GetColumnInfo(col,size,minSize);
  return (double)size / (double)client.Width();
}

void FlatSplitter::SetColumnFraction(int col, double fraction, int min)
{
  CRect client;
  GetClientRect(client);
  SetColumnInfo(col,(int)(fraction*client.Width()),min);
}

BOOL FlatSplitter::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  if (theApp.IsWaitCursor())
  {
    theApp.RestoreWaitCursor();
    return TRUE;
  }
  return CSplitterWnd::OnSetCursor(pWnd,nHitTest,message);
}

void FlatSplitter::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
{
  if ((pDC != NULL) && (nType == splitBorder))
  {
    if (m_cxBorder > 0)
    {
      COLORREF face = ::GetSysColor(COLOR_BTNFACE);
      pDC->Draw3dRect(rect,face,face);
    }
  }
  else
    CSplitterWnd::OnDrawSplitter(pDC,nType,rect);
}
