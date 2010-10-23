#include "stdafx.h"
#include "SourceEdit.h"
#include "Inform.h"
#include "Messages.h"
#include "EditFind.h"
#include "TextFormat.h"

#include "SciLexer.h"
#include <MultiMon.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SOURCE_FILE "story.ni"

IMPLEMENT_DYNAMIC(SourceEdit, CWnd)

BEGIN_MESSAGE_MAP(SourceEdit, CWnd)
  ON_WM_DESTROY()
  ON_WM_CONTEXTMENU()
  ON_WM_CHAR()

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
  ON_UPDATE_COMMAND_UI(ID_EDIT_SPELLING, OnUpdateNeedText)
  ON_COMMAND(ID_EDIT_SPELLING, OnEditSpelling)
  ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
  ON_COMMAND_RANGE(ID_FORMAT_SHIFT_RIGHT, ID_FORMAT_SHIFT_LEFT, OnFormatShift)
  ON_COMMAND_RANGE(ID_FORMAT_COMMENT, ID_FORMAT_UNCOMMENT, OnFormatComment)
  ON_COMMAND(ID_FORMAT_RENUMBER, OnFormatRenumber)
  ON_COMMAND_RANGE(IDC_TEXT_SMALLEST, IDC_TEXT_SMALLEST+4, OnTextSize)

  ON_NOTIFY_REFLECT(SCN_SAVEPOINTREACHED, OnSavePointReached)
  ON_NOTIFY_REFLECT(SCN_SAVEPOINTLEFT, OnSavePointLeft)
  ON_NOTIFY_REFLECT(SCN_STYLENEEDED, OnStyleNeeded)
  ON_NOTIFY_REFLECT(SCN_CHARADDED, OnCharAdded)
  ON_NOTIFY_REFLECT(SCNX_CONVERTPASTE, OnConvertPaste)
  ON_NOTIFY_REFLECT(SCNX_CONVERTCOPYTOCLIP, OnConvertCopyToClip)

  ON_REGISTERED_MESSAGE(EditFind::FINDMSG, OnFindReplaceCmd)
END_MESSAGE_MAP()

SourceEdit::SourceEdit() : m_spell(this)
{
  m_editPtr = 0;
  m_marker = 0;
  m_markSel.cpMin = -1;
  m_markSel.cpMax = -1;

  // Default preferences values
  m_textSize = 1;
  m_autoIndent = true;
}

