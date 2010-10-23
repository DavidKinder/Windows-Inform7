#pragma once

#include "GameBase.h"

class GameGrid : public CWnd, public GameBase
{
  DECLARE_DYNAMIC(GameGrid)

public:
  GameGrid(MainWindow* main);

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

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

private:
  void SelectFont(CDC& dc, int row, int column);
  void ResizeGrid(int rows, int columns);
  int LinkAtPoint(const CPoint& p);

  MainWindow* m_main;
  int m_rows, m_columns;
  int m_x, m_y;

  struct GridInfo
  {
    GridInfo();
    bool operator!=(const GridInfo& info) const;

    bool bold;
    bool italic;
    bool reverse;
    int link;

    COLORREF fore;
    COLORREF back;
  };

  class GridString
  {
  public:
    int GetLength(void) const;
    LPCWSTR GetString(void) const;
    const GridInfo& GetAttributes(int index) const;

    void SetLength(int length, wchar_t character);
    void SetChar(int index, wchar_t character);
    void SetAttributes(int index, const GridInfo& attributes);

  private:
    CStringW m_string;
    CArray<GridInfo> m_attributes;
  };

  CArray<GridString> m_grid;
  GridInfo m_current;

  CFont m_font, m_currentFont;
  int m_charWidth, m_charHeight;
  int m_topMargin;
};
