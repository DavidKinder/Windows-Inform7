#include "stdafx.h"
#include "GameWindow.h"
#include "GameBlank.h"
#include "GameGraphics.h"
#include "GameGrid.h"
#include "GamePair.h"
#include "GameText.h"
#include "Inform.h"
#include "Messages.h"
#include "OSLayer.h"
#include "DSoundEngine.h"
#include "GlkSound.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MANIFEST_FILE "\\manifest.plist"

static bool soundInit = false;

IMPLEMENT_DYNAMIC(GameWindow, CWnd)

BEGIN_MESSAGE_MAP(GameWindow, CWnd)
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_MESSAGE(WM_ENDLINEINPUT, OnEndLineInput)
  ON_MESSAGE(WM_ENDCHARINPUT, OnEndCharInput)
END_MESSAGE_MAP()

GameWindow::GameWindow(Skein& skein) : m_owner(NULL), m_skein(skein)
{
  m_inputPipe = 0;
  m_inputPipe2 = 0;
  m_outputPipe = 0;
  m_outputPipe2 = 0;
  m_interpreter = 0;
  m_state = TerpOutput;
  m_mainWndId = -1;
}

GameWindow::~GameWindow()
{
  ClearMedia();

  WindowMapIt it(m_windows);
  while (it.Iterate())
    delete it.Value();
}

void GameWindow::ExitInstance(void)
{
  if (soundInit)
  {
    // Shut down the sound system
    CDSoundEngine::GetSoundEngine().StopThread();
    CWinGlkSoundLoader::AllSoundStopped();
    CDSoundEngine::GetSoundEngine().Destroy();
    CWinGlkSoundLoader::RemoveLoaders();
  }
}

void GameWindow::Create(CWnd* parent)
{
  // Create the containing window
  if (!CWnd::Create(NULL,"",0,CRect(0,0,0,0),parent,0))
  {
    TRACE("Failed to create game display\n");
  }
}

void GameWindow::ShowWindow(LPRECT rect, void* owner)
{
  if ((m_owner == NULL) || (m_owner == owner))
  {
    MoveWindow(rect,TRUE);
    BringWindowToTop();
    CWnd::ShowWindow(SW_SHOW);
    m_owner = owner;

    WindowMapIt it(m_windows);
    if (it.Iterate(RUNTIME_CLASS(GameText)))
      ((GameText*)it.Value())->SetFocus();
  }
  else
    ASSERT(FALSE);
}

void GameWindow::HideWindow(void* owner)
{
  ASSERT(m_owner == owner);

  CWnd::ShowWindow(SW_HIDE);
  m_owner = NULL;
}

bool GameWindow::CanShow(void* owner)
{
  return ((m_owner == NULL) || (m_owner == owner));
}

void GameWindow::RunInterpreter(const char* dir, const char* file, bool glulx)
{
  ASSERT(m_interpreter == 0);
  if (m_interpreter != 0)
    return;

  // Delete any open windows
  DeleteAllWindows();

  // Reset the media cache
  CString manifestPath;
  manifestPath.Format("%s"MANIFEST_FILE,
    (LPCSTR)GetParentFrame()->SendMessage(WM_PROJECTDIR));
  m_manifest.Load(manifestPath);
  ClearMedia();
  WriteImageSizes();

  // Reset the game state
  m_state = TerpOutput;
  m_transcript.SetSize(0,TEXT_ARRAY_GROW);

  // Create pipes to connect to the interpreter
  SECURITY_ATTRIBUTES security;
  ::ZeroMemory(&security,sizeof security);
  security.nLength = sizeof security;
  security.bInheritHandle = TRUE;
  HANDLE inputRead, inputWrite;
  ::CreatePipe(&inputRead,&inputWrite,&security,0);
  HANDLE outputRead, outputWrite;
  ::CreatePipe(&outputRead,&outputWrite,&security,0);

  // Use the pipe for standard I/O and, for Windows 9X, make sure that the
  // console window is hidden
  STARTUPINFO start;
  ::ZeroMemory(&start,sizeof start);
  start.cb = sizeof start;
  start.hStdInput = inputRead;
  start.hStdOutput = outputWrite;
  start.hStdError = outputWrite;
  start.wShowWindow = SW_HIDE;
  start.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;

  // Get screen properties to send to the interpreter
  CRect screen;
  GetClientRect(screen);
  CFont font;
  font.CreatePointFont(10*theApp.GetFontPointSize(),theApp.GetFixedFontName());
  m_fontSize = theApp.MeasureFont(&font);

  // Work out the interpreter to run
  CString terp;
  if (glulx)
    terp = theApp.CWinApp::GetProfileString("Game","Glulx Interpreter","glulxe");
  else
    terp = "frotz";

  // Generate the command line for the interpreter
  CString command;
  command.Format("\"%s\\Interpreters\\%s\" \"%s\\%s\" %d %d %d %d \"%s\"",
    (LPCSTR)theApp.GetAppDir(),(LPCSTR)terp,
    dir,file,screen.Width(),screen.Height(),m_fontSize.cx,m_fontSize.cy,
    (LPCSTR)GetFileDir());

  // Create the process. If the application is being debugged we don't make ourselves
  // a debugger of the interpreter, as that stops the real debugger being attached to
  // the interpreter.
  DWORD flags = CREATE_NO_WINDOW;
  if (theOS.IsDebuggerPresent() == false)
    flags |= DEBUG_PROCESS;
  PROCESS_INFORMATION process;
  char* cmdLine = command.GetBuffer();
  BOOL created = ::CreateProcess(NULL,cmdLine,NULL,NULL,TRUE,flags,NULL,dir,&start,&process);
  command.ReleaseBuffer();

  // If the process started, save the process handles
  if (created)
  {
    m_interpreter = process.hProcess;
    ::CloseHandle(process.hThread);

    m_inputPipe = inputWrite;
    m_outputPipe = outputRead;
    m_inputPipe2 = inputRead;
    m_outputPipe2 = outputWrite;

    // Check the interpreter every 100ms and the sounds every 500ms
    SetTimer(0,100,NULL);
    SetTimer(1,500,NULL);
  }
  else
    StopInterpreter(false);
}

