// The skein tab

#include "stdafx.h"
#include "TabSkein.h"
#include "Inform.h"
#include "OSLayer.h"
#include "Panel.h"
#include "Dialogs.h"
#include "Messages.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabSkein, TabBase)

BEGIN_MESSAGE_MAP(TabSkein, TabBase)
  ON_WM_SIZE()
  ON_COMMAND(ID_SKEIN_PLAY_ALL, OnSkeinPlay)
  ON_COMMAND(ID_SKEIN_SAVE_TRANSCRIPT, OnSaveTranscript)
  ON_COMMAND(ID_SKEIN_TOGGLE_HELP, OnToggleHelp)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
  ON_MESSAGE(WM_UPDATEHELP, OnUpdateHelp)
END_MESSAGE_MAP()

TabSkein::TabSkein() : m_splitter(false), m_skeinWindow(NULL), m_helpWindow(NULL), m_skein(NULL)
{
}

const char* TabSkein::GetName(void)
{
  return "Skein";
}

void TabSkein::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the command buttons
  CFont* font = theApp.GetFont(InformApp::FontPanel);
  m_play.Create("Play All",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_PLAY_ALL);
  m_play.SetFont(font);
  m_save.Create("Save Transcript",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_SAVE_TRANSCRIPT);
  m_save.SetFont(font);
  m_help.Create("Hide Help",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_TOGGLE_HELP);
  m_help.SetFont(font);

  // Create the splitter and the windows in it
  m_splitter.CreateStatic(this,2,1,WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,AFX_IDW_PANE_FIRST+16);
  m_splitter.CreateView(0,0,RUNTIME_CLASS(SkeinWindow),CSize(0,0),NULL);
  m_splitter.CreateView(1,0,RUNTIME_CLASS(ReportHtml),CSize(0,0),NULL);
  m_skeinWindow = (SkeinWindow*)m_splitter.GetPane(0,0);
  m_helpWindow = (ReportHtml*)m_splitter.GetPane(1,0);
  m_helpWindow->Navigate(theApp.GetAppDir()+"\\Documentation\\windows\\TestingTemplate.html",false);
}

void TabSkein::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabSkein::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  m_skeinWindow->SetFocus();

  state.tab = Panel::Tab_Skein;
}

BOOL TabSkein::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_splitter.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabSkein::OpenProject(const char* path, bool primary)
{
  if (primary)
  {
    m_skein->Load(path);
    m_skein->GetRoot()->SetLine(GetStoryName());
  }
  m_skeinWindow->Layout(false);
}

bool TabSkein::SaveProject(const char* path, bool primary)
{
  if (primary)
  {
    m_skein->SetLine(m_skein->GetRoot(),GetStoryName());
    return m_skein->Save(path);
  }
  return true;
}

bool TabSkein::IsProjectEdited(void)
{
  return m_skein->IsEdited();
}

void TabSkein::LoadSettings(CRegKey& key, bool primary)
{
  DWORD heightPercent = 0;
  if (key.QueryDWORDValue(primary ?
    "Left Skein Height" : "Right Skein Height",heightPercent) != ERROR_SUCCESS)
  {
    heightPercent = 75;
  }

  bool showHelp = true;
  if (heightPercent >= 100)
  {
    showHelp = false;
    heightPercent = 75;
  }

  CRect allRect;
  m_splitter.GetClientRect(allRect);
  m_splitter.SetRowInfo(0,(int)((allRect.Height()*heightPercent)/100),16);
  ShowHideHelp(showHelp);
}

void TabSkein::SaveSettings(CRegKey& key, bool primary)
{
  DWORD heightPercent = 100;

  if (m_splitter.GetRowCount() == 2)
  {
    CRect allRect;
    m_splitter.GetClientRect(allRect);
    if (allRect.Height() > 0)
    {
      int skeinHeight, minHeight;
      m_splitter.GetRowInfo(0,skeinHeight,minHeight);
      heightPercent = (DWORD)(((skeinHeight*100.0)/allRect.Height())+0.5);
    }
  }

  key.SetDWORDValue(primary ?
    "Left Skein Height" : "Right Skein Height",heightPercent);
}

void TabSkein::PrefsChanged(CRegKey& key)
{
  m_skeinWindow->PrefsChanged();
}

void TabSkein::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_skeinWindow->SetSkein(skein);
}

void TabSkein::ShowNode(Skein::Node* node, Skein::Show why)
{
  m_skeinWindow->SkeinShowNode(node,why);
}

void TabSkein::SkeinChanged(void)
{
  m_skein->GetRoot()->SetLine(GetStoryName());
  m_skeinWindow->Layout(false);
}

CString TabSkein::GetToolTip(UINT_PTR id)
{
  switch (id)
  {
  case ID_SKEIN_PLAY_ALL:
  case ID_SKEIN_SAVE_TRANSCRIPT:
    {
      CString tt;
      tt.LoadString(id);
      return tt;
    }
    break;
  case ID_SKEIN_TOGGLE_HELP:
    return (m_splitter.GetRowCount() == 2) ?
      "Hide the skein help page" : "Show the skein help page";
    break;
  }
  return TabBase::GetToolTip(id);
}

