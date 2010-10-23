// The transcript tab

#include "stdafx.h"
#include "TabTranscript.h"
#include "Inform.h"
#include "Panel.h"
#include "Messages.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(TabTranscript, TabBase)

BEGIN_MESSAGE_MAP(TabTranscript, TabBase)
  ON_WM_SIZE()
  ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
  ON_COMMAND_RANGE(ID_REPLAY_DIFF_PREV, ID_REPLAY_DIFF_NEXT_SKEIN, OnActionButton)
  ON_COMMAND(ID_TRANSCRIPT_BLESS_ALL, OnBlessAll)
END_MESSAGE_MAP()

TabTranscript::TabTranscript() : m_skein(NULL),
  m_nextSkein(ArrowButton::DownCentre),
  m_nextDiff(ArrowButton::DownCentre),
  m_prevDiff(ArrowButton::UpCentre)
{
}

const char* TabTranscript::GetName(void)
{
  return "Transcript";
}

void TabTranscript::CreateTab(CWnd* parent)
{
  // Create the pane window
  Create(parent);

  // Create the command buttons
  m_nextSkein.Create("Next in skein",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_REPLAY_DIFF_NEXT_SKEIN);
  m_nextSkein.SetFont(&m_font);
  m_nextDiff.Create("Next diff",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_REPLAY_DIFF_NEXT);
  m_nextDiff.SetFont(&m_font);
  m_prevDiff.Create("Prev diff",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_REPLAY_DIFF_PREV);
  m_prevDiff.SetFont(&m_font);
  m_blessAll.Create("Bless all",WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,ID_TRANSCRIPT_BLESS_ALL);
  m_blessAll.SetFont(&m_font);

  // Create the window containing the actual transcript
  m_window.Create(NULL,NULL,WS_CHILD|WS_CLIPCHILDREN|WS_VISIBLE,CRect(0,0,0,0),this,1);
}

void TabTranscript::MoveTab(CRect& rect)
{
  MoveWindow(rect,TRUE);
  m_window.SetFocus();
}

void TabTranscript::MakeActive(TabState& state)
{
  ShowWindow(SW_SHOW);
  m_window.SetFocus();

  state.tab = Panel::Tab_Transcript;
}

BOOL TabTranscript::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
  // Only route on commands from the action buttons if they are from the buttons, not the menus
  if (nCode == CN_COMMAND)
  {
    if ((nID >= ID_REPLAY_DIFF_PREV) && (nID <= ID_REPLAY_DIFF_NEXT_SKEIN))
    {
      const MSG* msg = GetCurrentMessage();
      if (msg->lParam == 0)
        return FALSE;
    }
  }

  if (m_window.OnCmdMsg(nID,nCode,pExtra,pHandlerInfo))
    return TRUE;
  return CWnd::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}

void TabTranscript::OpenProject(const char* path, bool primary)
{
  m_window.Layout();
}

void TabTranscript::SetSkein(Skein* skein)
{
  m_skein = skein;
  m_window.SetSkein(skein);
}

void TabTranscript::ShowNode(Skein::Node* node, Skein::Show why)
{
  m_window.SkeinShowNode(node,why);
}

Skein::Node* TabTranscript::GetEndNode(void)
{
  return m_window.GetEndNode();
}

Skein::Node* TabTranscript::FindRelevantNode(TranscriptWindow::FindAction action, bool next)
{
  return m_window.FindRelevantNode(action,next,true);
}

CString TabTranscript::GetToolTip(UINT_PTR id)
{
  switch (id)
  {
  case ID_TRANSCRIPT_BLESS_ALL:
    return "Bless all the knots in the current thread in the transcript";
  case ID_REPLAY_DIFF_NEXT_SKEIN:
    return "Show the next difference in the whole skein";
  case ID_REPLAY_DIFF_NEXT:
    return "Show the next difference in the transcript";
  case ID_REPLAY_DIFF_PREV:
    return "Show the previous difference in the transcript";
  }
  return TabBase::GetToolTip(id);
}

void TabTranscript::OnSize(UINT nType, int cx, int cy)
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

  // Resize the command button
  int nsw = theApp.MeasureText(&m_nextSkein).cx+((fontSize.cx*11)/2);
  int ndw = theApp.MeasureText(&m_nextDiff).cx+((fontSize.cx*11)/2);
  int pdw = theApp.MeasureText(&m_prevDiff).cx+((fontSize.cx*11)/2);
  int bw = theApp.MeasureText(&m_blessAll).cx+(fontSize.cx*3);
  int x = client.Width()-bw-(fontSize.cx/3);
  m_blessAll.MoveWindow(x,h,bw,heading-(2*h),TRUE);
  x -= pdw+(fontSize.cx/3);
  m_prevDiff.MoveWindow(x,h,pdw,heading-(2*h),TRUE);
  x -= ndw+(fontSize.cx/3);
  m_nextDiff.MoveWindow(x,h,ndw,heading-(2*h),TRUE);
  x -= nsw+(fontSize.cx/3);
  m_nextSkein.MoveWindow(x,h,nsw,heading-(2*h),TRUE);

  // Resize the transcript window
  m_window.MoveWindow(client,TRUE);
}

LRESULT TabTranscript::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
  if (GetSafeHwnd() != 0)
  {
    UINT ids[] =
      { ID_REPLAY_DIFF_NEXT_SKEIN, ID_REPLAY_DIFF_NEXT, ID_REPLAY_DIFF_PREV };
    for (int i = 0; i < sizeof ids / sizeof ids[0]; i++)
    {
      CCmdUI state;
      state.m_pOther = GetDlgItem(ids[i]);
      state.m_nID = ids[i];
      state.DoUpdate(GetParentFrame(),FALSE);
    }

    m_blessAll.EnableWindow(m_skein->CanBless(m_skein->GetCurrent(),true) ? TRUE : FALSE);
  }
  return TabBase::OnIdleUpdateCmdUI(wParam,lParam);
}

void TabTranscript::OnActionButton(UINT nID)
{
  GetParentFrame()->PostMessage(WM_COMMAND,MAKEWPARAM(nID,0),0);
}

void TabTranscript::OnBlessAll()
{
  const char* msg =
    "Are you sure you want to bless all the items in the transcript?\n\n"
    "This will bless all the items currently in the transcript so that\n"
    "they appear as the expected text in the right-hand column.\n"
    "This operation cannot be undone.";
  if (MessageBox(msg,INFORM_TITLE,MB_ICONWARNING|MB_YESNO) == IDYES)
    m_window.BlessAll();
}