void GameWindow::StopInterpreter(bool clear)
{
  // Stop the timer events
  if (GetSafeHwnd())
    KillTimer(0);

  // Close any open pipes
  ::CloseHandle(m_inputPipe);
  ::CloseHandle(m_inputPipe2);
  ::CloseHandle(m_outputPipe);
  ::CloseHandle(m_outputPipe2);
  m_inputPipe = 0;
  m_inputPipe2 = 0;
  m_outputPipe = 0;
  m_outputPipe2 = 0;

  if (m_interpreter != 0)
  {
    ::TerminateProcess(m_interpreter,0);
    ::CloseHandle(m_interpreter);

    WindowMapIt it(m_windows);
    while (it.Iterate(RUNTIME_CLASS(GameText)))
      ((GameText*)it.Value())->StopLineInput(false);

    // Update the status of the last node in the skein
    UpdateTranscript();
  }
  m_interpreter = 0;

  // Stop any playing sounds
  for (SoundMap::const_iterator it = m_sounds.begin(); it != m_sounds.end(); ++it)
    delete it->second;
  m_sounds.clear();

  if (clear)
  {
    DeleteAllWindows();
    ClearMedia();
  }
  else
    DeferredMoveWindows();
}

bool GameWindow::IsRunning(void)
{
  return (m_interpreter != 0);
}

bool GameWindow::IsWaiting(void)
{
  if (m_interpreter != 0)
    return ((m_state == TerpInputLine) || (m_state == TerpInputChar));
  return false;
}

void GameWindow::InputFromSkein(void)
{
  // Only do anything if waiting for input
  switch (m_state)
  {
  case TerpInputLine:
    {
      // Discard any current input
      WindowMapIt it(m_windows);
      if (it.Iterate(RUNTIME_CLASS(GameText)))
      {
        ((GameText*)it.Value())->StopLineInput(true);

        // Reset the state and attempt to get line input again
        m_state = TerpOutput;
        CommandReadLine(it.Key(),0,true);
      }
    }
    break;
  case TerpInputChar:
    // Reset the state and attempt to get key input again
    m_state = TerpOutput;
    CommandReadKey(true);
    break;
  }
}

bool GameWindow::ReadFromPipe(LPBYTE data, DWORD length)
{
  DWORD readTotal = 0;
  while (true)
  {
    DWORD readThis = 0;
    if (!::ReadFile(m_outputPipe,data+readTotal,length-readTotal,&readThis,NULL))
      break;
    if (readThis == 0)
      break;
    readTotal += readThis;
    if (readTotal >= length)
      break;
    ::Sleep(50);
  }
  if (readTotal == length)
    return true;

  CString msg;
  msg.Format("Failed to read interpreter data\nExpected %d, actually read %d\nError code 0x%x",
    length,readTotal,::GetLastError());
  MessageBox(msg,INFORM_TITLE,MB_ICONEXCLAMATION|MB_OK);
  StopInterpreter(false);
  return false;
}

