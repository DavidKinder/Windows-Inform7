#include "stdafx.h"
#include "RecentProjectList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RecentProjectList::RecentProjectList(int size) : CRecentFileList(0,"Recent File List","File%d",size)
{
}

void RecentProjectList::RemoveAll(void)
{
  for (int i = 0; i < m_nSize; i++)
    m_arrNames[i].Empty();
}

void RecentProjectList::RemoveInvalid(void)
{
  int i = 0;
  for (; i < m_nSize;)
  {
    if (!m_arrNames[i].IsEmpty())
    {
      if (::GetFileAttributes(m_arrNames[i]) == INVALID_FILE_ATTRIBUTES)
      {
        Remove(i);
        continue;
      }
    }
    i++;
  }

  for (i = m_nSize-1; i >= 0; i--)
  {
    if (!m_arrNames[i].IsEmpty())
      break;
  }
  for (; i >= 0; i--)
  {
    if (m_arrNames[i].IsEmpty())
      Remove(i);
  }
}

void RecentProjectList::AppendDisplayName(int i, CString& output)
{
  LPCSTR name = ::PathFindFileName(m_arrNames[i]);
  for (; *name != '\0'; ++name)
  {
    if (*name == '&')
      output.AppendChar('&');
    output.AppendChar(*name);
  }
}

void RecentProjectList::UpdateMenu(CCmdUI* pCmdUI)
{
  RemoveInvalid();

  CMenu* pMenu = pCmdUI->m_pSubMenu;
  if (m_strOriginal.IsEmpty() && (pMenu != NULL))
    pMenu->GetMenuString(pCmdUI->m_nID,m_strOriginal,MF_BYCOMMAND);

  if (m_arrNames[0].IsEmpty())
  {
    if (!m_strOriginal.IsEmpty())
      pCmdUI->SetText(m_strOriginal);
    pCmdUI->Enable(FALSE);

    if (pCmdUI->m_pSubMenu != NULL)
    {
      for (int i = 1; i < m_nSize; i++)
        pCmdUI->m_pSubMenu->DeleteMenu(pCmdUI->m_nID+i,MF_BYCOMMAND);
    }
    return;
  }

  if (pCmdUI->m_pSubMenu == NULL)
    return;

  for (int i = 0; i < m_nSize; i++)
    pCmdUI->m_pSubMenu->DeleteMenu(pCmdUI->m_nID+i,MF_BYCOMMAND);

  for (int i = 0; i < m_nSize; i++)
  {
    if (!m_arrNames[i].IsEmpty())
    {
      CString menuItem;
      menuItem.Format("&%d ",i+1);
      AppendDisplayName(i,menuItem);
      pCmdUI->m_pSubMenu->InsertMenu(i,MF_STRING|MF_BYPOSITION,pCmdUI->m_nID++,menuItem);
    }
  }
  pCmdUI->m_bEnableChanged = TRUE;
}
