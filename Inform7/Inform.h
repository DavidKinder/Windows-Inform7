#pragma once

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "FileProtocol.h"
#include "Dib.h"

#include <map>
#include <string>
#include <vector>

#include "Resource.h"

// Title
#define INFORM_TITLE "Inform"

// Registry locations
#define REGISTRY_PATH_BROWSER "Software\\David Kinder\\Inform\\WebBrowser"
#define REGISTRY_PATH_WINDOW  "Software\\David Kinder\\Inform\\Window"

// Size to grow text arrays by
#define TEXT_ARRAY_GROW 8192

// Logging file name
#define LOG_FILE "\\Inform\\i7log.txt"

class InformApp : public CWinApp
{
public:
  InformApp();
  virtual BOOL InitInstance();
  virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnAppExit();
  afx_msg void OnAppPrefs();
  afx_msg void OnAppAbout();
  afx_msg void OnAppWebPage();

public:
  enum Colours
  {
    ColourBack,
    ColourText,
    ColourQuote,
    ColourQuoteBracket,
    ColourComment,
    ColourError,
    ColourHighlight,
    ColourLocked,
    ColourUnlocked,
    ColourSkeinInput,
    ColourTabBack,
    ColourInform6Code,
    ColourBorder,
    ColourTransDiffers,
    ColourTransSame,
    ColourTransNearlySame,
    ColourTransInput,
    ColourTransUnset,
    ColourTransPlayed,
    ColourTransSelect,
    ColourHyperlink,
    ColourContents,
    ColourContentsSelect,
    ColourContentsBelow,
  };

  CFont& GetFont(void);
  const char* GetFontName(void);
  const char* GetFixedFontName(void);
  int GetFontPointSize(void);
  int GetDialogFontSize(void);
  CSize MeasureFont(CFont* font);
  CSize MeasureText(CWnd* button);
  COLORREF GetColour(Colours colour);
  COLORREF BlendedColour(COLORREF col1, int rel1, COLORREF col2, int rel2);
  void SetIcon(CWnd* wnd);

  CString GetAppDir(void);
  CString GetLastProjectDir(void);
  CString GetHomeDir(void);
  FileProtocol& GetUrlProtocol(void);

  void NewFrame(CFrameWnd* frame);
  void FrameClosing(CFrameWnd* frame);
  void GetWindowFrames(CArray<CFrameWnd*>& frames);

  enum Changed
  {
    Extensions,
    Preferences,
    SourceTextSize,
    Spelling,
  };
  void SendAllFrames(Changed changed, int value);

  CDibSection* GetImage(const char* path);
  CSize GetImageSize(const char* path);
  CDibSection* GetCachedImage(const char* name);
  void CacheImage(const char* name, CDibSection* dib);
  CDibSection* CreateScaledImage(CDibSection* fromImage, double scaleX, double scaleY);
  int GetColourDepth(void);

  class OutputSink
  {
  public:
    virtual void Output(const char* msg) = 0;
  };

  void RunMessagePump(void);
  int RunCommand(const char* dir, CString& command, OutputSink& output);
  void RunCensus(bool wait);
  void WriteLog(const char* msg);
  bool IsWaitCursor(void);

  CStringW GetProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR defaultValue);
  BOOL WriteProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR value);

  struct ExtLocation
  {
    std::string author;
    std::string title;
    bool system;
    std::string path;

    ExtLocation(const char* a, const char* t, bool s, const char* p);
    bool operator<(const ExtLocation& el) const;
  };

  void FindExtensions(void);
  const std::vector<ExtLocation>& GetExtensions(void);

protected:
  void CheckIEVersion(double required);
  void CheckMSXML(void);
  void SetMyDocuments(void);
  bool CreateHomeDirs(void);
  void SetFonts(void);

  CString m_fontName;
  CString m_fixedFontName;
  int m_fontSize;
  CFont m_font;

  CArray<CFrameWnd*> m_frames;
  std::map<std::string,CDibSection*> m_bitmaps;
  std::vector<ExtLocation> m_extensions;

  CString m_home;
  FileProtocol m_protocol;
};

extern InformApp theApp;
