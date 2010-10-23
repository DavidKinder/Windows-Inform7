// The skein tab

#include "stdafx.h"
#include "TabSkein.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabSkein, TabBase)

BEGIN_MESSAGE_MAP(TabSkein, TabBase)
  ON_WM_SIZE()
  ON_COMMAND(ID_SKEIN_LABEL, OnSkeinLabel)
  ON_COMMAND(ID_SKEIN_TRIM, OnSkeinTrim)
  ON_COMMAND(ID_SKEIN_PLAY_ALL, OnSkeinPlay)
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

TabSkein::TabSkein() : m_skein(NULL), m_label(ArrowButton::DownLow)
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
  m_label.Create("Labels",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_LABEL);
  m_label.SetFont(&m_font);
  m_trim.Create("Trim",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_TRIM);
  m_trim.SetFont(&m_font);
  m_play.Create("Play all",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_SKEIN_PLAY_ALL);
  m_play.SetFont(&m_font);

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

    if (theApp.GetProfileInt("Skein","Warned Save Temp",0) == 0)
    {
      int max = 0;
      if (m_skein->NeedSaveWarn(max))
      {
        CString msg;
        msg.Format(
          "Inform 7 now only saves a limited number of unlocked skein nodes\n"
          "(currently %d), in order to keep the size of the skein under control.\n"
          "This project's skein contains more unlocked nodes than this.\n\n"
          "You should lock any skein threads you wish to keep before saving\n"
          "this project, otherwise these threads may be lost.",max);
        MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_OK);
        theApp.WriteProfileInt("Skein","Warned Save Temp",1);
      }
    }
  }
  m_window.Layout();
}

bool TabSkein::SaveProject(const char* path, bool primary)
{
  if (primary)
    return m_skein->Save(path);
  return true;
}

bool TabSkein::IsProjectEdited(void)
{
  return m_skein->IsEdited();
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

CString TabSkein::GetToolTip(UINT_PTR id)
{
  switch (id)
  {
  case ID_SKEIN_LABEL:
    return "Show a menu of the labelled knots: selecting a knot will go to it";
  case ID_SKEIN_TRIM:
    return "Remove all knots that have not been locked";
  case ID_SKEIN_PLAY_ALL:
    return "Play all threads that lead to a blessed knot";
  }
  return TabBase::GetToolTip(id);
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
  int heading, h;
  SizeTab(client,fontSize,heading,h);

  // Resize the command buttons
  int pw = theApp.MeasureText(&m_play).cx+(fontSize.cx*3);
  int tw = theApp.MeasureText(&m_trim).cx+(fontSize.cx*3);
  int lw = theApp.MeasureText(&m_label).cx+(fontSize.cx*6);
  int x = client.Width()-pw-(fontSize.cx/3);
  m_play.MoveWindow(x,h,pw,heading-(2*h),TRUE);
  x -= tw+(fontSize.cx/3);
  m_trim.MoveWindow(x,h,tw,heading-(2*h),TRUE);
  x -= lw+(fontSize.cx/3);
  m_label.MoveWindow(x,h,lw,heading-(2*h),TRUE);

  // Resize the skein window
  m_window.MoveWindow(client,TRUE);
}

LRESULT TabSkein::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    m_label.EnableWindow(m_skein->HasLabels() ? TRUE : FALSE);
    m_play.EnableWindow(GetParentFrame()->SendMessage(WM_CANPLAYALL) != 0);
  }
  return TabBase::OnIdleUpdateCmdUI(wParam,lParam);
}

void TabSkein::OnSkeinLabel()
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
        m_window.SkeinShowNode(it->second,Skein::JustShow);
    }
  }
}

void TabSkein::OnSkeinTrim()
{
  const char* msg =
    "Trim the skein?\n\n"
    "This will remove all knots from the skein and\n"
    "transcript that have not been locked.\n"
    "This operation cannot be undone.";
  if (MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_YESNO) == IDYES)
  {
    bool gameRunning = GetParentFrame()->SendMessage(WM_GAMERUNNING) != 0;
    m_skein->Trim(m_skein->GetRoot(),gameRunning);
  }
}

void TabSkein::OnSkeinPlay()
{
  GetParentFrame()->SendMessage(WM_COMMAND,ID_REPLAY_BLESSED);
}
