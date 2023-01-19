#include "stdafx.h"
#include "SearchEdit.h"
#include "Inform.h"
#include "Messages.h"
#include "DarkMode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(SearchEdit, UnicodeEdit)

BEGIN_MESSAGE_MAP(SearchEdit, UnicodeEdit)
  ON_WM_ERASEBKGND()
  ON_WM_CTLCOLOR_REFLECT()
	ON_WM_NCPAINT()
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
  CDibSection* image = theApp.GetCachedImage("Search");
  ASSERT(image != NULL);

  CRect editSize;
  GetRect(editSize);
  editSize.left += image->GetSize().cx+1;
  SetRect(editSize);
}

BOOL SearchEdit::OnEraseBkgnd(CDC* pDC)
{
  BOOL erased = UnicodeEdit::OnEraseBkgnd(pDC);

  DarkMode* dark = DarkMode::GetActive(this);
  COLORREF back = dark ? dark->GetColour(DarkMode::Darkest) : theApp.GetColour(InformApp::ColourBack);

  CRect client;
  GetClientRect(client);
  pDC->FillSolidRect(client,back);

  CDC dcImage;
  dcImage.CreateCompatibleDC(pDC);
  CDibSection* image = GetImage(back,dark != NULL);
  CBitmap* oldBitmap = CDibSection::SelectDibSection(dcImage,image);
  CSize imageSize = image->GetSize();
  pDC->BitBlt(1,(client.Height()-imageSize.cy)/2,imageSize.cx,imageSize.cy,
    &dcImage,0,0,SRCCOPY);
  dcImage.SelectObject(oldBitmap);

  return erased;
}

HBRUSH SearchEdit::CtlColor(CDC* dc, UINT color)
{
  DarkMode* dark = DarkMode::GetActive(this);
  if (dark)
  {
    dc->SetTextColor(dark->GetColour(m_editing ? DarkMode::Fore : DarkMode::Dark2));
    dc->SetBkColor(dark->GetColour(DarkMode::Darkest));
    return *(dark->GetBrush(DarkMode::Darkest));
  }
  else
  {
    dc->SetTextColor(
      theApp.GetColour(m_editing ? InformApp::ColourText : InformApp::ColourFaint));
    dc->SetBkColor(theApp.GetColour(InformApp::ColourBack));

    if (m_back.GetSafeHandle() == 0)
      m_back.CreateSolidBrush(theApp.GetColour(InformApp::ColourBack));
    return m_back;
  }

  return 0;
}

void SearchEdit::OnNcPaint()
{
  DarkMode* dark = DarkMode::GetActive(this);
  if (dark)
    dark->DrawNonClientBorder(this,
      m_editing ? DarkMode::Dark2 : DarkMode::Dark3,DarkMode::Darkest);
  else
    Default();
}

void SearchEdit::OnEnKillFocus()
{
  if (m_editing)
  {
    m_editing = false;
    GetWindowText(m_editText);
    SetWindowText(m_displayText);

    // Force the border to be redrawn in dark mode
    GetParent()->Invalidate();
  }
}

void SearchEdit::OnEnSetFocus()
{
  if (m_editing == false)
  {
    m_editing = true;
    SetWindowText(m_editText);
    SetSel(0,-1);

    // Force the border to be redrawn in dark mode
    GetParent()->Invalidate();
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

CDibSection* SearchEdit::GetImage(COLORREF back, bool dark)
{
  CDibSection* image = theApp.GetCachedImage("Search-blend");
  if (image != NULL)
    return image;

  CDibSection* original = theApp.GetCachedImage("Search");
  ASSERT(original != NULL);
  CSize imageSize = original->GetSize();

  image = new CDibSection();
  CDC* dc = GetDC();
  image->CreateBitmap(dc->GetSafeHdc(),imageSize.cx,imageSize.cy);
  ReleaseDC(dc);

  int br = GetRValue(back);
  int bg = GetGValue(back);
  int bb = GetBValue(back);

  if (dark)
  {
    br = 0xFF - br;
    bg = 0xFF - bg;
    bb = 0xFF - bb;
  }

  int r, g, b, a;
  DWORD src;
  for (int y = 0; y < imageSize.cy; y++)
  {
    for (int x = 0; x < imageSize.cx; x++)
    {
      src = original->GetPixel(x,y);
      b = src & 0xFF;
      src >>= 8;
      g = src & 0xFF;
      src >>= 8;
      r = src & 0xFF;
      src >>= 8;
      a = src & 0xFF;

      r = (r * br) >> 8;
      g = (g * bg) >> 8;
      b = (b * bb) >> 8;

      if (dark)
      {
        r = 0xFF - r;
        g = 0xFF - g;
        b = 0xFF - b;
      }

      image->SetPixel(x,y,(a<<24)|(r<<16)|(g<<8)|b);
    }
  }

  theApp.CacheImage("Search-blend",image);
  return image;
}
