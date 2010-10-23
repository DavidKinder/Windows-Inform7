#pragma once

#include "SearchEdit.h"

class SearchBar : public CDialogBar
{
  DECLARE_DYNAMIC(SearchBar)

public:
  SearchBar();

  void SearchSource(void);
  void SearchDocs(void);

  BOOL Create(CWnd* parent, UINT style, UINT id);
  int OnToolHitTest(CPoint point, TOOLINFO* ti) const;
  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* dc);
  afx_msg void OnMove(int, int);

  BOOL DialogBar_Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName, UINT nStyle, UINT nID);
  BOOL CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd);
  BOOL CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate, CWnd* pParentWnd, HINSTANCE hInst);

  SearchEdit m_source;
  SearchEdit m_docs;
};
