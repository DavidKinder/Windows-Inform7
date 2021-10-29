#pragma once

#include "FlatSplitter.h"

class BookFrame : public CFrameWnd
{
protected:
  DECLARE_DYNAMIC(BookFrame)

public:
  static void ShowBook(const char* dir);
  void ShowPage(const char* page);

  virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
    DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
    LPCTSTR lpszMenuName, DWORD dwExStyle, CCreateContext* pContext);

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  BookFrame(const char* dir);

  void SetFromRegistryPath(const char* path);

  CString ReadContents(void);
  void ReadContentsPoints(IXMLDOMNodeList* navPoints, HTREEITEM parentItem);
  CString StringFromXML(IXMLDOMNode* node, LPWSTR query);

  CTreeCtrl& GetTreeCtrl(void);

  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDestroy();
  afx_msg void OnClose();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);

  FlatSplitter m_splitter;

  CString m_dir;
};
