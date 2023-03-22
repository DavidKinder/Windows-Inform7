#include "stdafx.h"
#include "ReportEdit.h"
#include "Inform.h"

#include "Platform.h"
#include "Scintilla.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(ReportEdit, CWnd)

BEGIN_MESSAGE_MAP(ReportEdit, DrawScrollWindow)
  ON_WM_MOUSEWHEEL()
  ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateNeedSel)
  ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
  ON_NOTIFY_REFLECT(SCNX_SETSCROLLINFO, OnSetScrollInfo)
  ON_NOTIFY_REFLECT(SCNX_GETSCROLLINFO, OnGetScrollInfo)
END_MESSAGE_MAP()

ReportEdit::ReportEdit()
{
  EnableActiveAccessibility();
  m_fixed = false;
}

BOOL ReportEdit::Create(CWnd* parent, UINT id)
{
  if (!DrawScrollWindow::Create("Scintilla",WS_CHILD|WS_CLIPCHILDREN,parent,id))
    return FALSE;

  m_v.SetActive(true);
  m_h.SetActive(true);

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

void ReportEdit::SetAccName(LPCSTR name)
{
  m_accName = name;
}

void ReportEdit::FontChanged(void)
{
  SetFonts();
  Invalidate();
}

void ReportEdit::PrefsChanged(void)
{
  CallEdit(WM_SETTINGCHANGE);
}

void ReportEdit::SetDarkMode(DarkMode* dark)
{
  if (dark)
  {
    for (int i : { STYLE_DEFAULT,0,1 })
    {
      CallEdit(SCI_STYLESETFORE,i,dark->GetColour(DarkMode::Fore));

      // This matches the background of CEF HTML pages in dark mode
      CallEdit(SCI_STYLESETBACK,i,RGB(0x12,0x12,0x12));
    }
  }
  else
  {
    for (int i : { STYLE_DEFAULT,0,1 })
    {
      CallEdit(SCI_STYLESETFORE,i,theApp.GetColour(InformApp::ColourText));
      CallEdit(SCI_STYLESETBACK,i,theApp.GetColour(InformApp::ColourBack));
    }
  }
}

BOOL ReportEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
  if (nFlags & (MK_SHIFT|MK_CONTROL))
    return TRUE;
  return DrawScrollWindow::OnMouseWheel(nFlags,zDelta,pt);
}

void ReportEdit::OnSetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result)
{
  SCNXSetScrollInfo* ssi = (SCNXSetScrollInfo*)pNotifyStruct;
  ssi->nPos = SetScrollInfo(ssi->nBar,(LPCSCROLLINFO)ssi->lpsi,ssi->bRedraw);
  *result = 1;
}

void ReportEdit::OnGetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result)
{
  SCNXGetScrollInfo* gsi = (SCNXGetScrollInfo*)pNotifyStruct;
  *result = GetScrollInfo(gsi->nBar,(LPSCROLLINFO)gsi->lpsi) ? 1 : 0;
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

HRESULT ReportEdit::get_accName(VARIANT child, BSTR* accName)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  if (child.lVal == CHILDID_SELF)
  {
    *accName = m_accName.AllocSysString();
    return S_OK;
  }
  return S_FALSE;
}

HRESULT ReportEdit::get_accValue(VARIANT child, BSTR* accValue)
{
  if (child.vt != VT_I4)
    return E_INVALIDARG;

  if (child.lVal == CHILDID_SELF)
  {
    int len = (int)CallEdit(SCI_GETLENGTH);
    CString text;
    LPSTR textPtr = text.GetBufferSetLength(len+1);
    CallEdit(SCI_GETTEXT,len+1,(sptr_t)textPtr);
    text.ReleaseBuffer();

    *accValue = text.AllocSysString();
    return S_OK;
  }
  return S_FALSE;
}