BOOL SourceEdit::Create(CWnd* parent, UINT id)
{
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

  CSize fontSize = theApp.MeasureFont(&(theApp.GetFont()));
  m_editPtr = (sptr_t)SendMessage(SCI_GETDIRECTPOINTER);

  CallEdit(SCI_SETKEYSUNICODE,isUnicode);
  CallEdit(SCI_SETCODEPAGE,SC_CP_UTF8);
  CallEdit(SCI_SETEOLMODE,SC_EOL_LF);
  CallEdit(SCI_SETPASTECONVERTENDINGS,1);
  CallEdit(SCI_SETWRAPMODE,1);
  for (int i = 0; i < 5; i++)
    CallEdit(SCI_SETMARGINWIDTHN,i,0);
  CallEdit(SCI_SETMARGINLEFT,0,fontSize.cx);
  CallEdit(SCI_SETMARGINRIGHT,0,fontSize.cx);
  CallEdit(SCI_SETSELFORE,TRUE,::GetSysColor(COLOR_HIGHLIGHTTEXT));
  CallEdit(SCI_SETSELBACK,TRUE,::GetSysColor(COLOR_HIGHLIGHT));
  CallEdit(SCI_MARKERDEFINE,m_marker,SC_MARK_BACKGROUND);
  CallEdit(SCI_SETLEXER,SCLEX_CONTAINER);

  CallEdit(SCI_STYLESETFONT,STYLE_DEFAULT,(sptr_t)(LPCSTR)theApp.GetFontName());
  CallEdit(SCI_STYLESETSIZE,STYLE_DEFAULT,theApp.GetFontPointSize());
  CallEdit(SCI_STYLESETFORE,STYLE_DEFAULT,theApp.GetColour(InformApp::ColourText));
  CallEdit(SCI_STYLESETBACK,STYLE_DEFAULT,theApp.GetColour(InformApp::ColourBack));
  CallEdit(SCI_STYLECLEARALL);
  CallEdit(SCI_STYLESETFORE,STYLE_QUOTE,theApp.GetColour(InformApp::ColourQuote));
  CallEdit(SCI_STYLESETFORE,STYLE_QUOTEBRACKET,theApp.GetColour(InformApp::ColourQuoteBracket));
  CallEdit(SCI_STYLESETFORE,STYLE_INFORM6,theApp.GetColour(InformApp::ColourInform6Code));
  CallEdit(SCI_STYLESETBOLD,STYLE_HEADING,1);
  for (int i = 0; i < NEST_COMMENTS; i++)
    CallEdit(SCI_STYLESETFORE,STYLE_COMMENT+i,theApp.GetColour(InformApp::ColourComment));
  CallEdit(SCI_INDICSETSTYLE,0,INDIC_SQUIGGLE);
  CallEdit(SCI_INDICSETFORE,0,theApp.GetColour(InformApp::ColourError));
  CallEdit(SCI_SETINDICATORCURRENT,0);

  // Change the bindings for the Home and End keys
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME,SCI_HOMEDISPLAY);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+(SCMOD_SHIFT<<16),SCI_HOMEDISPLAYEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+(SCMOD_ALT<<16),SCI_HOME);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_HOME+((SCMOD_ALT|SCMOD_SHIFT)<<16),SCI_HOMEEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END,SCI_LINEENDDISPLAY);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+(SCMOD_SHIFT<<16),SCI_LINEENDDISPLAYEXTEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+(SCMOD_ALT<<16),SCI_LINEEND);
  CallEdit(SCI_ASSIGNCMDKEY,SCK_END+((SCMOD_ALT|SCMOD_SHIFT)<<16),SCI_LINEENDEXTEND);

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

  CMenu sizeMenu;
  sizeMenu.CreateMenu();
  sizeMenu.AppendMenu(MF_STRING,IDC_TEXT_LARGEST,"Lar&gest");
  sizeMenu.AppendMenu(MF_STRING,IDC_TEXT_LARGER,"&Larger");
  sizeMenu.AppendMenu(MF_STRING,IDC_TEXT_MEDIUM,"&Medium");
  sizeMenu.AppendMenu(MF_STRING,IDC_TEXT_SMALLER,"&Smaller");
  sizeMenu.AppendMenu(MF_STRING,IDC_TEXT_SMALLEST,"Smalles&t");
  sizeMenu.CheckMenuItem(IDC_TEXT_SMALLEST+m_textSize,MF_CHECKED|MF_BYCOMMAND);
  menu.AppendMenu(MF_SEPARATOR,0,(LPCSTR)NULL);
  menu.AppendMenu(MF_POPUP,(UINT_PTR)sizeMenu.GetSafeHmenu(),"Text Si&ze");

  menu.TrackPopupMenu(TPM_LEFTALIGN,point.x,point.y,GetParentFrame(),NULL);
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

