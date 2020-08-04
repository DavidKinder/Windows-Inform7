#include "stdafx.h"
#include "SourceEdit.h"
#include "Inform.h"
#include "Messages.h"
#include "EditFind.h"
#include "TextFormat.h"
#include "DpiFunctions.h"

#include "Platform.h"
#include "Scintilla.h"
#include "SciLexer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Elastic tabstops implementation
void ElasticTabStops_OnModify(sptr_t edit, int start, int end);
void ElasticTabStops_OnClear(sptr_t edit);

IMPLEMENT_DYNAMIC(SourceEdit, CWnd)

BEGIN_MESSAGE_MAP(SourceEdit, CWnd)
  ON_WM_DESTROY()
  ON_WM_CONTEXTMENU()
  ON_WM_CHAR()
  ON_WM_MOUSEWHEEL()
  ON_WM_GETDLGCODE()

  ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
  ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
  ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
  ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
  ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_CUT, OnEditCut)
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
  ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateNeedText)
  ON_COMMAND(ID_EDIT_FIND, OnEditFind)
  ON_UPDATE_COMMAND_UI(ID_EDIT_REPLACE, OnUpdateNeedText)
  ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_NEXT, OnUpdateNeedFindText)
  ON_COMMAND(ID_EDIT_FIND_NEXT, OnEditFindNext)
  ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_PREV, OnUpdateNeedFindText)
  ON_COMMAND(ID_EDIT_FIND_PREV, OnEditFindPrev)
  ON_UPDATE_COMMAND_UI(ID_EDIT_SPELLING, OnUpdateNeedText)
  ON_UPDATE_COMMAND_UI(ID_EDIT_USE_SEL, OnUpdateEditUseSel)
  ON_COMMAND(ID_EDIT_USE_SEL, OnEditUseSel)
  ON_COMMAND(ID_EDIT_SCROLL, OnEditScroll)
  ON_COMMAND(ID_EDIT_SPELLING, OnEditSpelling)
  ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
  ON_COMMAND_RANGE(ID_FORMAT_SHIFT_RIGHT, ID_FORMAT_SHIFT_LEFT, OnFormatShift)
  ON_COMMAND_RANGE(ID_FORMAT_COMMENT, ID_FORMAT_UNCOMMENT, OnFormatComment)
  ON_COMMAND(ID_FORMAT_RENUMBER, OnFormatRenumber)

  ON_NOTIFY_REFLECT(SCN_SAVEPOINTREACHED, OnSavePointReached)
  ON_NOTIFY_REFLECT(SCN_SAVEPOINTLEFT, OnSavePointLeft)
  ON_NOTIFY_REFLECT(SCN_STYLENEEDED, OnStyleNeeded)
  ON_NOTIFY_REFLECT(SCN_CHARADDED, OnCharAdded)
  ON_NOTIFY_REFLECT(SCN_MODIFIED, OnDocModified)
  ON_NOTIFY_REFLECT(SCNX_CONVERTPASTE, OnConvertPaste)
  ON_NOTIFY_REFLECT(SCNX_CONVERTCOPYTOCLIP, OnConvertCopyToClip)

  ON_REGISTERED_MESSAGE(FINDMSG, OnFindReplaceCmd)
END_MESSAGE_MAP()

SourceEdit::SourceEdit() : m_fileTime(CTime::GetCurrentTime()), m_spell(this)
{
  m_editPtr = 0;
  m_marker = 0;
  m_markSel.cpMin = -1;
  m_markSel.cpMax = -1;

  m_includeExt = false;

  // Default preferences values
  m_fontName = theApp.GetFontName(InformApp::FontDisplay);
  m_fontSize = theApp.GetFontSize(InformApp::FontDisplay);
  m_syntaxHighlight = true;
  m_colourHead = theApp.GetColour(InformApp::ColourText);
  m_colourMain = theApp.GetColour(InformApp::ColourText);
  m_colourComment = theApp.GetColour(InformApp::ColourComment);
  m_colourQuote = theApp.GetColour(InformApp::ColourQuote);
  m_colourSubst = theApp.GetColour(InformApp::ColourSubstitution);
  m_styleHead = 2; // Bold
  m_styleMain = 0; // Regular
  m_styleComment = 2;
  m_styleQuote = 2;
  m_styleSubst = 0;
  m_underHead = false;
  m_underMain = false;
  m_underComment = false;
  m_underQuote = false;
  m_underSubst = false;
  m_sizeHead = 0; // Normal
  m_sizeMain = 0;
  m_sizeComment = 1; // Small
  m_sizeQuote = 0;
  m_sizeSubst = 0;
  m_autoIndent = true;
  m_autoNumber = false;
  m_elasticTabStops = false;
}

BOOL SourceEdit::Create(CWnd* parent, UINT id, COLORREF back, bool includeExt)
{
  m_includeExt = includeExt;

  CREATESTRUCT cs;
  ::ZeroMemory(&cs,sizeof cs);
  cs.lpszClass = "Scintilla";
  cs.style = WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN;
  cs.hwndParent = parent->GetSafeHwnd();
  cs.hMenu = (HMENU)(UINT_PTR)id;
  cs.hInstance = AfxGetInstanceHandle();

  if (!PreCreateWindow(cs))
  {
    PostNcDestroy();
    return FALSE;
  }
  HWND wnd = ::CreateWindowEx(cs.dwExStyle,cs.lpszClass,cs.lpszName,cs.style,
    cs.x,cs.y,cs.cx,cs.cy,cs.hwndParent,cs.hMenu,cs.hInstance,cs.lpCreateParams);
  if (wnd == NULL)
    return FALSE;

  // Get the Unicode state of the 'real' window procedure before subclassing
  BOOL isUnicode = ::IsWindowUnicode(wnd);
  if (!SubclassWindow(wnd))
    return FALSE;

  CSize fontSize = theApp.MeasureFont(theApp.GetFont(InformApp::FontDisplay));
  m_editPtr = (sptr_t)SendMessage(SCI_GETDIRECTPOINTER);

  CallEdit(SCI_SETKEYSUNICODE,isUnicode);
  CallEdit(SCI_SETCODEPAGE,SC_CP_UTF8);
  CallEdit(SCI_SETEOLMODE,SC_EOL_LF);
  CallEdit(SCI_SETPASTECONVERTENDINGS,1);
  CallEdit(SCI_SETWRAPMODE,SC_WRAP_WORD);
  CallEdit(SCI_SETMODEVENTMASK,SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT);
  for (int i = 0; i < 5; i++)
    CallEdit(SCI_SETMARGINWIDTHN,i,0);
  CallEdit(SCI_SETMARGINLEFT,0,fontSize.cx);
  CallEdit(SCI_SETMARGINRIGHT,0,fontSize.cx);
  CallEdit(SCI_SETSELFORE,TRUE,::GetSysColor(COLOR_HIGHLIGHTTEXT));
  CallEdit(SCI_SETSELBACK,TRUE,::GetSysColor(COLOR_HIGHLIGHT));
  CallEdit(SCI_MARKERDEFINE,m_marker,SC_MARK_BACKGROUND);
  CallEdit(SCI_SETLEXER,SCLEX_CONTAINER);
  CallEdit(SCI_INDICSETSTYLE,0,INDIC_SQUIGGLE);
  CallEdit(SCI_INDICSETFORE,0,theApp.GetColour(InformApp::ColourError));
  CallEdit(SCI_SETINDICATORCURRENT,0);
  SetStyles(back);

  // Change the bindings for the Home and End keys
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME,SCI_HOMEDISPLAY);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+(SCMOD_SHIFT<<16),SCI_HOMEDISPLAYEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+(SCMOD_ALT<<16),SCI_HOME);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+((SCMOD_ALT|SCMOD_SHIFT)<<16),SCI_HOMEEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END,SCI_LINEENDDISPLAY);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+(SCMOD_SHIFT<<16),SCI_LINEENDDISPLAYEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+(SCMOD_ALT<<16),SCI_LINEEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+((SCMOD_ALT|SCMOD_SHIFT)<<16),SCI_LINEENDEXTEND);

  // Remove unwanted key bindings
  CallEdit(SCI_CLEARCMDKEY,SCK_ADD+(SCMOD_CTRL<<16));
  CallEdit(SCI_CLEARCMDKEY,SCK_SUBTRACT+(SCMOD_CTRL<<16));
  CallEdit(SCI_CLEARCMDKEY,SCK_DIVIDE+(SCMOD_CTRL<<16));

  return TRUE;
}

