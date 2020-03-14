#pragma once

#include "BaseDialog.h"
#include "UnicodeEdit.h"

class ProjectFrame;

class FindInFiles : public I7BaseDialog
{
public:
  FindInFiles();

  void Show(ProjectFrame* project);
  void Hide(ProjectFrame* project);

  void InitInstance(void);
  void ExitInstance(void);

  LPCWSTR GetAutoComplete(int index);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();

  DECLARE_MESSAGE_MAP()

  afx_msg void OnClose();
  afx_msg void OnFindAll();

  ProjectFrame* m_project;

  UnicodeEdit m_find;
  CStringW m_findText;
  CComPtr<IAutoComplete2> m_auto;
  CList<CStringW> m_findComplete;

  int m_lookSource;
  int m_lookExts;
  int m_lookDocPhrases;
  int m_lookDocMain;
  int m_lookDocExamples;

  int m_ignoreCase;
  int m_findRule;
};

extern FindInFiles theFinder;
