#pragma once

#include <string>
#include <vector>

class SearchWindow : public CFrameWnd
{
  DECLARE_DYNAMIC(SearchWindow)

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual void PostNcDestroy();

  afx_msg void OnClose();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnResultsSelect(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnResultsEndScroll(NMHDR* pNotifyStruct, LRESULT* result);

  CListCtrl m_resultsList;
  CFont m_font;
  bool m_shown;

public:
  SearchWindow();
  BOOL Create(CWnd* parent);
  void DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format);

  struct Result
  {
    Result();

    std::wstring context;
    CHARRANGE inContext;

    std::string sourceSort;
    std::string sourceLocation;
    std::string sourceFile;
    CHARRANGE inSource;
  };

  class Source
  {
  public:
    virtual void Search(LPCWSTR text, std::vector<Result>& results) = 0;
    virtual void Highlight(const Result& result) = 0;
    virtual CString Description(void) = 0;
    virtual CRect WindowRect(void) = 0;
  };

  void Search(Source* source, LPCWSTR text, CRect& windowRect);

protected:
  CWnd* m_parent;
  Source* m_source;

  bool m_searching;
  std::vector<Result> m_results;
};