void SourceEdit::OnDestroy()
{
  m_spell.DestroyWindow();
  m_find.Destroy();
  CWnd::OnDestroy();
}

void SourceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  // Do not allow control characters to be added
  if (nChar >= 32)
    CWnd::OnChar(nChar,nRepCnt,nFlags);
}

void SourceEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
  // If invoked from the keyboard, show the menu at the top-left of the window
  if ((point.x == -1) && (point.y == -1))
    ClientToScreen(&point);

  CMenu menu;
  menu.CreatePopupMenu();
  menu.AppendMenu(MF_STRING,ID_EDIT_UNDO,"&Undo");
  menu.AppendMenu(MF_SEPARATOR,0,(LPCSTR)NULL);
  menu.AppendMenu(MF_STRING,ID_EDIT_CUT,"Cu&t");
  menu.AppendMenu(MF_STRING,ID_EDIT_COPY,"&Copy");
  menu.AppendMenu(MF_STRING,ID_EDIT_PASTE,"&Paste");
  menu.AppendMenu(MF_STRING,ID_EDIT_DELETE,"&Delete");
  menu.AppendMenu(MF_SEPARATOR,0,(LPCSTR)NULL);
  menu.AppendMenu(MF_STRING,ID_EDIT_SELECT_ALL,"Select &All");
  menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,GetParentFrame(),NULL);
}

BOOL SourceEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  // Disallow zooming
  if (nFlags & MK_CONTROL)
    return TRUE;
  return CWnd::OnMouseWheel(nFlags,zDelta,pt);
}

UINT SourceEdit::OnGetDlgCode()
{
  const MSG* msg = GetCurrentMessage();
  if (msg->lParam)
  {
    msg = (MSG*)(msg->lParam);

    // Let the dialog manager process dialog related keys
    if (msg->message == WM_KEYDOWN)
    {
      switch (msg->wParam)
      {
      case VK_ESCAPE:
      case VK_RETURN:
      case VK_TAB:
        return 0;
      }
    }
  }
  return CWnd::OnGetDlgCode();
}

void SourceEdit::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_CANUNDO) == 1);
}

void SourceEdit::OnEditUndo()
{
  CallEdit(SCI_UNDO);
}

void SourceEdit::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_CANREDO) == 1);
}

void SourceEdit::OnEditRedo()
{
  CallEdit(SCI_REDO);
}

void SourceEdit::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_GETSELECTIONSTART) != CallEdit(SCI_GETSELECTIONEND));
}

void SourceEdit::OnEditCut()
{
  CallEdit(SCI_CUT);
}

void SourceEdit::OnEditCopy()
{
  CallEdit(SCI_COPY);
}

void SourceEdit::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_CANPASTE) != 0);
}

void SourceEdit::OnEditPaste()
{
  CallEdit(SCI_PASTE);
}

void SourceEdit::OnEditSelectAll()
{
  CallEdit(SCI_SELECTALL);
}

void SourceEdit::OnUpdateNeedText(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_GETLENGTH) > 0);
}

void SourceEdit::OnEditFind()
{
  m_find.Create(this,true);
}

void SourceEdit::OnEditReplace()
{
  m_find.Create(this,false);
}

LRESULT SourceEdit::OnFindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  return m_find.FindReplaceCmd(wParam,lParam);
}

void SourceEdit::OnUpdateNeedFindText(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(!m_find.GetLastFind().IsEmpty());
}

void SourceEdit::OnEditFindNext()
{
  m_find.RepeatFind(true);
}

void SourceEdit::OnEditFindPrev()
{
  m_find.RepeatFind(false);
}

void SourceEdit::OnUpdateEditUseSel(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(theApp.GetProfileInt("Window","Find Uses Selection",0) != 0);
}

void SourceEdit::OnEditUseSel()
{
  bool use = theApp.GetProfileInt("Window","Find Uses Selection",0) != 0;
  theApp.WriteProfileInt("Window","Find Uses Selection",use ? 0 : 1);
}

void SourceEdit::OnEditScroll()
{
  CHARRANGE sel = GetSelect();
  CallEdit(SCI_SETSEL,sel.cpMin,sel.cpMax);
}

void SourceEdit::OnEditSpelling()
{
  // Find a mis-spelled word, if there is one
  m_spell.StartCheck();
  if (m_spell.FindWord(true) == false)
    m_spell.DoneMessage();
  else
  {
    // Show the spell check window and select the current word
    m_spell.ShowWordFromSelection();
  }
}

void SourceEdit::OnEditDelete()
{
  CallEdit(SCI_REPLACESEL,0,(sptr_t)"");
}

