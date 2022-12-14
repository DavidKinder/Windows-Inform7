#pragma once

class RecentProjectList : public CRecentFileList
{
public:
  RecentProjectList(int size);

  void RemoveAll(void);
  void RemoveInvalid(void);
  void AppendDisplayName(int i, CString& output);

  virtual void UpdateMenu(CCmdUI* pCmdUI);
};
