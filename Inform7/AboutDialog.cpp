#include "stdafx.h"
#include "AboutDialog.h"
#include "Inform.h"
#include "OSLayer.h"
#include "Build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

AboutDialog::AboutDialog() : I7BaseDialog(AboutDialog::IDD,FALSE),
  m_creditHeight(0), m_initialSize(0,0), m_bitmap(NULL)
{
}

BEGIN_MESSAGE_MAP(AboutDialog, I7BaseDialog)
  ON_WM_ERASEBKGND()
  ON_WM_GETMINMAXINFO()
  ON_WM_SIZE()
  ON_NOTIFY(EN_REQUESTRESIZE, IDC_CREDITS, OnCreditsResize)
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
  theApp.SetIcon(this);

  CDibSection* bitmap = theApp.GetCachedImage("Inform");
  CSize bitmapSize = bitmap->GetSize();

  m_bitmap.reset(new CDibSection());
  CDC* dc = AfxGetMainWnd()->GetDC();
  m_bitmap->CreateBitmap(dc->GetSafeHdc(),bitmapSize.cx,bitmapSize.cy);
  AfxGetMainWnd()->ReleaseDC(dc);
  ::CopyMemory(m_bitmap->GetBits(),bitmap->GetBits(),
    bitmapSize.cx*bitmapSize.cy*sizeof(DWORD));
  m_bitmap->AlphaBlend(::GetSysColor(COLOR_BTNFACE));

  m_logo.SetBitmap(m_bitmap->GetSafeHandle());

  LOGFONT titleFont;
  m_title.GetFont()->GetLogFont(&titleFont);
  titleFont.lfWeight = FW_BOLD;
  titleFont.lfHeight = (LONG)(titleFont.lfHeight*1.6);
  m_titleFont.CreateFontIndirect(&titleFont);
  m_title.SetFont(&m_titleFont);

  CString version1, version2;
  m_version.GetWindowText(version1);
  version2.Format(version1,BUILD_DATE,NI_BUILD);
  m_version.SetWindowText(version2);

  const char* text1 =
    "\\b Inform written by\\b0\\par"
    "\\tab Graham Nelson with help from\\par"
    "\\tab Emily Short and many others:\\par"
    "\\par"
    "\\b Windows Inform front-end written by\\b0\\par"
    "\\tab David Kinder\\par"
    "\\par"
    "\\b Reference Inform front-end written by\\b0\\par"
    "\\tab Andrew Hunter\\par"
    "\\par"
    "\\b Inform 6 updates\\b0\\par"
    "\\tab Jesse McGrew";
  const char* text2 =
    "\\par"
    "\\par"
    "\\b Contributions to Inform 6\\b0\\par"
    "\\tab Emily Short\\par"
    "\\tab Gunther Schmidl\\par"
    "\\tab Andrew Plotkin\\par"
    "\\tab Jason Penney\\par"
    "\\tab Joe Mason\\par"
    "\\tab Cedric Knight\\par"
    "\\tab David Kinder\\par"
    "\\tab Dave Griffith\\par"
    "\\tab Roger Firth\\par"
    "\\tab Michael Coyne\\par"
    "\\tab David Cornelson\\par"
    "\\tab Neil Cerutti\\par"
    "\\tab Kevin Bracey\\par"
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
    "\\b Parchment Z-code interpreter written by\\b0\\par"
    "\\tab Thomas Thurman\\par"
    "\\tab Eric Liga\\par"
    "\\tab Atul Varma\\par"
    "\\tab Fredrik Ramsberg\\par"
    "\\tab Dannii Willis\\par"
    "\\tab Andrew Plotkin\\par"
    "\\par"
    "\\b libpng written by\\b0\\par"
    "\\tab Glenn Randers-Pehrson\\par"
    "\\tab Andreas Dilger\\par"
    "\\tab Guy Eric Schalnat, and others\\par"
    "\\par"
    "\\b zlib written by\\b0\\par"
    "\\tab Jean-loup Gailly\\par"
    "\\tab Mark Adler\\par"
    "\\par"
    "\\b jpeglib written by\\b0\\par"
    "\\tab The Independent JPEG Group\\par"
    "\\par"
    "\\b libogg and libvorbis written by\\b0\\par"
    "\\tab The Xiph.org Foundation\\par"
    "\\par"
    "\\b ModPlug written by\\b0\\par"
    "\\tab Olivier Lapicque\\par"
    "\\par"
    "\\b hunspell written by\\b0\\par"
    "\\tab Németh László\\par"
    "\\tab Kevin Hendricks, and others\\par"
    "\\par"
    "\\b Scintilla written by\\b0\\par"
    "\\tab Neil Hodgson}";

  // Start with only as much text as should be visible
  {
    CString creditsRTF;
    creditsRTF.Format("{\\rtf1\\ansi{\\fs%d%s",
      (theApp.GetDialogFontSize()/5)+2,text1);
    CMemFile creditsFile((BYTE*)(LPCSTR)creditsRTF,creditsRTF.GetLength());
    EDITSTREAM stream;
    stream.dwCookie = (DWORD_PTR)&creditsFile;
    stream.dwError = 0;
    stream.pfnCallback = FileReadCallback;
    m_credits.StreamIn(SF_RTF,stream);
  }

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
  CSize fontSize = theApp.MeasureFont(m_credits.GetFont());
  dlgRect.bottom += 2*(fontSize.cy/4);
  MoveWindow(dlgRect);

  // Show all the credits text
  {
    CString creditsRTF;
    creditsRTF.Format("{\\rtf1\\ansi{\\fs%d%s%s",
      (theApp.GetDialogFontSize()/5)+2,text1,text2);
    CMemFile creditsFile((BYTE*)(LPCSTR)creditsRTF,creditsRTF.GetLength());
    EDITSTREAM stream;
    stream.dwCookie = (DWORD_PTR)&creditsFile;
    stream.dwError = 0;
    stream.pfnCallback = FileReadCallback;
    m_credits.StreamIn(SF_RTF,stream);
  }

  m_initialSize = dlgRect.Size();
  return TRUE;
}

