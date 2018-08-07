#pragma once

#define STYLE_TEXT         0
#define STYLE_QUOTE        1
#define STYLE_SUBSTITUTION 2
#define STYLE_INFORM6      3
#define STYLE_HEADING      4
#define STYLE_COMMENT      5
#define NEST_COMMENTS     16

class SourceEdit;

class SourceLexer
{
public:
  enum LexAction
  {
    LexApply,
    LexHeadings
  };

  SourceLexer(SourceEdit* edit, LexAction action);
  void Process(int startPos, int endPos, bool includeExt);

  enum HeadingLevel
  {
    Root = 0,
    Title,
    ExtensionPart,
    Volume,
    Book,
    Part,
    Chapter,
    Section,
    Example,
    No_Heading = -1
  };

  struct Heading
  {
    HeadingLevel level;
    CStringW name;
    int line;

    Heading();
    Heading(HeadingLevel lv, LPCSTR n, int ln);
  };

  static HeadingLevel IsHeading(const char* line, bool includeExt);
  const CArray<Heading>& GetHeadings(void) const;

  static const int StyleMask;

private:
  void ApplyStyle(int& start, int pos, int& style, int newStyle, int mask);
  void AddHeading(HeadingLevel level, LPCSTR name, int pos);
  bool IsQuote(unsigned char c, int& pos, bool advance);
  LONG_PTR CallEdit(UINT msg, DWORD wp = 0, LONG_PTR lp = 0);

  SourceEdit* m_edit;
  LexAction m_action;
  CArray<Heading> m_headings;
};
