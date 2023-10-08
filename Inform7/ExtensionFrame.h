#pragma once

#include "Inform.h"
#include "ProjectSettings.h"
#include "SourceEdit.h"
#include "SourceSettings.h"

#include "MenuBar.h"

class DarkMode;

class ExtensionFrame : public MenuBarFrameWnd
{
protected:
  DECLARE_DYNAMIC(ExtensionFrame)

public:
  ExtensionFrame();

  virtual void SetDarkMode(DarkMode* dark);

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnClose();
  afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  afx_msg LRESULT OnProjectEdited(WPARAM wparam, LPARAM lparam);

  afx_msg void OnFileClose();
  afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();

  afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
  afx_msg void OnWindowList(UINT nID);

public:
  static void StartNew(CWnd* parent, const char* dir, const ProjectSettings& settings);
  static void StartExisting(const char* path, const ProjectSettings& settings);
  static bool StartHighlight(const char* url, COLORREF colour, const ProjectSettings& settings);
  static void StartSelect(const char* path, const CHARRANGE& range, const ProjectSettings& settings);

  CString GetDisplayName(bool fullName);
  void SendChanged(InformApp::Changed changed, int value);

protected:
  static ExtensionFrame* NewFrame(const ProjectSettings& settings);
  
  void OpenFile(const char* path);
  void SetFromRegistryPath(const char* path);
  bool IsProjectEdited(void);
  COLORREF GetBackColour(SourceSettings& set);

  SourceEdit m_edit;
  CString m_extension;
};
