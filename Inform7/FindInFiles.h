#pragma once

#include "BaseDialog.h"
#include "FindAllHelper.h"
#include "UnicodeEdit.h"

#include "DarkMode.h"

#include <vector>

class ProjectFrame;
class RichDrawText;

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
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg void OnFindAll();
  afx_msg void OnChangeFindRule();
  afx_msg void OnChangeFindText();
  afx_msg void OnResultsDraw(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnResultsSelect(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg LRESULT OnResultsResize(WPARAM, LPARAM);

private:
  void FindInExtensions(void);
  size_t CountExtensions(void);
  void WaitForCensus(void);
  void FindInDocumentation(void);
  size_t CountDocumentation(void);
  void WaitForDocThread(void);
  void UpdateProgress(void);

  void SetRichTextRTF(const char* fragment);
  void ShowResult(const FindResult& result);

  ProjectFrame* m_project;
  CSize m_minSize;
  UINT m_dpi;

  CStringW m_findText;
  UnicodeEdit m_findEdit;
  DarkModeButton m_findAllBtn;

  DarkModeGroupBox m_lookGroup;
  DarkModeCheckButton m_lookSourceCtrl;
  DarkModeCheckButton m_lookExtsCtrl;
  DarkModeCheckButton m_lookDocPhrasesCtrl;
  DarkModeCheckButton m_lookDocMainCtrl;
  DarkModeCheckButton m_lookDocCodeCtrl;
  int m_lookSource;
  int m_lookExts;
  int m_lookDocPhrases;
  int m_lookDocMain;
  int m_lookDocCode;

  DarkModeGroupBox m_howGroup;
  DarkModeCheckButton m_ignoreCaseCtrl;
  int m_ignoreCase;
  DarkModeComboBox m_findRuleCtrl;
  FindRule m_findRule;

  CComPtr<IAutoComplete2> m_findAutoComplete;
  static CList<CStringW> m_findHistory;

  size_t m_total;
  size_t m_current;

  FindAllHelper m_findHelper;
  FindResultsCtrl m_resultsList;
  CSize m_gapBottomRight;

  CStatic m_found;
  DarkModeProgressCtrl m_progress;
  CStatic m_regexHelp;
  RichDrawText* m_richText;

  // Caching of text extracted from the documentation

  struct DocSection
  {
    DocSection(int start_, int end_, const char* id_);

    int start;
    int end;
    CString id;
  };

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
    std::vector<DocSection> codeSections, phraseSections;
  };

  static void DecodeHTML(const char* filename, FoundIn docType);
  static UINT BackgroundDecodeThread(LPVOID);
  static CString GetSectionId(int pos, const std::vector<DocSection>& sections);

  struct DocData
  {
    CCriticalSection lock;
    bool stop;
    bool done;
    CArray<FindInFiles::DocText*> texts;

    DocData() : stop(false), done(false)
    {
    }
  };

  static DocData* m_data;
  static CWinThread* m_pThread;
};