CStringW TabSkein::GetStoryName(void)
{
  CString* name = (CString*)(GetParentFrame()->SendMessage(WM_STORYNAME));
  if (name != NULL)
  {
    CStringW result(*name);
    delete name;
    return result;
  }
  else
    return L"story";
}

void TabSkein::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);

  if (m_splitter.GetSafeHwnd() == 0)
    return;

  CRect client;
  GetClientRect(client);

  // Call the base class to resize and get parameters
  CSize fontSize;
  int heading;
  SizeTab(client,fontSize,heading);

  // Resize the command buttons
  int pw = theApp.MeasureText(&m_play).cx+(fontSize.cx*3);
  int gapx = (fontSize.cx/4);
  int gapy = (fontSize.cx/4);
  int x = client.Width()-pw-gapx;
  m_play.MoveWindow(x,gapy,pw,heading-(2*gapy),TRUE);
  int sw = theApp.MeasureText(&m_save).cx+(fontSize.cx*3);
  x -= sw+gapx;
  m_save.MoveWindow(x,gapy,sw,heading-(2*gapy),TRUE);
  int hw = theApp.MeasureText("Show Help",m_help.GetFont()).cx+(fontSize.cx*3);
  x -= hw+gapx;
  m_help.MoveWindow(x,gapy,hw,heading-(2*gapy),TRUE);

  // Resize the window
  m_splitter.MoveWindow(client,TRUE);
}

LRESULT TabSkein::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    if (m_skein->IsActive())
    {
      m_play.EnableWindow(GetParentFrame()->SendMessage(WM_CANPLAYALL) != 0);
      m_save.EnableWindow(GetParentFrame()->SendMessage(WM_TRANSCRIPTEND) != 0);
    }
    else
    {
      m_play.EnableWindow(FALSE);
      m_save.EnableWindow(FALSE);
    }
  }
  return TabBase::OnIdleUpdateCmdUI(wParam,lParam);
}

LRESULT TabSkein::OnUpdateHelp(WPARAM, LPARAM)
{
  bool active = m_skein->IsActive();
  bool showWelcome = false;
  bool anyPurple = false, anyGrey = false, anyBlue = false, anyBadge = false;
  int count = 0;
  if (active)
    m_skeinWindow->SkeinNodesShown(anyGrey,anyBlue,anyPurple,anyBadge,count);

  SetHelpVisible(L"welcome",active && showWelcome);
  SetHelpVisible(L"title",active);
  SetHelpVisible(L"purple",anyPurple);
  SetHelpVisible(L"grey",anyGrey || anyBlue);
  SetHelpVisible(L"blue",anyGrey || anyBlue);
  SetHelpVisible(L"report",false);
  SetHelpVisible(L"tick",false);
  SetHelpVisible(L"cross",false);
  SetHelpVisible(L"badge",anyBadge);
  SetHelpVisible(L"threads",count >= 2);
  SetHelpVisible(L"knots",count == 1);
  SetHelpVisible(L"moreknots",(count >= 2) && (count <= 10));
  SetHelpVisible(L"menu",count >= 5);
  SetHelpVisible(L"welcomead",active && !showWelcome);
  return 0;
}

void TabSkein::SetHelpVisible(LPCWSTR node, bool visible)
{
  CComVariant vnode(node);
  m_helpWindow->Invoke(visible ? L"showBlock" : L"hideBlock",&vnode);
}

void TabSkein::OnSkeinPlay()
{
  GetParentFrame()->SendMessage(WM_COMMAND,ID_REPLAY_BLESSED);
}

void TabSkein::OnSaveTranscript()
{
  SimpleFileDialog dialog(FALSE,"txt",NULL,OFN_HIDEREADONLY|OFN_ENABLESIZING,
    "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||",this);
  dialog.m_ofn.lpstrTitle = "Save Transcript";
  if (dialog.DoModal() == IDOK)
  {
    Skein::Node* threadEnd = (Skein::Node*)
      GetParentFrame()->SendMessage(WM_TRANSCRIPTEND);
    m_skein->SaveTranscript(threadEnd,dialog.GetPathName());
  }
}

void TabSkein::OnToggleHelp()
{
  ShowHideHelp(m_splitter.GetRowCount() != 2);
}

void TabSkein::ShowHideHelp(bool show)
{
  if (show)
  {
    m_splitter.SetRows(2);
    m_helpWindow->ShowWindow(SW_SHOW);
    m_help.SetWindowText("Hide Help");
  }
  else
  {
    m_splitter.SetRows(1);
    m_helpWindow->ShowWindow(SW_HIDE);
    m_help.SetWindowText("Show Help");
  }
  m_splitter.RecalcLayout();
}
