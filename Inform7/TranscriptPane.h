#pragma once

#include "Skein.h"
#include "RichEdit.h"

#include <vector>

class TranscriptPane
{
public:
  TranscriptPane();
  ~TranscriptPane();

  void SetFontsBitmaps(CWnd* wnd, int nodeHeight);

  void SetOrigin(int x, int y);
  CPoint GetOrigin(void);

  void Layout(CDC& dc);
  void Draw(CDC& dc, CPoint origin, Skein::Node* rootNode, int skeinIndex);

  void SetEndNode(Skein::Node* node, CWnd* wnd);
  bool ContainsNode(Skein::Node* node);
  Skein::Node* ContainsChildNode(Skein::Node* node);
  void GetNodes(std::vector<Skein::Node*>& nodes);
  bool AreNodesValid(Skein* skein);

  bool IsActive(void);
  int GetWidth(void);
  int GetHeight(void);
  int GetRowHeight(int row);

  void SaveTranscript(const char* path);

private:
  void ClearNodes(void);

  struct NodeLayout
  {
    NodeLayout(Skein::Node* node);

    Skein::Node* node;
    RichDrawText* draw;
    int height;
  };
  std::vector<NodeLayout> m_nodes;

  CPoint m_origin;
  CSize m_fontSize;
  int m_nodeHeight;
};
