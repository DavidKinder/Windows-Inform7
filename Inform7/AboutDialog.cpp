#include "stdafx.h"
#include "AboutDialog.h"
#include "Build.h"
#include "Inform.h"

#include "DarkMode.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

AboutDialog::AboutDialog() : I7BaseDialog(AboutDialog::IDD),
  m_dpi(96), m_creditHeight(0), m_initialSize(0,0)
{
}

BEGIN_MESSAGE_MAP(AboutDialog, I7BaseDialog)
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_NOTIFY(EN_REQUESTRESIZE, IDC_CREDITS, OnCreditsResize)
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

void AboutDialog::DoDataExchange(CDataExchange* pDX)
{
  I7BaseDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LOGO, m_logo);
  DDX_Control(pDX, IDC_TITLE, m_title);
  DDX_Control(pDX, IDC_VERSION, m_version);
  DDX_Control(pDX, IDC_COPYRIGHT, m_copyright);
  DDX_Control(pDX, IDC_CREDITS, m_credits);
}

static DWORD CALLBACK FileReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  CFile* file = (CFile*)dwCookie;
  *pcb = file->Read(pbBuff,cb);
  return 0;
}

BOOL AboutDialog::OnInitDialog()
{
  I7BaseDialog::OnInitDialog();
  m_dpi = DPI::getWindowDPI(this);

  theApp.SetIcon(this);
  SetTitleFont();
  SetLogoBitmap();

  CString version1, version2;
  m_version.GetWindowText(version1);
  version2.Format(version1,INFORM_VER,BUILD_DATE);
  m_version.SetWindowText(version2);

  const char* text1 =
    "\\b Inform written by\\b0\\par"
    "\\tab Graham Nelson with help from\\par"
    "\\tab Emily Short and many others\\par"
    "\\par"
    "\\b Windows Inform front-end written by\\b0\\par"
    "\\tab David Kinder\\par"
    "\\par"
    "\\b Reference Inform front-end written by\\b0\\par"
    "\\tab Andrew Hunter\\par"
    "\\tab Toby Nelson\\par"
    "\\par"
    "\\b Contributions to Inform 6\\b0\\par"
    "\\tab Jesse McGrew\\par";
  const char* text2 =
    "\\tab Andrew Plotkin\\par"
    "\\tab David Kinder\\par"
    "\\tab Emily Short\\par"
    "\\tab Gunther Schmidl\\par"
    "\\tab Jason Penney\\par"
    "\\tab Joe Mason\\par"
    "\\tab Cedric Knight\\par"
    "\\tab Dave Griffith\\par"
    "\\tab Roger Firth\\par"
    "\\tab Michael Coyne\\par"
    "\\tab David Cornelson\\par"
    "\\tab Neil Cerutti\\par"
    "\\tab Kevin Bracey\\par"
    "\\tab Brady Garvin\\par"
    "\\tab Xun Gong\\par"
    "\\par"
    "\\b Frotz Z-code interpreter written by\\b0\\par"
    "\\tab Stefan Jokisch\\par"
    "\\par"
    "\\b Glulxe Glulx interpreter written by\\b0\\par"
    "\\tab Andrew Plotkin\\par"
    "\\par"
    "\\b Git Glulx interpreter written by\\b0\\par"
    "\\tab Iain Merrick\\par"
    "\\par"
    "\\b Parchment interpreter written by\\b0\\par"
    "\\tab Dannii Willis\\par"
    "\\tab Andrew Plotkin\\par"
    "\\tab Inspired by the earlier work of Atul Varma,\\par"
    "\\tab Thomas Thurman and Eric Liga.\\par"
    "\\par"
    "\\b Quixe Glulx interpreter written by\\b0\\par"
    "\\tab Andrew Plotkin\\par"
    "\\par"
    "\\b Elastic tabstops invented by\\b0\\par"
    "\\tab Nick Gravgaard";

  // Start with only as much text as should be visible
  m_creditsRTF.Format("{\\rtf1\\ansi{\\fs%d%s}}",
    (theApp.GetFontSize(InformApp::FontSystem)*2)+2,text1);
  m_credits.SetTextRTF(m_creditsRTF);

  // Ask the control how big the credits text is
  LayoutControls();
  m_credits.SetEventMask(ENM_REQUESTRESIZE);
  m_credits.SendMessage(EM_REQUESTRESIZE);
  m_credits.SetEventMask(0);

  // Resize the window to set the appropriate credits height
  CRect creditsRect;
  m_credits.GetWindowRect(creditsRect);
  CRect dlgRect;
  GetWindowRect(dlgRect);
  dlgRect.bottom += (m_creditHeight - creditsRect.Height());
  CSize fontSize = theApp.MeasureFont(this,m_credits.GetFont());
  dlgRect.bottom += 2*(fontSize.cy/4);
  MoveWindow(dlgRect);

  // Show all the credits text
  m_creditsRTF.Format("{\\rtf1\\ansi{\\fs%d%s%s}}",
    (theApp.GetFontSize(InformApp::FontSystem)*2)+2,text1,text2);
  m_credits.SetTextRTF(m_creditsRTF);

  m_initialSize = dlgRect.Size();
  return TRUE;
}