BOOL AboutDialog::OnEraseBkgnd(CDC* dc)
{
  CRect dlgRect, creditsRect;
  GetClientRect(dlgRect);
  m_credits.GetWindowRect(creditsRect);
  ScreenToClient(creditsRect);

  // Don't erase behind the credits edit control, as that will make it flicker
  COLORREF back = ::GetSysColor(COLOR_BTNFACE);
  dc->FillSolidRect(CRect(0,0,dlgRect.right,creditsRect.top),back);
  dc->FillSolidRect(CRect(0,creditsRect.bottom,dlgRect.right,dlgRect.bottom),back);
  dc->FillSolidRect(CRect(0,0,creditsRect.left,dlgRect.bottom),back);
  dc->FillSolidRect(CRect(creditsRect.right,0,dlgRect.right,dlgRect.bottom),back);

  CRect gripRect = dlgRect;
  gripRect.left = gripRect.right - ::GetSystemMetrics(SM_CXHSCROLL);
  gripRect.top = gripRect.bottom - ::GetSystemMetrics(SM_CYVSCROLL);

  // Draw a gripper to show that the dialog can be resized
  bool drawn = false;
  if (theOS.IsAppThemed())
  {
    // Open the status bar theme
    HTHEME theme = theOS.OpenThemeData(this,L"Status");
    if (theme)
    {
      theOS.DrawThemeBackground(theme,dc,SP_GRIPPER,0,gripRect);
      theOS.CloseThemeData(theme);
      drawn = true;
    }
  }
  if (!drawn)
    dc->DrawFrameControl(gripRect,DFC_SCROLL,DFCS_SCROLLSIZEGRIP);

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
  CSize fontSize = theApp.MeasureFont(m_credits.GetFont());
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

IMPLEMENT_DYNAMIC(AboutCreditsEdit, RichEdit)

BEGIN_MESSAGE_MAP(AboutCreditsEdit, RichEdit)
  ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void AboutCreditsEdit::OnSetFocus(CWnd* pOldWnd)
{
  RichEdit::OnSetFocus(pOldWnd);

  // By default the entire contents are selected,
  // so move the selection to the start of the text.
  SetSel(0,0);
}

BOOL AboutCreditsEdit::PreTranslateMessage(MSG* pMsg)
{
  if (RejectKey(pMsg))
    return TRUE;
  return RichEdit::PreTranslateMessage(pMsg);
}
