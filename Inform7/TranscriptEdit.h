#pragma once

#include "Skein.h"
#include "RichEdit.h"

class TranscriptEdit : public RichEdit
{
  DECLARE_DYNAMIC(TranscriptEdit)

public:
  TranscriptEdit();

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

  void StartEdit(Skein::Node* node, const CRect& nodeRect, CPoint point);
  void EndEdit(void);

protected:
  DECLARE_MESSAGE_MAP()

  Skein::Node* m_node;
};
