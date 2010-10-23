#pragma once

class FlatSplitter : public CSplitterWnd
{
  DECLARE_DYNCREATE(FlatSplitter)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

public:
  FlatSplitter();

  virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);
};
