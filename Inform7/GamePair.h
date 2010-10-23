#pragma once

#include "GameBase.h"

class GamePair : public CObject, public GameBase
{
  DECLARE_DYNAMIC(GamePair)

public:
  GamePair(MainWindow* main, int child1, int child2, int method, int size);

  bool IsKindOf(const CRuntimeClass* rclass) const;
	BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
  HWND GetSafeHwnd(void) const;

  void Layout(const CRect& r);
  void GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r);
  void AddText(const CStringW& text, bool fromSkein);
  void ClearText(bool styles, bool reverse);
  void SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size);
  void SetColours(COLORREF fore, COLORREF back);
  void SetCursor(int x, int y);
  void MoveToEnd(void);
  void Draw(CDibSection* image, int val1, int val2, int width, int height);
  COLORREF GetAlphaColour(void);
  void SetLink(int link);
  void SetParagraph(Justify justify);
  void SetBackColour(COLORREF colour);

  int GetChild1(void);
  int GetChild2(void);
  void ReplaceChild(int oldChild, int newChild);
  void RemoveChildren(void);
  void RemoveKey(int key);
  void SetArrangement(int method, int size, int key, bool swap);

private:
  MainWindow* m_main;
  int m_child1;
  int m_child2;
  int m_method;
  int m_size;
  int m_key;
};
