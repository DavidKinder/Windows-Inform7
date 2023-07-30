#pragma once

namespace TextFormat
{
  CStringW UTF8ToUnicode(const CString& in);
  CString UnicodeToUTF8(const CStringW& in);
  CString AnsiToUTF8(const CString& in);
  CString ToXML_UTF8(const CStringW& in);

  CString UnicodeToLatin1(const CStringW& in);
  CStringW Latin1ToUnicode(const CString& in);

  CString FormatNumber(int value);

  bool StartsWith(const CString& test, LPCSTR start);
  bool StartsWith(const CStringW& test, LPCWSTR start);
  bool EndsWith(const CString& test, LPCSTR end);
  bool EndsWith(const CStringW& test, LPCWSTR end);

  // Remove HTML-like escapes (e.g. "%20") from the input string
  CString Unescape(const char* input);
};