void SourceEdit::OnFormatShift(UINT id)
{
  CallEdit(SCI_BEGINUNDOACTION);

  int line1 = (int)CallEdit(SCI_LINEFROMPOSITION,CallEdit(SCI_GETSELECTIONSTART));
  int line2 = (int)CallEdit(SCI_LINEFROMPOSITION,CallEdit(SCI_GETSELECTIONEND));
  for (int line = line1; line <= line2; line++)
  {
    // Get the start of the line
    int pos = (int)CallEdit(SCI_POSITIONFROMLINE,line);
    if (pos >= 0)
    {
      // Check that the line is not empty
      int posEnd = (int)CallEdit(SCI_GETLINEENDPOSITION,line);
      if ((posEnd - pos) <= 0)
        continue;

      // Check that this is not the last line of a multi-line selection with the cursor at the line's start
      if ((line == line2) && (line2 > line1) && (pos == CallEdit(SCI_GETSELECTIONEND)))
        continue;

      if (id == ID_FORMAT_SHIFT_RIGHT)
      {
        CallEdit(SCI_SETTARGETSTART,pos);
        CallEdit(SCI_SETTARGETEND,pos);
        CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"\t");
      }
      else
      {
        if (CallEdit(SCI_GETCHARAT,pos) == '\t')
        {
          CallEdit(SCI_SETTARGETSTART,pos);
          CallEdit(SCI_SETTARGETEND,pos+1);
          CallEdit(SCI_REPLACETARGET,0,(LONG_PTR)"");
        }
      }
    }
  }

  CallEdit(SCI_ENDUNDOACTION);
}

namespace {
bool isStringStyle(int style)
{
  return ((style == STYLE_QUOTE) || (style == STYLE_SUBSTITUTION));
}

int commentDepth(int style)
{
  return ((style >= STYLE_COMMENT) && (style < STYLE_COMMENT+NEST_COMMENTS)) ? style-STYLE_COMMENT+1 : 0;
}
} // unnamed namespace

void SourceEdit::OnFormatComment(UINT id)
{
  int pos1 = CallEdit(SCI_GETSELECTIONSTART);
  int pos2 = CallEdit(SCI_GETSELECTIONEND);
  if (pos2 <= pos1)
    return;

  // Don't change anything if:
  // 1) The selection starts or ends in a string
  // 2) The comment depth changes between the start and the end
  // 3) The comment depth drops below the initial depth in the selection
  int style1 = (int)CallEdit(SCI_GETSTYLEAT,pos1) & SourceLexer::StyleMask;
  if (isStringStyle(style1))
    return;
  int comment1 = commentDepth(style1);
  int style2 = 0, comment2 = 0;
  for (int i = pos1+1; i < pos2; i++)
  {
    style2 = (int)CallEdit(SCI_GETSTYLEAT,i) & SourceLexer::StyleMask;
    comment2 = commentDepth(style2);
    if (comment2 < comment1)
      return;
  }
  if (isStringStyle(style2))
    return;
  if (comment1 != comment2)
    return;

  if (id == ID_FORMAT_COMMENT)
  {
    // Is this the last line of a multi-line selection with the cursor at the line's start?
    int line1 = (int)CallEdit(SCI_LINEFROMPOSITION,pos1);
    int line2 = (int)CallEdit(SCI_LINEFROMPOSITION,pos2);
    if ((line2 > line1) && (CallEdit(SCI_POSITIONFROMLINE,line2) == pos2))
      pos2--;
    if (pos2 <= pos1)
      return;

    CallEdit(SCI_BEGINUNDOACTION);
    CallEdit(SCI_SETTARGETSTART,pos2);
    CallEdit(SCI_SETTARGETEND,pos2);
    CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"]");
    CallEdit(SCI_SETTARGETSTART,pos1);
    CallEdit(SCI_SETTARGETEND,pos1);
    CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"[");
    CallEdit(SCI_SETSEL,pos1,pos2+2);
    CallEdit(SCI_ENDUNDOACTION);
  }
  else if (id == ID_FORMAT_UNCOMMENT)
  {
    // Get the first and last characters
    int len = (int)CallEdit(SCI_GETLENGTH);
    CStringW text1 = GetTextRange(pos1,pos1+1,len);
    CStringW text2 = GetTextRange(pos2-1,pos2,len);

    if ((text1 == L"[") && (text2 == L"]"))
    {
      CallEdit(SCI_BEGINUNDOACTION);
      CallEdit(SCI_SETTARGETSTART,pos2-1);
      CallEdit(SCI_SETTARGETEND,pos2);
      CallEdit(SCI_REPLACETARGET,0,(LONG_PTR)"");
      CallEdit(SCI_SETTARGETSTART,pos1);
      CallEdit(SCI_SETTARGETEND,pos1+1);
      CallEdit(SCI_REPLACETARGET,0,(LONG_PTR)"");
      CallEdit(SCI_SETSEL,pos1,pos2-2);
      CallEdit(SCI_ENDUNDOACTION);
    }
    else if (comment2 > 0)
    {
      CallEdit(SCI_BEGINUNDOACTION);
      CallEdit(SCI_SETTARGETSTART,pos2);
      CallEdit(SCI_SETTARGETEND,pos2);
      CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"[");
      CallEdit(SCI_SETTARGETSTART,pos1);
      CallEdit(SCI_SETTARGETEND,pos1);
      CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"]");
      CallEdit(SCI_SETSEL,pos1,pos2+2);
      CallEdit(SCI_ENDUNDOACTION);
    }
  }
}

void SourceEdit::OnFormatRenumber()
{
  CArray<SourceLexer::Heading> headings;
  GetAllHeadings(headings);

  CallEdit(SCI_BEGINUNDOACTION);
  RenumberHeadings(headings);
  CallEdit(SCI_ENDUNDOACTION);
}

void SourceEdit::OnSavePointReached(NMHDR*, LRESULT*)
{
  GetParentFrame()->PostMessage(WM_PROJECTEDITED,0);
}

void SourceEdit::OnSavePointLeft(NMHDR*, LRESULT*)
{
  GetParentFrame()->PostMessage(WM_PROJECTEDITED,1);
}

void SourceEdit::OnStyleNeeded(NMHDR* hdr, LRESULT*)
{
  SCNotification* notify = (SCNotification*)hdr;

  SourceLexer lex(this,SourceLexer::LexApply);
  lex.Process((int)CallEdit(SCI_GETENDSTYLED),notify->position,m_includeExt);
}

