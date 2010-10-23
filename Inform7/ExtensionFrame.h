#pragma once

#include "Inform.h"
#include "SourceEdit.h"
#include "MenuBar.h"

#include <deque>

class ExtensionFrame : public MenuBarFrameWnd
{
protected:
  DECLARE_DYNAMIC(ExtensionFrame)

public:
  ExtensionFrame();

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
  virtual void GetMessageString(UINT nID, CString& rMessage) const;

protected:
  CStatusBar m_statusBar;

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnClose();

  afx_msg LRESULT OnProjectEdited(WPARAM wparam, LPARAM lparam);

  afx_msg void OnFileClose();
  afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();

  afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
  afx_msg void OnWindowList(UINT nID);

public:
  static void StartNew(CWnd* parent);
  static void StartExisting(const char* path);
  static bool StartHighlight(const char* url, COLORREF colour);
  static bool InstallExtensions(CWnd* parent);

  CString GetDisplayName(bool showEdited);
  void SendChanged(InformApp::Changed changed, int value);

protected:
  static ExtensionFrame* NewFrame(void);
  static bool IsValidExtension(const CStringW& firstLine, CStringW& name, CStringW& author);
  static bool RemoveI7X(CString& path);
  static void DeleteOldExtension(CString path);

  void OpenFile(const char* path);
  void SetFromRegistryPath(const char* path);
  bool IsProjectEdited(void);
  bool IsUserExtension(void);

  SourceEdit m_edit;
  CString m_extension;
};
