#pragma once

#include <map>

#include "Skein.h"
#include "GameBase.h"
#include "MapIterator.h"
#include "InterpreterCommands.h"
#include "PropList.h"

class CDibSection;
class CWinGlkSound;
class GamePair;

typedef CMap<int,int,GameBase*,GameBase*> WindowMap;
typedef MapIterator<int,int,GameBase*,GameBase*> WindowMapIt;

class GameWindow : public CWnd, public GameBase::MainWindow
{
protected: 
  DECLARE_DYNAMIC(GameWindow)

public:
  GameWindow(Skein& skein);
  ~GameWindow();

  static void ExitInstance(void);

  void Create(CWnd* parent);

  void ShowWindow(LPRECT rect, void* owner);
  void HideWindow(void* owner);
  bool CanShow(void* owner);

  void RunInterpreter(const char* dir, const char* file, bool glulx);
  void StopInterpreter(bool clear);
  bool IsRunning(void);
  bool IsWaiting(void);
  void InputFromSkein(void);

  bool GameKeyEvent(CWnd* wnd, WPARAM wParam, LPARAM lParam);
  void GameMouseEvent(GameBase* wnd, int x, int y);
  void GameLinkEvent(GameBase* wnd, int link);
  bool GetLineFromHistory(CStringW& line, int history);
  CWnd* GetWnd(void);
  void GetNeededSize(int wndId, int size, int& w, int& h, const CRect& r);
  void Layout(int wndId, const CRect& r);
  void DeleteWindow(int wndId);
  void DeleteAllWindows(void);
  void DeferMoveWindow(HWND wnd, const CRect& r);

public:
  virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnTimer(UINT nIDEvent);

  afx_msg LRESULT OnEndLineInput(WPARAM, LPARAM);
  afx_msg LRESULT OnEndCharInput(WPARAM, LPARAM);

private:
  bool ReadFromPipe(LPBYTE data, DWORD length);
  void RunInterpreterCommand(void);

  void CommandFatalError(char* error, int errLength);
  void CommandCreateWindow(int wndId, int splitId, int pairId, int method, int size, TerpWindow type);
  void CommandDestroyWindow(int wndId);
  void CommandPrintOutput(int wndId, wchar_t* text, int textLength);
  void CommandSetStyle(int wndId, TerpTextStyle style, int size);
  void CommandSetColour(int wndId, int* fore, int* back);
  void CommandSetCursor(int wndId, int row, int column);
  void CommandReadLine(int wndId, int initial, bool restart);
  void CommandReadKey(bool restart);
  void CommandClear(int wndId, int reverse);
  void CommandDraw(int wndId, int image, int val1, int val2, int width, int height);
  void CommandArrange(int wndId, int method, int size, int keyId, bool swap);
  void CommandPlaySound(int channelId, int sound, int repeats, int volume);
  void CommandStopSound(int channelId);
  void CommandSetVolume(int channelId, int volume);
  void CommandFillRect(int wndId, int* rect, int* colour);
  void CommandBackColour(int wndId, int* colour);
  void CommandSetLink(int wndId, int link);
  void CommandSetParagraph(int wndId, TerpJustify justify);
  void CommandCancelLine(int wndId);

  void Resize(void);
  void SendReturn(int returnCommand, int dataLength, const void* data);
  void SendInputLine(int wndId, const CStringW& line);
  void SendInputKey(int key);
  void UpdateTranscript(void);

  CDibSection* GetImage(int image, COLORREF alpha);
  CString GetMediaPath(const wchar_t* section, int resource);
  void ClearMedia(void);
  void WriteImageSizes(void);
  CString GetFileDir(void);

  int GetWindowId(GameBase* wnd);
  GamePair* GetParentPair(int wndId, int pairId);

  void DeferredMoveWindows(void);

  void* m_owner;
  CSize m_fontSize;

  WindowMap m_windows;
  int m_mainWndId;
  CMap<HWND,HWND,CRect,CRect&> m_deferred;

  HANDLE m_inputPipe, m_inputPipe2;
  HANDLE m_outputPipe, m_outputPipe2;
  HANDLE m_interpreter;

  enum TerpState
  {
    TerpOutput,
    TerpInputLine,
    TerpInputChar,
  };
  TerpState m_state;

  Skein& m_skein;
  CArray<wchar_t> m_transcript;

  typedef std::map<std::pair<int,COLORREF>,CDibSection*> ImageMap;
  typedef std::map<int,CWinGlkSound*> SoundMap;

  PropList m_manifest;
  ImageMap m_images;
  SoundMap m_sounds;
};
