#pragma once

#include "Dib.h"
#include "Messages.h"
#include "SourceLexer.h"

#include <memory>

class ContentsPane : public CScrollView
{
  DECLARE_DYNAMIC(ContentsPane)

public:
  typedef SourceLexer::Heading Item;
  typedef SourceLexer::HeadingLevel Level;

  ContentsPane();

  void SetHeadings(const CArray<Item>& headings, const SourceHeading& selected);
  void UpdateSmallest(Level smallest);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

  afx_msg LRESULT OnPrint(WPARAM, LPARAM);

  virtual void OnDraw(CDC* pDC);
  virtual void PostNcDestroy();

private:
  struct Node
  {
    Node* parent;
    CArray<Node*> children;
    const Item* item;
    UINT id;
    int indent;
    CRect selectRect1, selectRect2;

    enum NodeSelection
    {
      NodeNotSelected,
      NodeSelected,
      NodeBelowSelection
    };
    NodeSelection selected;

    Node(const Item* item_, Node* parent_, UINT id_, int indent_);
    ~Node();
    Level GetLevel(void);
    int GetCount(Level smallest, bool title);
    void ZeroRects(void);
    Node* GetNextChild(Node* child);
    void SelectNode(NodeSelection sel);
  };

  void BuildTree(Node* parent);
  void Draw(CDC* dc, int origin_y);
  void DrawNode(CDC* dc, Node* node, bool& title, int& h, int origin_y);
  void DrawSelectBack(CDC* dc, Node* node, const CRect& textRect);
  Node* NodeAtPoint(const CPoint& point, Node* node = NULL);
  CWnd* GetParentTab(CWnd* wnd = NULL);
  void SetScrollSize(void);
  CDibSection* GetCircle(COLORREF back);
  void SetSelectedNode(const SourceHeading& selected);

  CArray<Item> m_items;
  std::auto_ptr<Node> m_tree;
  Level m_smallest;

  CFont m_font, m_boldFont;
};

class ContentsWindow : public CWnd
{
  DECLARE_DYNAMIC(ContentsWindow)

public:
  ContentsWindow();

  void SetHeadings(const CArray<SourceLexer::Heading>& headings, const SourceHeading& selected);
  void SetFocus(void);

  void SlideIn(CWnd* source);
  void SlideOut(CWnd* source);

  void LoadSettings(CRegKey& key);
  void SaveSettings(CRegKey& key);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

  afx_msg LRESULT OnPrint(WPARAM, LPARAM);

private:
  enum Animation
  {
    NoAnim,
    AnimSlideIn,
    AnimSlideOut
  };

  void Draw(CDC* dc);
  void UpdateSmallest(void);
  void AnimateSlide(Animation anim, CWnd* source);
  void PrintWindow(CDibSection& dib, CWnd* wnd, COLORREF back);

  ContentsPane m_contents;
  CSliderCtrl m_depth;

  Animation m_animation;
  int m_animateStep;
  CDibSection m_contentsImage, m_sourceImage;
};
