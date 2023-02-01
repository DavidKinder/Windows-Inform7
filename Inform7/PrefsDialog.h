#pragma once

#include "BaseDialog.h"
#include "ColourScheme.h"
#include "NoFocusCheck.h"
#include "Resource.h"
#include "SourceSettings.h"
#include "SourceWindow.h"

#include "ColourButton.h"
#include "DarkMode.h"

class PrefsDialog;

class PrefsEditPage : public DarkModePropertyPage
{
  DECLARE_DYNAMIC(PrefsEditPage)

public:
  PrefsEditPage(PrefsDialog* dlg);

  enum { IDD = IDD_PREFS_EDIT };

  void ReadSettings(void);
  void WriteSettings(void);

  bool GetDWord(const char* name, DWORD& value);
  bool GetString(const char* name, char* value, ULONG len);

  void PreviewChanged(void);
  void SetDarkMode(DarkMode* dark);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClickedRestore();
  afx_msg void OnClickedEnableStyles();
  afx_msg void OnChangeFont();
  afx_msg void OnChangeStyle();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg LRESULT OnAfterFontSet(WPARAM, LPARAM);
  afx_msg LRESULT OnUpdatePreview(WPARAM, LPARAM);

  void UpdateControlStates(void);
  void UpdatePreview(void);
  void SetDefaults(void);
  void AdjustControlRow(int ctrlId, int top, int ctrlId1, int ctrlId2);

  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

private:
  PrefsDialog* m_dialog;

  BOOL m_styles;
  DarkModeCheckButton m_stylesCheck;
  DarkModeButton m_restoreBtn;

  DarkModeGroupBox m_fontBox;
  CString m_fontName;
  CString m_fontSize;
  DarkModeComboBox m_fontNameCtrl;
  DarkModeComboBox m_fontSizeCtrl;

  DarkModeGroupBox m_stylesBox;
  DarkModeStatic m_labelHead;
  DarkModeStatic m_labelMain;
  DarkModeStatic m_labelComment;
  DarkModeStatic m_labelQuote;
  DarkModeStatic m_labelSubst;
  NoFocusCheck m_boldHead;
  NoFocusCheck m_boldMain;
  NoFocusCheck m_boldComment;
  NoFocusCheck m_boldQuote;
  NoFocusCheck m_boldSubst;
  NoFocusCheck m_italicHead;
  NoFocusCheck m_italicMain;
  NoFocusCheck m_italicComment;
  NoFocusCheck m_italicQuote;
  NoFocusCheck m_italicSubst;
  NoFocusCheck m_underHead;
  NoFocusCheck m_underMain;
  NoFocusCheck m_underComment;
  NoFocusCheck m_underQuote;
  NoFocusCheck m_underSubst;
  DarkModeComboBox m_sizeHeadCtrl;
  DarkModeComboBox m_sizeMainCtrl;
  DarkModeComboBox m_sizeCommentCtrl;
  DarkModeComboBox m_sizeQuoteCtrl;
  DarkModeComboBox m_sizeSubstCtrl;
  int m_sizeHead;
  int m_sizeMain;
  int m_sizeComment;
  int m_sizeQuote;
  int m_sizeSubst;

  DarkModeGroupBox m_tabSizeBox;
  int m_tabSize;
  DarkModeSliderCtrl m_tabSizeCtrl;

  SourceWindow m_preview;
  SourceWindow m_tabPreview;

  DarkModeGroupBox m_indentBox;
  DarkModeCheckButton m_autoIndentCtrl;
  DarkModeCheckButton m_autoSpaceTablesCtrl;
  BOOL m_autoIndent;
  BOOL m_autoSpaceTables;

  DarkModeGroupBox m_numberBox;
  DarkModeCheckButton m_autoNumberCtrl;
  BOOL m_autoNumber;
};

class PrefsColourPage : public DarkModePropertyPage
{
  DECLARE_DYNAMIC(PrefsColourPage)

public:
  PrefsColourPage(PrefsDialog* dlg);

  enum { IDD = IDD_PREFS_COLOUR };

  void ReadSettings(void);
  void WriteSettings(void);

