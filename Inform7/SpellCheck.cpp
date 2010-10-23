#include "stdafx.h"
#include "Inform.h"
#include "Panel.h"
#include "SourceEdit.h"
#include "SpellCheck.h"
#include "TextFormat.h"

#include "hunspell.hxx"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
  // The instance of the actual spell checking object
  Hunspell* spell = NULL;

  // Map of all possible dictionary names
  std::map<std::string,std::string> allLanguages;
  // Set of all installed dictionary names
  typedef std::set<std::pair<std::string,std::string> > StrPairSet;
  StrPairSet languages;
  // Iterator pointing to the currently active dictionary
  StrPairSet::iterator currentLanguage;
}

BOOL CALLBACK SpellCheck::HandleLocale(LPTSTR localeText)
{
  LCID locale = 0;
  if (sscanf(localeText,"%x",&locale) == 1)
  {
    // Ignore the anomalous "Spanish (Traditional Sort)" locale
    if (locale == 0x40A)
      return TRUE;

    // Get the language name, with a work-around for the "Spanish (International Sort)" locale
    char langName[256];
    ::GetLocaleInfo(locale,LOCALE_SLANGUAGE,langName,sizeof langName);
    if (locale == 0xC0A)
    {
      char* end = strstr(langName," (");
      if (end != NULL)
      {
        int offset = (int)(end-langName)+2;
        ::GetLocaleInfo(locale,LOCALE_SCOUNTRY,langName+offset,(sizeof langName)-offset);
        strcat(langName,")");
      }
    }

    // Get the ISO 3166 country and ISO 639 language names. If these are not available
    // (as on Windows 95), guess from the abbreviated name.
    char iso3166[8], iso639[8];
    if (::GetLocaleInfo(locale,LOCALE_SISO3166CTRYNAME,iso3166,sizeof iso3166) == 0)
    {
      ::GetLocaleInfo(locale,LOCALE_SABBREVCTRYNAME,iso3166,sizeof iso3166);
      iso3166[2] = '\0';
    }
    if (::GetLocaleInfo(locale,LOCALE_SISO639LANGNAME,iso639,sizeof iso639) == 0)
    {
      ::GetLocaleInfo(locale,LOCALE_SABBREVLANGNAME,iso639,sizeof iso639);
      iso639[2] = '\0';
    }

    std::ostringstream shortName;
    shortName << iso639 << '_' << iso3166;
    allLanguages[shortName.str()] = langName;
  }
  return TRUE;
}

void SpellCheck::Initialize(void)
{
  if (spell != NULL)
    return;

  // Get long and short names for all possible dictionaries
  ::EnumSystemLocales(HandleLocale,LCID_SUPPORTED);

  // Set up the installed dictionaries
  CFileFind find;
  BOOL found = find.FindFile(theApp.GetAppDir()+"\\Dictionaries\\*.dic");
  while (found)
  {
    found = find.FindNextFile();
    std::map<std::string,std::string>::const_iterator it = allLanguages.find((LPCSTR)find.GetFileTitle());
    if (it != allLanguages.end())
      languages.insert(std::make_pair(it->second,it->first));
  }

  // Load registry settings
  char fileName[MAX_PATH] = "";
  CRegKey registryKey;
  if (registryKey.Open(HKEY_CURRENT_USER,"Software\\David Kinder\\Inform\\Spelling",KEY_READ) == ERROR_SUCCESS)
  {
    ULONG len = sizeof fileName;
    if (registryKey.QueryStringValue("Language",fileName,&len) != ERROR_SUCCESS)
      strcpy(fileName,"");
  }

  // As the default, use the UK English dictionary for UK English, otherwise US English
  if (fileName[0] == '\0')
  {
    switch (::GetUserDefaultLangID())
    {
    case MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_UK):
      strcpy(fileName,"en_GB");
      break;
    default:
      strcpy(fileName,"en_US");
      break;
    }
  }

  // Select an initial current language
  currentLanguage = languages.begin();
  for (StrPairSet::iterator it = languages.begin(); it != languages.end(); ++it)
  {
    if (it->second == fileName)
      currentLanguage = it;
  }
  InitSpellObject();
}

void SpellCheck::InitSpellObject(void)
{
  delete spell;
  spell = NULL;

  // Get the path to the dictionary files
  CString appDir = theApp.GetAppDir();
  CString affPath, dicPath;
  affPath.Format("%s\\Dictionaries\\%s.aff",(LPCSTR)appDir,currentLanguage->second.c_str());
  dicPath.Format("%s\\Dictionaries\\%s.dic",(LPCSTR)appDir,currentLanguage->second.c_str());
  spell = new Hunspell(affPath,dicPath);

  // Only Latin-1 encoded dictionaries are currently supported
  if (strcmp(spell->get_dic_encoding(),"ISO8859-1") != 0)
  {
    CString msg;
    msg.Format("Dictionary %s has an unsupported encoding: %s",
      currentLanguage->second.c_str(),spell->get_dic_encoding());
    AfxMessageBox(msg);
  }
}

