#pragma once

#include "BaseDialog.h"
#include "Resource.h"

class PrefsDialog : public I7BaseDialog
{
public:
  PrefsDialog();

  enum { IDD = IDD_PREFERENCES };

  virtual INT_PTR DoModal();

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();

  afx_msg void OnBnClickedCleanFiles();

  void UpdateControlStates(void);

  DECLARE_MESSAGE_MAP()

  DWORD m_tabSize;
  BOOL m_indentWrapped;
  BOOL m_autoIndent;
  BOOL m_startWithLast;

  BOOL m_cleanFiles;
  BOOL m_cleanIndexes;
  CButton m_cleanFilesCheck;
  CButton m_cleanIndexCheck;
  CString m_glulxTerp;
};