void GameWindow::RunInterpreterCommand(void)
{
  // Get the command
  int command = 0;
  if (!ReadFromPipe((LPBYTE)&command,sizeof command))
    return;

  // Get the length of data associated with the command
  int dataLength = 0;
  if (!ReadFromPipe((LPBYTE)&dataLength,sizeof dataLength))
    return;

  // Get the data associated with the command
  void* data = NULL;
  if (dataLength > 0)
  {
    data = alloca(dataLength);
    if (!ReadFromPipe((LPBYTE)data,dataLength))
      return;
  }

  int* idata = (int*)data;
  switch (command)
  {
  case Command_FatalError:
    CommandFatalError((char*)data,dataLength);
    break;
  case Command_CreateWindow:
    CommandCreateWindow(idata[0],idata[1],idata[2],idata[3],idata[4],(TerpWindow)idata[5]);
    break;
  case Command_DestroyWindow:
    CommandDestroyWindow(idata[0]);
    break;
  case Command_ArrangeWindow:
    CommandArrange(idata[0],idata[1],idata[2],idata[3],idata[4] != 0);
    break;
  case Command_PrintOutput:
    CommandPrintOutput(*((short*)data),((wchar_t*)data)+1,(dataLength/2)-1);
    break;
  case Command_SetStyle:
    CommandSetStyle(idata[0],(TerpTextStyle)idata[1],idata[2]);
    break;
  case Command_SetColour:
    CommandSetColour(idata[0],idata+1,idata+4);
    break;
  case Command_SetCursor:
    CommandSetCursor(idata[0],idata[1],idata[2]);
    break;
  case Command_ReadLine:
    CommandReadLine(idata[0],idata[1],false);
    break;
  case Command_ReadKey:
    CommandReadKey(false);
    break;
  case Command_Clear:
    CommandClear(idata[0],idata[1]);
    break;
  case Command_Draw:
    CommandDraw(idata[0],idata[1],idata[2],idata[3],idata[4],idata[5]);
    break;
  case Command_PlaySound:
    CommandPlaySound(idata[0],idata[1],idata[2],idata[3]);
    break;
  case Command_StopSound:
    CommandStopSound(idata[0]);
    break;
  case Command_SetVolume:
    CommandSetVolume(idata[0],idata[1]);
    break;
  case Command_FillRect:
    CommandFillRect(idata[0],idata+1,idata+5);
    break;
  case Command_BackColour:
    CommandBackColour(idata[0],idata+1);
    break;
  case Command_SetLink:
    CommandSetLink(idata[0],idata[1]);
    break;
  case Command_SetParagraph:
    CommandSetParagraph(idata[0],(TerpJustify)idata[1]);
    break;
  case Command_CancelLine:
    CommandCancelLine(idata[0]);
    break;
  case Command_CancelKey:
    m_state = TerpOutput;
    break;
  }
}

void GameWindow::CommandFatalError(char* error, int errLength)
{
  WindowMapIt it(m_windows);
  if (it.Iterate(RUNTIME_CLASS(GameText)))
  {
    // Show the error message in bold text
    it.Value()->SetStyle(true,false,false,false,0);
    it.Value()->AddText(L"Fatal Error: ",false);
    it.Value()->AddText(CStringW(error,errLength),false);
  }
}

void GameWindow::CommandCreateWindow(int wndId, int splitId, int pairId, int method, int size, TerpWindow type)
{
  // Check that the split window is compatible with whether a window is open
  if ((splitId == -1) && (m_mainWndId != -1))
    return;
  if ((splitId != -1) && (m_mainWndId == -1))
    return;
  if ((splitId != -1) && (pairId == -1))
    return;

  // Create the new window
  GameBase* wnd = NULL;
  switch (type)
  {
  case Window_Blank:
    wnd = new GameBlank(this);
    break;
  case Window_Text:
    wnd = new GameText(this);
    break;
  case Window_Grid:
    wnd = new GameGrid(this);
    break;
  case Window_Graphics:
    wnd = new GameGraphics(this);
    break;
  }
  if (wnd == NULL)
    return;
  m_windows[wndId] = wnd;

  if (splitId == -1)
  {
    // If there is no window to split, this must be the new main window
    m_mainWndId = wndId;
  }
  else
  {
    // Create a new pair window for the split
    GamePair* pair = new GamePair(this,splitId,wndId,method,size);
    m_windows[pairId] = pair;

    // Find the parent of the split window, if any, and reorder the windows
    GamePair* parent = GetParentPair(splitId,m_mainWndId);
    if (parent != NULL)
      parent->ReplaceChild(splitId,pairId);
    else
      m_mainWndId = pairId;
  }
  Resize();
}

