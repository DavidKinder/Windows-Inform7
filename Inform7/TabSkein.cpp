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
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

TabSkein::TabSkein() : m_skein(NULL)
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

  // Create the window containing the actual skein
  m_window.Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,CRect(0,0,0,0),this,1);
}

void TabSkein::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
}

void TabSkein::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  m_window.SetFocus();

  state.tab = Panel::Tab_Skein;
}

BOOL TabSkein::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  if (m_window.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
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
  m_window.Layout(false);
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

void TabSkein::PrefsChanged(CRegKey& key)
{
  m_window.PrefsChanged();
}

void TabSkein::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_window.SetSkein(skein);
}

void TabSkein::ShowNode(Skein::Node* node, Skein::Show why)
{
  m_window.SkeinShowNode(node,why);
}

void TabSkein::SkeinChanged(void)
{
  m_skein->GetRoot()->SetLine(GetStoryName());
  m_window.Layout(false);
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

  if (m_window.GetSafeHwnd() == 0)
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

  // Resize the skein window
  m_window.MoveWindow(client,TRUE);
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
