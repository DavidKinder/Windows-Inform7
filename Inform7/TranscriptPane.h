#pragma once

#include "Skein.h"
#include "RichEdit.h"
#include "Dib.h"

#include <vector>

class TranscriptPane
{
public:
  TranscriptPane();
  ~TranscriptPane();

  void SetFontsBitmaps(CWnd* wnd, CDibSection** bitmaps);

  void SetOrigin(int x, int y);
  CPoint GetOrigin(void);

  void Layout(CDC& dc);
  void DrawArrows(CDC& dc, CPoint origin, int skeinIndex);
  void Draw(CDC& dc, CPoint origin, CDibSection& bitmap);
  bool MouseMove(CPoint point);
  bool LButtonUp(CPoint point, Skein* skein);

  void SetEndNode(Skein::Node* node, CWnd* wnd);
  bool ContainsNode(Skein::Node* node);
  Skein::Node* ContainsChildNode(Skein::Node* node);
  void GetNodes(std::vector<Skein::Node*>& nodes);
  void ValidateNodes(Skein* skein, CWnd* wnd);

  bool IsActive(void);
  void Shown(bool& anyTick, bool& anyCross);
  int GetWidth(void);
  int GetHeight(void);
  int GetRowHeight(int row);

  void SaveTranscript(const char* path);

private:
  void ClearNodes(void);
  CSize GetBorder(void);

  struct NodeLayout
  {
    NodeLayout(Skein::Node* node);

    void AddText(LPCWSTR text, bool bold, bool strike, COLORREF back);

    Skein::Node* node;
    RichDrawText* draw;
    int height;
    CRect buttonRect;
  };
  std::vector<NodeLayout> m_nodes;

  CPoint m_origin;
  CSize m_fontSize;
  int m_nodeHeight;
  CDibSection** m_bitmaps;

  int m_mouseOverButton;
};