void GameWindow::CommandDestroyWindow(int wndId)
{
  // Remove this window from its parent, if any
  GamePair* parent1 = GetParentPair(wndId,m_mainWndId);
  if (parent1 != NULL)
  {
    int otherChildId = -1;
    if (parent1->GetChild1() == wndId)
      otherChildId = parent1->GetChild2();
    else
      otherChildId = parent1->GetChild1();

    int parent1Id = GetWindowId(parent1);
    GamePair* parent2 = GetParentPair(parent1Id,m_mainWndId);
    if (parent2 != NULL)
      parent2->ReplaceChild(parent1Id,otherChildId);
    else
      m_mainWndId = otherChildId;
    parent1->RemoveChildren();
    DeleteWindow(parent1Id);
  }

  // Make sure that no key window points to this window
  WindowMapIt it(m_windows);
  if (it.Iterate(RUNTIME_CLASS(GamePair)))
    ((GamePair*)it.Value())->RemoveKey(wndId);

  // Delete this window and rebuild remaining windows
  DeleteWindow(wndId);
  Resize();
}

void GameWindow::CommandPrintOutput(int wndId, wchar_t* text, int textLength)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  CStringW str(text,textLength);
  wnd->AddText(str,false);

  if (wnd->IsKindOf(RUNTIME_CLASS(GameText)))
  {
    for (int i = 0; i < textLength; i++)
      m_transcript.Add(text[i]);

    // Has a run-time error occurred?
    int problem = 0;
    if (swscanf(str,L"*** Run-time problem P%d",&problem) == 1)
      GetParentFrame()->SendMessage(WM_RUNTIMEPROB,(WPARAM)problem);
  }
}

void GameWindow::CommandSetStyle(int wndId, TerpTextStyle style, int size)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  wnd->SetStyle(
    (style & StyleBold) != 0,
    (style & StyleItalic) != 0,
    (style & StyleReverse) != 0,
    (style & StyleFixed) != 0,
    size);
}

void GameWindow::CommandSetColour(int wndId, int* fore, int* back)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  COLORREF foreColour = RGB(fore[0],fore[1],fore[2]);
  COLORREF backColour = RGB(back[0],back[1],back[2]);
  wnd->SetColours(foreColour,backColour);
}

void GameWindow::CommandSetCursor(int wndId, int column, int row)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  wnd->SetCursor(column,row);
}

void GameWindow::CommandReadLine(int wndId, int initial, bool restart)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  if (wnd->IsKindOf(RUNTIME_CLASS(GameText)))
  {
    if (!restart)
      UpdateTranscript();
    ((GameText*)wnd)->AllowLineInput(initial);

    CStringW skeinLine;
    if (m_skein.NextLine(skeinLine))
    {
      wnd->AddText(skeinLine,true);
      wnd->MoveToEnd();
      SendInputLine(GetWindowId(wnd),skeinLine);
    }
    else
    {
      m_state = TerpInputLine;
      GetParentFrame()->PostMessage(WM_PLAYNEXTTHREAD);
    }
  }
}

void GameWindow::CommandReadKey(bool restart)
{
  WindowMapIt it(m_windows);
  if (it.Iterate(RUNTIME_CLASS(GameText)) == false)
    return;

  if (!restart)
    UpdateTranscript();
  ((GameText*)it.Value())->AllowCharInput();

  CStringW skeinLine;
  if (m_skein.NextLine(skeinLine))
  {
    int key = 0;
    if (skeinLine.IsEmpty() == FALSE)
      key = skeinLine[0];
    SendInputKey(key);
  }
  else
  {
    m_state = TerpInputChar;
    GetParentFrame()->PostMessage(WM_PLAYNEXTTHREAD);
  }
}

void GameWindow::CommandClear(int wndId, int reverse)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  wnd->ClearText(false,reverse != 0);

  if (wnd->IsKindOf(RUNTIME_CLASS(GameText)))
    m_transcript.Add('\n');
}

void GameWindow::CommandDraw(int wndId, int image, int val1, int val2, int width, int height)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  CDibSection* bitmap = GetImage(image,wnd->GetAlphaColour());
  if (bitmap != NULL)
    wnd->Draw(bitmap,val1,val2,width,height);
}

void GameWindow::CommandArrange(int wndId, int method, int size, int keyId, bool swap)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  if (keyId != -1)
  {
    GameBase* keyWnd = NULL;
    if (m_windows.Lookup(keyId,keyWnd) == FALSE)
    {
      ASSERT(FALSE);
      return;
    }
  }

  if (wnd->IsKindOf(RUNTIME_CLASS(GamePair)))
  {
    ((GamePair*)wnd)->SetArrangement(method,size,keyId,swap);
    Resize();
  }
}

