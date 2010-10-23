#pragma once

#include "TabBase.h"
#include "ReportHtml.h"
#include "SearchWindow.h"

class TabDoc : public TabBase, public SearchWindow::Source
{
  DECLARE_DYNAMIC(TabDoc)

public:
  TabDoc();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void CompileProject(CompileStage stage, int code);

  void Show(const char* url);
  void SetFocusFlag(bool set);

  // Implementation of SearchWindow::Source
  void Search(LPCWSTR text, std::vector<SearchWindow::Result>& results);
  void Highlight(const SearchWindow::Result& result);
  CString Description(void);
  CRect WindowRect(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnContents();
  afx_msg LRESULT OnUserNavigate(WPARAM, LPARAM);

  void GetTabState(TabState& state);
  CString GetToolTip(UINT_PTR id);

private:
  CButton m_contents;

  struct DocText
  {
    DocText();
    void AddToBody(WCHAR ch, bool inExample);

    bool example;
    CString section;
    CString title;
    CString sort;
    CStringW body;
    CString file;
  };

  bool DecodeHTML(const char* file, DocText& docText);

  ReportHtml* m_html;
  bool m_initialised;

  static CArray<DocText> m_docTexts;
};
