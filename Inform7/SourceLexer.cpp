#include "stdafx.h"
#include "SourceLexer.h"
#include "SourceEdit.h"
#include "TextFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
const char* headings[] = { "volume", "book", "part", "chapter", "section" };
const char* beforeQuote = " \t\r\n.,:;?!(){}[]";
}

SourceLexer::SourceLexer(SourceEdit* edit, LexAction action)
{
  m_edit = edit;
  m_action = action;
}

void SourceLexer::Process(int startPos, int endPos)
{
  const int mask = 0x1F;

  // Find where to start applying styles
  int line = (int)CallEdit(SCI_LINEFROMPOSITION,startPos);
  startPos = (int)CallEdit(SCI_POSITIONFROMLINE,line);

  // Find where to stop applying styles
  int len = (int)CallEdit(SCI_GETLENGTH);
  if (endPos < 0)
    endPos = len;

  // Get the initial style to start with
  int style = STYLE_TEXT;
  if (startPos > 0)
    style = (int)CallEdit(SCI_GETSTYLEAT,startPos-1) & mask;

  // Set the character to '\n' to indicate that we always start at the start of a line
  unsigned char c = '\n';

  // Apply styles as far as is required
  int pos = startPos;
  while (pos < endPos)
  {
    // Get the next character, and note if it is the start of a new line
    bool newLine = ((c == '\r') || (c == '\n'));
    c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);

    if (style == STYLE_QUOTE)
    {
      if (c == '\"')
        ApplyStyle(startPos,pos+1,style,STYLE_TEXT,mask);
      else if (c == '[')
        ApplyStyle(startPos,pos,style,STYLE_QUOTEBRACKET,mask);
    }
    else if (style == STYLE_QUOTEBRACKET)
    {
      if (c == '\"')
        ApplyStyle(startPos,pos+1,style,STYLE_TEXT,mask);
      else if (c == ']')
        ApplyStyle(startPos,pos+1,style,STYLE_QUOTE,mask);
    }
    else if (style == STYLE_INFORM6)
    {
      if (c == ')')
      {
        if ((char)CallEdit(SCI_GETCHARAT,pos-1) == '-')
          ApplyStyle(startPos,pos+1,style,STYLE_TEXT,mask);
      }
    }
    else if (style == STYLE_HEADING)
    {
      if ((c == '\r') || (c == '\n'))
        ApplyStyle(startPos,pos,style,STYLE_TEXT,mask);
    }
    else if ((style >= STYLE_COMMENT) && (style < STYLE_COMMENT+NEST_COMMENTS))
    {
      int nested = style-STYLE_COMMENT;

      if (c == '[')
      {
        // For a nested comment, increase the nesting value
        if (nested < NEST_COMMENTS-1)
          ApplyStyle(startPos,pos,style,style+1,mask);
      }
      else if (c == ']')
      {
        // For the end of a comment, either decrease the nesting value, or end the comment
        if (nested < 1)
          ApplyStyle(startPos,pos+1,style,STYLE_TEXT,mask);
        else
          ApplyStyle(startPos,pos+1,style,style-1,mask);
      }
    }
    else
    {
      // Check for a title?
      if ((pos == 0) && (c == '\"'))
      {
        // Get the first line
        int endPos = (int)CallEdit(SCI_GETLINEENDPOSITION,0);
        TextRange range;
        range.chrg.cpMin = pos;
        range.chrg.cpMax = endPos;
        range.lpstrText = (char*)alloca(endPos+2);
        CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);

        // Look for an opening quote
        if (range.lpstrText[0] == '\"')
        {
          bool title = false;
          if (strstr(range.lpstrText,"\" by ") != NULL)
          {
            // Found a title followed by the author name
            title = true;
          }
          else if (range.lpstrText[endPos-1] == '\"')
          {
            // Found a title on its own
            title = true;
          }

          if (title)
          {
            ApplyStyle(startPos,pos,style,STYLE_HEADING,mask);
            pos = endPos;
            c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);
            AddHeading(Title,range.lpstrText,startPos);
            ApplyStyle(startPos,pos,style,STYLE_TEXT,mask);
            pos++;
            continue;
          }
        }
      }
      if (pos == 0)
        AddHeading(Title,"The Whole Source Text",0);

      // Check for a heading?
      if (newLine && isalpha(c))
      {
        // Get the start of the line
        int len = (int)CallEdit(SCI_GETLENGTH);
        TextRange range;
        range.chrg.cpMin = pos;
        range.chrg.cpMax = min(pos+8,len);
        char text[10];
        range.lpstrText = text;
        CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);

        // Check for a heading match
        for (int i = 0; i < sizeof headings / sizeof headings[0]; i++)
        {
          size_t hlen = strlen(headings[i]);
          if (strnicmp(text,headings[i],hlen) == 0)
          {
            // Got a possible match, so check it is followed by whitespace
            wchar_t follow = text[hlen];
            if ((follow == L' ') || (follow == L'\t'))
            {
              ApplyStyle(startPos,pos,style,STYLE_HEADING,mask);

              // Style the whole line
              int line = (int)CallEdit(SCI_LINEFROMPOSITION,pos);
              pos = (int)CallEdit(SCI_GETLINEENDPOSITION,line);
              c = (unsigned char)CallEdit(SCI_GETCHARAT,pos);
              AddHeading((HeadingLevel)(Volume+i),NULL,startPos);
              ApplyStyle(startPos,pos,style,STYLE_TEXT,mask);
              pos++;
              continue;
            }
          }
        }
      }

      // Plain text
      if (c == '[')
        ApplyStyle(startPos,pos,style,STYLE_COMMENT,mask);
      else if (c == '(')
      {
        if ((char)CallEdit(SCI_GETCHARAT,pos+1) == '-')
          ApplyStyle(startPos,pos,style,STYLE_INFORM6,mask);
      }
      else if (c == '\"')
      {
        // A double quote indicates the start of a string only if it is at the
        // start of a line or preceeded by white space or punctuation
        if (strchr(beforeQuote,(char)CallEdit(SCI_GETCHARAT,pos-1)) != NULL)
          ApplyStyle(startPos,pos,style,STYLE_QUOTE,mask);
      }
    }
    pos++;
  }

  // Apply the final style
  if (pos > len)
    pos = len;
  ApplyStyle(startPos,pos,style,style,mask);
}

