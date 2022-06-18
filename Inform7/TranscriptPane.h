#pragma once

#include "Skein.h"

class TranscriptPane
{
public:
  TranscriptPane();
  void SetSkeinIndex(int idx);

  void SetOrigin(int x, int y);
  void Draw(CDC& dc, CPoint origin, Skein::Node* rootNode);

  Skein::Node* GetEnd(void);
  void SetEnd(Skein::Node* node);

  int GetWidth(void);

private:
  int m_skeinIndex;
  Skein::Node* m_endNode;
  CPoint m_origin;
};
