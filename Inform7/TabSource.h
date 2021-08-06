#pragma once

#include "TabBase.h"
#include "ButtonTab.h"
#include "ContentsWindow.h"
#include "SourceWindow.h"

class TabSource : public TabBase
{
  DECLARE_DYNAMIC(TabSource)

public:
  // Implementation of TabInterface
  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);
  void MakeInactive(void);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  bool IsProjectEdited(void);
  void LoadSettings(CRegKey& key, bool primary);
  void SaveSettings(CRegKey& key, bool primary);
  void PrefsChanged(CRegKey& key);
  void UpdateDPI(const std::map<CWnd*,double>& layout);

  void SetDocument(TabSource* master);
  bool Highlight(const char* url, COLORREF colour);
  void Select(const CHARRANGE& range);
  void PasteCode(const wchar_t* code);
  void UpdateSpellCheck(void);
  bool CheckNeedReopen(const char* path);
  CTime GetFileTimestamp(const char* path);
  void UpdateElasticTabStops(void);
  CString GetSource(void);
  int GetTabHeight(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnSourceRange(WPARAM, LPARAM);
  afx_msg LRESULT OnNextRange(WPARAM, LPARAM);
  afx_msg void OnHeadingsShow();
  afx_msg void OnHeadingsCurrent();
  afx_msg void OnHeadingsIncrease();
  afx_msg void OnHeadingsReduce();
  afx_msg void OnHeadingsAll();
  afx_msg void OnHeadingsPrevious();
  afx_msg void OnHeadingsNext();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

private:
  void Resize(void);

  enum SourceTabs
  {
    SrcTab_Contents = 0,
    SrcTab_Source,
    Number_SrcTabs,
    No_SrcTab = -1
  };

  SourceTabs GetActiveTab(void);
  void SetActiveTab(SourceTabs tab, bool focus);

  int FindHeading(const CArray<SourceLexer::Heading>& headings, const CStringW& find, int i);
  int FindNextHeading(const CArray<SourceLexer::Heading>& headings, bool next, int i);
  int FindCurrentHeading(const CArray<SourceLexer::Heading>& headings);
  void ShowHeading(const CArray<SourceLexer::Heading>& headings, int idx);
  CString GetSourcePath(const char* path);

  ButtonTab m_tab;

  CString m_sourceFile;
  SourceWindow m_source;
  ContentsWindow m_contents;
};
