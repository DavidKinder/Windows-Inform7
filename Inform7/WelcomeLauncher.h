#pragma once

#include "CommandButton.h"
#include "FormScrollArea.h"
#include "Inform.h"
#include "ReportHtml.h"
#include "Resource.h"

#include "DarkMode.h"
#include "Dib.h"

#include <string>
#include <vector>

class WelcomeLauncherView : public FormScrollArea
{
  DECLARE_DYNAMIC(WelcomeLauncherView)

public:
  WelcomeLauncherView();

  virtual BOOL Create(DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
  CSize GetTotalSize() const;

  void UpdateDPI(void);
  void UpdateRecent(void);
  void UpdateNews(void);
  void SetBannerBitmap(void);

  virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

protected:
  virtual void PostNcDestroy();
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnToolTipText(NMHDR*, LRESULT*);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnOpenProject(UINT nID);
  afx_msg void OnCreateProject();
  afx_msg void OnCreateExtProject();
  afx_msg void OnCopySampleProject(UINT nID);
  afx_msg void OnClickedAdvice(UINT nID);
  afx_msg void OnClickedLink(UINT nID);
  afx_msg LRESULT OnCmdListClicked(WPARAM, LPARAM);

  enum { IDD = IDD_LAUNCHER };

  DECLARE_MESSAGE_MAP()

  void GetRegions(CArray<CRect>& regions);
  void SetFonts(void);
  void SetLayout(void);
  void ShowHtml(bool show);
  CString GetToolTip(UINT_PTR id);
  CString GetUrl(UINT_PTR id, int index);

  CDibSection m_banner;
  CFont m_bigFont;
  CFont m_titleFont;
  CommandButton m_cmds[26];
  CommandListBox m_news;
  ReportHtml m_html;

  CStringArray m_recentProjects;
  double m_rightGapPerDpi;
  double m_bottomGapPerDpi;
  double m_newsTabPerDpi;

  std::vector<std::string> m_newsUrls;
};

class WelcomeLauncherFrame : public CFrameWnd
{
protected:
  DECLARE_DYNAMIC(WelcomeLauncherFrame)

public:
  static void DownloadNews(void);
  static void ShowLauncher(void);

  void UpdateNews(void);
  void SetDarkMode(DarkMode* dark);
  void SendChanged(InformApp::Changed changed, int value);

  virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
    DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
    LPCTSTR lpszMenuName, DWORD dwExStyle, CCreateContext* pContext);

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  WelcomeLauncherFrame();
  ~WelcomeLauncherFrame();

  void Resize(bool centre);

  static UINT DownloadThread(LPVOID);

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnDarkModeActive(WPARAM, LPARAM);

  WelcomeLauncherView m_view;
  DarkMode* m_dark;
};