void SourceEdit::OnCharAdded(NMHDR* hdr, LRESULT* res)
{
  SCNotification* notify = (SCNotification*)hdr;

  // Detect and indent a new line
  if (m_autoIndent && ((notify->ch == '\r') || (notify->ch == '\n')))
  {
    // Get the current style
    int pos = CallEdit(SCI_GETCURRENTPOS);
    int style = (int)CallEdit(SCI_GETSTYLEAT,pos) & SourceLexer::StyleMask;
    if ((style != STYLE_QUOTE) && (style != STYLE_SUBSTITUTION))
    {
      // Get the previous line
      int line = CallEdit(SCI_LINEFROMPOSITION,pos);
      if (line > 0)
      {
        int len = CallEdit(SCI_LINELENGTH,line-1);
        char* buffer = (char*)alloca(len+1);
        CallEdit(SCI_GETLINE,line-1,(LONG_PTR)buffer);
        buffer[len] = '\0';

        // Count the number of leading tabs in the previous line
        int tabs = 0;
        for (int i = 0; buffer[i]; i++)
        {
          if (buffer[i] == '\t')
            tabs++;
          else
            break;
        }

        // Increase tab depth if the last character of the previous line was ':'
        char last = 0;
        for (int i = len-1; (last == 0) && (i >= 0); i--)
        {
          char c = buffer[i];
          if ((c != '\n') && (c != '\r'))
            last = c;
        }
        if (last == ':')
          tabs++;

        // If the previous line is entirely white space then reduce tabs to 0
        bool white = true;
        for (int i = 0; white && buffer[i]; i++)
        {
          char c = buffer[i];
          if ((c != '\n') && (c != '\r') && (c != '\t') && (c != ' '))
            white = false;
        }
        if (white)
          tabs = 0;

        // Add the required number of tabs
        buffer = (char*)alloca(tabs+1);
        memset(buffer,'\t',tabs);
        buffer[tabs] = '\0';
        CallEdit(SCI_REPLACESEL,0,(LONG_PTR)buffer);
      }
    }
  }

  // Auto-number sections
  if (m_autoNumber && (notify->ch == ' '))
  {
    // Get the current style
    int pos = CallEdit(SCI_GETCURRENTPOS);
    int style = (int)CallEdit(SCI_GETSTYLEAT,pos) & SourceLexer::StyleMask;
    if ((style == STYLE_TEXT) || (style == STYLE_HEADING))
    {
      // Get the current line
      int line = CallEdit(SCI_LINEFROMPOSITION,pos);
      if (line >= 0)
      {
        int len = CallEdit(SCI_LINELENGTH,line);
        char* buffer = (char*)alloca(len+1);
        CallEdit(SCI_GETLINE,line,(LONG_PTR)buffer);
        buffer[len] = '\0';

        int white = 0;
        for (int i = 0; i < len; i++)
        {
          char c = buffer[i];
          if ((c == '\n') || (c == '\r'))
          {
            buffer[i] = '\0';
            break;
          }
          else if ((c == L' ') || (c == L'\t'))
            white++;
        }

        // Has the user just started a heading?
        if (white == 1)
        {
          switch (SourceLexer::IsHeading(buffer,false))
          {
          case SourceLexer::Volume:
          case SourceLexer::Book:
          case SourceLexer::Part:
          case SourceLexer::Chapter:
          case SourceLexer::Section:
            // Give it a number and then renumber the headings
            {
              CallEdit(SCI_BEGINUNDOACTION);
              CallEdit(SCI_REPLACESEL,0,(LONG_PTR)"1 - ");

              CArray<SourceLexer::Heading> headings;
              GetAllHeadings(headings);

              bool renumbered = false;
              if (!IsLineInExtDoc(headings,line))
              {
                RenumberHeadings(headings);
                renumbered = true;
              }

              CallEdit(SCI_ENDUNDOACTION);
              if (!renumbered)
                CallEdit(SCI_UNDO);
            }
            break;
          }
        }
      }
    }
  }
}

void SourceEdit::OnDocModified(NMHDR* hdr, LRESULT*)
{
  if (hdr->code == SCN_MODIFIED)
  {
    // If text has been added or removed, update the elastic tabstops
    if (m_elasticTabStops)
    {
      SCNotification* notify = (SCNotification*)hdr;
      if (notify->modificationType & SC_MOD_INSERTTEXT)
        ElasticTabStops_OnModify(m_editPtr,notify->position,notify->position+notify->length);
      else if (notify->modificationType & SC_MOD_DELETETEXT)
        ElasticTabStops_OnModify(m_editPtr,notify->position,notify->position);
    }
  }
}

void SourceEdit::OnConvertPaste(NMHDR* hdr, LRESULT* res)
{
  SCNXConvertPaste* cp = (SCNXConvertPaste*)hdr;
  *res = 0;

  // Get the source of the data
  COleDataObject data;
  if (cp->source)
    data.Attach((LPDATAOBJECT)(cp->source),FALSE);
  else
    data.AttachClipboard();

  // Try to interpret tables and leading white space
  if (data.IsDataAvailable(CF_UNICODETEXT))
  {
    CStringW theText(cp->utext,cp->ulen);
    CStringW newText, line;
    newText.Preallocate(theText.GetLength());

    bool foundTable = false;
    bool inTable = false;

    int charPos = 0, lineCount = 0;
    while (GetNextLine(theText,line,charPos))
    {
      if (inTable)
      {
        CArray<CStringW> tokens;
        TokenizeLine(line,tokens);

        // Separate multiple tokens with tabs: if less than two tokens,
        // we're at the end of the table
        if (tokens.GetSize() > 1)
        {
          line.Empty();
          for (int j = 0; j < tokens.GetSize(); j++)
          {
            if (j > 0)
              line.AppendChar(L'\t');
            line.Append(tokens.GetAt(j));
          }
        }
        else
          inTable = false;
      }
      else
      {
        // Look for the start of a table
        if (line.Left(6).CompareNoCase(L"table ") == 0)
        {
          inTable = true;
          foundTable = true;
        }

        // Replace any leading blocks of 4 spaces
        int i = 0;
        while (i >= 0)
        {
          if (line.Mid(i,4).Compare(L"    ") == 0)
          {
            line.Delete(i,3);
            line.SetAt(i,L'\t');
            i++;
          }
          else
            i = -1;
        }
      }

      if (lineCount > 0)
        newText.AppendChar(L'\n');
      newText.Append(line);
      lineCount++;
    }

    CString newTextUtf = TextFormat::UnicodeToUTF8(newText);
    cp->text = new char[newTextUtf.GetLength() + 1];
    strcpy(cp->text,newTextUtf);
    *res = 1;
  }
}

