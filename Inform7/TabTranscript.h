#pragma once

#include "TabBase.h"
#include "TranscriptWindow.h"
#include "Skein.h"
#include "ArrowButton.h"

class TabTranscript : public TabBase
{
  DECLARE_DYNAMIC(TabTranscript)

public:
  TabTranscript();

  const char* GetName(void);
  void CreateTab(CWnd* parent);
  void MoveTab(CRect& rect);
  void MakeActive(TabState& state);

  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

  void OpenProject(const char* path, bool primary);

  void SetSkein(Skein* skein);
  void ShowNode(Skein::Node* node, Skein::Show why);
  Skein::Node* GetEndNode(void);
  Skein::Node* FindRelevantNode(TranscriptWindow::FindAction action, bool next);

  CString GetToolTip(UINT_PTR id);

private:
  Skein* m_skein;
  TranscriptWindow m_window;

  ArrowButton m_nextSkein, m_nextDiff, m_prevDiff;
  CButton m_blessAll;

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM, LPARAM);

  afx_msg void OnActionButton(UINT nID);
  afx_msg void OnBlessAll();
};
