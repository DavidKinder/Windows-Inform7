#include "stdafx.h"
#include "ColourScheme.h"
#include "Inform.h"

#include "DarkMode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ColourScheme::ColourScheme()
{
  sortIndex = CUSTOM_SORT_INDEX;
  head = theApp.GetColour(InformApp::ColourHeading);
  main = theApp.GetColour(InformApp::ColourText);
  comment = theApp.GetColour(InformApp::ColourComment);
  quote = theApp.GetColour(InformApp::ColourQuote);
  subst = theApp.GetColour(InformApp::ColourSubstitution);
  source = theApp.GetColour(InformApp::ColourBack);
  ext = theApp.GetColour(InformApp::ColourI7XP);
}

ColourScheme::ColourScheme(int srtIdx,
  COLORREF hd, COLORREF mn, COLORREF cmt, COLORREF qt, COLORREF sbst, COLORREF src, COLORREF xt)
{
  sortIndex = srtIdx;
  head = hd;
  main = mn;
  comment = cmt;
  quote = qt;
  subst = sbst;
  source = src;
  ext = xt;
}

void ColourScheme::AdjustForDarkMode(void)
{
  bool dark = DarkMode::IsEnabled(REGISTRY_INFORM_WINDOW);
  const char* convertFrom = dark ? "Light Mode" : "Dark Mode";
  const char* convertTo = dark ? "Dark Mode" : "Light Mode";

  CRegKey regKey;
  if (regKey.Open(HKEY_CURRENT_USER,REGISTRY_INFORM_WINDOW,KEY_READ|KEY_WRITE) == ERROR_SUCCESS)
  {
    char schemeName[MAX_PATH];
    DWORD schemeNameLen = sizeof schemeName;
    if (regKey.QueryStringValue("Colour Scheme",schemeName,&schemeNameLen) == ERROR_SUCCESS)
    {
      if (strcmp(schemeName,convertFrom) == 0)
        regKey.SetStringValue("Colour Scheme",convertTo);
    }
  }
}

const char* ColourScheme::GetDefaultSchemeName(CWnd* wnd)
{
  DarkMode* dark = DarkMode::GetActive(wnd);
  return dark ? "Dark Mode" : "Light Mode";
}

const std::map<std::string,ColourScheme> ColourScheme::DefaultSchemes =
{
  { "Light Mode",
      ColourScheme(1,
        RGB(0xe6,0x3d,0x42), // Headings
        RGB(0x00,0x00,0x00), // Main text
        RGB(0x46,0xaa,0x25), // Comments
        RGB(0x41,0x83,0xfa), // Quoted text
        RGB(0xe8,0x3c,0xf9), // Substitutions
        RGB(0xff,0xff,0xff), // Source background
        RGB(0xff,0xff,0xe4)) // Extension project background
  },
  { "Dark Mode",
      ColourScheme(2,
        RGB(0xea,0x6c,0x69), // Headings
        RGB(0xff,0xff,0xff), // Main text
        RGB(0x95,0xf9,0x9c), // Comments
        RGB(0x5d,0xd5,0xfd), // Quoted text
        RGB(0xee,0x8b,0xe7), // Substitutions
        RGB(0x00,0x00,0x00), // Source background
        RGB(0x42,0x0d,0x00)) // Extension project background
  },
  { "Seven Seas",
      ColourScheme(3,
        RGB(0x34,0x00,0xff), // Headings
        RGB(0x00,0x00,0x64), // Main text
        RGB(0x6d,0xa7,0xe9), // Comments
        RGB(0x00,0x6a,0xff), // Quoted text
        RGB(0x2d,0x6c,0xd0), // Substitutions
        RGB(0xe9,0xe9,0xff), // Source background
        RGB(0xe9,0xff,0xe9)) // Extension project background
  },
  { "Traditional",
      ColourScheme(4,
        RGB(0x00,0x00,0x00), // Headings
        RGB(0x00,0x00,0x00), // Main text
        RGB(0x24,0x6e,0x24), // Comments
        RGB(0x00,0x4d,0x99), // Quoted text
        RGB(0x4d,0x4d,0xff), // Substitutions
        RGB(0xff,0xff,0xff), // Source background
        RGB(0xff,0xff,0xe4)) // Extension project background
  }
};
