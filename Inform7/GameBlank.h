#pragma once

#include "GameBase.h"

class GameBlank : public CWnd, public GameBase
{
  DECLARE_DYNAMIC(GameBlank)

public:
  GameBlank(MainWindow* main);

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

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();

private:
  MainWindow* m_main;
};
