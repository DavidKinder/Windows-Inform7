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
  FindInFiles(ProjectFrame* project);

  void Show(void);
  void Hide(void);

  void FindInSource(LPCWSTR text);
  void FindInDocs(LPCWSTR text);

  static void InitInstance(void);
  static void ExitInstance(void);

  static LPCWSTR GetAutoComplete(int index);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnDestroy();
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT di);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnFindAll();
  afx_msg void OnChangeFindRule();
  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnResultsSelect(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg LRESULT OnResultsResize(WPARAM, LPARAM);

private:
  enum FoundIn
  {
    FoundInSource,
    FoundInExtension,
    FoundInWritingWithInform,
    FoundInRecipeBook,
    FoundInUnknown
  };

  void Find(const CString& text, const char* doc, const char* docSort,
    const char* path, const char* prefix, FoundIn type);
  void FindInExtensions(void);
  void FindInDocumentation(void);

  int FindLineStart(const CString& text, int pos);
  int FindLineEnd(const CString& text, int pos);
  CStringW GetMatchRange(const CString& text, int start, int end);

  void DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format);
  int MeasureText(CDC* dc, LPCWSTR text, int length);
  COLORREF Darken(COLORREF colour);

  struct FindResult
  {
    FindResult();
    bool operator<(const FindResult& fr) const;

    CString TypeName(void);
    COLORREF Colour(void);

    CStringW prefix;
    CStringW context;
    CHARRANGE inContext;

    FoundIn type;
    CString doc;
    CString docSort;
    CString path;
    CHARRANGE loc;
  };

  void ShowResult(const FindResult& result);

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
  static CList<CStringW> m_findHistory;

  std::vector<FindResult> m_results;
  FindResultsCtrl m_resultsList;
  CSize m_resultsBottomRight;

  CStatic m_regexHelp;
  RichDrawText* m_richText;

  // Caching of text extracted from the documentation

  struct DocText
  {
    DocText(FoundIn docType);

    CString section;
    CString title;
    CString sort;
    CString prefix;
    CString body; // UTF-8
    CString link;
    FoundIn type;
  };

  static void DecodeHTML(const char* filename, FoundIn docType);
  static UINT BackgroundDecodeThread(LPVOID);

  struct DocData
  {
    CCriticalSection lock;
    bool done;
    CArray<FindInFiles::DocText*> texts;

    DocData() : done(false)
    {
    }
  };

  static DocData* m_data;
  static CWinThread* m_pThread;
};
