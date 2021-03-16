#pragma once

class FlatSplitter : public CSplitterWnd
{
  DECLARE_DYNAMIC(FlatSplitter)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

  virtual void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect);

public:
  FlatSplitter(bool border);
  void SetRows(int rows);

  double GetColumnFraction(int col);
  double GetRowFraction(int row);
  void SetColumnFraction(int col, double fraction, int min);
  void SetRowFraction(int row, double fraction, int min);
};