void SpellCheck::Finalize(void)
{
  if (spell != NULL)
  {
    // Save registry settings
    CRegKey registryKey;
    if (registryKey.Open(HKEY_CURRENT_USER,"Software\\David Kinder\\Inform\\Spelling",KEY_WRITE) == ERROR_SUCCESS)
      registryKey.SetStringValue("Language",currentLanguage->second.c_str());
  }

  delete spell;
  spell = NULL;
}

BEGIN_MESSAGE_MAP(SpellCheck, I7BaseDialog)
  ON_BN_CLICKED(IDC_IGNORE, OnClickedIgnore)
  ON_BN_CLICKED(IDC_REPLACE, OnClickedReplace)
  ON_BN_CLICKED(IDC_ADD_DICTIONARY, OnClickedAdd)
  ON_LBN_DBLCLK(IDC_SUGGESTIONS, OnClickedReplace)
  ON_EN_CHANGE(IDC_BADWORD, OnChangedBadWord)
  ON_CBN_SELCHANGE(IDC_LANGUAGE, OnChangedLanguage)
END_MESSAGE_MAP()

SpellCheck::SpellCheck(SourceEdit* edit) : I7BaseDialog(FALSE), m_edit(edit)
{
  m_settingBadWord = false;
}

void SpellCheck::ShowWordFromSelection(void)
{
  // Create the dialog, if needed
  if (GetSafeHwnd() == 0)
    I7BaseDialog::Create(MAKEINTRESOURCE(IDD),m_edit);

  // Get the text of the current selection
  CHARRANGE select = m_edit->GetSelect();
  m_currentBadWord = m_edit->GetTextRange(select.cpMin,select.cpMax);

  // Use the selection as the word to spell check
  m_settingBadWord = true;
  m_badWord.SetWindowText(m_currentBadWord);
  m_settingBadWord = false;
  int len = m_currentBadWord.GetLength();
  m_badWord.SetSel(len,len);
  OnChangedBadWord();

  // Get suggestions for the correct spelling
  char** suggested = NULL;
  int numSuggested = spell->suggest(&suggested,
    TextFormat::UnicodeToLatin1(m_currentBadWord));

  // Update the list of suggestions
  m_suggestions.ResetContent();
  if (numSuggested > 0)
  {
    for (int i = 0; i < numSuggested; i++)
    {
      CStringW suggest = TextFormat::Latin1ToUnicode(suggested[i]);
      m_suggestions.AddString(CString(suggest));
      free(suggested[i]);
    }
    m_suggestions.SetCurSel(0);
  }
  free(suggested);

  // Move the window so that the selection is visible
  m_edit->MoveShowSelect(this);

  // If not visible, show the dialog
  if (IsWindowVisible() == FALSE)
  {
    UpdateDialogControls();
    ShowWindow(SW_SHOW);
    m_badWord.SetFocus();
  }
}

bool SpellCheck::FindWord(bool current)
{
  Initialize();

  // Get the position of the current or next word
  CHARRANGE range = m_edit->GetCurrentWord();
  if (!current)
    range = m_edit->GetNextWord(range);

  while (true)
  {
    // Check that the word is visible
    CHARRANGE lines = m_edit->GetRangeLines(range);
    if (!m_edit->IsLineShown(lines.cpMin) || !m_edit->IsLineShown(lines.cpMax))
      return false;

    // Check for an unknown word
    if (TestWord(range))
      return true;

    // Move to the next word
    CHARRANGE nextRange = m_edit->GetNextWord(range);

    // If the start position has not changed, we are at the end
    if (range.cpMin == nextRange.cpMin)
      return false;
    range = nextRange;
  }
  return false;
}

bool SpellCheck::TestWord(CHARRANGE range)
{
  // Get the word to be checked
  CStringW text = m_edit->GetTextRange(range.cpMin,range.cpMax);

  // Discard any leading and trailing non-alpha characters
  int start = 0;
  int length = text.GetLength();
  while (length > 0)
  {
    wchar_t c = text[start];
    if (IsTestCharacter(c))
      break;
    start++;
    length--;
  }
  while (length > 0)
  {
    wchar_t c = text[start+length-1];
    if (IsTestCharacter(c))
      break;
    length--;
  }

  // Make sure that there is a word to be checked
  if (length == 0)
    return false;

  // Check the list of ignored words
  CStringW wordU((LPCWSTR)text+start,length);
  if (m_ignoreWords.find(wordU) != m_ignoreWords.end())
    return false;

  // Check the dictionary for the word
  CString word = TextFormat::UnicodeToLatin1(wordU);
  if (spell->spell(word) != 0)
    return false;

  // Select the word, as it is not in the dictionary
  m_edit->SetSelect(range);

  // Make sure that the tab containing the selected word is visible
  if (m_edit->IsWindowVisible() == FALSE)
  {
    Panel::GetPanel(m_edit)->SetActiveTab(Panel::Tab_Source);
    m_badWord.SetFocus();
  }
  return true;
}

