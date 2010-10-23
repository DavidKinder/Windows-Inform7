#pragma once

#include "BaseDialog.h"
#include "Resource.h"
#include "UnicodeEdit.h"

#include <set>

class SourceEdit;

class SpellCheck : public I7BaseDialog
{
public:
  static void Initialize(void);
  static void InitSpellObject(void);
  static void Finalize(void);

  SpellCheck(SourceEdit* edit);
  void ShowWordFromSelection(void);

  bool FindWord(bool current);
  void DoneMessage(void);
  void StartCheck(void);
  void SettingsChanged(void);

  enum { IDD = IDD_SPELLING };

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnClickedIgnore();
  afx_msg void OnClickedReplace();
  afx_msg void OnClickedAdd();
  afx_msg void OnChangedBadWord();
  afx_msg void OnChangedLanguage();

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnOK();

private:
  bool TestWord(CHARRANGE range);
  bool IsTestCharacter(wchar_t c);
  void NextWord(void);
  void UpdateDialogControls(void);

  static BOOL CALLBACK HandleLocale(LPTSTR localeText);

  SourceEdit* m_edit;

  UnicodeEdit m_badWord;
  CListBox m_suggestions;
  CComboBox m_language;

  bool m_settingBadWord;
  CStringW m_currentBadWord;

  std::set<CStringW> m_ignoreWords;
};
