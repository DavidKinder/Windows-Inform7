#pragma once

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "BaseDialog.h"
#include "Resource.h"

#include <map>
#include <set>
#include <string>
#include <vector>

// Title
#define INFORM_TITLE "Inform"
#define L_INFORM_TITLE L"Inform"

// Registry location
#define REGISTRY_INFORM "Software\\David Kinder\\Inform"
#define REGISTRY_INFORM_WINDOW REGISTRY_INFORM "\\Window"

// Size to grow text arrays by
#define TEXT_ARRAY_GROW 8192

// Logging file name
#define LOG_FILE "\\Inform\\i7log.txt"

// The base URL of the Inform Public Library
#define PUBLIC_LIBRARY_URL "https://ganelson.github.io/inform-public-library"

class CDibSection;
class RecentProjectList;

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

  afx_msg BOOL OnOpenRecentFile(UINT nID);
  afx_msg void OnFileClearRecent();
  afx_msg void OnAppExit();
  afx_msg void OnAppPrefs();
  afx_msg void OnAppAbout();
  afx_msg void OnAppWebPage();
  afx_msg void OnAppLauncher();
  afx_msg void OnUpdateEditUseSel(CCmdUI *pCmdUI);
  afx_msg void OnNewsDownload(WPARAM, LPARAM);

public:
  enum Colours
  {
    ColourBack,
    ColourText,
    ColourHeading,
    ColourQuote,
    ColourSubstitution,
    ColourComment,
    ColourError,
    ColourHighlight,
    ColourFaint,
    ColourSkeinLine,
    ColourTabBack,
    ColourInform6Code,
    ColourBorder,
    ColourTranscriptBack,
    ColourTranscriptLine,
    ColourTranscriptDelete,
    ColourTranscriptInsert,
    ColourHyperlink,
    ColourContents,
    ColourContentsSelect,
    ColourContentsBelow,
    ColourI7XP,
    ColourTestMe
  };

  COLORREF GetColour(Colours colour);
  COLORREF BlendedColour(COLORREF col1, int rel1, COLORREF col2, int rel2);

  void DrawSelectRect(CWnd* wnd, CDC& dc, CRect& rect, bool hot);
  void SetIcon(CWnd* wnd);

  enum Fonts
  {
    FontSystem = 0,
    FontDisplay = 1,
    FontFixedWidth = 2,
    FontSmall = 3,
    FontVertical = 4,
    FONT_COUNT = 5
  };

  CFont* GetFont(CWnd* wnd, Fonts font);
  const char* GetFontName(Fonts font);
  int GetFontSize(Fonts font);
  CSize MeasureFont(CWnd* wnd, CFont* font);
  CSize MeasureText(CWnd* button);
  CSize MeasureText(CWnd* wnd, LPCSTR text);
  void CreatePointFont(CWnd* wnd, CFont* font, int pointSize, LPCSTR faceName);

  bool GetTestMode(void) const;
  CString GetAppDir(void) const;
  CString GetLastProjectDir(void);
  CString GetHomeDir(void);
  CString PathToUrl(const char* path);

  CFrameWnd* GetActiveFrame(void);
  bool IsValidFrame(CFrameWnd* frame);
  void NewFrame(CFrameWnd* frame);
  void FrameClosing(CFrameWnd* frame);
  void GetWindowFrames(CArray<CFrameWnd*>& frames);

  enum Changed
  {
    Extensions,
    Preferences,
    Spelling,
    DownloadedExt,
    LightDarkMode
  };
  void SendAllFrames(Changed changed, int value);
  void AddModalDialog(CWnd* dialog);
  void RemoveModalDialog(CWnd* dialog);

  CDibSection* GetImage(const char* path, bool adjustGamma);
  CSize GetImageSize(const char* path);
  CDibSection* GetCachedImage(const char* name);
  void CacheImage(const char* name, CDibSection* dib);
  CDibSection* CreateScaledImage(CDibSection* fromImage, double scaleX, double scaleY);
  CDibSection* CreateScaledImage(CDibSection* fromImage, CSize size);

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
  CreatedProcess CreateProcess(const char* dir, CString& command, STARTUPINFO& start, bool debug);
  void WaitForProcessEnd(HANDLE process);
  void AddProcessToJob(HANDLE process);
  int RunCommand(const char* dir, CString& command, OutputSink& output);
  void HandleDebugEvents(void);
  CString GetTraceForProcess(DWORD processId);
  void WriteLog(const char* msg);
  bool IsWaitCursor(void);

  CStringW GetProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR defaultValue);
  BOOL WriteProfileString(LPCSTR section, LPCWSTR entry, LPCWSTR value);

  void WriteOpenProjectsOnExit(void);
  void OpenPreviousProjects(void);
  RecentProjectList* GetRecentProjectList(void);

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

  CString PickDirectory(const char* title, const char* folderLabel, const char* okLabel,
    const char* initialDir, CWnd* parent);

  void UpdateMaterialsFolder(UINT_PTR token, LPCSTR path);
  CString GetMaterialsFolder(UINT_PTR token);

protected:
  void ClearGeneratedImages(void);
  void SetMyDocuments(bool showMsgs);
  bool CreateHomeDirs(void);
  void SetFonts(void);

  CString m_fontNames[FONT_COUNT];
  int m_fontSizes[FONT_COUNT];
  std::map<std::pair<Fonts,int>,CFont*> m_fonts;

  CArray<CFrameWnd*> m_frames;
  std::set<CWnd*> m_modalDialogs;
  std::map<std::string,CDibSection*> m_bitmaps;
  std::vector<ExtLocation> m_extensions;

  void HookApiFunction(const char* callingDllName, const char* calledDllName, const char* functionName, PROC newFunction);

  struct DebugProcess
  {
    HANDLE process;
    HANDLE thread;
    DWORD threadId;

    DebugProcess() : process(0), thread(0), threadId(0)
    {
    }
  };
  std::map<DWORD,DebugProcess> m_debugging;
  std::map<DWORD,CString> m_traces;

  std::vector<CompilerVersion> m_versions;

  // Map of materials folder locations, accessible from any thread. The map keys are really pointers
  // to ProjectFrame instances, but cast to UINT_PTR to indicate that on any thread but the UI thread,
  // such pointers can only be treated as opaque tokens.
  CCriticalSection m_materialsLock;
  std::map<UINT_PTR,CString> m_materials;

  CString m_home;
  HANDLE m_job;
  bool m_doneProjectsOnExit;
};

extern InformApp theApp;
