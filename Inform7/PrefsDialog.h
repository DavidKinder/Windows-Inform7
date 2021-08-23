#pragma once

#include "BaseDialog.h"
#include "ColourButton.h"
#include "NoFocusCheck.h"
#include "SourceSettings.h"
#include "SourceWindow.h"
#include "Resource.h"
#include "afxcmn.h"

class PrefsEditPage : public CPropertyPage, public SourceSettings
{
public:
  PrefsEditPage();

  enum { IDD = IDD_PREFS_EDIT };

  void ReadSettings(void);
  void WriteSettings(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClickedRestore();
  afx_msg void OnClickedEnableHighlight();
  afx_msg void OnChangeFont();
  afx_msg void OnChangeStyle();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg LRESULT OnAfterFontSet(WPARAM, LPARAM);
  afx_msg LRESULT OnUpdatePreview(WPARAM, LPARAM);

  void UpdateControlStates(void);
  void UpdatePreview(void);
  void SetDefaults(void);
  void AdjustControlRow(int ctrlId, int top, int ctrlId1, int ctrlId2);

  virtual bool GetDWord(const char* name, DWORD& value);
  virtual bool GetString(const char* name, char* value, ULONG len);

  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

private:
  CString m_fontName;
  CString m_fontSize;
  CComboBox m_font;

  ColourButton m_colourSource;
  ColourButton m_colourExt;

  BOOL m_highlight;
  CButton m_highlightCheck;
  ColourButton m_colourHead;
  ColourButton m_colourMain;
  ColourButton m_colourComment;
  ColourButton m_colourQuote;
  ColourButton m_colourSubst;
  int m_styleHead;
  int m_styleMain;
  int m_styleComment;
  int m_styleQuote;
  int m_styleSubst;
  NoFocusCheck m_underHead;
  NoFocusCheck m_underMain;
  NoFocusCheck m_underComment;
  NoFocusCheck m_underQuote;
  NoFocusCheck m_underSubst;
  int m_sizeHead;
  int m_sizeMain;
  int m_sizeComment;
  int m_sizeQuote;
  int m_sizeSubst;

  int m_tabSize;
  CSliderCtrl m_tabSizeCtrl;

  SourceWindow m_preview;
  SourceWindow m_tabPreview;

  BOOL m_indentWrapped;
  BOOL m_autoIndent;
  BOOL m_autoSpaceTables;
  BOOL m_autoNumber;
};

class PrefsTextPage : public CPropertyPage
{
public:
  PrefsTextPage();

  enum { IDD = IDD_PREFS_TEXT };

  void ReadSettings(void);
  void WriteSettings(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

private:
  CString m_fontName;
  CString m_fixedFontName;
  CString m_fontSize;
  CComboBox m_font;
  CComboBox m_fixedFont;
};

class PrefsAdvancedPage : public CPropertyPage
{
public:
  PrefsAdvancedPage();

  enum { IDD = IDD_PREFS_ADVANCED };

  void ReadSettings(void);
  void WriteSettings(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();

  afx_msg void OnClickedCleanFiles();
  afx_msg LRESULT OnAfterFontSet(WPARAM, LPARAM);

  void UpdateControlStates(void);

  DECLARE_MESSAGE_MAP()

private:
  BOOL m_cleanFiles;
  BOOL m_cleanIndexes;
  CButton m_cleanFilesCheck;
  CButton m_cleanIndexCheck;
  CString m_glulxTerp;
  BOOL m_tabsHorizontal;
  BOOL m_startWithLast;
  BOOL m_I6debug;

  CFont m_smallFont;
};

class PrefsDialog : public CPropertySheet
{
public:
  PrefsDialog();
  void ShowDialog(void);

  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnResizePage(WPARAM, LPARAM);

protected:
  DECLARE_MESSAGE_MAP()

  void ChangeDialogFont(CWnd* wnd, CFont* font, double scale, double extScaleX);

private:
  UINT m_dpi;
  double m_fontHeightPerDpi;
  LOGFONT m_logFont;

  CRect m_page;
  CFont m_font;
};