void SourceEdit::OnTextSize(UINT nID)
{
  int size = nID - IDC_TEXT_SMALLEST;
  if ((size >= 0) && (size <= 4))
    theApp.SendAllFrames(InformApp::SourceTextSize,size);
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

void SourceEdit::OnFormatComment(UINT id)
{
  CallEdit(SCI_BEGINUNDOACTION);

  int line1 = (int)CallEdit(SCI_LINEFROMPOSITION,CallEdit(SCI_GETSELECTIONSTART));
  int line2 = (int)CallEdit(SCI_LINEFROMPOSITION,CallEdit(SCI_GETSELECTIONEND));

  // Check that this is not the last line of a multi-line selection with the cursor at the line's start
  if ((line2 > line1) && (CallEdit(SCI_POSITIONFROMLINE,line2) == CallEdit(SCI_GETSELECTIONEND)))
    line2--;

  // Get the start of the first line and the end of the last line
  int pos1 = (int)CallEdit(SCI_POSITIONFROMLINE,line1);
  int pos2 = (int)CallEdit(SCI_GETLINEENDPOSITION,line2);

  if ((pos1 >= 0) && (pos2 > pos1))
  {
    // Get the first and last characters
    int len = (int)CallEdit(SCI_GETLENGTH);
    CStringW text1 = GetTextRange(pos1,pos1+1,len);
    CStringW text2 = GetTextRange(pos2-1,pos2,len);

    if ((id == ID_FORMAT_COMMENT) && (text1 != L"[") && (text2 != L"]"))
    {
      CallEdit(SCI_SETTARGETSTART,pos2);
      CallEdit(SCI_SETTARGETEND,pos2);
      CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"]");
      CallEdit(SCI_SETTARGETSTART,pos1);
      CallEdit(SCI_SETTARGETEND,pos1);
      CallEdit(SCI_REPLACETARGET,1,(LONG_PTR)"[");
    }
    else if ((id == ID_FORMAT_UNCOMMENT) && (text1 == L"[") && (text2 == L"]"))
    {
      CallEdit(SCI_SETTARGETSTART,pos2-1);
      CallEdit(SCI_SETTARGETEND,pos2);
      CallEdit(SCI_REPLACETARGET,0,(LONG_PTR)"");
      CallEdit(SCI_SETTARGETSTART,pos1);
      CallEdit(SCI_SETTARGETEND,pos1+1);
      CallEdit(SCI_REPLACETARGET,0,(LONG_PTR)"");
    }
  }
  CallEdit(SCI_ENDUNDOACTION);
}

