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

// Sent to the skein window to rename the label of the skein node given by wParam
// to the Unicode string given by lParam.
#define WM_LABELNODE      WM_APP+7

// Sent to the window frame to determine the height of the section selection area
// present at the top of some panes.
#define WM_PANEHEADING    WM_APP+8

// Sent to the window frame to switch to the panel determined by the string in
// wParam. Which side is switched is determined by the window handle in lParam:
// the side switched is the one that the window handle is not part of.
#define WM_SELECTVIEW     WM_APP+9

// Sent to the window frame to paste the string in wParam into the source code
// panel.
#define WM_PASTECODE      WM_APP+10

// Sent to the window frame to indicate that the game display has caught the
// run-time problem given by the number in wParam.
#define WM_RUNTIMEPROB    WM_APP+11

// Sent to the window frame to switch the focus to the side given by the
// number in wParam.
#define WM_SELECTSIDE     WM_APP+12

// Sent to the window frame to search the source code for the string given in
// wParam.
#define WM_SEARCHSOURCE   WM_APP+13

// Sent to the window frame to search the documentation for the string given in
// wParam.
#define WM_SEARCHDOC      WM_APP+14

// Sent to the window frame to switch to the transcript tab and show the node
// given by wParam. The originating window is given by lParam.
#define WM_SHOWTRANSCRIPT WM_APP+15

// Sent to the window frame to switch to the skein tab and show the node
// given by wParam. The originating window is given by lParam.
#define WM_SHOWSKEIN      WM_APP+16

// Sent to the window frame to indicate that the interpreter running the game
// in the game tab has failed.
#define WM_TERPFAILED     WM_APP+17

// Sent to the window frame to get the project directory
#define WM_PROJECTDIR     WM_APP+18

// Sent to the window frame to get the end node of the transcript thread.
#define WM_TRANSCRIPTEND  WM_APP+19

// Sent to the window frame to cause the next thread to be played, if any.
#define WM_PLAYNEXTTHREAD WM_APP+20

// Sent to the transcript window to set the expected text for the skein
// node given by wParam to the Unicode string given by lParam.
#define WM_SETEXPECTED    WM_APP+21

// Sent to the window frame to check if there are any blessed threads
// to be played
#define WM_CANPLAYALL     WM_APP+22

// Sent from a HTML control window to its parent to indicate that the
// user has selected a link
#define WM_USERNAVIGATE   WM_APP+23

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
#define WM_SOURCERANGE    WM_APP+24

// Sent to a source tab to indicate that the next (wParam == 0) or
// previous (wParam == 1) source range should be shown.
#define WM_NEXTRANGE      WM_APP+25

// Sent to the window frame to indicate that the status of whether
// the current project has been edited since it was last saved
// should be checked, and updated if necessary.
#define WM_PROJECTEDITED  WM_APP+26