bool SpellCheck::IsTestCharacter(wchar_t c)
{
  if ((c >= L'a') && (c <= L'z'))
    return true;
  if ((c >= L'A') && (c <= L'Z'))
    return true;
  if (c >= 192)
    return true;
  return false;
}

void SpellCheck::DoneMessage(void)
{
  m_edit->MessageBox("The spelling check is complete.",INFORM_TITLE,MB_ICONINFORMATION|MB_OK);
}

void SpellCheck::StartCheck(void)
{
  m_ignoreWords.clear();
}

void SpellCheck::NextWord(void)
{
  if (FindWord(false) == false)
  {
    ShowWindow(SW_HIDE);
    DoneMessage();
  }
  else
    ShowWordFromSelection();
}

void SpellCheck::SettingsChanged(void)
{
  m_ignoreWords.clear();
  if (GetSafeHwnd() != 0)
  {
    UpdateDialogControls();
    ShowWordFromSelection();
  }
}

void SpellCheck::UpdateDialogControls(void)
{
  if (GetSafeHwnd() != 0)
    m_language.SelectString(-1,currentLanguage->first.c_str());
}

void SpellCheck::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_BADWORD, m_badWord);
  DDX_Control(pDX, IDC_SUGGESTIONS, m_suggestions);
  DDX_Control(pDX, IDC_LANGUAGE, m_language);
}

BOOL SpellCheck::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();

  m_language.ResetContent();
  for (StrPairSet::iterator it = languages.begin(); it != languages.end(); ++it)
    m_language.AddString(it->first.c_str());

  theApp.SetIcon(this);
  return TRUE;
}

void SpellCheck::OnOK()
{
  // As there is no OK button, we can only get here by pressing the Enter key
  // on the dialog, so work out which control is active to decide what to do.
  if (GetFocus() == &m_suggestions)
    OnClickedReplace();
}

void SpellCheck::OnClickedIgnore()
{
  m_ignoreWords.insert(m_currentBadWord);
  NextWord();
}

void SpellCheck::OnClickedReplace()
{
  // Get the replacement text
  CStringW replace;
  if (m_suggestions.IsWindowEnabled())
  {
    int sel = m_suggestions.GetCurSel();
    if (sel == -1)
    {
      MessageBox("There is no suggestion selected to replace the word with.",
        INFORM_TITLE,MB_ICONERROR|MB_OK);
      return;
    }
    CString replaceA;
    m_suggestions.GetText(sel,replaceA);
    replace = replaceA;
  }
  else
    m_badWord.GetWindowText(replace);

  // Replace the selection, then move to the next word
  m_edit->ReplaceSelect(replace);
  NextWord();
}

void SpellCheck::OnClickedAdd()
{
  // Check for invalid characters
  bool invalid = false;
  for (int i = 0; i < m_currentBadWord.GetLength(); i++)
  {
    if (m_currentBadWord[i] > 255)
      invalid = true;
  }
  if (invalid)
  {
    CString msg;
    msg.Format(
      "Cannot add the word to the dictionary, as it contains characters\n"
      "outside of the dictionary's character set, %s.",spell->get_dic_encoding());
    MessageBox(msg,INFORM_TITLE,MB_ICONERROR|MB_OK);
    return;
  }

  // Add the word if it is not already in the dictionary
  CString word = TextFormat::UnicodeToLatin1(m_currentBadWord);
  if (spell->spell(word) == 0)
    spell->add(word);
  NextWord();
}

void SpellCheck::OnChangedBadWord()
{
  if (m_settingBadWord)
    return;

  CStringW badWord;
  m_badWord.GetWindowText(badWord);

  // If the user has changed the current word, disable suggestions
  bool changed = (m_currentBadWord.Compare(badWord) != 0);
  m_suggestions.EnableWindow(!changed);
  if (changed)
    m_suggestions.SetCurSel(-1);
  GetDlgItem(IDC_ADD_DICTIONARY)->EnableWindow(!changed);
}

void SpellCheck::OnChangedLanguage()
{
  int select = m_language.GetCurSel();
  if (select == CB_ERR)
    return;
  CString selectName;
  m_language.GetLBText(select,selectName);

  for (StrPairSet::iterator it = languages.begin(); it != languages.end(); ++it)
  {
    if (it->first == (LPCSTR)selectName)
    {
      CWaitCursor wc;
      currentLanguage = it;
      InitSpellObject();
      theApp.SendAllFrames(InformApp::Spelling,0);
      return;
    }
  }
}