void SourceEdit::OnConvertCopyToClip(NMHDR* hdr, LRESULT* res)
{
  SCNXConvertCopyToClip* ccc = (SCNXConvertCopyToClip*)hdr;

  // Ensure that the text sent to the clipboard has Windows-style
  // (that is, "\r\n") line endings.
  char* newText = new char[(strlen(ccc->text)*2)+1];
  char* np = newText;
  for (const char* p = ccc->text; *p != '\0'; ++p)
  {
    switch (*p)
    {
    case '\n':
      *(np++) = '\r';
      *(np++) = '\n';
      break;
    case '\r':
      *(np++) = '\r';
      *(np++) = '\n';
      if (*(p+1) == '\n')
        ++p;
      break;
    default:
      *(np++) = *p;
      break;
    }
  }
  *np = '\0';

  ccc->text = newText;
  *res = 1;
}

BOOL SourceEdit::PreTranslateMessage(MSG* pMsg)
{
  if ((m_markSel.cpMin >= 0) && (pMsg->hwnd == GetSafeHwnd()))
  {
    CHARRANGE sel = GetSelect();
    if ((m_markSel.cpMin != sel.cpMin) || (m_markSel.cpMax != sel.cpMax))
    {
      m_markSel.cpMin = -1;
      m_markSel.cpMax = -1;
      CallEdit(SCI_MARKERDELETEALL,m_marker);
    }
  }
  return CWnd::PreTranslateMessage(pMsg);
}

void SourceEdit::SetStyles(COLORREF back)
{
  CallEdit(SCI_STYLESETFONT,STYLE_DEFAULT,(sptr_t)(LPCSTR)m_fontName);
  CallEdit(SCI_STYLESETSIZE,STYLE_DEFAULT,10 * m_fontSize);
  if (m_syntaxHighlight)
  {
    CallEdit(SCI_STYLESETFORE,STYLE_DEFAULT,m_colourMain);
    CallEdit(SCI_STYLESETBACK,STYLE_DEFAULT,back);
    CallEdit(SCI_STYLECLEARALL);

    CallEdit(SCI_STYLESETFORE,STYLE_QUOTE,m_colourQuote);
    CallEdit(SCI_STYLESETFORE,STYLE_SUBSTITUTION,m_colourSubst);
    CallEdit(SCI_STYLESETFORE,STYLE_INFORM6,theApp.GetColour(InformApp::ColourInform6Code));
    CallEdit(SCI_STYLESETFORE,STYLE_HEADING,m_colourHead);
    for (int i = 0; i < NEST_COMMENTS; i++)
      CallEdit(SCI_STYLESETFORE,STYLE_COMMENT+i,m_colourComment);

    SetSourceStyle(STYLE_TEXT,m_styleMain,m_underMain,m_sizeMain);
    SetSourceStyle(STYLE_QUOTE,m_styleQuote,m_underQuote,m_sizeQuote);
    SetSourceStyle(STYLE_SUBSTITUTION,m_styleSubst,m_underSubst,m_sizeSubst);
    SetSourceStyle(STYLE_HEADING,m_styleHead,m_underHead,m_sizeHead);
    for (int i = 0; i < NEST_COMMENTS; i++)
      SetSourceStyle(STYLE_COMMENT+i,m_styleComment,m_underComment,m_sizeComment);
  }
  else
  {
    CallEdit(SCI_STYLESETFORE,STYLE_DEFAULT,theApp.GetColour(InformApp::ColourText));
    CallEdit(SCI_STYLESETBACK,STYLE_DEFAULT,back);
    CallEdit(SCI_STYLECLEARALL);
  }
}

void SourceEdit::SetReadOnly(bool readOnly)
{
  CallEdit(SCI_SETREADONLY,readOnly);
}

void SourceEdit::SetShowScrollBars(bool show)
{
  CallEdit(SCI_SETHSCROLLBAR,show);
  CallEdit(SCI_SETVSCROLLBAR,show);
}

void SourceEdit::SetLineWrap(bool wrap)
{
  CallEdit(SCI_SETWRAPMODE,wrap ? SC_WRAP_WORD : SC_WRAP_NONE);
}

void SourceEdit::HideCaret(void)
{
  CallEdit(SCI_SETCARETSTYLE,CARETSTYLE_INVISIBLE);
}

void SourceEdit::DisableUserControl(void)
{
  CallEdit(SCI_CLEARALLCMDKEYS);
  CallEdit(SCI_SETMOUSEDOWNCAPTURES,0);
  CallEdit(SCI_HIDESELECTION,1);
}

void SourceEdit::SetDocument(SourceEdit* master)
{
  sptr_t doc = master->CallEdit(SCI_GETDOCPOINTER);
  CallEdit(SCI_SETDOCPOINTER,0,doc);

  // As this editor is using the other's document, use a separate marker
  CallEdit(SCI_MARKERDEFINE,m_marker,SC_MARK_EMPTY);
  m_marker = 1;
  CallEdit(SCI_MARKERDEFINE,m_marker,SC_MARK_BACKGROUND);
}

void SourceEdit::OpenFile(CFile* file)
{
  int utfLen = (int)file->GetLength();

  // Read in the file as UTF-8
  CString utfText;
  LPSTR utfPtr = utfText.GetBufferSetLength(utfLen);
  file->Read(utfPtr,utfLen);
  utfText.ReleaseBuffer(utfLen);

  // Check for a UTF-8 BOM
  if (utfText.GetLength() >= 3)
  {
    if (utfText.Left(3) == "\xEF\xBB\xBF")
      utfText = utfText.Mid(3);
  }

  CallEdit(SCI_SETTEXT,0,(sptr_t)(LPCSTR)utfText);
  CallEdit(SCI_EMPTYUNDOBUFFER);
  CallEdit(SCI_SETSAVEPOINT);

  CFileStatus status;
  if (file->GetStatus(status))
    m_fileTime = status.m_mtime;
}

bool SourceEdit::SaveFile(CFile* file)
{
  // Write out the document contents
  bool success = true;
  try
  {
    CString utfText = GetSource();
    file->Write(utfText,utfText.GetLength());
    CallEdit(SCI_SETSAVEPOINT);
  }
  catch (CException* ex)
  {
    ex->Delete();
    success = false;
  }

  CFileStatus status;
  if (file->GetStatus(status))
    m_fileTime = status.m_mtime;

  return success;
}

bool SourceEdit::IsEdited(void)
{
  return (CallEdit(SCI_GETMODIFY) != 0);
}

const CTime& SourceEdit::GetFileTime(void)
{
  return m_fileTime;
}

