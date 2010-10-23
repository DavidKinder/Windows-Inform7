#pragma once

#include "Skein.h"
#include "RichEdit.h"
#include "TranscriptEdit.h"

#include <deque>
#include <vector>

class TranscriptWindow : public CView, public Skein::Listener
{
  DECLARE_DYNAMIC(TranscriptWindow)

public:
  TranscriptWindow();

  void SetSkein(Skein* skein);
  void Layout(void);
  void BlessAll(void);

  void SkeinChanged(Skein::Change change);
  void SkeinEdited(bool edited);
  void SkeinShowNode(Skein::Node* node, Skein::Show why);
  Skein::Node* GetEndNode(void);

  enum FindAction
  {
    TranscriptChanged,
    TranscriptDifferent,
    SkeinDifferent,
  };

  Skein::Node* FindRelevantNode(FindAction action, bool next, bool selected);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

  afx_msg LRESULT OnSetExpected(WPARAM, LPARAM);

  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void PostNcDestroy();
  virtual void OnDraw(CDC* pDC);

public:
  BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

private:
  void Scroll(UINT code, int pos, int lines);

  int GetHeight(void);
  COLORREF Brighter(COLORREF colour);

  CFont m_btnFont;
  bool m_mouseOver;

  // Skein node pointers must be validated whenever a
  // notification is receieved of the skein being changed
  Skein* m_skein;
  Skein::Node* m_skeinPlayed;
  Skein::Node* m_skeinSelected;
  Skein::Node* m_skeinEndThread;

  void GetSkeinNodes(void);
  bool ScrollToNode(Skein::Node* node);

  struct NodeLayout
  {
    Skein::Node* node;
    int height;
  };

  struct ScriptLayout
  {
    CSize clientSize;
    int columnWidth;

    CFont* font;
    CSize fontSize;
    CSize margin;
    int centreMargin;

    std::deque<NodeLayout> nodes;
  };

  ScriptLayout m_layout;

  enum ButtonAction
  {
    NoAction,
    ButtonBless,
    ButtonPlay,
    ButtonShow,
  };

  struct Button
  {
    Button()
    {
      node = NULL;
      action = NoAction;
    }

    Button(Skein::Node* n, ButtonAction a)
    {
      node = n;
      action = a;
    }

    bool operator==(const Button& b) const
    {
      if (node != b.node)
        return false;
      if (action != b.action)
        return false;
      return true;
    }

    Skein::Node* node;
    ButtonAction action;
  };

  std::vector<std::pair<CRect,Button> > m_buttons;
  Button m_buttonDown;

  struct Expected
  {
    Expected() : editRect(0,0,0,0)
    {
      node = NULL;
    }

    Expected(Skein::Node* n, LPRECT r)
    {
      node = n;
      editRect = r;
    }

    Skein::Node* node;
    CRect editRect;
  };

  std::vector<std::pair<CRect,Expected> > m_expecteds;
  TranscriptEdit m_edit;

  RichDrawText m_draw;

  CRect DrawButton(CDC& dc, CRect& rect, bool centre, const char* text, Button button, bool enable);
  void SizeText(CDC& dc, CRect& rect, const CStringW& text);
  void DrawText(CDC& dc, CRect& rect, const CStringW& text, const Diff::DiffResults& diffs);
  void DrawInsideRect(CDC& dc, const CRect& rect, const CSize& size, COLORREF colour);
};
