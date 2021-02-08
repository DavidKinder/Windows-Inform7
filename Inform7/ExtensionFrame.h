#pragma once

#include "Inform.h"
#include "SourceEdit.h"
#include "MenuBar.h"
#include "ProjectSettings.h"

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
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnActivate(UINT nState, CWnd*, BOOL);
  afx_msg void OnClose();
  afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  afx_msg LRESULT OnProjectEdited(WPARAM wparam, LPARAM lparam);

  afx_msg void OnFileClose();
  afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
  afx_msg void OnFileSave();
  afx_msg void OnFileSaveAs();

  afx_msg void OnUpdateWindowList(CCmdUI *pCmdUI);
  afx_msg void OnWindowList(UINT nID);

public:
  static void StartNew(CWnd* parent, const ProjectSettings& settings);
  static void StartExisting(const char* path, const ProjectSettings& settings);
  static bool StartHighlight(const char* url, COLORREF colour, const ProjectSettings& settings);
  static void StartSelect(const char* path, const CHARRANGE& range, const ProjectSettings& settings);
  static void InstallExtensions(CFrameWnd* parent);
  static void InstallExtensions(CFrameWnd* parent, CStringArray& paths);
  static void DownloadExtensions(CFrameWnd* parent, CStringArray* urls);

  static CStringW ReadExtensionFirstLine(const char* path);
  static bool IsValidExtension(const CStringW& firstLine,
    CStringW& name, CStringW& author, CStringW& version);

  CString GetDisplayName(bool fullName);
  void SendChanged(InformApp::Changed changed, int value);

protected:
  static ExtensionFrame* NewFrame(const ProjectSettings& settings);
  static bool RemoveI7X(CString& path);
  static void DeleteOldExtension(CString path);
  static void SetDownloadProgress(CFrameWnd* parent, int total, int current, int installed);
  static void ShowInstalledMessage(CWnd* parent, int installed, int total, LPCWSTR lastExt);
  
  void OpenFile(const char* path);
  void SetFromRegistryPath(const char* path);
  bool IsProjectEdited(void);
  bool IsUserExtension(void);
  COLORREF GetBackColour(CRegKey& key);

  SourceEdit m_edit;
  CString m_extension;
};