const CArray<SourceLexer::Heading>& SourceLexer::GetHeadings(void)
{
  return m_headings;
}

void SourceLexer::ApplyStyle(int& start, int pos, int& style, int newStyle, int mask)
{
  if (m_action == LexApply)
  {
    CallEdit(SCI_STARTSTYLING,start,mask);
    CallEdit(SCI_SETSTYLING,pos-start,style);
  }
  start = pos;
  style = newStyle;
}

void SourceLexer::AddHeading(HeadingLevel level, LPCSTR name, int pos)
{
  if (m_action == LexHeadings)
  {
    int line = (int)CallEdit(SCI_LINEFROMPOSITION,pos);
    if (name == NULL)
    {
      int endPos = (int)CallEdit(SCI_GETLINEENDPOSITION,line);
      name = (char*)alloca(endPos-pos+2);
      TextRange range;
      range.chrg.cpMin = pos;
      range.chrg.cpMax = endPos;
      range.lpstrText = (char*)name;
      CallEdit(SCI_GETTEXTRANGE,0,(sptr_t)&range);
    }
    m_headings.Add(Heading(level,name,line));
  }
}

LONG_PTR SourceLexer::CallEdit(UINT msg, DWORD wp, LONG_PTR lp)
{
  return m_edit->CallEdit(msg,wp,lp);
}

SourceLexer::Heading::Heading()
{
  level = Section;
  line = 0;
}

SourceLexer::Heading::Heading(HeadingLevel lv, LPCSTR n, int ln)
{
  level = lv;
  line = ln;
  name = TextFormat::UTF8ToUnicode(n).Trim();
  for (int i = 0; i < name.GetLength(); i++)
  {
    if (iswspace(name.GetAt(i)))
      name.SetAt(i,L' ');
  }
}
