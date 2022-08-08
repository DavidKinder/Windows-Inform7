#pragma once

#include "Skein.h"
#include "UnicodeEdit.h"

class SkeinEdit : public UnicodeEdit
{
  DECLARE_DYNAMIC(SkeinEdit)

public:
  SkeinEdit();
  virtual ~SkeinEdit();

  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

  void StartEdit(Skein::Node* node, const CRect& nodeRect);
  void EndEdit(void);

protected:
  DECLARE_MESSAGE_MAP()

  Skein::Node* m_node;
};
