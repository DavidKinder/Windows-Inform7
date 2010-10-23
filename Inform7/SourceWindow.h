#pragma once

#include "SourceEdit.h"
#include "Dib.h"
#include "Messages.h"

class SourceWindow : public CWnd
{
  DECLARE_DYNAMIC(SourceWindow)

public:
  SourceWindow();

  void Create(CWnd* parent);
  SourceEdit& GetEdit(void);

  const SourceHeading& GetHeading(void);
  void GetAllHeadings(CArray<SourceLexer::Heading>& headings);
  void ShowBetween(int startLine, int endLine, const SourceHeading* heading);

  void Highlight(int line, COLORREF colour, bool centre);
  void Highlight(CHARRANGE range, bool centre);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnEditSetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnEditGetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result);

  afx_msg LRESULT OnPrint(WPARAM, LPARAM);

private:
  void Resize(void);
  void Draw(CDC& dc);
  CRect PaintEdge(CDC& dcPaint, int y, int w, CDibSection* image, bool top);

  SourceEdit m_edit;
  SourceHeading m_heading;

  bool m_tearTop, m_tearBottom;
  CRect m_arrowTop, m_arrowBottom;

  CDibSection* m_imageTop;
  CDibSection* m_imageBottom;
};