  bool GetDWord(const char* name, DWORD& value);
  bool GetString(const char* name, char* value, ULONG len);

  void PreviewChanged(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();

  afx_msg void OnClickedNewScheme();
  afx_msg void OnClickedDeleteScheme();
  afx_msg void OnClickedRestore();
  afx_msg void OnClickedEnableColours();
  afx_msg void OnChangeColourScheme();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg LRESULT OnUpdatePreview(WPARAM, LPARAM);
  afx_msg LRESULT OnColourChanged(WPARAM, LPARAM);

  void UpdateSchemeChoices(void);
  void UpdateColourButtons(void);
  void UpdateControlStates(void);
  void UpdatePreview(void);
  void SetDefaults(void);

  DECLARE_MESSAGE_MAP()

private:
  PrefsDialog* m_dialog;

  BOOL m_colours;
  DarkModeCheckButton m_coloursCheck;

  DarkModeStatic m_colourSchemeLabel;
  CString m_colourScheme;
  DarkModeComboBox m_colourSchemeCombo;
  DarkModeButton m_newBtn;
  DarkModeButton m_deleteBtn;

  DarkModeGroupBox m_schemeBox;
  DarkModeStatic m_labelHead;
  DarkModeStatic m_labelMain;
  DarkModeStatic m_labelComment;
  DarkModeStatic m_labelQuote;
  DarkModeStatic m_labelSubst;
  DarkModeStatic m_labelSource;
  DarkModeStatic m_labelExt;
  ColourButton m_colourHead;
  ColourButton m_colourMain;
  ColourButton m_colourComment;
  ColourButton m_colourQuote;
  ColourButton m_colourSubst;
  ColourButton m_colourSource;
  ColourButton m_colourExt;
  DarkModeButton m_restoreBtn;

  std::map<std::string,ColourScheme> m_schemes;

  SourceWindow m_preview;
};

class PrefsAdvancedPage : public DarkModePropertyPage
{
public:
  PrefsAdvancedPage();

  enum { IDD = IDD_PREFS_ADVANCED };

  void ReadSettings(void);
  void WriteSettings(void);

protected:
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  afx_msg void OnClickedCleanFiles();
  afx_msg LRESULT OnAfterFontSet(WPARAM, LPARAM);

  void UpdateControlStates(void);

  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

private:
  CFont m_smallFont;

  DarkModeGroupBox m_uiBox;
  BOOL m_tabsHorizontal;
  DarkModeCheckButton m_tabsHorizontalCheck;
  CString m_fixedFontName;
  DarkModeComboBox m_fixedFontCombo;

  DarkModeGroupBox m_cleanBox;
  DarkModeCheckButton m_cleanFilesCheck;
  BOOL m_cleanFiles;
  DarkModeCheckButton m_cleanIndexCheck;
  BOOL m_cleanIndexes;

  DarkModeGroupBox m_terpBox;
  CString m_glulxTerp;
  DarkModeComboBox m_glulxTerpCombo;
};

class PrefsDialog : public DarkModePropertySheet, public SourceSettings
{
  DECLARE_DYNAMIC(PrefsDialog)

public:
  PrefsDialog();
  void ShowDialog(void);
  void UpdatePreviews(void);

  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  virtual bool GetDWord(const char* name, DWORD& value);
  virtual bool GetString(const char* name, char* value, ULONG len);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnResizePage(WPARAM, LPARAM);

  void ChangeDialogFont(CWnd* wnd, CFont* font, double scale, double extScaleX);

private:
  PrefsEditPage m_editPage;
  PrefsColourPage m_colourPage;
  PrefsAdvancedPage m_advPage;

  UINT m_dpi;
  double m_fontHeightPerDpi;
  LOGFONT m_logFont;

  CRect m_page;
  CFont m_font;
};

class NewColourSchemeDialog : public I7BaseDialog
{
public:
  NewColourSchemeDialog();
  CString GetName(void);

  enum { IDD = IDD_NEW_COLOUR_SCHEME };

protected:
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  afx_msg void OnChangedEdit();

  DECLARE_MESSAGE_MAP()

  CString m_name;
  DarkModeEdit m_nameEdit;
  DarkModeButton m_ok, m_cancel;
};
