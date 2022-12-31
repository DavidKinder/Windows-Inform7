#pragma once

#include "BaseDialog.h"
#include "ColourButton.h"
#include "NoFocusCheck.h"
#include "SourceSettings.h"
#include "SourceWindow.h"
#include "Resource.h"
#include "afxcmn.h"

#include <map>
#include <string>

class PrefsDialog;

class PrefsEditPage : public CPropertyPage
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

  CString m_fontName;
  CString m_fontSize;
  CComboBox m_font;

  BOOL m_styles;
  CButton m_stylesCheck;
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
  int m_sizeHead;
  int m_sizeMain;
  int m_sizeComment;
  int m_sizeQuote;
  int m_sizeSubst;

  int m_tabSize;
  CSliderCtrl m_tabSizeCtrl;

  SourceWindow m_preview;
  SourceWindow m_tabPreview;

  BOOL m_autoIndent;
  BOOL m_autoSpaceTables;
  BOOL m_autoNumber;
};

class PrefsColourPage : public CPropertyPage
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
  CButton m_coloursCheck;

  CString m_colourScheme;
  CComboBox m_colourSchemeCombo;

  ColourButton m_colourHead;
  ColourButton m_colourMain;
  ColourButton m_colourComment;
  ColourButton m_colourQuote;
  ColourButton m_colourSubst;
  ColourButton m_colourSource;
  ColourButton m_colourExt;

  struct ColourScheme
  {
    ColourScheme();
    ColourScheme(int srtIdx,
      COLORREF hd, COLORREF mn, COLORREF cmt, COLORREF qt, COLORREF sbst, COLORREF src, COLORREF xt);

    int sortIndex;
    COLORREF head;
    COLORREF main;
    COLORREF comment;
    COLORREF quote;
    COLORREF subst;
    COLORREF source;
    COLORREF ext;
  };
  std::map<std::string,ColourScheme> m_schemes;

  SourceWindow m_preview;
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
  virtual void OnOK();

  afx_msg void OnClickedCleanFiles();
  afx_msg LRESULT OnAfterFontSet(WPARAM, LPARAM);

  void UpdateControlStates(void);

  static int CALLBACK ListFonts(ENUMLOGFONTEX *font, NEWTEXTMETRICEX *metric, DWORD fontType, LPARAM param);

  DECLARE_MESSAGE_MAP()

private:
  BOOL m_cleanFiles;
  BOOL m_cleanIndexes;
  CButton m_cleanFilesCheck;
  CButton m_cleanIndexCheck;
  CString m_glulxTerp;
  BOOL m_tabsHorizontal;

  CString m_fixedFontName;
  CComboBox m_fixedFont;

  CFont m_smallFont;
};

class PrefsDialog : public CPropertySheet, public SourceSettings
{
public:
  PrefsDialog();
  void ShowDialog(void);
  void UpdatePreviews(void);

  virtual BOOL OnInitDialog();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  afx_msg LRESULT OnResizePage(WPARAM, LPARAM);

  virtual bool GetDWord(const char* name, DWORD& value);
  virtual bool GetString(const char* name, char* value, ULONG len);

protected:
  DECLARE_MESSAGE_MAP()

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
};
