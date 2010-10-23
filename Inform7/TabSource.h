#pragma once

#include "TabBase.h"
#include "FlatTab.h"
#include "ContentsWindow.h"
#include "SearchWindow.h"
#include "SourceWindow.h"

class TabSource : public TabBase, public SearchWindow::Source
{
  DECLARE_DYNAMIC(TabSource)

public:
  TabSource();

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
  void SaveToPath(const char* path);
  void LoadSettings(CRegKey& key);
  void SaveSettings(CRegKey& key);
  void PrefsChanged(CRegKey& key);

  void SetDocument(TabSource* master);
  bool Highlight(const char* url, COLORREF colour);
  void PasteCode(const wchar_t* code);
  void SetTextSize(int size);
  void UpdateSpellCheck(void);

  // Implementation of SearchWindow::Source
  void Search(LPCWSTR text, std::vector<SearchWindow::Result>& results);
  void Highlight(const SearchWindow::Result& result);
  CString Description(void);
  CRect WindowRect(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnSourceRange(WPARAM, LPARAM);
  afx_msg LRESULT OnNextRange(WPARAM, LPARAM);
  afx_msg void OnSourceAll();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

private:
  enum SourceTabs
  {
    SrcTab_Contents = 0,
    SrcTab_Source,
    Number_SrcTabs,
    No_SrcTab = -1
  };

  SourceTabs GetActiveTab(void);
  void SetActiveTab(SourceTabs tab, bool focus);

  FlatTab m_tab;

  SourceWindow m_source;
  ContentsWindow m_contents;
};
