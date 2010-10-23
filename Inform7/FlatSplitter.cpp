#include "stdafx.h"
#include "FlatSplitter.h"
#include "Inform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(FlatSplitter, CSplitterWnd)

BEGIN_MESSAGE_MAP(FlatSplitter, CSplitterWnd)
  ON_WM_SETCURSOR()
END_MESSAGE_MAP()

FlatSplitter::FlatSplitter()
{
  m_cxSplitter = m_cySplitter = 5;
  m_cxBorderShare = m_cyBorderShare = 0;
  m_cxSplitterGap = m_cySplitterGap = 5;
  m_cxBorder = m_cyBorder = 1;
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
  if ((nType != splitBorder) || (pDC == NULL))
    CSplitterWnd::OnDrawSplitter(pDC,nType,rect);
  else
    pDC->Draw3dRect(rect,::GetSysColor(COLOR_BTNSHADOW),::GetSysColor(COLOR_BTNHIGHLIGHT));
}