BOOL AboutDialog::OnEraseBkgnd(CDC* dc)
{
  EraseWithGripper(dc);
  return TRUE;
}

void AboutDialog::OnGetMinMaxInfo(MINMAXINFO* mmi)
{
  I7BaseDialog::OnGetMinMaxInfo(mmi);
  if (m_initialSize.cx > 0)
    mmi->ptMinTrackSize = CPoint(m_initialSize);
}

void AboutDialog::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType,cx,cy);
  LayoutControls();
}

void AboutDialog::OnCreditsResize(NMHDR* notify, LRESULT*)
{
  REQRESIZE* resize = (REQRESIZE*)notify;
  m_creditHeight = resize->rc.bottom - resize->rc.top;
}

LRESULT AboutDialog::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  int newDpi = (int)HIWORD(wparam);
  m_initialSize.cx = MulDiv(m_initialSize.cx,newDpi,m_dpi);
  m_initialSize.cy = MulDiv(m_initialSize.cy,newDpi,m_dpi);

  Default();

  if (m_dpi != newDpi)
  {
    m_dpi = newDpi;

    SetTitleFont();
    if (m_credits.GetSafeHwnd() != 0)
      m_credits.SetTextRTF(m_creditsRTF);
  }
  return 0;
}

void AboutDialog::SetDarkMode(DarkMode* dark)
{
  I7BaseDialog::SetDarkMode(dark);

  if (GetSafeHwnd() != 0)
  {
    SetLogoBitmap();
    m_credits.SetDarkMode(dark);

    // Set colours for the credits control
    CHARFORMAT format;
    format.cbSize = sizeof format;
    format.dwMask = CFM_COLOR;
    format.dwEffects = 0;
    if (dark)
    {
      format.crTextColor = dark->GetColour(DarkMode::Fore);
      m_credits.SetDefaultCharFormat(format);
      m_credits.SetBackgroundColor(FALSE,dark->GetColour(DarkMode::Back));
    }
    else
    {
      format.crTextColor = ::GetSysColor(COLOR_BTNTEXT);
      m_credits.SetDefaultCharFormat(format);
      m_credits.SetBackgroundColor(TRUE,0);
    }
    m_credits.SetTextRTF(m_creditsRTF);
  }
}

