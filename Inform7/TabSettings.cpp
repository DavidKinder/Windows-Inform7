// The settings tab

#include "stdafx.h"
#include "TabSettings.h"
#include "Inform.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabSettings, CFormView)

BEGIN_MESSAGE_MAP(TabSettings, CFormView)
  ON_CBN_SELCHANGE(IDC_VERSION_COMBO, OnChangedVersion)
END_MESSAGE_MAP()

CString TabSettings::m_labelTexts[4] =
{
  "Inform translates the source text into a story file which can be in either of two standard "
    "formats. You can change your mind about the format at any time, but some language features "
    "require Glulx to be used.",
  "When released, the story file is normally bound up into a Blorb file along with bibliographic "
    "data, cover art and any other resources it needs. If you need the raw story file, "
    "uncheck this option.",
  "If the story involves randomised outcomes or events, it may be harder to check with the "
    "Replay options or with TEST commands, because the same input may produce different results "
    "every time. This option makes testing more predictable. (It has no effect on the final "
    "Release version, which will still be randomised.)",
  ""
};

TabSettings::TabSettings() : CFormView(IDD_SETTINGS), m_settings(NULL), m_notify(NULL)
{
}

void TabSettings::OnChangedVersion()
{
  const auto& versions = theApp.GetCompilerVersions();

  int select = m_version.GetCurSel();
  if ((select >= 0) && (select < versions.size()))
    m_labelTexts[3] = versions.at(select).description;
  else
    m_labelTexts[3] = "";

  InvalidateRect(m_labelRects[3]);
}

void TabSettings::PostNcDestroy()
{
  // Do nothing
}

void TabSettings::SetSettings(ProjectSettings* settings)
{
  m_settings = settings;
  UpdateFromSettings();
}

void TabSettings::UpdateSettings(void)
{
  // Compiler settings
  m_settings->m_predictable = (m_predictable.GetCheck() == BST_CHECKED);

  // Output format
  m_settings->m_blorb = (m_blorb.GetCheck() == BST_CHECKED);
  if (m_outputZ8.GetCheck() == BST_CHECKED)
    m_settings->m_output = ProjectSettings::OutputZ8;
  else if (m_outputGlulx.GetCheck() == BST_CHECKED)
    m_settings->m_output = ProjectSettings::OutputGlulx;

  // Compiler version
  const auto& versions = theApp.GetCompilerVersions();
  int i = m_version.GetCurSel();
  if ((i >= 0) && (i < versions.size()))
    m_settings->m_compilerVersion = versions[i].id;
  else
    m_settings->m_compilerVersion = "****"; // Current version

  m_settings->m_changed = true;
  if (m_notify)
    m_notify->OnSettingsChange(this);
}

void TabSettings::UpdateFromSettings(void)
{
  // Compiler settings
  m_predictable.SetCheck(m_settings->m_predictable ? BST_CHECKED : BST_UNCHECKED);

  // Output format
  m_blorb.SetCheck(m_settings->m_blorb ? BST_CHECKED : BST_UNCHECKED);
  switch (m_settings->m_output)
  {
  case ProjectSettings::OutputZ8:
    m_outputZ8.SetCheck(BST_CHECKED);
    m_outputGlulx.SetCheck(BST_UNCHECKED);
    break;
  case ProjectSettings::OutputGlulx:
  default:
    m_outputZ8.SetCheck(BST_UNCHECKED);
    m_outputGlulx.SetCheck(BST_CHECKED);
    break;
  }

  // Compiler version
  int versionIdx = 0, i = 0;
  const auto& versions = theApp.GetCompilerVersions();
  for (auto it = versions.begin(); it != versions.end(); ++it, ++i)
  {
    if (it->id == m_settings->m_compilerVersion)
      versionIdx = i;
  }
  m_version.SetCurSel(versionIdx);
  OnChangedVersion();
}

void TabSettings::SetNotify(SettingsTabNotify* notify)
{
  m_notify = notify;
}

const char* TabSettings::GetName(void)
{
  return "Settings";
}

void TabSettings::CreateTab(CWnd* parent)
{
  // Create the dialog
  Create(WS_CHILD|WS_CLIPCHILDREN,CRect(0,0,0,0),parent,0);

  // Set up the dialog controls
  m_predictable.SubclassDlgItem(IDC_PREDICTABLE,this);
  m_blorb.SubclassDlgItem(IDC_BLORB,this);
  m_outputZ8.SubclassDlgItem(IDC_OUTPUT_Z8,this);
  m_outputGlulx.SubclassDlgItem(IDC_OUTPUT_GLULX,this);
  m_version.SubclassDlgItem(IDC_VERSION_COMBO,this);
}

void TabSettings::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabSettings::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  SetFocus();

  state.tab = Panel::No_Tab;
}

void TabSettings::MakeInactive(void)
{
  ShowWindow(SW_HIDE);
}

bool TabSettings::IsEnabled(void)
{
  return true;
}

CWnd* TabSettings::GetWindow(void)
{
  return this;
}

