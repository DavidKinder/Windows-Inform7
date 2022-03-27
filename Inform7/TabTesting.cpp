// The skein tab

#include "stdafx.h"
#include "TabTesting.h"
#include "Inform.h"
#include "Panel.h"
#include "TextFormat.h"
#include "Dialogs.h"
#include "Messages.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabTesting, TabBase)

BEGIN_MESSAGE_MAP(TabTesting, TabBase)
  ON_WM_SIZE()
  ON_COMMAND(ID_SKEIN_LABEL, OnSkeinLabel)
  ON_COMMAND(ID_SKEIN_PLAY_ALL, OnSkeinPlay)
  ON_COMMAND(ID_SKEIN_SAVE_TRANSCRIPT, OnSaveTranscript)
  ON_COMMAND(ID_SKEIN_TOGGLE_HELP, OnToggleHelp)
  ON_MESSAGE(WM_FINDREPLACECMD, OnFindReplaceCmd)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
  ON_MESSAGE(WM_UPDATEHELP, OnUpdateHelp)
END_MESSAGE_MAP()

TabTesting::TabTesting() : m_splitter(false),
  m_skeinWindow(NULL), m_helpWindow(NULL), m_skein(NULL), m_label(ArrowButton::DownLow)
{
}

TabTesting::~TabTesting()
{
  delete m_helpWindow;
}

const char* TabTesting::GetName(void)
{
  return "Testing";
}

void TabTesting::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the command buttons
  CFont* font = theApp.GetFont(this,InformApp::FontSystem);
  m_label.Create("Labels",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_LABEL);
  m_label.SetFont(font);
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
  m_helpWindow->SetLinkConsumer(this);
  m_helpWindow->Navigate(TextFormat::AnsiToUTF8(
    theApp.GetAppDir()+"\\Documentation\\windows\\TestingTemplate.html"),false);

  // Set window text for accessibility
  m_splitter.SetWindowText("Testing page");
  m_skeinWindow->SetWindowText("Skein");
  m_helpWindow->SetWindowText("Skein help");
}

void TabTesting::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabTesting::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  m_skeinWindow->SetFocus();

  state.tab = Panel::Tab_Testing;
}

BOOL TabTesting::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_splitter.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  if (m_helpWindow->IsWindowVisible())
  {
    if (m_helpWindow->OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
      return TRUE;
  }
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabTesting::OpenProject(const char* path, bool primary)
{
  if (primary)
  {
    m_skein->Load(path);
    m_skein->GetRoot()->SetLine(GetStoryName());
  }
  m_skeinWindow->Layout(false);
}

bool TabTesting::SaveProject(const char* path, bool primary)
{
  if (primary)
  {
    m_skein->SetLine(m_skein->GetRoot(),GetStoryName());
    return m_skein->Save(path);
  }
  return true;
}

bool TabTesting::IsProjectEdited(void)
{
  return m_skein->IsEdited();
}

void TabTesting::LoadSettings(CRegKey& key, bool primary)
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

  m_splitter.SetRowFraction(0,0.01*heightPercent,16);
  ShowHideHelp(showHelp);
}

void TabTesting::SaveSettings(CRegKey& key, bool primary)
{
  DWORD heightPercent = 100;
  if (m_splitter.GetRowCount() == 2)
    heightPercent = (DWORD)(0.5+(100*m_splitter.GetRowFraction(0)));
  key.SetDWORDValue(primary ?
    "Left Skein Height" : "Right Skein Height",heightPercent);
}

void TabTesting::PrefsChanged(CRegKey& key)
{
  m_skeinWindow->PrefsChanged();
  m_helpWindow->Refresh();
}

void TabTesting::BeforeUpdateDPI(std::map<CWnd*,double>& layout)
{
  TabBase::BeforeUpdateDPI(layout);
  layout.insert(std::make_pair(&m_splitter,m_splitter.GetRowFraction(0)));
}

void TabTesting::UpdateDPI(const std::map<CWnd*,double>& layout)
{
  TabBase::UpdateDPI(layout);

  CFont* font = theApp.GetFont(this,InformApp::FontSystem);
  m_label.SetFont(font);
  m_play.SetFont(font);
  m_save.SetFont(font);
  m_help.SetFont(font);
  Resize();

  auto layoutIt = layout.find(&m_splitter);
  if (layoutIt != layout.end())
  {
    m_splitter.SetRowFraction(0,layoutIt->second,16);
    m_splitter.RecalcLayout();
  }

  m_skeinWindow->PrefsChanged();
}

void TabTesting::SourceLink(const char* url)
{
}

void TabTesting::LibraryLink(const char* url)
{
}

void TabTesting::SkeinLink(const char* url)
{
}

bool TabTesting::DocLink(const char* url)
{
  return false;
}

void TabTesting::LinkDone(void)
{
  PostMessage(WM_UPDATEHELP);
}

void TabTesting::LinkError(const char* url)
{
}

void TabTesting::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_skeinWindow->SetSkein(skein);
}

void TabTesting::ShowNode(Skein::Node* node, Skein::Show why)
{
  m_skeinWindow->SkeinShowNode(node,why);
}

void TabTesting::SkeinChanged(void)
{
  m_skein->GetRoot()->SetLine(GetStoryName());
  m_skeinWindow->Layout(false);
}

void TabTesting::Animate(int pct)
{
  m_skeinWindow->Animate(pct);
}

