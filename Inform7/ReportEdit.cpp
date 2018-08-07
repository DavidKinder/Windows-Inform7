#include "stdafx.h"
#include "Inform.h"
#include "ReportEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ReportEdit, CWnd)

BEGIN_MESSAGE_MAP(ReportEdit, CWnd)
  ON_WM_MOUSEWHEEL()
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
END_MESSAGE_MAP()

ReportEdit::ReportEdit()
{
  m_fixed = false;
}

BOOL ReportEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  if (nFlags & MK_CONTROL)
    return TRUE;
  return CWnd::OnMouseWheel(nFlags,zDelta,pt);
}

BOOL ReportEdit::Create(CWnd* parent, UINT id)
{
  if (!CWnd::Create("Scintilla",NULL,WS_CHILD|WS_CLIPCHILDREN,CRect(0,0,0,0),parent,id))
    return FALSE;

  m_editPtr = (sptr_t)SendMessage(SCI_GETDIRECTPOINTER);
  CallEdit(SCI_SETREADONLY,TRUE);
  CallEdit(SCI_SETSCROLLWIDTH,100);
  CallEdit(SCI_SETSCROLLWIDTHTRACKING,TRUE);
  CallEdit(SCI_SETSELFORE,TRUE,::GetSysColor(COLOR_HIGHLIGHTTEXT));
  CallEdit(SCI_SETSELBACK,TRUE,::GetSysColor(COLOR_HIGHLIGHT));
  CallEdit(SCI_STYLESETFORE,STYLE_DEFAULT,theApp.GetColour(InformApp::ColourText));
  CallEdit(SCI_STYLESETBACK,STYLE_DEFAULT,theApp.GetColour(InformApp::ColourBack));
  for (int i = 0; i < 5; i++)
    CallEdit(SCI_SETMARGINWIDTHN,i,0);
  CallEdit(SCI_SETMARGINLEFT,0,4);
  CallEdit(SCI_SETMARGINRIGHT,0,4);
  CallEdit(SCI_SETCARETSTYLE,CARETSTYLE_INVISIBLE,0);
  CallEdit(SCI_CLEARCMDKEY,SCK_ADD+(SCMOD_CTRL<<16));
  CallEdit(SCI_CLEARCMDKEY,SCK_SUBTRACT+(SCMOD_CTRL<<16));
  CallEdit(SCI_CLEARCMDKEY,SCK_DIVIDE+(SCMOD_CTRL<<16));
  SetFonts();

  return TRUE;
}

void ReportEdit::FontChanged(void)
{
  SetFonts();
  Invalidate();
}

void ReportEdit::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(CallEdit(SCI_GETSELECTIONSTART) != CallEdit(SCI_GETSELECTIONEND));
}

void ReportEdit::OnEditCopy()
{
  CallEdit(SCI_COPY);
}

void ReportEdit::OnEditSelectAll()
{
  CallEdit(SCI_SELECTALL);
}

void ReportEdit::AppendText(const char* text)
{
  CallEdit(SCI_SETREADONLY,FALSE);

  int len = (int)CallEdit(SCI_GETLENGTH);
  CallEdit(SCI_APPENDTEXT,(DWORD)strlen(text),(LONG_PTR)text);
  if (m_fixed)
  {
    CallEdit(SCI_STARTSTYLING,len,31);
    CallEdit(SCI_SETSTYLING,(DWORD)strlen(text),1);
  }

  CallEdit(SCI_SETREADONLY,TRUE);
}

void ReportEdit::ClearText(void)
{
  CallEdit(SCI_SETREADONLY,FALSE);
  CallEdit(SCI_CLEARALL);
  CallEdit(SCI_SETSCROLLWIDTH,100);
  CallEdit(SCI_SETREADONLY,TRUE);
  m_fixed = false;
}

void ReportEdit::SetFormat(bool fixed)
{
  m_fixed = fixed;
}

void ReportEdit::SetFonts(void)
{
  CallEdit(SCI_STYLESETFONT,STYLE_DEFAULT,(sptr_t)(LPCSTR)theApp.GetFontName(InformApp::FontDisplay));
  CallEdit(SCI_STYLESETSIZE,STYLE_DEFAULT,10 * theApp.GetFontSize(InformApp::FontDisplay));
  CallEdit(SCI_STYLECLEARALL);
  CallEdit(SCI_STYLESETFONT,1,(sptr_t)(LPCSTR)theApp.GetFontName(InformApp::FontFixedWidth));
}

extern "C" sptr_t __stdcall Scintilla_DirectFunction(sptr_t, UINT, uptr_t, sptr_t);

LONG_PTR ReportEdit::CallEdit(UINT msg, DWORD wp, LONG_PTR lp)
{
  return Scintilla_DirectFunction(m_editPtr,msg,wp,lp);
}
