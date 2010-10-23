#pragma once

class UnicodeEdit : public CEdit
{
  DECLARE_DYNAMIC(UnicodeEdit)

public:
  UnicodeEdit();

  BOOL Create(DWORD style, CWnd* parent, UINT id);
  BOOL SubclassDlgItem(UINT id, CWnd* parent);
  BOOL SubclassWindow(HWND wnd);

  void SetWindowText(LPCWSTR string);
  void GetWindowText(CStringW& string) const;

protected:
  BOOL m_isUnicode;
};

void AFXAPI DDX_TextW(CDataExchange* dx, int idc, CStringW& value);
void AFXAPI DDX_Control(CDataExchange* dx, int idc, UnicodeEdit& control);
