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
  bool EndsWith(const CString& test, LPCSTR end);
};
