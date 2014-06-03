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
  virtual void OnOK();

  afx_msg void OnBnClickedCleanFiles();

  void UpdateControlStates(void);
  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

  CString m_fontName;
  CString m_fixedFontName;
  CString m_fontSize;
  CComboBox m_font;
  CComboBox m_fixedFont;

  DWORD m_tabSize;
  BOOL m_indentWrapped;
  BOOL m_autoIndent;
  BOOL m_startWithLast;

  BOOL m_cleanFiles;
  BOOL m_cleanIndexes;
  BOOL m_I6debug;
  CButton m_cleanFilesCheck;
  CButton m_cleanIndexCheck;
  CString m_glulxTerp;

  CFont m_smallFont;
};