void SourceEdit::Select(CHARRANGE range, bool centre)
{
  SetFocus();

  // If the range is currently visible, just select it
  int y1 = (int)CallEdit(SCI_POINTYFROMPOSITION,0,range.cpMin);
  int y2 = (int)CallEdit(SCI_POINTYFROMPOSITION,0,range.cpMax);
  CRect client;
  GetClientRect(client);
  client.DeflateRect(0,CallEdit(SCI_TEXTHEIGHT,0));
  if (client.PtInRect(CPoint(0,y1)) && client.PtInRect(CPoint(0,y2)))
  {
    CallEdit(SCI_SETSEL,range.cpMin,range.cpMax);
    return;
  }

  // Go to the end of the document first
  int len = (int)CallEdit(SCI_GETLENGTH);
  CallEdit(SCI_SETSEL,len,len);

  // Select the required range
  CallEdit(SCI_SETSEL,range.cpMin,range.cpMax);

  if (centre)
  {
    int firstLine = (int)CallEdit(SCI_DOCLINEFROMVISIBLE,CallEdit(SCI_GETFIRSTVISIBLELINE));
    int rangeLine = (int)CallEdit(SCI_LINEFROMPOSITION,range.cpMin);
    int linesShown = (int)CallEdit(SCI_LINESONSCREEN);

    // If the range is now near the top, scroll up so that the line is centred
    if ((rangeLine - firstLine) < (linesShown / 3))
      CallEdit(SCI_LINESCROLL,0,linesShown / -2);
  }
}

void SourceEdit::Highlight(int line, COLORREF colour, bool centre)
{
  CallEdit(SCI_MARKERDELETEALL,m_marker);

  if (line < (int)CallEdit(SCI_GETLINECOUNT))
  {
    int pos = (int)CallEdit(SCI_POSITIONFROMLINE,line);
    if (pos >= 0)
    {
      CHARRANGE range = { pos, pos };
      Select(range,centre);

      if (colour != -1)
      {
        m_markSel.cpMin = pos;
        m_markSel.cpMax = pos;
        CallEdit(SCI_MARKERSETBACK,m_marker,colour);
        CallEdit(SCI_MARKERADD,line,m_marker);
      }
    }
  }
}

void SourceEdit::ShowBetween(int startLine, int endLine)
{
  if (endLine > startLine)
  {
    int endLen = CallEdit(SCI_LINELENGTH,endLine);
    if (endLen <= 1)
      endLine--;
  }

  CallEdit(SCIX_VISIBLEREGION,startLine,endLine);
  Highlight(startLine,-1,false);
}

bool SourceEdit::IsLineShown(int line)
{
  return (CallEdit(SCIX_ISLINEINVISIBLEREGION,line) != 0);
}

void SourceEdit::PasteCode(const wchar_t* code)
{
  CString theCodeUtf = TextFormat::UnicodeToUTF8(code);
  CallEdit(SCI_REPLACESEL,0,(sptr_t)(LPCSTR)theCodeUtf);
}

void SourceEdit::UpdateSpellCheck(void)
{
  m_spell.SettingsChanged();
}

void SourceEdit::MoveShowSelect(CWnd* child)
{
  // Get the screen position of the selected word
  CRect wordR = GetSelectRect();

  // Get the size and position of the dialog
  CRect wndR;
  child->GetWindowRect(wndR);

  CRect extWndR;
  if (SUCCEEDED(::DwmGetWindowAttribute(child->GetSafeHwnd(),
    DWMWA_EXTENDED_FRAME_BOUNDS,(LPRECT)extWndR,sizeof (RECT))))
  {
    // If the window frame extends beyond the window bounds, work out by how much,
    // and make sure that the window is moved to avoid overlap by the extended frame.
    wordR.InflateRect(
      (extWndR.Width()-wndR.Width())/2,(extWndR.Height()-wndR.Height())/2);
  }

  // If the dialog is over the word, move it
  CRect intersectR;
  if (intersectR.IntersectRect(wordR,wndR))
  {
    // Get the size of the display
    CRect workRect = DPI::getMonitorWorkRect(child);

    // Try moving the dialog, but keep it on-screen
    if (wordR.top-wndR.Height() >= 0)
    {
      child->SetWindowPos(&CWnd::wndTop,wndR.left,wordR.top-wndR.Height(),0,0,
        SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOSIZE);
    }
    else if (wordR.bottom+wndR.Height() < workRect.bottom)
    {
      child->SetWindowPos(&CWnd::wndTop,wndR.left,wordR.bottom,0,0,
        SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOSIZE);
    }
  }
}

CHARRANGE SourceEdit::GetSelect(void)
{
  CHARRANGE select;
  select.cpMin = (int)CallEdit(SCI_GETSELECTIONSTART);
  select.cpMax = (int)CallEdit(SCI_GETSELECTIONEND);
  return select;
}

RECT SourceEdit::GetSelectRect(void)
{
  int p1 = (int)CallEdit(SCI_GETSELECTIONSTART);
  int x1 = (int)CallEdit(SCI_POINTXFROMPOSITION,0,p1);
  int y1 = (int)CallEdit(SCI_POINTYFROMPOSITION,0,p1);
  int p2 = (int)CallEdit(SCI_GETSELECTIONEND);
  int x2 = (int)CallEdit(SCI_POINTXFROMPOSITION,0,p2);
  int y2 = (int)CallEdit(SCI_POINTYFROMPOSITION,0,p2);
  int h2 = (int)CallEdit(SCI_TEXTHEIGHT,CallEdit(SCI_LINEFROMPOSITION,p2));

  CRect r(CPoint(x1,y1),CPoint(x2,y2+h2));
  r.NormalizeRect();
  ClientToScreen(r);
  return r;
}

void SourceEdit::SetSelect(CHARRANGE select)
{
  CallEdit(SCI_SETSEL,select.cpMin,select.cpMax);
}

void SourceEdit::ReplaceSelect(LPCWSTR text)
{
  CString textUtf = TextFormat::UnicodeToUTF8(text);
  CallEdit(SCI_REPLACESEL,0,(sptr_t)(LPCSTR)textUtf);
}

CHARRANGE SourceEdit::GetCurrentWord(void)
{
  int anchor = (int)CallEdit(SCI_GETANCHOR);
  CHARRANGE current;
  current.cpMin = (int)CallEdit(SCI_WORDSTARTPOSITION,anchor,1);
  current.cpMax = (int)CallEdit(SCI_WORDENDPOSITION,anchor,1);
  return current;
}

CHARRANGE SourceEdit::GetNextWord(CHARRANGE word)
{
  word.cpMin = (int)CallEdit(SCI_WORDENDPOSITION,word.cpMax,0);
  word.cpMax = (int)CallEdit(SCI_WORDENDPOSITION,word.cpMin,1);
  return word;
}

