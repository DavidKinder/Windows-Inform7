#pragma once

class CDibSection;

class GameBase
{
public:
  virtual ~GameBase() {}

  class MainWindow
  {
  public:
    virtual bool GameKeyEvent(CWnd* wnd, WPARAM wParam, LPARAM lParam) = 0;
    virtual void GameMouseEvent(GameBase* wnd, int x, int y) = 0;
    virtual void GameLinkEvent(GameBase* wnd, int link) = 0;
    virtual bool GetLineFromHistory(CStringW& line, int history) = 0;

    virtual CWnd* GetWnd(void) = 0;
    virtual void Layout(int wndId, const CRect& r) = 0;
    virtual void GetNeededSize(int wndId, int size, int& w, int& h, const CRect& r) = 0;
    virtual void DeleteWindow(int wndId) = 0;
    virtual void DeferMoveWindow(HWND wnd, const CRect& r) = 0;
  };

  virtual bool IsKindOf(const CRuntimeClass* rclass) const = 0;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) = 0;
  virtual HWND GetSafeHwnd(void) const = 0;

  enum Justify
  {
    JustifyLeft,
    JustifyRight,
    JustifyCentre,
    JustifyFull
  };

  virtual void Layout(const CRect& r) = 0;
  virtual void GetNeededSize(int size, int& w, int& h, CSize fontSize, const CRect& r) = 0;
  virtual void AddText(const CStringW& text, bool fromSkein) = 0;
  virtual void ClearText(bool styles, bool reverse) = 0;
  virtual void SetStyle(bool bold, bool italic, bool reverse, bool fixed, int size) = 0;
  virtual void SetColours(COLORREF fore, COLORREF back) = 0;
  virtual void SetCursor(int x, int y) = 0;
  virtual void MoveToEnd(void) = 0;
  virtual void Draw(CDibSection* image, int val1, int val2, int width, int height) = 0;
  virtual COLORREF GetAlphaColour(void) = 0;
  virtual void SetLink(int link) = 0;
  virtual void SetParagraph(Justify justify) = 0;
  virtual void SetBackColour(COLORREF colour) = 0;
};