void SourceEdit::OnFormatRenumber()
{
  CArray<SourceLexer::Heading> headings;
  GetAllHeadings(headings);

  int indexes[SourceLexer::Section+1];
  for (int j = 0; j < sizeof(indexes) / sizeof(indexes[0]); j++)
    indexes[j] = 1;

  CallEdit(SCI_BEGINUNDOACTION);
  for (int i = 0; i < headings.GetSize(); i++)
  {
    const SourceLexer::Heading& heading = headings.GetAt(i);
    switch (heading.level)
    {
    case SourceLexer::Volume:
    case SourceLexer::Book:
    case SourceLexer::Part:
    case SourceLexer::Chapter:
    case SourceLexer::Section:
      {
        int pos = 0;
        CStringW sectionName = heading.name.Tokenize(L" ",pos);
        if (pos <= 0)
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
  CallEdit(SCI_ENDUNDOACTION);
}

void SourceEdit::OnSavePointReached(NMHDR*, LRESULT*)
{
  GetParentFrame()->PostMessage(WM_PROJECTEDITED);
}

void SourceEdit::OnSavePointLeft(NMHDR*, LRESULT*)
{
  GetParentFrame()->PostMessage(WM_PROJECTEDITED);
}

void SourceEdit::OnStyleNeeded(NMHDR* hdr, LRESULT*)
{
  SCNotification* notify = (SCNotification*)hdr;

  SourceLexer lex(this,SourceLexer::LexApply);
  lex.Process((int)CallEdit(SCI_GETENDSTYLED),notify->position);
}

void SourceEdit::OnCharAdded(NMHDR* hdr, LRESULT* res)
{
  SCNotification* notify = (SCNotification*)hdr;

  // Detect a new line
  if (m_autoIndent && ((notify->ch == '\r') || (notify->ch == '\n')))
  {
    // Add the same number of leading tabs and spaces as the previous line
    int line = CallEdit(SCI_LINEFROMPOSITION,CallEdit(SCI_GETCURRENTPOS));
    if (line > 0)
    {
      int len = CallEdit(SCI_LINELENGTH,line-1);
      char* buffer = (char*)alloca(len+1);
      CallEdit(SCI_GETLINE,line-1,(LONG_PTR)buffer);
      buffer[len] = '\0';
      for (int i = 0; buffer[i]; i++)
      {
        if ((buffer[i] != '\t') && (buffer[i] != ' '))
        {
          buffer[i] = '\0';
          break;
        }
      }
      CallEdit(SCI_REPLACESEL,0,(LONG_PTR)buffer);
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

  // If the pasted text came from HTML, try to interpret tables
  UINT CF_HTML = ::RegisterClipboardFormat("HTML Format");
  if (data.IsDataAvailable(CF_HTML))
  {
    CStringW theText(cp->utext,cp->ulen);
    CStringW newText, line;
    newText.Preallocate(theText.GetLength());

    bool foundTable = false;
    bool inTable = false;

    int i = 0;
    while (GetNextLine(theText,line,i))
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
          for (int i = 0; i < tokens.GetSize(); i++)
          {
            if (i > 0)
              line.AppendChar(L'\t');
            line.Append(tokens.GetAt(i));
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
      }
      if (!newText.IsEmpty())
        newText.AppendChar(L'\n');
      newText.Append(line);
    }

    // Only change the text to be pasted if a table was found
    if (foundTable)
    {
      CString newTextUtf = TextFormat::UnicodeToUTF8(newText);
			cp->text = new char[newTextUtf.GetLength() + 1];
      strcpy(cp->text,newTextUtf);
      *res = 1;
    }
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
}

bool SourceEdit::SaveFile(CFile* file)
{
  // Get the contents of the document as UTF-8
  CallEdit(SCI_CONVERTEOLS,SC_EOL_LF);
  int len = (int)CallEdit(SCI_GETLENGTH);
  CString utfText;
  LPSTR utfPtr = utfText.GetBufferSetLength(len+1);
  CallEdit(SCI_GETTEXT,len+1,(sptr_t)utfPtr);
  utfText.ReleaseBuffer();

  // Write out the document contents
  try
  {
    file->Write(utfText,utfText.GetLength());
    CallEdit(SCI_SETSAVEPOINT);
  }
  catch (CException* ex)
  {
    ex->Delete();
    return false;
  }
  return true;
}

bool SourceEdit::IsEdited(void)
{
  return (CallEdit(SCI_GETMODIFY) != 0);
}

void SourceEdit::Search(LPCWSTR text, std::vector<SearchWindow::Result>& results)
{
  CWaitCursor wc;

  int len = (int)CallEdit(SCI_GETLENGTH);
  TextToFind find;
  find.chrg.cpMin = 0;
  find.chrg.cpMax = len;
  CString textUtf = TextFormat::UnicodeToUTF8(text);
  find.lpstrText = (char*)(LPCSTR)textUtf;

  while (true)
  {
    // Search for the text
    if (CallEdit(SCI_FINDTEXT,0,(sptr_t)&find) == -1)
      return;

    // Get the surrounding text as context
    CStringW leading = GetTextRange(find.chrgText.cpMin-4,find.chrgText.cpMin,len);
    CStringW match = GetTextRange(find.chrgText.cpMin,find.chrgText.cpMax,len);
    CStringW trailing = GetTextRange(find.chrgText.cpMax,find.chrgText.cpMax+32,len);
    CStringW context = leading + match + trailing;
    context.Replace(L'\n',L' ');
    context.Replace(L'\r',L' ');
    context.Replace(L'\t',L' ');

    // Store the found result
    SearchWindow::Result result;
    result.context = context;
    result.inContext.cpMin = leading.GetLength();
    result.inContext.cpMax = leading.GetLength() + match.GetLength();
    result.sourceLocation = SOURCE_FILE;
    result.inSource.cpMin = find.chrgText.cpMin;
    result.inSource.cpMax = find.chrgText.cpMax;
    results.push_back(result);

    // Look for the next match
    find.chrg.cpMin = find.chrgText.cpMax;
    theApp.RunMessagePump();
  }
}

void SourceEdit::Highlight(CHARRANGE range, bool centre)
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
      Highlight(range,centre);

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
  CallEdit(SCIX_VISIBLEREGION,startLine,endLine);
  Highlight(startLine,-1,false);
}

bool SourceEdit::IsLineShown(int line)
{
  return (CallEdit(SCIX_ISLINEINVISIBLEREGION,line) != 0);
}

void SourceEdit::PasteCode(const wchar_t* code)
{
  // Process escapes in the code string
  size_t len = wcslen(code);
  CStringW theCode;
  theCode.Preallocate((int)len);
  for (size_t i = 0; i < len; i++)
  {
    wchar_t c = code[i];
    if (c == '[')
    {
      int unicode = 0;
      if (swscanf(code+i,L"[=0x%x=]",&unicode) == 1)
      {
        theCode.AppendChar((wchar_t)unicode);
        i += 9;
        continue;
      }
    }
    theCode.AppendChar(c);
  }

  CString theCodeUtf = TextFormat::UnicodeToUTF8(theCode);
  CallEdit(SCI_REPLACESEL,0,(sptr_t)(LPCSTR)theCodeUtf);
}

void SourceEdit::SetTextSize(int size)
{
  if (m_textSize != size)
  {
    m_textSize = size;
    switch (m_textSize)
    {
    case 0:
      CallEdit(SCI_SETZOOM,-1);
      break;
    case 1:
      CallEdit(SCI_SETZOOM,0);
      break;
    case 2:
      CallEdit(SCI_SETZOOM,1);
      break;
    case 3:
      CallEdit(SCI_SETZOOM,2);
      break;
    case 4:
      CallEdit(SCI_SETZOOM,4);
      break;
    default:
      ASSERT(FALSE);
      break;
    }
  }
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

  // If the dialog is over the word, move it
  CRect intersectR;
  if (intersectR.IntersectRect(wordR,wndR))
  {
    // Get the size of the display
    MONITORINFO monInfo;
    ::ZeroMemory(&monInfo,sizeof monInfo);
    monInfo.cbSize = sizeof monInfo;
    HMONITOR mon = ::MonitorFromWindow(child->GetSafeHwnd(),MONITOR_DEFAULTTOPRIMARY);
    ::GetMonitorInfo(mon,&monInfo);

    // Try moving the dialog, but keep it on-screen
    if (wordR.top-wndR.Height() >= 0)
    {
      child->SetWindowPos(&CWnd::wndTop,wndR.left,wordR.top-wndR.Height(),0,0,
        SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_NOSIZE);
    }
    else if (wordR.bottom+wndR.Height() < monInfo.rcWork.bottom)
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

void SourceEdit::LoadSettings(CRegKey& key, bool prefsChange)
{
  DWORD value;
  if (key.QueryDWORDValue("Source Text Size",value) == ERROR_SUCCESS)
    SetTextSize(value);

  if (key.QueryDWORDValue("Source Tab Size Chars",value) == ERROR_SUCCESS)
  {
    if (value > 0)
      CallEdit(SCI_SETTABWIDTH,value);
  }

  if (key.QueryDWORDValue("Auto Indent",value) == ERROR_SUCCESS)
    m_autoIndent = (value != 0);

  // Adjust wrapped line indentation
  bool indent = true;
  if (key.QueryDWORDValue("Indent Wrapped Lines",value) == ERROR_SUCCESS)
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

void SourceEdit::SaveSettings(CRegKey& key)
{
  key.SetDWORDValue("Source Text Size",m_textSize);
}

void SourceEdit::GetAllHeadings(CArray<SourceLexer::Heading>& headings)
{
  SourceLexer lex(this,SourceLexer::LexHeadings);
  lex.Process(0,-1);
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