void GameWindow::CommandPlaySound(int channelId, int sound, int repeats, int volume)
{
  // Initialize the sound system
  if (soundInit == false)
  {
    CWinGlkSoundLoader::InitLoaders();
    CDSoundEngine::GetSoundEngine().Initialize();
    soundInit = true;
  }

  // If there is a sound already playing on the channel, stop it
  SoundMap::iterator it = m_sounds.find(channelId);
  if (it != m_sounds.end())
  {
    delete it->second;
    m_sounds.erase(it);
  }

  // Get the path to the resource file and work out its file extension
  CString path = GetMediaPath(L"Sounds",sound);
  if (path.IsEmpty())
    return;
  int lastPeriod = path.ReverseFind('.');
  if (lastPeriod == -1)
    return;
  CString pathExt = path.Mid(lastPeriod+1);

  // Identify the loader for the resource
  CWinGlkSoundLoader* loader = NULL;
  for (int i = 0; i < CWinGlkSoundLoader::GetLoaderCount(); i++)
  {
    CWinGlkSoundLoader* testLoader = CWinGlkSoundLoader::GetLoader(i);
    for (int j = 0; j < testLoader->GetNumberFileExtensions(); j++)
    {
      if (pathExt.CompareNoCase(testLoader->GetFileExtension(j)) == 0)
        loader = testLoader;
    }
  }
  if (loader == NULL)
    return;

  // Create a sound object
  CWinGlkSound* soundObj = loader->GetSound(path);
  if (soundObj == NULL)
    return;

  // Play the sound and store the sound object
  if (soundObj->Play(repeats,volume) == false)
  {
    delete soundObj;
    return;
  }
  m_sounds[channelId] = soundObj;
}

void GameWindow::CommandStopSound(int channelId)
{
  SoundMap::iterator it = m_sounds.find(channelId);
  if (it != m_sounds.end())
  {
    delete it->second;
    m_sounds.erase(it);
  }
}

void GameWindow::CommandSetVolume(int channelId, int volume)
{
  // Find the sound on the channel, if any, and set its volume
  SoundMap::iterator it = m_sounds.find(channelId);
  if (it == m_sounds.end())
    return;
  it->second->SetVolume(volume);
}

void GameWindow::CommandFillRect(int wndId, int* rect, int* colour)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  if (wnd->IsKindOf(RUNTIME_CLASS(GameGraphics)))
  {
    ((GameGraphics*)wnd)->FillRect(
      CRect(rect[0],rect[1],rect[2],rect[3]),RGB(colour[0],colour[1],colour[2]));
  }
}

void GameWindow::CommandBackColour(int wndId, int* colour)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  wnd->SetBackColour(RGB(colour[0],colour[1],colour[2]));
}

void GameWindow::CommandSetLink(int wndId, int link)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  wnd->SetLink(link);
}

void GameWindow::CommandSetParagraph(int wndId, TerpJustify justify)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  switch (justify)
  {
  case Just_LeftFlush:
    wnd->SetParagraph(GameBase::JustifyLeft);
    break;
  case Just_LeftRight:
    wnd->SetParagraph(GameBase::JustifyFull);
    break;
  case Just_Centred:
    wnd->SetParagraph(GameBase::JustifyCentre);
    break;
  case Just_RightFlush:
    wnd->SetParagraph(GameBase::JustifyRight);
    break;
  default:
    ASSERT(FALSE);
    break;
  }
}

void GameWindow::CommandCancelLine(int wndId)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd) == FALSE)
  {
    ASSERT(FALSE);
    return;
  }

  if (wnd->IsKindOf(RUNTIME_CLASS(GameText)))
  {
    ((GameText*)wnd)->StopLineInput(true);
    m_state = TerpOutput;
  }
}

bool GameWindow::GameKeyEvent(CWnd* wnd, WPARAM wParam, LPARAM lParam)
{
  switch (m_state)
  {
  case TerpInputLine:
    if (wParam == VK_RETURN)
    {
      // Post a message to indicate that line input should
      // end. The actual handling of line input needs to
      // occur after this return key event has completed.
      PostMessage(WM_ENDLINEINPUT,(WPARAM)wnd->GetSafeHwnd());
      return false;
    }
    break;
  case TerpInputChar:
    {
      UINT key = 0;

      // Examine the key code to see if it is a special key
      switch (wParam)
      {
      case VK_LEFT:
        key = Key_Left;
        break;
      case VK_RIGHT:
        key = Key_Right;
        break;
      case VK_UP:
        key = Key_Up;
        break;
      case VK_DOWN:
        key = Key_Down;
        break;
      default:
        // If not a special key, get the Unicode value of the key
        key = theOS.ToUnicode((UINT)wParam,(UINT)lParam,0);
        break;
      }

      if (key != 0)
      {
        // Post a message to indicate that char input should end
        PostMessage(WM_ENDCHARINPUT,key);
        return false;
      }
    }
    break;
  }
  return true;
}