void AboutDialog::LayoutControls(void)
{
  if (m_logo.GetSafeHwnd() == 0)
    return;

  CRect dlgRect;
  GetClientRect(dlgRect);

  CRect logoRect, titleRect, versionRect, creditsRect, copyrightRect;
  m_logo.GetWindowRect(logoRect);
  ScreenToClient(logoRect);
  m_title.GetWindowRect(titleRect);
  ScreenToClient(titleRect);
  m_version.GetWindowRect(versionRect);
  ScreenToClient(versionRect);
  m_credits.GetWindowRect(creditsRect);
  ScreenToClient(creditsRect);
  m_copyright.GetWindowRect(copyrightRect);
  ScreenToClient(copyrightRect);

  int gap = titleRect.top-logoRect.bottom;

  CRect newLogoRect(logoRect.TopLeft(),m_bitmap->GetSize());
  newLogoRect.MoveToX((dlgRect.Width()-newLogoRect.Width())/2);
  m_logo.MoveWindow(newLogoRect,TRUE);

  CRect newTitleRect(titleRect);
  newTitleRect.MoveToX((dlgRect.Width()-newTitleRect.Width())/2);
  newTitleRect.MoveToY(newLogoRect.bottom+gap);
  m_title.MoveWindow(newTitleRect,TRUE);

  CRect newVersionRect(versionRect);
  newVersionRect.MoveToX((dlgRect.Width()-newVersionRect.Width())/2);
  newVersionRect.MoveToY(newTitleRect.bottom+gap);
  m_version.MoveWindow(newVersionRect,TRUE);

  CRect newCopyrightRect(copyrightRect);
  newCopyrightRect.MoveToX((dlgRect.Width()-newCopyrightRect.Width())/2);
  newCopyrightRect.MoveToY(dlgRect.bottom-gap-newCopyrightRect.Height());
  m_copyright.MoveWindow(newCopyrightRect,TRUE);

  CRect newCreditsRect(creditsRect);
  newCreditsRect.right = dlgRect.right-newCreditsRect.left;
  newCreditsRect.top = newVersionRect.bottom+gap;
  newCreditsRect.bottom = newCopyrightRect.top-gap;
  m_credits.MoveWindow(newCreditsRect,TRUE);

  // Add margins to the credits
  newCreditsRect.MoveToXY(0,0);
  CSize fontSize = theApp.MeasureFont(this,m_credits.GetFont());
  newCreditsRect.left += fontSize.cx/2;
  newCreditsRect.right -= fontSize.cx/2;
  newCreditsRect.top += fontSize.cy/4;
  newCreditsRect.bottom -= fontSize.cy/4;
  m_credits.SetRect(newCreditsRect);

  // Invalidate the bottom right corner of the dialog, so that the
  // gripper is redrawn correctly
  CRect cornerRect = dlgRect;
  cornerRect.left = newCopyrightRect.right+1;
  cornerRect.top = newCreditsRect.bottom+1;
  InvalidateRect(cornerRect,TRUE);
}

void AboutDialog::SetTitleFont(void)
{
  if (m_title.GetSafeHwnd() != 0)
  {
    LOGFONT titleFont;
    m_title.GetFont()->GetLogFont(&titleFont);
    titleFont.lfWeight = FW_BOLD;
    titleFont.lfHeight = (LONG)(titleFont.lfHeight*1.6);
    m_titleFont.DeleteObject();
    m_titleFont.CreateFontIndirect(&titleFont);
    m_title.SetFont(&m_titleFont);
  }
}

void AboutDialog::SetLogoBitmap(void)
{
  CDibSection* bitmap = theApp.GetCachedImage("Inform");
  CSize bitmapSize = bitmap->GetSize();
  DarkMode* dark = DarkMode::GetActive(this);

  m_bitmap.reset(new CDibSection());
  CDC* dc = GetDC();
  m_bitmap->CreateBitmap(dc->GetSafeHdc(),bitmapSize.cx,bitmapSize.cy);
  ReleaseDC(dc);
  ::CopyMemory(m_bitmap->GetBits(),bitmap->GetBits(),
    (SIZE_T)bitmapSize.cx*(SIZE_T)bitmapSize.cy*sizeof(DWORD));
  m_bitmap->AlphaBlend(dark ?
    dark->GetColour(DarkMode::Darkest) : ::GetSysColor(COLOR_BTNFACE));
  m_logo.SetBitmap(m_bitmap->GetSafeHandle());
}

IMPLEMENT_DYNAMIC(AboutCreditsEdit, RichEdit)

BEGIN_MESSAGE_MAP(AboutCreditsEdit, RichEdit)
  ON_WM_SETFOCUS()
END_MESSAGE_MAP()

AboutCreditsEdit::AboutCreditsEdit() : RichEdit(InformApp::FontSystem)
{
}

void AboutCreditsEdit::OnSetFocus(CWnd* pOldWnd)
{
  RichEdit::OnSetFocus(pOldWnd);

  // By default the entire contents are selected,
  // so move the selection to the start of the text.
  SetSel(0,0);
}

BOOL AboutCreditsEdit::PreTranslateMessage(MSG* pMsg)
{
  if (RejectMsg(pMsg))
    return TRUE;
  return RichEdit::PreTranslateMessage(pMsg);
}
