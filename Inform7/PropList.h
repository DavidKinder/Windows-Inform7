#pragma once

class PropList
{
public:
  bool Load(LPCSTR path);

  CStringW GetString(LPCWSTR dict1, LPCWSTR dict2);
  int GetNumber(LPCWSTR dict1, LPCWSTR dict2);
  bool GetBoolean(LPCWSTR dict1, LPCWSTR dict2, bool defaultValue);
  bool Exists(LPCWSTR dict1, LPCWSTR dict2);

  typedef CMap<CStringW,LPCWSTR,CStringW,LPCWSTR> KeyValueMap;
  void GetKeyValues(LPCWSTR dict, KeyValueMap& values);

private:
  IXMLDOMNode* GetNode(LPCWSTR dict1, LPCWSTR dict2);

  CComPtr<IXMLDOMDocument> m_doc;
};
