#include "stdafx.h"
#include "SearchEdit.h"
#include "Inform.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SearchEdit, UnicodeEdit)

BEGIN_MESSAGE_MAP(SearchEdit, UnicodeEdit)
  ON_WM_ERASEBKGND()
  ON_WM_CTLCOLOR_REFLECT()
  ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillFocus)
  ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetFocus)
  ON_COMMAND(ID_EDIT_CUT, Cut)
  ON_COMMAND(ID_EDIT_COPY, Copy)
  ON_COMMAND(ID_EDIT_PASTE, Paste)
  ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
END_MESSAGE_MAP()

SearchEdit::SearchEdit(UINT msg, LPCWSTR displayText, LPCSTR accName)
  : m_msg(msg), m_displayText(displayText), m_accName(accName)
{
  EnableActiveAccessibility();
  m_editing = false;
  m_image = theApp.GetCachedImage("Search");
  m_image->AlphaBlend(::GetSysColor(COLOR_WINDOW));
}

void SearchEdit::Init(UINT id, CWnd* parent)
{
  SubclassDlgItem(id,parent);

  m_editing = false;
  SetWindowText(m_displayText);
  SetEditRect();
}

void SearchEdit::SetEditRect(void)
{
  CRect editSize;
  GetRect(editSize);
  editSize.left += m_image->GetSize().cx+1;
  SetRect(editSize);
}

BOOL SearchEdit::OnEraseBkgnd(CDC* pDC)
{
  BOOL erased = UnicodeEdit::OnEraseBkgnd(pDC);

  CDC dcImage;
  dcImage.CreateCompatibleDC(pDC);
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dcImage,m_image);
  CSize imageSize = m_image->GetSize();

  CRect client;
  GetClientRect(client);
  pDC->FillSolidRect(client,theApp.GetColour(InformApp::ColourBack));

  pDC->BitBlt(1,(client.Height()-imageSize.cy)/2,imageSize.cx,imageSize.cy,
    &dcImage,0,0,SRCCOPY);
  dcImage.SelectObject(oldBitmap);

  return erased;
}

HBRUSH SearchEdit::CtlColor(CDC* dc, UINT color)
{
  HBRUSH brush = (HBRUSH)Default();
  dc->SetTextColor(
    theApp.GetColour(m_editing ? InformApp::ColourText : InformApp::ColourFaint));
  dc->SetBkColor(theApp.GetColour(InformApp::ColourBack));
  return brush;
}

void SearchEdit::OnEnKillFocus()
{
  if (m_editing)
  {
    m_editing = false;
    GetWindowText(m_editText);
    SetWindowText(m_displayText);
  }
}

void SearchEdit::OnEnSetFocus()
{
  if (m_editing == false)
  {
    m_editing = true;
    SetWindowText(m_editText);
    SetSel(0,-1);
  }
}

void SearchEdit::OnEditSelectAll()
{
  SetSel(0,-1);
}

BOOL SearchEdit::PreTranslateMessage(MSG* pMsg)
{
  if (pMsg->message == WM_KEYDOWN)
  {
    bool ctrl = ((::GetKeyState(VK_CONTROL) & 0x8000) != 0);

    switch (pMsg->wParam)
    {
    case VK_RETURN:
      {
        CStringW text;
        GetWindowText(text);
        if (text.GetLength() > 0)
          GetParentFrame()->SendMessage(m_msg,(WPARAM)(LPCWSTR)text);
      }
      break;
    case VK_ESCAPE:
      GetParentFrame()->PostMessage(WM_SELECTSIDE,0);
      break;
    case 'A':
      if (ctrl)
        PostMessage(WM_COMMAND,ID_EDIT_SELECT_ALL);
      break;
    }
  }
  return UnicodeEdit::PreTranslateMessage(pMsg);
}

HRESULT SearchEdit::get_accName(VARIANT child, BSTR* accName)
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