CString TabTesting::GetToolTip(UINT_PTR id)
{
  switch (id)
  {
  case ID_SKEIN_LABEL:
  case ID_SKEIN_PLAY_ALL:
  case ID_SKEIN_SAVE_TRANSCRIPT:
    {
      CString tt;
      tt.LoadString((UINT)id);
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

CStringW TabTesting::GetStoryName(void)
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

void TabTesting::OnSize(UINT nType, int cx, int cy)
{
  TabBase::OnSize(nType,cx,cy);
  Resize();
}

LRESULT TabTesting::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    if (m_skein->IsActive())
    {
      m_label.EnableWindow(m_skein->HasLabels() ? TRUE : FALSE);
      m_play.EnableWindow(GetParentFrame()->SendMessage(WM_CANPLAYALL) != 0);
      m_save.EnableWindow(GetParentFrame()->SendMessage(WM_TRANSCRIPTEND) != 0);
    }
    else
    {
      m_label.EnableWindow(FALSE);
      m_play.EnableWindow(FALSE);
      m_save.EnableWindow(FALSE);
    }
  }
  return TabBase::OnIdleUpdateCmdUI(wParam,lParam);
}

LRESULT TabTesting::OnUpdateHelp(WPARAM, LPARAM)
{
  bool active = m_skein->IsActive();
  bool showWelcome = false;
  bool anyPurple = false, anyGrey = false, anyBlue = false, anyBadge = false;
  int count = 0;
  if (active)
    m_skeinWindow->SkeinNodesShown(anyGrey,anyBlue,anyPurple,anyBadge,count);

  SetHelpVisible("welcome",active && showWelcome);
  SetHelpVisible("title",active);
  SetHelpVisible("purple",anyPurple);
  SetHelpVisible("grey",anyGrey || anyBlue);
  SetHelpVisible("blue",anyGrey || anyBlue);
  SetHelpVisible("report",false);
  SetHelpVisible("tick",false);
  SetHelpVisible("cross",false);
  SetHelpVisible("badge",anyBadge);
  SetHelpVisible("threads",count >= 2);
  SetHelpVisible("knots",count == 1);
  SetHelpVisible("moreknots",(count >= 2) && (count <= 10));
  SetHelpVisible("menu",count >= 5);
  SetHelpVisible("welcomead",active && !showWelcome);
  return 0;
}

LRESULT TabTesting::OnFindReplaceCmd(WPARAM wParam, LPARAM lParam)
{
  return m_helpWindow->OnFindReplaceCmd(wParam,lParam);
}

void TabTesting::SetHelpVisible(const char* node, bool visible)
{
  CString code;
  code.Format("%s('%s');",visible ? "showBlock" : "hideBlock",node);
  m_helpWindow->RunJavaScript(code);
}

void TabTesting::OnSkeinLabel()
{
  std::map<CStringW,Skein::Node*> labels;
  m_skein->GetLabels(labels);

  if (labels.empty() == false)
  {
    CMenu popup;
    popup.CreatePopupMenu();

    int i = 1;
    std::map<CStringW,Skein::Node*>::const_iterator it;
    for (it = labels.begin(); it != labels.end(); ++it, ++i)
    {
      CString labelA(it->first);
      popup.AppendMenu(MF_STRING,i,labelA);
    }

    CRect labelRect;
    m_label.GetWindowRect(labelRect);

    int cmd = popup.TrackPopupMenuEx(
      TPM_RIGHTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD,
      labelRect.right,labelRect.bottom,this,NULL);
    if (cmd != 0)
    {
      CString labelA;
      popup.GetMenuString(cmd,labelA,MF_BYCOMMAND);

      CStringW labelW(labelA);
      it = labels.find(labelW);
      if (it != labels.end())
        m_skeinWindow->SkeinShowNode(it->second,Skein::JustShow);
    }
  }
}

void TabTesting::OnSkeinPlay()
{
  GetParentFrame()->SendMessage(WM_COMMAND,ID_REPLAY_ALL);
}

void TabTesting::OnSaveTranscript()
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

void TabTesting::OnToggleHelp()
{
  ShowHideHelp(m_splitter.GetRowCount() != 2);
}

void TabTesting::ShowHideHelp(bool show)
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

void TabTesting::Resize(void)
{
  if (m_splitter.GetSafeHwnd() != 0)
  {
    CRect client;
    GetClientRect(client);

    // Call the base class to resize and get parameters
    CSize fontSize;
    int heading;
    SizeTab(client,fontSize,heading);

    // Resize the command buttons
    int pw = theApp.MeasureText(&m_play).cx+(fontSize.cx*3);
    int lw = theApp.MeasureText(&m_label).cx+(fontSize.cx*3)+16;
    int gapx = (fontSize.cx/4);
    int gapy = (fontSize.cx/4);
    int x = client.Width()-pw-gapx;
    m_play.MoveWindow(x,gapy,pw,heading-(2*gapy),TRUE);
    int sw = theApp.MeasureText(&m_save).cx+(fontSize.cx*3);
    x -= sw+gapx;
    m_save.MoveWindow(x,gapy,sw,heading-(2*gapy),TRUE);
    int hw = theApp.MeasureText(&m_help,"Show Help").cx+(fontSize.cx*3);
    x -= hw+gapx;
    m_help.MoveWindow(x,gapy,hw,heading-(2*gapy),TRUE);
    x -= lw+gapx;
    m_label.MoveWindow(x,gapy,lw,heading-(2*gapy),TRUE);

    // Resize the window
    m_splitter.MoveWindow(client,TRUE);
  }
}