CStringW SourceEdit::GetTextRange(int cpMin, int cpMax, int len)
{
  if (cpMin < 0)
    cpMin = 0;
  if ((len >= 0) && (cpMax > len))
    cpMax = len;

  TextRange range;
  range.chrg.cpMin = cpMin;
  range.chrg.cpMax = cpMax;

  CString utfText;
  range.lpstrText = utfText.GetBufferSetLength(cpMax-cpMin+1);
  CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);
  utfText.ReleaseBuffer();
  return TextFormat::UTF8ToUnicode(utfText);
}

CHARRANGE SourceEdit::FindText(LPCWSTR text, bool fromSelect, bool down, bool matchCase, bool wholeWord)
{
  int flags = 0;
  if (matchCase)
    flags |= SCFIND_MATCHCASE;
  if (wholeWord)
    flags |= SCFIND_WHOLEWORD;

  TextToFind find;
  if (down)
  {
    find.chrg.cpMin = (int)CallEdit(fromSelect ? SCI_GETSELECTIONEND : SCIX_VISIBLEREGIONSTART);
    find.chrg.cpMax = (int)CallEdit(SCIX_VISIBLEREGIONEND);
  }
  else
  {
    find.chrg.cpMin = (int)CallEdit(fromSelect ? SCI_GETSELECTIONSTART : SCIX_VISIBLEREGIONEND);
    find.chrg.cpMax = (int)CallEdit(SCIX_VISIBLEREGIONSTART);
  }
  CString textUtf = TextFormat::UnicodeToUTF8(text);
  find.lpstrText = (char*)(LPCSTR)textUtf;

  CHARRANGE result = { -1,-1 };
  if (CallEdit(SCI_FINDTEXT,flags,(sptr_t)&find) >= 0)
  {
    result.cpMin = find.chrgText.cpMin;
    result.cpMax = find.chrgText.cpMax;
  }
  return result;
}

void SourceEdit::LoadSettings(SourceSettings& set, COLORREF back)
{
  DWORD value;
  {
    char fontName[MAX_PATH] = "";
    if (set.GetString("Source Font Name",fontName,MAX_PATH))
      m_fontName = fontName;
    if (set.GetDWord("Source Font Size",value))
      m_fontSize = value;
  }
  if (set.GetDWord("Syntax Highlighting",value))
    m_syntaxHighlight = (value != 0);
  if (set.GetDWord("Headings Colour",value))
    m_colourHead = (COLORREF)value;
  if (set.GetDWord("Main Text Colour",value))
    m_colourMain = (COLORREF)value;
  if (set.GetDWord("Comments Colour",value))
    m_colourComment = (COLORREF)value;
  if (set.GetDWord("Quoted Text Colour",value))
    m_colourQuote = (COLORREF)value;
  if (set.GetDWord("Substitutions Colour",value))
    m_colourSubst = (COLORREF)value;
  if (set.GetDWord("Headings Style",value))
    m_styleHead = (int)value;
  if (set.GetDWord("Main Text Style",value))
    m_styleMain = (int)value;
  if (set.GetDWord("Comments Style",value))
    m_styleComment = (int)value;
  if (set.GetDWord("Quoted Text Style",value))
    m_styleQuote = (int)value;
  if (set.GetDWord("Substitutions Style",value))
    m_styleSubst = (int)value;
  if (set.GetDWord("Headings Underline",value))
    m_underHead = (value != 0);
  if (set.GetDWord("Main Text Underline",value))
    m_underMain = (value != 0);
  if (set.GetDWord("Comments Underline",value))
    m_underComment = (value != 0);
  if (set.GetDWord("Quoted Text Underline",value))
    m_underQuote = (value != 0);
  if (set.GetDWord("Substitutions Underline",value))
    m_underSubst = (value != 0);
  if (set.GetDWord("Headings Size",value))
    m_sizeHead = (int)value;
  if (set.GetDWord("Main Text Size",value))
    m_sizeMain = (int)value;
  if (set.GetDWord("Comments Size",value))
    m_sizeComment = (int)value;
  if (set.GetDWord("Quoted Text Size",value))
    m_sizeQuote = (int)value;
  if (set.GetDWord("Substitutions Size",value))
    m_sizeSubst = (int)value;
  SetStyles(back);

  if (set.GetDWord("Source Tab Size Chars",value))
  {
    if (value > 0)
      CallEdit(SCI_SETTABWIDTH,value);
  }

  if (set.GetDWord("Auto Indent",value))
    m_autoIndent = (value != 0);
  if (set.GetDWord("Auto Number Sections",value))
    m_autoNumber = (value != 0);

  // Adjust elastic tabstops
  bool elastic = true;
  if (set.GetDWord("Auto Space Tables",value))
    elastic = (value != 0);
  SetElasticTabStops(elastic);

  // Adjust wrapped line indentation
  bool indent = true;
  if (set.GetDWord("Indent Wrapped Lines",value))
    indent = (value != 0);
  if (indent)
  {
    int indent = (int)((CallEdit(SCI_GETTABWIDTH) / 3.0) + 0.5);
    CallEdit(SCI_SETWRAPSTARTINDENT,indent);
    CallEdit(SCI_SETWRAPINDENTMODE,SC_WRAPINDENT_INDENT);
  }
  else
  {
    CallEdit(SCI_SETWRAPSTARTINDENT,0);
    CallEdit(SCI_SETWRAPINDENTMODE,SC_WRAPINDENT_FIXED);
  }
}

void SourceEdit::PrefsChanged(void)
{
  // Somewhat tortuously, this causes Scintilla to update its internal style state, so
  // that any calls before the next re-paint get the correct style or sizing information.
  Invalidate();
  CallEdit(WM_QUERYNEWPALETTE);
}

bool SourceEdit::GetElasticTabStops(void)
{
  return m_elasticTabStops;
}

void SourceEdit::SetElasticTabStops(bool enable)
{
  m_elasticTabStops = enable;
  if (m_elasticTabStops)
    ElasticTabStops_OnModify(m_editPtr,0,CallEdit(SCI_GETLENGTH));
  else
    ElasticTabStops_OnClear(m_editPtr);
}

void SourceEdit::SetCustomTabStops(int num, int tabPixels)
{
  std::vector<int> tab_array;
  tab_array.resize(num+1);
  tab_array[num] = 0;
  for (int i = 0; i < num; i++)
    tab_array[i] = (i+1)*tabPixels;
  CallEdit(SCIX_SETTABSTOPS,0,(LONG_PTR)&(tab_array.at(0)));
}

int SourceEdit::GetTabWidthPixels(void)
{
  return CallEdit(SCI_GETTABWIDTH) * CallEdit(SCI_TEXTWIDTH,0,(LONG_PTR)" ");
}

