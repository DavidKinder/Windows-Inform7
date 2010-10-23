#pragma once

#include "Dib.h"
#include "Skein.h"
#include "SkeinEdit.h"

#include <map>
#include <vector>

class SkeinWindow : public CScrollView, public Skein::Listener
{
  DECLARE_DYNAMIC(SkeinWindow)

public:
  SkeinWindow();

  void SetSkein(Skein* skein);
  void Layout(void);

  void SkeinChanged(Skein::Change change);
  void SkeinEdited(bool edited);
  void SkeinShowNode(Skein::Node* node, Skein::Show why);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);

  afx_msg LRESULT OnRenameNode(WPARAM, LPARAM);
  afx_msg LRESULT OnLabelNode(WPARAM, LPARAM);

  virtual void OnDraw(CDC* pDC);
  virtual void PostNcDestroy();

private:
  CSize GetLayoutSize(void);

  void DrawNodeTree(Skein::Node* node, Skein::Node* transcriptEnd, CDC& dc,
    CDibSection& bitmap, const CRect& client, const CPoint& parentCentre,
    const CPoint& siblingCentre, int spacing);

  void DrawNode(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& centre);
  void DrawNodeBack(Skein::Node* node, CDC& dc, CDibSection& bitmap, const CPoint& centre,
    int width, CDibSection* back);

  void DrawNodeLine(CDC& dc, CDibSection& bitmap, const CRect& client,
    const CPoint& from, const CPoint& to, COLORREF fore, bool bold, bool label, bool dashed);
  void DrawLinePixel(CDC& dc, CDibSection& bitmap, int x, int y, double i, COLORREF fore);
  COLORREF LinePixelColour(double i, COLORREF fore, COLORREF back);

  CDibSection* GetImage(const char* name, bool dark, bool blend);

  enum NodeBitmaps
  {
    BackPlayed = 0,
    BackPlayedDark,
    BackUnplayed,
    BackUnplayedDark,
    BackAnnotate,
    DiffersBadge,
    Number_Bitmaps,
    No_Bitmap = -1
  };

  Skein::Node* NodeAtPoint(const CPoint& point);
  bool ShowLabel(Skein::Node* node);
  void StartEdit(Skein::Node* node, bool label);

  bool CanRemove(Skein::Node* node);

  Skein* m_skein;
  std::map<Skein::Node*,CRect> m_nodes;
  CDibSection* m_bitmaps[Number_Bitmaps];

  CFont* m_font;
  CSize m_fontSize;
  CFont m_labelFont;
  SkeinEdit m_edit;
};
