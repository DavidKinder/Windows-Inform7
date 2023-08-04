// The settings tab

#include "stdafx.h"
#include "TabSettings.h"
#include "Inform.h"
#include "Panel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabSettings, FormScrollArea)

BEGIN_MESSAGE_MAP(TabSettings, FormScrollArea)
  ON_WM_CTLCOLOR()
  ON_WM_ERASEBKGND()
  ON_WM_PRINTCLIENT()
  ON_CBN_SELCHANGE(IDC_VERSION_COMBO, OnChangedVersion)
END_MESSAGE_MAP()

CString TabSettings::m_labelTexts[5] =
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
  "",
  "This restricts to a minimal version of the Inform programming language which is "
    "uninteractive, and does not have a command parser. This is available only in version 10.1.0 "
    "or greater of Inform."
};

TabSettings::TabSettings() : FormScrollArea(IDD_SETTINGS), m_settings(NULL), m_notify(NULL)
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
  m_settings->m_basic = (m_basic.GetCheck() == BST_CHECKED);

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
  m_basic.SetCheck(m_settings->m_basic ? BST_CHECKED : BST_UNCHECKED);

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
  AddVersions();
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
  m_boxStory.SubclassDlgItem(IDC_STORY_BOX,this);
  m_boxRandom.SubclassDlgItem(IDC_RANDOM_BOX,this);
  m_boxVersion.SubclassDlgItem(IDC_VERSION_BOX,this);
  m_boxBasic.SubclassDlgItem(IDC_BASIC_BOX,this);
  m_predictable.SubclassDlgItem(IDC_PREDICTABLE,this,IDR_DARK_CHECK);
  m_basic.SubclassDlgItem(IDC_BASIC,this,IDR_DARK_CHECK);
  m_blorb.SubclassDlgItem(IDC_BLORB,this,IDR_DARK_CHECK);
  m_outputZ8.SubclassDlgItem(IDC_OUTPUT_Z8,this,IDR_DARK_RADIO);
  m_outputGlulx.SubclassDlgItem(IDC_OUTPUT_GLULX,this,IDR_DARK_RADIO);
  m_version.SubclassDlgItem(IDC_VERSION_COMBO,this);

  // Set window text for accessibility
  SetWindowText(GetName());
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
  return FormScrollArea::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
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

void TabSettings::BeforeUpdateDPI(std::map<CWnd*,double>& layout)
{
}

void TabSettings::UpdateDPI(const std::map<CWnd*,double>& layout)
{
  m_labelFont.DeleteObject();
  Layout();
}

void TabSettings::SetDarkMode(DarkMode* dark)
{
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
  FormScrollArea::OnInitialUpdate();
  AddVersions();
  Layout();
}

BOOL TabSettings::OnCommand(WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam))
  {
  case IDC_PREDICTABLE:
  case IDC_BLORB:
  case IDC_BASIC:
  case IDC_OUTPUT_Z8:
  case IDC_OUTPUT_GLULX:
  case IDC_VERSION_COMBO:
    UpdateSettings();
    break;
  }
  return FormScrollArea::OnCommand(wParam,lParam);
}

HBRUSH TabSettings::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
  HBRUSH brush = FormScrollArea::OnCtlColor(pDC,pWnd,nCtlColor);
  DarkMode* dark = DarkMode::GetActive(this);

  switch (nCtlColor)
  {
  case CTLCOLOR_STATIC:
    if (dark)
    {
      brush = *(dark->GetBrush(DarkMode::Back));
      pDC->SetBkColor(dark->GetColour(DarkMode::Back));
      pDC->SetTextColor(dark->GetColour(DarkMode::Fore));
    }
    else
    {
      brush = (HBRUSH)::GetStockObject(NULL_BRUSH);
      pDC->SetBkColor(theApp.GetColour(InformApp::ColourBack));
    }
    break;
  case CTLCOLOR_LISTBOX:
    if (dark)
    {
      brush = *(dark->GetBrush(DarkMode::Darkest));
      pDC->SetTextColor(dark->GetColour(DarkMode::Fore));
    }
    break;
  }

  return brush;
}

BOOL TabSettings::OnEraseBkgnd(CDC* dc)
{
  CRect r;
  GetClientRect(r);

  DarkMode* dark = DarkMode::GetActive(this);
  dc->FillSolidRect(r,dark ?
    dark->GetColour(DarkMode::Back) : theApp.GetColour(InformApp::ColourBack));
  return TRUE;
}

void TabSettings::OnDraw(CDC* pDC)
{
  FormScrollArea::OnDraw(pDC);
  DrawLabels(*pDC,CPoint(0,0));
}

LRESULT TabSettings::OnPrintClient(CDC* pDC, UINT nFlags)
{
  if (nFlags & PRF_ERASEBKGND)
    OnEraseBkgnd(pDC);
  if (nFlags & PRF_CLIENT)
    DrawLabels(*pDC,GetDeviceScrollPosition());
  return 0;
}

void TabSettings::AddVersions(void)
{
  if (m_version.GetCount() == 0)
  {
    const auto& versions = theApp.GetCompilerVersions();
    for (auto it = versions.begin(); it != versions.end(); ++it)
     m_version.AddString(it->label);
  }
}

void TabSettings::Layout(void)
{
  theApp.CreatePointFont(this,&m_labelFont,
    9*theApp.GetFontSize(InformApp::FontSystem),theApp.GetFontName(InformApp::FontSystem));

  CRect story = getRect(this,IDC_STORY_BOX);
  CRect outputZ8 = getRect(this,IDC_OUTPUT_Z8);
  CRect outputG = getRect(this,IDC_OUTPUT_GLULX);
  CRect check = getRect(this,IDC_BLORB);
  for (int i = 0; i < 5; i++)
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
  int vh = comboRect.Height()+m_labelRects[3].Height()+(fh*9/2);
  moveWnd(this,IDC_VERSION_BOX,story.left,sb+rh+fh,story.Width(),vh);

  sizeText(dc,m_labelTexts[4],m_labelRects[4],sb+rh+vh+(fh*7/2));
  moveWnd(this,IDC_BASIC,outputZ8.left,m_labelRects[4].bottom+fh);
  int bh = m_labelRects[4].bottom+check.Height()-sb-rh-vh+(fh/2);
  moveWnd(this,IDC_BASIC_BOX,story.left,sb+rh+vh+(fh*3/2),story.Width(),bh);

  dc->SelectObject(oldFont);
  ReleaseDC(dc);
}
