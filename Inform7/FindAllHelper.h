#pragma once

#include "Inform.h"

#include <regex>

enum FindRule
{
  FindRule_Contains = 0,
  FindRule_StartsWith = 1,
  FindRule_FullWord = 2,
  FindRule_Regex = 3
};

enum FoundIn
{
  FoundIn_Source,
  FoundIn_Extension,
  FoundIn_WritingWithInform,
  FoundIn_RecipeBook,
  FoundIn_Unknown
};

class DarkMode;

struct FindResult
{
  FindResult();
  bool operator<(const FindResult& fr) const;

  CString TypeName(void);
  COLORREF Colour(DarkMode* dark, bool darker);

  CStringW context;
  CHARRANGE inContext;

  FoundIn type;
  CString doc;
  CString docSort;
  CString path;
  CHARRANGE loc;
};

class FindResultsHeaderCtrl : public CHeaderCtrl
{
protected:
  DECLARE_MESSAGE_MAP()
  afx_msg void OnPaint();
};

class FindResultsListCtrl : public CListCtrl
{
public:
  void SubclassHeader(FindResultsHeaderCtrl* header);
  void SetDarkMode(DarkMode* dark);

protected:
  DECLARE_MESSAGE_MAP()

  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg void OnHeaderDividerDblClick(NMHDR* pNotifyStruct, LRESULT* result);
};

class FindAllHelper
{
public:
  void InitResultsCtrl(FindResultsListCtrl* ctrl, bool details);
  void UpdateResultsCtrl(FindResultsListCtrl* ctrl, bool details);

  void OnResultsDraw(FindResultsListCtrl* ctrl, NMLVCUSTOMDRAW* custom, LRESULT* result);
  void OnResultsResize(FindResultsListCtrl* ctrl);

  // Find results in the given text
  void Find(const CString& textUtf8, const CStringW& findText, bool ignoreCase, FindRule findRule,
    const char* doc, const char* docSort, const char* path, FoundIn type);

  std::vector<FindResult> results;

  static const char* RegexError(const std::regex_error& ex);

private:
  int FindLineStart(const CString& text, int pos);
  int FindLineEnd(const CString& text, int pos);
  CStringW GetMatchRange(const CString& text, int start, int end);

  void DrawText(CDC* dc, LPCWSTR text, int length, CRect& rect, UINT format);
  int MeasureText(CDC* dc, LPCWSTR text, int length);
};