BOOL TabSettings::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  return CFormView::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabSettings::OpenProject(const char* path, bool primary)
{
  if (primary)
    m_settings->Load(path);
  UpdateFromSettings();
}

bool TabSettings::SaveProject(const char* path, bool primary)
{
  if (primary)
    return m_settings->Save(path);
  return true;
}

void TabSettings::CompileProject(CompileStage stage, int code)
{
}

bool TabSettings::IsProjectEdited(void)
{
  return false;
}

void TabSettings::Progress(const char* msg)
{
}

void TabSettings::LoadSettings(CRegKey& key, bool primary)
{
}

void TabSettings::SaveSettings(CRegKey& key, bool primary)
{
}

void TabSettings::PrefsChanged(CRegKey& key)
{
}

void TabSettings::UpdateDPI(void)
{
  m_labelFont.DeleteObject();
  Layout();
}

namespace
{
  CRect getRect(CWnd* parent, UINT id)
  {
    CRect r;
    parent->GetDlgItem(id)->GetWindowRect(r);
    parent->ScreenToClient(r);
    return r;
  }

  void sizeText(CDC* dc, const CString& text, CRect& r, int top)
  {
    r.bottom = r.top = top;
    dc->DrawText(text,r,DT_WORDBREAK|DT_CALCRECT);
  }

  void moveWnd(CWnd* parent, UINT id, int x, int y)
  {
    parent->GetDlgItem(id)->SetWindowPos(NULL,x,y,0,0,
      SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER);
  }

  void moveWnd(CWnd* parent, UINT id, int x, int y, int w, int h)
  {
    parent->GetDlgItem(id)->SetWindowPos(NULL,x,y,w,h,
      SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER);
  }
} // unnamed namespace

void TabSettings::OnInitialUpdate()
{
  CFormView::OnInitialUpdate();

  const auto& versions = theApp.GetCompilerVersions();
  for (auto it = versions.begin(); it != versions.end(); ++it)
    m_version.AddString(it->label);

  Layout();
}

BOOL TabSettings::OnCommand(WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam))
  {
  case IDC_PREDICTABLE:
  case IDC_BLORB:
  case IDC_OUTPUT_Z8:
  case IDC_OUTPUT_GLULX:
  case IDC_VERSION_COMBO:
    UpdateSettings();
    break;
  }
  return CFormView::OnCommand(wParam,lParam);
}

void TabSettings::OnDraw(CDC* pDC)
{
  CFormView::OnDraw(pDC);

  pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
  pDC->SetBkMode(TRANSPARENT);
  CFont* oldFont = pDC->SelectObject(&m_labelFont);
  for (int i = 0; i < 4; i++)
    pDC->DrawText(m_labelTexts[i],m_labelRects[i],DT_WORDBREAK);
  pDC->SelectObject(oldFont);
}

