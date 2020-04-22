#pragma once

#include "Inform.h"
#include "BaseDialog.h"
#include "UnicodeEdit.h"

#include <vector>

class ProjectFrame;
class RichDrawText;

class FindResultsCtrl : public CListCtrl
{
public:
  FindResultsCtrl();

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnHeaderDividerDblClick(NMHDR* pNotifyStruct, LRESULT* result);
};

class FindInFiles : public I7BaseDialog
{
public:
  FindInFiles();

  void Show(ProjectFrame* project);
  void Hide(ProjectFrame* project);

  void FindInSource(LPCWSTR text);
  void FindInDocs(LPCWSTR text);

  void InitInstance(void);
  void ExitInstance(void);

  LPCWSTR GetAutoComplete(int index);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnFindAll();
  afx_msg void OnChangeFindRule();
  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg LRESULT OnResultsResize(WPARAM, LPARAM);

  void Find(const CString& text, const CString& doc, const char* type);

  int FindLineStart(const CString& text, int pos);
  int FindLineEnd(const CString& text, int pos);
  CStringW GetMatchRange(const CString& text, int start, int end);

  void DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format);
  int MeasureText(CDC* dc, LPCWSTR text, int length);
  COLORREF Darken(COLORREF colour);

  struct FindResult
  {
    FindResult();

    CStringW context;
    CHARRANGE inContext;

    CString sourceSort;
    CString sourceDocument;
    CString sourceType;
    CHARRANGE inSource;

    InformApp::Colours colour;
  };

  ProjectFrame* m_project;
  CSize m_minSize;

  CStringW m_findText;

  int m_lookSource;
  int m_lookExts;
  int m_lookDocPhrases;
  int m_lookDocMain;
  int m_lookDocExamples;

  int m_ignoreCase;
  int m_findRule;

  UnicodeEdit m_find;
  CComPtr<IAutoComplete2> m_findAutoComplete;
  CList<CStringW> m_findHistory;

  std::vector<FindResult> m_results;
  FindResultsCtrl m_resultsList;
  CSize m_resultsBottomRight;

  CStatic m_regexHelp;
  RichDrawText* m_richText;
};

extern FindInFiles theFinder;
