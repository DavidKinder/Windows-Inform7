#pragma once

#include "TabBase.h"
#include "Skein.h"
#include "SkeinWindow.h"
#include "ArrowButton.h"
#include "FlatSplitter.h"
#include "ReportHtml.h"

class TabSkein : public TabBase, public ReportHtml::LinkConsumer
{
  DECLARE_DYNAMIC(TabSkein)

public:
  TabSkein();
  ~TabSkein();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);
  bool SaveProject(const char* path, bool primary);
  bool IsProjectEdited(void);
  void LoadSettings(CRegKey& key, bool primary);
  void SaveSettings(CRegKey& key, bool primary);
  void PrefsChanged(CRegKey& key);

  // Implementation of ReportHtml::LinkConsumer
  void SourceLink(const char* url);
  void LibraryLink(const char* url);
  void SkeinLink(const char* url);
  bool DocLink(const char* url);
  void LinkError(const char* url);
  void LinkDone(void);

  void SetSkein(Skein* skein);
  void ShowNode(Skein::Node* node, Skein::Show why);
  void SkeinChanged(void);
  void Animate(int pct);

  CString GetToolTip(UINT_PTR id);
  CStringW GetStoryName(void);

private:
  ArrowButton m_label;
  CButton m_play, m_save, m_help;
  FlatSplitter m_splitter;
  SkeinWindow* m_skeinWindow;
  ReportHtml* m_helpWindow;
  Skein* m_skein;

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);
  afx_msg LRESULT OnUpdateHelp(WPARAM, LPARAM);
  afx_msg LRESULT OnFindReplaceCmd(WPARAM, LPARAM);

  afx_msg void OnSkeinLabel();
  afx_msg void OnSkeinPlay();
  afx_msg void OnSaveTranscript();
  afx_msg void OnToggleHelp();

  void SetHelpVisible(const char* node, bool visible);
  void ShowHideHelp(bool show);
};