void GameWindow::GameMouseEvent(GameBase* wnd, int x, int y)
{
  int mouse[3];
  mouse[0] = GetWindowId(wnd);
  mouse[1] = x;
  mouse[2] = y;
  SendReturn(Return_Mouse,sizeof mouse,mouse);
}

void GameWindow::GameLinkEvent(GameBase* wnd, int link)
{
  int event[2];
  event[0] = GetWindowId(wnd);
  event[1] = link;
  SendReturn(Return_Link,sizeof event,event);
}

bool GameWindow::GetLineFromHistory(CStringW& line, int history)
{
  return m_skein.GetLineFromHistory(line,history);
}

CWnd* GameWindow::GetWnd(void)
{
  return this;
}

void GameWindow::Layout(int wndId, const CRect& r)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd))
    wnd->Layout(r);
}

void GameWindow::DeleteWindow(int wndId)
{
  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd))
  {
    if (wnd->IsKindOf(RUNTIME_CLASS(GamePair)))
    {
      int child1 = ((GamePair*)wnd)->GetChild1();
      if (child1 != -1)
        DeleteWindow(child1);

      int child2 = ((GamePair*)wnd)->GetChild2();
      if (child2 != -1)
        DeleteWindow(child2);
    }

    m_windows.RemoveKey(wndId);
    if (m_mainWndId == wndId)
      m_mainWndId = -1;

    delete wnd;
  }
}

void GameWindow::DeleteAllWindows(void)
{
  CArray<GameBase*> windows;
  WindowMapIt it(m_windows);
  while (it.Iterate())
    windows.Add(it.Value());

  m_windows.RemoveAll();
  m_deferred.RemoveAll();
  m_mainWndId = -1;

  for (int i = 0; i < windows.GetSize(); i++)
    delete windows.GetAt(i);
}

void GameWindow::DeferMoveWindow(HWND wnd, const CRect& r)
{
  m_deferred[wnd] = r;
}

void GameWindow::GetNeededSize(int wndId, int size, int& w, int& h, const CRect& r)
{
  w = 0;
  h = 0;

  GameBase* wnd = NULL;
  if (m_windows.Lookup(wndId,wnd))
    wnd->GetNeededSize(size,w,h,m_fontSize,r);
}

BOOL GameWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  WindowMapIt it(m_windows);
  while (it.Iterate())
  {
    if (it.Value()->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void GameWindow::OnPaint()
{
  CPaintDC dc(this);

  // If there are game windows open, there is no need to paint this window
  if (m_windows.GetSize() > 0)
    return;

  CRect client;
  GetClientRect(client);
  dc.FillSolidRect(client,theApp.GetColour(InformApp::ColourBack));
}

void GameWindow::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType,cx,cy);
  Resize();

  // Send back the size of the display
  CRect screen;
  GetClientRect(screen);
  int size[2];
  size[0] = screen.Width();
  size[1] = screen.Height();
  SendReturn(Return_Size,sizeof size,size);
}

void GameWindow::OnTimer(UINT nIDEvent)
{
  static bool processingTimer = false;
  if (processingTimer)
    return;
  processingTimer = true;

  switch (nIDEvent)
  {
  case 0:
    if (m_interpreter != 0)
    {
      while (true)
      {
        // Is there any output remaining?
        DWORD available = 0;
        ::PeekNamedPipe(m_outputPipe,NULL,0,NULL,&available,NULL);
        if (available > 0)
          RunInterpreterCommand();
        else
          break;
      }
      DeferredMoveWindows();

      // Has the interpreter failed?
      DEBUG_EVENT debug;
      while (::WaitForDebugEvent(&debug,2))
      {
        DWORD status = DBG_CONTINUE;
        if (debug.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
          switch (debug.u.Exception.ExceptionRecord.ExceptionCode)
          {
          case EXCEPTION_ACCESS_VIOLATION:
            if (debug.u.Exception.dwFirstChance)
              status = DBG_EXCEPTION_NOT_HANDLED;
            else
            {
              ::TerminateProcess(m_interpreter,10);
              GetParentFrame()->PostMessage(WM_TERPFAILED);
            }
            break;
          case EXCEPTION_STACK_OVERFLOW:
            ::TerminateProcess(m_interpreter,11);
            GetParentFrame()->PostMessage(WM_TERPFAILED);
            break;
          }
        }
        ::ContinueDebugEvent(debug.dwProcessId,debug.dwThreadId,status);
      }

      // Is the interpreter still running?
      DWORD code = 0;
      ::GetExitCodeProcess(m_interpreter,&code);
      if (code != STILL_ACTIVE)
      {
        // Is there any final output remaining?
        while (true)
        {
          DWORD available = 0;
          ::PeekNamedPipe(m_outputPipe,NULL,0,NULL,&available,NULL);
          if (available > 0)
            RunInterpreterCommand();
          else
            break;
        }
        DeferredMoveWindows();
        StopInterpreter(false);
      }
    }
    break;

  case 1:
    {
      // Find any sounds that have finished playing
      std::set<int> done;
      for (SoundMap::const_iterator it = m_sounds.begin(); it != m_sounds.end(); ++it)
      {
        if (it->second->IsPlaying() == false)
          done.insert(it->first);
      }

      // Delete finished sounds and notify the interpreter
      for (std::set<int>::iterator dit = done.begin(); dit != done.end(); ++dit)
      {
        SoundMap::iterator it = m_sounds.find(*dit);
        delete it->second;
        m_sounds.erase(it);

        int notify[1];
        notify[0] = *dit;
        SendReturn(Return_SoundOver,sizeof notify,notify);
      }
    }
    break;
  }

  CWnd::OnTimer(nIDEvent);
  processingTimer = false;
}

