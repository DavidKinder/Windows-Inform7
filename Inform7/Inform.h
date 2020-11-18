#pragma once

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "Dib.h"

#include <map>
#include <string>
#include <vector>

#include "Resource.h"

// Title
#define INFORM_TITLE "Inform"
#define L_INFORM_TITLE L"Inform"

// Registry location
#define REGISTRY_PATH_WINDOW  "Software\\David Kinder\\Inform\\Window"

// Size to grow text arrays by
#define TEXT_ARRAY_GROW 8192

// Logging file name
#define LOG_FILE "\\Inform\\i7log.txt"

enum ProjectType
{
  Project_I7,
  Project_I7XP
};

class InformApp : public CWinApp
{
public:
  InformApp();
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoWaitCursor(int nCode);
	virtual BOOL OnIdle(LONG lCount);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnAppExit();
  afx_msg void OnAppPrefs();
  afx_msg void OnAppAbout();
  afx_msg void OnAppWebPage();
  afx_msg void OnUpdateEditUseSel(CCmdUI *pCmdUI);

public:
  enum Colours
  {
    ColourBack,
    ColourText,
    ColourQuote,
    ColourSubstitution,
    ColourComment,
    ColourError,
    ColourHighlight,
    ColourFaint,
    ColourSkeinLine,
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
    ColourI7XP
  };

  COLORREF GetColour(Colours colour);
  COLORREF BlendedColour(COLORREF col1, int rel1, COLORREF col2, int rel2);
  void SetIcon(CWnd* wnd);

  enum Fonts
  {
    FontSystem = 0,
    FontDisplay = 1,
    FontFixedWidth = 2,
    FontPanel = 3,
    FontSmall = 4
  };

  CFont* GetFont(Fonts font);
  const char* GetFontName(Fonts font);
  int GetFontSize(Fonts font);
  CSize MeasureFont(CFont* font);
  CSize MeasureText(CWnd* button);
  CSize MeasureText(LPCSTR text, CFont* font);

  bool GetTestMode(void) const;
  CString GetAppDir(void) const;
  CString GetLastProjectDir(void);
  CString GetHomeDir(void);
  CString PathToUrl(const char* path);

  void NewFrame(CFrameWnd* frame);
  void FrameClosing(CFrameWnd* frame);
  void GetWindowFrames(CArray<CFrameWnd*>& frames);

  enum Changed
  {
    Extensions,
    Preferences,
    Spelling,
    DownloadedExt
  };
  void SendAllFrames(Changed changed, int value);

  CDibSection* GetImage(const char* path, bool adjustGamma);
  CSize GetImageSize(const char* path);
  CDibSection* GetCachedImage(const char* name);
  void CacheImage(const char* name, CDibSection* dib);
  CDibSection* CreateScaledImage(CDibSection* fromImage, double scaleX, double scaleY);
  int GetColourDepth(void);

  class OutputSink
  {
  public:
    virtual void Output(const char* msg) = 0;
    virtual bool WantStop() = 0;
  };

  struct CreatedProcess
  {
    HANDLE process;
    DWORD processId;

    CreatedProcess();
    void set(PROCESS_INFORMATION pi);
    void close();
  };

  void RunMessagePump(void);
  CreatedProcess CreateProcess(const char* dir, CString& command, STARTUPINFO& start, bool debug, const char* exeFile);
  void WaitForProcessEnd(HANDLE process);
  CreatedProcess RunCensus(void);
  int RunCommand(const char* dir, CString& command, const char* exeFile, OutputSink& output, bool hasSymbols);
  void HandleDebugEvents(void);
  CString GetTraceForProcess(DWORD processId);
  void WriteLog(const char* msg);
  bool IsWaitCursor(void);

  CStringW GetProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR defaultValue);
  BOOL WriteProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR value);

  struct CompilerVersion
  {
    CString id;
    CString label;
    CString description;

    CompilerVersion()
    {
    }

    CompilerVersion(const char* i, const char* l, const char* d) : id(i), label(l), description(d)
    {
    }
  };

  void FindCompilerVersions(void);
  const std::vector<CompilerVersion>& GetCompilerVersions(void);

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
  void AddToExtensions(const char* author, const char* title, const char* path);
  const std::vector<ExtLocation>& GetExtensions(void);
  const ExtLocation* GetExtension(const char* author, const char* title);

protected:
  void ClearScaledImages(void);
  void SetMyDocuments(bool showMsgs);
  bool CreateHomeDirs(void);
  void SetFonts(void);

  CString m_fontNames[5];
  int m_fontSizes[5];
  CFont m_fonts[5];

  CArray<CFrameWnd*> m_frames;
  std::map<std::string,CDibSection*> m_bitmaps;
  std::vector<ExtLocation> m_extensions;

  struct DebugProcess
  {
    HANDLE process;
    HANDLE thread;
    DWORD threadId;
    CString imageFile;
    LPVOID imageBase;
    DWORD imageSize;

    DebugProcess() : process(0), thread(0), threadId(0), imageBase(0), imageSize(0)
    {
    }
  };
  std::map<DWORD,DebugProcess> m_debugging;
  std::map<DWORD,CString> m_traces;

  std::vector<CompilerVersion> m_versions;

  CString m_home;
  HANDLE m_job;
};

extern InformApp theApp;

#ifdef _WIN64
void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);
#else
extern "C" __declspec(dllimport) void ScaleGfx(COLORREF*, UINT, UINT, COLORREF*, UINT, UINT);
#endif