void SourceEdit::GetAllHeadings(CArray<SourceLexer::Heading>& headings)
{
  SourceLexer lex(this,SourceLexer::LexHeadings);
  lex.Process(0,-1,m_includeExt);
  headings.Copy(lex.GetHeadings());
}

int SourceEdit::GetLineHeight(void)
{
  return CallEdit(SCI_TEXTHEIGHT,0);
}

CHARRANGE SourceEdit::GetRangeLines(CHARRANGE range)
{
  CHARRANGE lines;
  lines.cpMin = (int)CallEdit(SCI_LINEFROMPOSITION,range.cpMin);
  lines.cpMax = (int)CallEdit(SCI_LINEFROMPOSITION,range.cpMax);
  return lines;
}

CString SourceEdit::GetSource(void)
{
  // Get the contents of the document as UTF-8
  CallEdit(SCI_CONVERTEOLS,SC_EOL_LF);
  int len = (int)CallEdit(SCI_GETLENGTH);
  CString utfText;
  LPSTR utfPtr = utfText.GetBufferSetLength(len+1);
  CallEdit(SCI_GETTEXT,len+1,(sptr_t)utfPtr);
  utfText.ReleaseBuffer();
  return utfText;
}

extern "C" sptr_t __stdcall Scintilla_DirectFunction(sptr_t, UINT, uptr_t, sptr_t);

LONG_PTR SourceEdit::CallEdit(UINT msg, DWORD wp, LONG_PTR lp)
{
  return Scintilla_DirectFunction(m_editPtr,msg,wp,lp);
}

bool SourceEdit::GetNextLine(const CStringW& text, CStringW& line, int& i)
{
  if (i == text.GetLength())
  {
    // If at the very end, the final line must be blank
    line = "";
    i++;
    return true;
  }
  else if (i > text.GetLength())
  {
    // Past the end of the text, so stop reading lines
    return false;
  }

  line.Empty();
  while (i < text.GetLength())
  {
    WCHAR c = text.GetAt(i);
    i++;

    switch (c)
    {
    case L'\r':
      // Check for a "\r\n" sequence
      if (i < text.GetLength())
      {
        if (text.GetAt(i) == L'\n')
          i++;
      }
      return true;
    case L'\n':
      return true;
    default:
      line.AppendChar(c);
      break;
    }
  }

  // Having got here a line must have ended without a trailing carriage return,
  // so move beyond the end of text to make sure this is the last line.
  i++;
  return true;
}

void SourceEdit::TokenizeLine(const CStringW& line, CArray<CStringW>& tokens)
{
  int i = 0;
  while (true)
  {
    // We are either at the start of the line, or the end of the previous token,
    // so scan forward to find the start of the next token.
    while (true)
    {
      if (i == line.GetLength())
        return;
      WCHAR c = line.GetAt(i);
      if ((c != L' ') && (c != L'\t'))
        break;
      i++;
    }

    // Find the end of this token
    int j = line.Find(L"  ",i);
    if (j == -1)
    {
      // No final delimiter, so this must be the last token
      if (i < line.GetLength())
        tokens.Add(line.Mid(i));
      return;
    }
    else
    {
      // Store this token and move to the end of it
      tokens.Add(line.Mid(i,j-i));
      i = j;
    }
  }
}

void SourceEdit::RenumberHeadings(const CArray<SourceLexer::Heading>& headings)
{
  int indexes[SourceLexer::Example+1];
  for (int j = 0; j < sizeof(indexes) / sizeof(indexes[0]); j++)
    indexes[j] = 1;

  for (int i = 0; i < headings.GetSize(); i++)
  {
    const SourceLexer::Heading& heading = headings.GetAt(i);
    switch (heading.level)
    {
    case SourceLexer::ExtensionPart:
      // Don't renumber in documentation
      if (heading.line > 0)
        return;
    case SourceLexer::Volume:
    case SourceLexer::Book:
    case SourceLexer::Part:
    case SourceLexer::Chapter:
    case SourceLexer::Section:
    case SourceLexer::Example:
      {
        int pos = 0;
        CStringW sectionName = heading.name.Tokenize(L" ",pos);
        if (pos <= 0)
          break;
        if (sectionName.Right(1) == L":")
          break;
        CStringW sectionNumber = heading.name.Tokenize(L" ",pos);
        if (pos <= 0)
          break;

        CString replace;
        replace.Format("%S %d ",(LPCWSTR)sectionName,indexes[heading.level]);
        indexes[heading.level]++;
        for (int j = heading.level+1; j < sizeof(indexes) / sizeof(indexes[0]); j++)
          indexes[j] = 1;

        int linePos = CallEdit(SCI_POSITIONFROMLINE,heading.line);
        int lineEndPos = CallEdit(SCI_GETLINEENDPOSITION,heading.line);
        if (linePos+pos < lineEndPos)
          lineEndPos = linePos+pos;
        CallEdit(SCI_SETTARGETSTART,linePos);
        CallEdit(SCI_SETTARGETEND,lineEndPos);
        CallEdit(SCI_REPLACETARGET,replace.GetLength(),(LONG_PTR)(LPCSTR)replace);
      }
      break;
    }
  }
}

bool SourceEdit::IsLineInExtDoc(const CArray<SourceLexer::Heading>& headings, int line)
{
  if (!m_includeExt)
    return false;

  CStringW dh(L"Documentation");
  for (int i = 0; i < headings.GetSize(); i++)
  {
    const SourceLexer::Heading& heading = headings.GetAt(i);
    if (heading.level == SourceLexer::ExtensionPart)
    {
      if (heading.name.Left(dh.GetLength()).CompareNoCase(dh) == 0)
        return (line >= heading.line);
    }
  }
  return false;
}

void SourceEdit::SetSourceStyle(int style, int boldItalic, bool underline, int size)
{
  // Values for the "boldItalic" argument are:
  //  0 - Regular
  //  1 - Italic
  //  2 - Bold
  //  3 - Bold + Italic
  // Values for the "size" argument are:
  //  0 - Normal
  //  1 - Small
  CallEdit(SCI_STYLESETITALIC,style,((boldItalic == 1) || (boldItalic == 3)) ? 1 : 0);
  CallEdit(SCI_STYLESETBOLD,style,((boldItalic == 2) || (boldItalic == 3)) ? 1 : 0);

  CallEdit(SCI_STYLESETUNDERLINE,style,underline ? 1 : 0);

  int sizeShift = (size == 1) ? -m_fontSize : 0;
  CallEdit(SCI_STYLESETSIZE,style,(10 * m_fontSize) + sizeShift);
}
