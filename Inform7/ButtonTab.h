#pragma once

#include <string>
#include <vector>

class CDibSection;

class ButtonTab : public CWnd
{
  DECLARE_DYNAMIC(ButtonTab)

protected:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);

public:
  ButtonTab();
  int GetDefaultHeight(void);

  CFont* GetFont(void);

  int GetItemCount(void) const;
  CString GetItem(int item) const;
  CRect GetItemRect(int item);
  void InsertItem(int item, LPCSTR name);

  int GetCurSel(void) const;
  void SetCurSel(int item);

protected:
  void SetActiveTab(int tab);
  CDibSection* GetImage(const char* name, const CSize& size);

  std::vector<CString> m_items;
  int m_currentItem;

  int m_mouseOverItem;
  bool m_mouseTrack;

public:
  virtual HRESULT get_accChildCount(long* count);
  virtual HRESULT get_accChild(VARIANT child, IDispatch** disp);
  virtual HRESULT get_accName(VARIANT child, BSTR* name);
  virtual HRESULT get_accRole(VARIANT child, VARIANT* role);
  virtual HRESULT get_accState(VARIANT child, VARIANT* state);
  virtual HRESULT accDoDefaultAction(VARIANT child);
  virtual HRESULT accHitTest(long left, long top, VARIANT* child);
  virtual HRESULT accLocation(long* left, long* top, long* width, long* height, VARIANT child);
};