void TabSettings::Layout(void)
{
  theApp.CreatePointFont(this,&m_labelFont,
    9*theApp.GetFontSize(InformApp::FontSystem),theApp.GetFontName(InformApp::FontSystem));

  CRect story = getRect(this,IDC_STORY_BOX);
  CRect outputZ8 = getRect(this,IDC_OUTPUT_Z8);
  CRect outputG = getRect(this,IDC_OUTPUT_GLULX);
  CRect check = getRect(this,IDC_BLORB);
  for (int i = 0; i < 4; i++)
  {
    m_labelRects[i].left = outputZ8.left;
    m_labelRects[i].top = 0;
    m_labelRects[i].right = story.right-(2*(outputZ8.left-story.left));
    m_labelRects[i].bottom = 0;
  }

  LOGFONT lf;
  GetDlgItem(IDC_STORY_BOX)->GetFont()->GetLogFont(&lf);
  int fh = abs(lf.lfHeight);

  CDC* dc = GetDC();
  CFont* oldFont = dc->SelectObject(&m_labelFont);

  sizeText(dc,m_labelTexts[0],m_labelRects[0],story.top+(2*fh));
  moveWnd(this,IDC_OUTPUT_Z8,outputZ8.left,m_labelRects[0].bottom+fh);
  moveWnd(this,IDC_OUTPUT_GLULX,outputZ8.left,m_labelRects[0].bottom+fh+(outputG.top-outputZ8.top));
  sizeText(dc,m_labelTexts[1],m_labelRects[1],m_labelRects[0].bottom+(2*fh)+(2*(outputG.top-outputZ8.top)));
  moveWnd(this,IDC_BLORB,outputZ8.left,m_labelRects[1].bottom+fh);
  int sb = m_labelRects[1].bottom+(2*fh)+check.Height();
  moveWnd(this,IDC_STORY_BOX,story.left,story.top,story.Width(),sb-story.top);

  sizeText(dc,m_labelTexts[2],m_labelRects[2],sb+(fh*5/2));
  moveWnd(this,IDC_PREDICTABLE,outputZ8.left,m_labelRects[2].bottom+fh);
  int rh = m_labelRects[2].bottom+(fh*3/2)+check.Height()-sb;
  moveWnd(this,IDC_RANDOM_BOX,story.left,sb+(fh/2),story.Width(),rh);

  CRect labelRect = getRect(this,IDC_VERSION_LABEL);
  CRect comboRect = getRect(this,IDC_VERSION_COMBO);
  moveWnd(this,IDC_VERSION_LABEL,labelRect.left,sb+rh+(fh*7/2)+(labelRect.top-comboRect.top));
  moveWnd(this,IDC_VERSION_COMBO,comboRect.left,sb+rh+(fh*7/2));

  // Find the longest compiler description
  comboRect = getRect(this,IDC_VERSION_COMBO);
  const auto& versions = theApp.GetCompilerVersions();
  for (auto it = versions.begin(); it != versions.end(); ++it)
  {
    CRect descRect = m_labelRects[3];
    sizeText(dc,it->description,descRect,comboRect.bottom+fh);
    if (descRect.bottom > m_labelRects[3].bottom)
      m_labelRects[3] = descRect;
  }
  moveWnd(this,IDC_VERSION_BOX,
    story.left,sb+rh+fh,story.Width(),comboRect.Height()+m_labelRects[3].Height()+(fh*9/2));

  dc->SelectObject(oldFont);
  ReleaseDC(dc);
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL TabSettings::Create(DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  m_pCreateContext = NULL;

  CREATESTRUCT cs;
  memset(&cs,0,sizeof(CREATESTRUCT));
  if (dwRequestedStyle == 0)
    dwRequestedStyle = AFX_WS_DEFAULT_VIEW;
  cs.style = dwRequestedStyle;
  if (!PreCreateWindow(cs))
    return FALSE;

  if (!CreateDlg(m_lpszTemplateName,pParentWnd))
    return FALSE;

  ModifyStyle(WS_BORDER|WS_CAPTION,cs.style & (WS_BORDER|WS_CAPTION));
  ModifyStyleEx(WS_EX_CLIENTEDGE,cs.dwExStyle & WS_EX_CLIENTEDGE);

  SetDlgCtrlID(nID);

  // Don't use scrollbars
  SetScrollSizes(MM_TEXT,CSize(1,1));

  if (!ExecuteDlgInit(m_lpszTemplateName))
    return FALSE;

  SetWindowPos(NULL,rect.left,rect.top,
    rect.right - rect.left,rect.bottom - rect.top,SWP_NOZORDER|SWP_NOACTIVATE);

  if (dwRequestedStyle & WS_VISIBLE)
    ShowWindow(SW_NORMAL);
  return TRUE;
}

// Copied from MFC sources to enable a call to our CreateDlgIndirect()
BOOL TabSettings::CreateDlg(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
  LPCDLGTEMPLATE lpDialogTemplate = NULL;
  HGLOBAL hDialogTemplate = NULL;
  HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName,RT_DIALOG);
  HRSRC hResource = ::FindResource(hInst,lpszTemplateName,RT_DIALOG);
  hDialogTemplate = LoadResource(hInst,hResource);
  if (hDialogTemplate != NULL)
    lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

  BOOL bSuccess = CreateDlgIndirect(lpDialogTemplate,pParentWnd,hInst);
  UnlockResource(hDialogTemplate);
  FreeResource(hDialogTemplate);
  return bSuccess;
}

INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

// Copied from MFC sources to allow the dialog font to be set
BOOL TabSettings::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
  CWnd* pParentWnd, HINSTANCE hInst)
{
  m_pOccDialogInfo = NULL;

  if(!hInst)
    hInst = AfxGetResourceHandle();

  HGLOBAL hTemplate = NULL;
  HWND hWnd = NULL;

  TRY
  {
    CDialogTemplate dlgTemp(lpDialogTemplate);
    dlgTemp.SetFont(
      theApp.GetFontName(InformApp::FontSystem),theApp.GetFontSize(InformApp::FontSystem));
    hTemplate = dlgTemp.Detach();
    lpDialogTemplate = (DLGTEMPLATE*)GlobalLock(hTemplate);

    m_nModalResult = -1;
    m_nFlags |= WF_CONTINUEMODAL;

    AfxHookWindowCreate(this);
    hWnd = ::CreateDialogIndirect(hInst,lpDialogTemplate,
      pParentWnd->GetSafeHwnd(),AfxDlgProc);
  }
  CATCH_ALL(e)
  {
    e->Delete();
    m_nModalResult = -1;
  }
  END_CATCH_ALL

  if (!AfxUnhookWindowCreate())
    PostNcDestroy();

  if (hWnd != NULL && !(m_nFlags & WF_CONTINUEMODAL))
  {
    ::DestroyWindow(hWnd);
    hWnd = NULL;
  }

  if (hTemplate != NULL)
  {
    GlobalUnlock(hTemplate);
    GlobalFree(hTemplate);
  }

  if (hWnd == NULL)
    return FALSE;
  return TRUE;
}
