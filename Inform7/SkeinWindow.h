#pragma once

#include "DrawScrollArea.h"
#include "Messages.h"
#include "Skein.h"
#include "SkeinEdit.h"
#include "TranscriptPane.h"

#include "DarkMode.h"
#include "Dib.h"

#include <map>

class SkeinWindow : public DrawScrollArea, public Skein::Listener
{
  DECLARE_DYNCREATE(SkeinWindow)

public:
  SkeinWindow();
  virtual ~SkeinWindow();

  void SetSkein(Skein* skein, int idx);
  void Layout(Skein::LayoutMode mode);
  void PrefsChanged(void);

  void SkeinLayout(CDC& dc, Skein::LayoutMode mode);
  void SkeinChanged(Skein::Change change);
  void SkeinEdited(bool edited);
  void SkeinShowNode(Skein::Node* node, bool select);
  void SkeinNodesShown(
    bool& unselected, bool& selected, bool& active, bool& differs, int& count);
  void TranscriptShown(bool& transcript, bool& anyTick, bool& anyCross);
  void AnimatePrepare();
  void AnimatePrepareOnlyThis();
  void Animate(int pct);

  bool IsTranscriptActive(void);
  void SaveTranscript(const char* path);

  virtual CSize GetWheelScrollDistance(CSize distance, BOOL horz, BOOL vert);

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
    StarBadge,
    BlessButton,
    BlessButtonOver,
    CurseButton,
    CurseButtonOver,
    Number_Bitmaps,
    No_Bitmap = -1
  };

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnCancelMode();
  afx_msg void OnCaptureChanged(CWnd* pWnd);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnTimer(UINT_PTR nIDEvent);

  afx_msg LRESULT OnRenameNode(WPARAM, LPARAM);

  virtual void OnDraw(CDC* pDC);
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

private:
  CSize GetLayoutSize(Skein::LayoutMode mode);
  CSize GetLayoutSpacing(void);
  CSize GetLayoutBorder(void);

  void SetFontsBitmaps(void);

  void DrawNodeTree(int phase, Skein::Node* node, CDC& dc, CDibSection& bitmap,
    const CRect& client, const CPoint& origin, const CPoint& parent,
    DarkMode* dark, bool gameRunning);

  void DrawNode(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& centre, bool selected, bool gameRunning);
  void DrawNodeBack(Skein::Node* node, CDibSection& bitmap, const CPoint& centre,
    int width, CDibSection* back);

  void DrawNodeLine(CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& from, const CPoint& to, COLORREF fore, COLORREF back, bool bold);
  void DrawLinePixel(CDC& dc, CDibSection& bitmap, int x, int y, double i, COLORREF fore);
  COLORREF LinePixelColour(double i, COLORREF fore, COLORREF back);

  CDibSection* GetImage(const char* name);
  CRect GetMenuButtonRect(const CRect& nodeRect, CDibSection* menu = NULL);
  void RemoveExcessSeparators(CMenu* menu);

  Skein::Node* NodeAtPoint(const CPoint& point);
  bool NodeFullyVisible(Skein::Node* node);
  void StartEdit(Skein::Node* node);

  NodeBitmap GetNodeBack(Skein::Node* node, bool selected, bool gameRunning);
  void SkeinNodesShown(Skein::Node* node, bool gameRunning,
    bool& unselected, bool& selected, bool& active, bool& differs, int& count);
  void UpdateHelp(void);

  void RemoveWinningLabels(Skein::Node* node);

  void PerformDrop(SkeinWindow* fromWnd, Skein::Node* dragNode, Skein::Node* dropNode, bool move);

  Skein* m_skein;
  int m_skeinIndex;

  std::map<Skein::Node*,CRect> m_nodes;
  CDibSection* m_bitmaps[Number_Bitmaps];

  CSize m_fontSize;
  CFont m_rootFont;
  CFont m_testMeFont;
  SkeinEdit m_edit;
  int m_pctAnim;

  Skein::Node* m_mouseOverNode;
  bool m_mouseOverMenu;

  enum MouseMode
  {
    MouseNormal,
    MouseClicked,
    MouseDragMove,
    MouseDragCopy
  };

  MouseMode m_mouseMode;
  DWORD m_clickTime;
  CPoint m_clickPoint;
  Skein::Node* m_dragNode;

  HCURSOR m_arrowDragCopy;
  HCURSOR m_arrowDragMove;

  TranscriptPane m_transcript;
  bool m_showTranscriptAfterAnim;

  class CommandStartEdit : public Command
  {
  public:
    CommandStartEdit(SkeinWindow* wnd, Skein::Node* node);
    void Run(void);

  private:
    SkeinWindow* m_wnd;
    Skein::Node* m_node;
  };
};
