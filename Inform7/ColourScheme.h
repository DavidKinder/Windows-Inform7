#pragma once

#include <map>
#include <string>

#define CUSTOM_SORT_INDEX 1000

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

  static void AdjustForDarkMode(void);
  static const char* GetDefaultSchemeName(CWnd* wnd);

  const static std::map<std::string,ColourScheme> DefaultSchemes;
};