LRESULT GameWindow::OnEndLineInput(WPARAM wParam, LPARAM)
{
  m_state = TerpOutput;

  // Find a text buffer window
  GameText* wnd = NULL;
  WindowMapIt it(m_windows);
  while (it.Iterate(RUNTIME_CLASS(GameText)))
  {
    if (it.Value()->GetSafeHwnd() == (HWND)wParam)
      wnd = (GameText*)it.Value();
  }
  if (wnd == NULL)
    return 0;

  // Get the input line
  CStringW input = wnd->StopLineInput(false);

  // If the interpreter is still running, send back data
  if (m_interpreter != 0)
  {
    SendInputLine(GetWindowId(wnd),input);

    // Add the input line to the skein
    m_skein.NewLine(input);
  }
  return 0;
}

LRESULT GameWindow::OnEndCharInput(WPARAM wParam, LPARAM)
{
  int key = (int)wParam;
  m_state = TerpOutput;

  // If the interpreter is still running, send back data
  if (m_interpreter != 0)
  {
    SendInputKey(key);

    // Add the input key to the skein
    CStringW input;
    input.AppendChar(key);
    m_skein.NewLine(input);
  }
  return 0;
}

void GameWindow::Resize(void)
{
  GameBase* mainWnd = NULL;
  if (m_windows.Lookup(m_mainWndId,mainWnd) == FALSE)
    return;

  CRect client;
  GetClientRect(client);
  mainWnd->Layout(client);
}

void GameWindow::SendReturn(int returnCommand, int dataLength, const void* data)
{
  if (m_interpreter != 0)
  {
    DWORD written = 0;
    ::WriteFile(m_inputPipe,&returnCommand,sizeof returnCommand,&written,NULL);
    ::WriteFile(m_inputPipe,&dataLength,sizeof dataLength,&written,NULL);
    if (dataLength > 0)
      ::WriteFile(m_inputPipe,data,dataLength,&written,NULL);
  }
}

void GameWindow::SendInputLine(int wndId, const CStringW& line)
{
  // Send back the input line
  int dataLen = (sizeof (int)) + (line.GetLength() * sizeof (wchar_t));
  void* data = alloca(dataLen);
  *((int*)data) = wndId;
  memcpy((int*)data+1,(LPCWSTR)line,line.GetLength() * sizeof (wchar_t));
  SendReturn(Return_ReadLine,dataLen,data);
}

void GameWindow::SendInputKey(int key)
{
  // Send back the input key
  SendReturn(Return_ReadKey,sizeof key,&key);
}

void GameWindow::UpdateTranscript(void)
{
  CStringW transcript(m_transcript.GetData(),(int)m_transcript.GetSize());
  transcript.Trim();
  m_transcript.SetSize(0,TEXT_ARRAY_GROW);
  m_skein.UpdateAfterPlaying(transcript);
}

CDibSection* GameWindow::GetImage(int image, COLORREF alpha)
{
  // Check if the image is cached
  ImageMap::const_iterator it = m_images.find(std::make_pair(image,alpha));
  if (it != m_images.end())
    return it->second;

  // Look up the image number in the manifest file
  CString path = GetMediaPath(L"Graphics",image);
  if (path.IsEmpty())
    return NULL;

  // Get the image and cache it
  CDibSection* dib = theApp.GetImage(path);
  if (dib == NULL)
    return NULL;

  // Alpha blend with the background colour
  if (alpha != -1)
    dib->AlphaBlend(alpha);

  // Cache the image and return it
  m_images[std::make_pair(image,alpha)] = dib;
  return dib;
}

