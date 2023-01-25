#pragma once

#include "SourceEdit.h"
#include "Inform.h"
#include "Messages.h"
#include "SourceSettings.h"

class DarkMode;
class DibSection;

class SourceWindow : public CWnd
{
  DECLARE_DYNAMIC(SourceWindow)

public:
  SourceWindow();

  enum WindowType
  {
    NoBorder,
    Border,
    SingleLine
  };

  void Create(CWnd* parent, ProjectType projectType, WindowType windowType);
  SourceEdit& GetEdit(void);
  void LoadSettings(SourceSettings& set);
  void PrefsChanged(void);
  void SetDarkMode(DarkMode* dark);

  const SourceHeading& GetHeading(void);
  void GetAllHeadings(CArray<SourceLexer::Heading>& headings);
  void ShowBetween(int startLine, int endLine, const SourceHeading* heading);
  void GetTears(bool& top, bool& bottom);

  void Highlight(int line, COLORREF colour, bool centre);
  void Highlight(CHARRANGE range, bool centre);
  int GetCurrentLine(void);

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnEditSetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result);
  afx_msg void OnEditGetScrollInfo(NMHDR* pNotifyStruct, LRESULT* result);

  afx_msg LRESULT OnPrint(WPARAM, LPARAM);

private:
  void Resize(void);
  void Draw(CDC& dc);
  CRect PaintEdge(CDC& dcPaint, int y, int w, CDibSection* image, DarkMode* dark, bool top);
  void GetImages(ProjectType projectType);
  CDibSection* CreateTornImage(const char* inputImage, const char* outputName);

  WindowType m_windowType;
  COLORREF m_back;
  SourceEdit m_edit;
  SourceHeading m_heading;

  bool m_tearTop, m_tearBottom;
  CRect m_arrowTop, m_arrowBottom;

  CDibSection* m_imageTop;
  CDibSection* m_imageBottom;
};
