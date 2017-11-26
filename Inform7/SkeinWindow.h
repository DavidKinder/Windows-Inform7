#pragma once

#include "Dib.h"
#include "Skein.h"
#include "SkeinEdit.h"
#include "Messages.h"

#include <map>
#include <vector>

class SkeinWindow : public CScrollView, public Skein::Listener
{
  DECLARE_DYNCREATE(SkeinWindow)

public:
  SkeinWindow();

  void SetSkein(Skein* skein);
  void Layout(bool force);
  void PrefsChanged(void);

  void SkeinChanged(Skein::Change change);
  void SkeinEdited(bool edited);
  void SkeinShowNode(Skein::Node* node, Skein::Show why);
  void SkeinNodesShown(
    bool& unselected, bool& selected, bool& active, bool& differs, int& count);
  void AnimatePrepare();
  void Animate(int pct);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnTimer(UINT nIDEvent);

  afx_msg LRESULT OnRenameNode(WPARAM, LPARAM);
  afx_msg LRESULT OnLabelNode(WPARAM, LPARAM);

  virtual void OnDraw(CDC* pDC);
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void PostNcDestroy();

private:
  CSize GetLayoutSize(bool force);
  int GetNodeYPos(int nodes, int ends);
  void SetFontsBitmaps(void);

  void DrawNodeTree(int phase, Skein::Node* node, Skein::Node* threadEnd, CDC& dc,
    CDibSection& bitmap, const CRect& client, const CPoint& parentCentre,
    const CPoint& siblingCentre, int depth, int spacing, bool gameRunning);

  void DrawNode(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& centre, bool selected, bool gameRunning);
  void DrawNodeBack(Skein::Node* node, CDibSection& bitmap, const CPoint& centre,
    int width, CDibSection* back);

  void DrawNodeLine(CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& from, const CPoint& to, COLORREF fore, bool bold);
  void DrawLinePixel(CDC& dc, CDibSection& bitmap, int x, int y, double i, COLORREF fore);
  COLORREF LinePixelColour(double i, COLORREF fore, COLORREF back);

  CDibSection* GetImage(const char* name);
  CRect GetMenuButtonRect(const CRect& nodeRect, CDibSection* menu = NULL);
  CRect GetBadgeRect(const CRect& nodeRect);
  void RemoveExcessSeparators(CMenu* menu);

  Skein::Node* NodeAtPoint(const CPoint& point);
  bool ShowLabel(Skein::Node* node);
  void StartEdit(Skein::Node* node, bool label);

  enum NodeBitmap
  {
    BackActive = 0,
    BackUnselected,
    BackSelected,
    MenuActive,
    MenuUnselected,
    MenuSelected,
    MenuOver,
    DiffersBadge,
    Number_Bitmaps,
    No_Bitmap = -1
  };

  NodeBitmap GetNodeBack(Skein::Node* node, bool selected, bool gameRunning);
  void SkeinNodesShown(Skein::Node* node, Skein::Node* threadEnd, bool gameRunning,
    bool& unselected, bool& selected, bool& active, bool& differs, int& count);

  Skein* m_skein;
  std::map<Skein::Node*,CRect> m_nodes;
  CDibSection* m_bitmaps[Number_Bitmaps];

  CSize m_fontSize;
  CFont m_boldFont;
  SkeinEdit m_edit;
  int m_pctAnim;

  Skein::Node* m_mouseOverNode;
  bool m_mouseOverMenu;

  bool m_lastClick;
  DWORD m_lastClickTime;
  CPoint m_lastPoint;

  class CommandStartEdit : public Command
  {
  public:
    CommandStartEdit(SkeinWindow* wnd, Skein::Node* node, bool label);
    void Run(void);

  private:
    SkeinWindow* m_wnd;
    Skein::Node* m_node;
    bool m_label;
  };
};
