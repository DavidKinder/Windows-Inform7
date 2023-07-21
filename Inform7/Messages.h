#pragma once

// Sent to the game display to indicate that the user has input a complete line,
// ending the current line input.
#define WM_ENDLINEINPUT   WM_APP+1

// Sent to the game display to indicate that the user has input a single character,
// ending the current single character input.
#define WM_ENDCHARINPUT   WM_APP+2

// Sent to the window frame to make the game be played through to the skein node
// given by wParam. If necessary the game will be rebuilt.
#define WM_PLAYSKEIN      WM_APP+3

// Sent to the window frame to determine if the game is currently playing.
#define WM_GAMERUNNING    WM_APP+4

// Sent to the window frame to determine if the game is currently waiting for input.
#define WM_GAMEWAITING    WM_APP+5

// Sent to the skein window to rename the skein node given by wParam to the
// Unicode string given by lParam.
#define WM_RENAMENODE     WM_APP+6

// Sent to the window frame to determine the height of the section selection area
// present at the top of some panes.
#define WM_PANEHEADING    WM_APP+7

// Sent to the window frame to switch to the panel determined by the string in
// wParam. Which side is switched is determined by the window handle in lParam:
// the side switched is the one that the window handle is not part of.
#define WM_SELECTVIEW     WM_APP+8

// Sent to the window frame to paste the string in wParam into the source code
// panel.
#define WM_PASTECODE      WM_APP+9

// Sent to the window frame to indicate that the game display has caught the
// run-time problem given by the number in wParam.
#define WM_RUNTIMEPROB    WM_APP+10

// Sent to the window frame to switch the focus to the side given by the
// number in wParam.
#define WM_SELECTSIDE     WM_APP+11

// Sent to the window frame to search the source code for the string given in
// wParam.
#define WM_SEARCHSOURCE   WM_APP+12

// Sent to the window frame to search the documentation for the string given in
// wParam.
#define WM_SEARCHDOC      WM_APP+13

// Sent to the window frame to switch to the skein tab and show the node
// given by wParam. The originating window is given by lParam.
#define WM_SHOWSKEIN      WM_APP+14

// Sent to the window frame to indicate that the interpreter running the game
// in the story tab has failed.
#define WM_TERPFAILED     WM_APP+15

// Sent to the window frame to get the project directory.
#define WM_PROJECTDIR     WM_APP+16

// Sent to the window frame to cause the next thread to be played, if any.
#define WM_PLAYNEXTTHREAD WM_APP+17

// Sent to the window frame to check if there are any skein threads
// to be played
#define WM_CANPLAYALL     WM_APP+18

// Sent from a HTML control window to its parent to indicate that the
// user has selected a link
#define WM_USERNAVIGATE   WM_APP+19

typedef CArray<CStringW,CStringW&> SourceHeading;

struct SourceRange
{
  int startLine;
  int endLine;
  SourceHeading heading;
  bool full;
};

// Sent to a source tab to show the range described by the
// SourceRange structure pointed to by wParam. This structure
// is allocated on the heap and is freed by the recipient.
#define WM_SOURCERANGE    WM_APP+20

// Sent to a source tab to indicate that the next (wParam == 0) or
// previous (wParam == 1) source range should be shown.
#define WM_NEXTRANGE      WM_APP+21

// Sent to the window frame to indicate that the status of whether
// the current project has been edited since it was last saved
// should be checked, and updated if necessary. If wParam is not
// zero then the project must now be recompiled.
#define WM_PROJECTEDITED  WM_APP+22

// Sent to the window frame to download and install the extensions
// given by the string array in wParam. This string array is
// allocated on the heap and is freed by the recipient.
#define WM_EXTDOWNLOAD    WM_APP+23

// Sent to the extensions tab window to indicate that navigating
// to a Public Library web page has failed.
#define WM_PUBLIBERROR    WM_APP+24

// Sent to the window frame to set the progress control position
// (in wParam) and progress text (in lParam). If the position is
// -1 then the progress window is hidden.
#define WM_PROGRESS       WM_APP+25

// Sent to the window frame to create a new project, named by the
// string in lParam, and to paste the string in wParam into the
// source code panel.
#define WM_NEWPROJECT     WM_APP+26

// Sent to the window frame to get the file extension for the
// project (e.g. ".inform").
#define WM_PROJECTEXT     WM_APP+27

// Sent to the window frame to get the current project type (i.e.
// the appropriate value of the ProjectType enumeration).
#define WM_PROJECTTYPE    WM_APP+28

// Sent to the window frame to notify that the story window has
// been activated.
#define WM_STORYACTIVE    WM_APP+29

// Sent to the window frame to check if the current operation should
// stop: 1 is returned if yes, otherwise 0.
#define WM_WANTSTOP       WM_APP+30

// Sent to the window frame to get the name of the current story.
// This is returned as a pointer to a CString allocated on the heap.
#define WM_STORYNAME      WM_APP+31

// Sent to the a tab window to update the context sensitive help.
#define WM_UPDATEHELP     WM_APP+32

class Command
{
public:
  virtual ~Command()
  {
  }

  virtual void Run(void) = 0;
};

// Sent to the window frame to run the overall process of running an
// animation in the skein window. If lParam is non-zero if specifies
// an instance of Command to be run after the animation completes.
// This command object is allocated on the heap and is freed by the
// recipient.
#define WM_ANIMATESKEIN   WM_APP+33

// Sent to a property sheet page after the dialog font has been set
// and the page resized.
#define WM_AFTERFONTSET   WM_APP+34

// Sent to a property sheet to resize the currently active page.
#define WM_RESIZEPAGE     WM_APP+35

// Sent to a preferences dialog sheet to indicate that the preview
// is to be updated.
#define WM_UPDATEPREVIEW  WM_APP+36

// Sent to a preferences dialog sheet to indicate that a colour button
// has selected a new colour.
#define WM_COLOURCHANGED  WM_APP+37

// Sent to the Find in Files dialog to resize the results column.
#define WM_RESIZERESULTS  WM_APP+38

enum FindCommand
{
  FindCmd_Close,
  FindCmd_Next,
  FindCmd_Previous,
  FindCmd_Replace,
  FindCmd_ReplaceAll
};

// Sent to the parent window of Find and Replace dialogs to indicate
// a command has been triggered in that dialog.
#define WM_FINDREPLACECMD WM_APP+39

// Sent to the window frame to trigger replaying all skein threads.
#define WM_REPLAYALL      WM_APP+40

// Sent to the window frame to get the number of times that the Testing tab
// has been shown for this project. If wParam is not zero the number of times
// is incremented.
#define WM_TESTINGTABSHOWN WM_APP+41

// Sent to the parent window when an item is clicked on a CommandListBox
// control. wParam contains the control ID and lParam the index of the clicked
// item.
#define WM_CMDLISTCLICKED WM_APP+42

// Sent to the main application thread when the IFTF news file has been
// downloaded.
#define WM_NEWSDOWNLOAD   WM_APP+43

// Set to the window frame to determine if the file given in wParam was
// built during the last compilation.
#define WM_ISBUILDFILE    WM_APP+44

// Sent to the window frame to confirm the current action (currently only
// installing an extension via inbuild).
#define WM_CONFIRMACTION  WM_APP+45
