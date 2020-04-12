#pragma once

#include "Inform.h"
#include "BaseDialog.h"
#include "UnicodeEdit.h"

#include <vector>

class ProjectFrame;

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
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnFindAll();
  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);

  void Find(LONG_PTR editPtr);

  LONG_PTR CallEdit(LONG_PTR editPtr, UINT msg, DWORD wp = 0, LONG_PTR lp = 0);
  CStringW GetTextRange(LONG_PTR editPtr, int cpMin, int cpMax, int len = -1);

  void DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format);
  COLORREF Darken(COLORREF colour);
  void SetResultsWidths(void);

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
  CListCtrl m_resultsList;
  CSize m_resultsBottomRight;
};

extern FindInFiles theFinder;