CString GameWindow::GetMediaPath(const wchar_t* section, int resource)
{
  // Look up the resource number in the manifest file
  CStringW num;
  num.Format(L"%d",resource);
  CStringW file = m_manifest.GetString(section,num);
  if (file.IsEmpty())
    return "";

  // Work out the path to the resource file
  CString projectDir = (LPCSTR)GetParentFrame()->SendMessage(WM_PROJECTDIR);
  int projectExt = projectDir.Find(".inform");
  if (projectExt == -1)
    return "";
  CString path;
  path.Format("%s Materials\\%S",projectDir.Left(projectExt),file);
  return path;
}

void GameWindow::ClearMedia(void)
{
  for (ImageMap::const_iterator it = m_images.begin(); it != m_images.end(); ++it)
    delete it->second;
  m_images.clear();

  for (SoundMap::const_iterator it = m_sounds.begin(); it != m_sounds.end(); ++it)
    delete it->second;
  m_sounds.clear();
}

void GameWindow::WriteImageSizes(void)
{
  // Get the project directory
  CString projectDir = (LPCSTR)GetParentFrame()->SendMessage(WM_PROJECTDIR);
  int projectExt = projectDir.Find(".inform");
  if (projectExt == -1)
    return;

  // Open a file to write the image sizes to
  CString sizePath;
  sizePath.Format("%s\\Build\\image.txt",(LPCSTR)projectDir);
  CStdioFile sizeFile;
  if (!sizeFile.Open(sizePath,CFile::modeWrite|CFile::typeText|CFile::modeCreate))
    return;

  // Get all the image resources listed in the manifest
  PropList::KeyValueMap values;
  m_manifest.GetKeyValues(L"Graphics",values);

  // For each one, write out its size
  MapIterator<CStringW,LPCWSTR,CStringW,LPCWSTR> it(values);
  while (it.Iterate())
  {
    int num = _wtoi(it.Key());
    if (num > 0)
    {
      CString imagePath;
      imagePath.Format("%s Materials\\%S",projectDir.Left(projectExt),it.Value());
      CSize size = theApp.GetImageSize(imagePath);

      CString sizeLine;
      sizeLine.Format("%d %d %d\n",num,size.cx,size.cy);
      sizeFile.WriteString(sizeLine);
    }
  }
}

CString GameWindow::GetFileDir(void)
{
  // Get the project name, minus the extension
  CString projectDir = (LPCSTR)GetParentFrame()->SendMessage(WM_PROJECTDIR);
  int projectExt = projectDir.Find(".inform");
  if (projectExt == -1)
    return "";

  // Check if a "Materials/Files" directory exists
  CString path;
  path.Format("%s Materials\\Files",projectDir.Left(projectExt));
  if (::GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
    return path;

  // If not, just use the same directory as the project
  int sep = projectDir.ReverseFind('\\');
  return projectDir.Left(sep);
}

int GameWindow::GetWindowId(GameBase* wnd)
{
  WindowMapIt it(m_windows);
  while (it.Iterate())
  {
    if (it.Value() == wnd)
      return it.Key();
  }
  return -1;
}

GamePair* GameWindow::GetParentPair(int wndId, int pairId)
{
  GameBase* pair = NULL;
  if (m_windows.Lookup(pairId,pair))
  {
    if (pair->IsKindOf(RUNTIME_CLASS(GamePair)))
    {
      if (((GamePair*)pair)->GetChild1() == wndId)
        return (GamePair*)pair;
      if (((GamePair*)pair)->GetChild2() == wndId)
        return (GamePair*)pair;

      GamePair* childPair = GetParentPair(wndId,((GamePair*)pair)->GetChild1());
      if (childPair != NULL)
        return childPair;
      childPair = GetParentPair(wndId,((GamePair*)pair)->GetChild2());
      if (childPair != NULL)
        return childPair;
    }
  }
  return NULL;
}

void GameWindow::DeferredMoveWindows(void)
{
  if (m_deferred.IsEmpty())
    return;

  MapIterator<HWND,HWND,CRect,CRect&> it(m_deferred);
  while (it.Iterate())
  {
    if (::IsWindow(it.Key()))
      CWnd::FromHandle(it.Key())->MoveWindow(it.Value());
  }
  m_deferred.RemoveAll();

  {
    WindowMapIt it(m_windows);
    while (it.Iterate())
      it.Value()->MoveToEnd();
  }
}
