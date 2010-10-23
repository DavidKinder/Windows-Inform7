#include "I7GlkTextWindow.h"
#include "../../Inform7/InterpreterCommands.h"

#include <malloc.h>

extern int charWidth;
extern int charHeight;

extern gidispatch_rock_t (*registerArrFn)(void *array, glui32 len, char *typecode);
extern void (*unregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock);

void sendCommand(int command, int dataLength, const void* data);
void readReturnData(void* data, int length);

I7GlkStyle I7GlkTextWindow::defaultStyles[2][style_NUMSTYLES] =
{
  {
    I7GlkStyle(0,true),  // style_Normal
    I7GlkStyle(1,true),  // style_Emphasized
    I7GlkStyle(0,false), // style_Preformatted
    I7GlkStyle(1,true),  // style_Header
    I7GlkStyle(1,true),  // style_Subheader
    I7GlkStyle(0,true),  // style_Alert
    I7GlkStyle(0,true),  // style_Note
    I7GlkStyle(0,true),  // style_BlockQuote
    I7GlkStyle(0,true),  // style_Input
    I7GlkStyle(0,true),  // style_User1
    I7GlkStyle(0,true),  // style_User2
  },
  {
    I7GlkStyle(0,true),  // style_Normal
    I7GlkStyle(1,true),  // style_Emphasized
    I7GlkStyle(0,false), // style_Preformatted
    I7GlkStyle(1,true),  // style_Header
    I7GlkStyle(1,true),  // style_Subheader
    I7GlkStyle(0,true),  // style_Alert
    I7GlkStyle(0,true),  // style_Note
    I7GlkStyle(0,true),  // style_BlockQuote
    I7GlkStyle(0,true),  // style_Input
    I7GlkStyle(0,true),  // style_User1
    I7GlkStyle(0,true),  // style_User2
  }
};

I7GlkTextWindow::I7GlkTextWindow(glui32 rock) : I7GlkWindow(rock)
{
  m_lineBuffer = NULL;
  m_lineUBuffer = NULL;
  m_lineLength = 0;

  m_arrayRock.num = 0;

  for (int i = 0; i < style_NUMSTYLES; i++)
    m_styles[i] = defaultStyles[1][i];
}

I7GlkTextWindow::~I7GlkTextWindow()
{
  if ((m_lineBuffer != NULL) && (unregisterArrFn))
    (*unregisterArrFn)(m_lineBuffer,m_lineLength,"&+#!Cn",m_arrayRock);
  if ((m_lineUBuffer != NULL) && (unregisterArrFn))
    (*unregisterArrFn)(m_lineUBuffer,m_lineLength,"&+#!Iu",m_arrayRock);
}

void I7GlkTextWindow::requestLine(char *buf, glui32 maxlen, glui32 initlen)
{
  m_lineBuffer = buf;
  m_lineLength = maxlen;

  if (registerArrFn)
    m_arrayRock = (*registerArrFn)(m_lineBuffer,m_lineLength,"&+#!Cn");

  if (initlen > 0)
  {
    m_stream->putStr(buf,initlen);
    m_stream->flush();
  }

  int data[2];
  data[0] = m_id;
  data[1] = initlen;
  sendCommand(Command_ReadLine,sizeof data,data);
}

void I7GlkTextWindow::requestLine(glui32 *buf, glui32 maxlen, glui32 initlen)
{
  m_lineUBuffer = buf;
  m_lineLength = maxlen;

  if (registerArrFn)
    m_arrayRock = (*registerArrFn)(m_lineUBuffer,m_lineLength,"&+#!Iu");

  if (initlen > 0)
  {
    m_stream->putStr(buf,initlen);
    m_stream->flush();
  }

  int data[2];
  data[0] = m_id;
  data[1] = initlen;
  sendCommand(Command_ReadLine,sizeof data,data);
}

void I7GlkTextWindow::endLine(event_t* event, glui32 len, bool cancel)
{
  if ((m_lineBuffer == NULL) && (m_lineUBuffer == NULL))
  {
    if (event != NULL)
      event->type = evtype_None;
    return;
  }

  int charLen = len / sizeof (wchar_t);
  if (charLen > m_lineLength)
    charLen = m_lineLength;

  if (event != NULL)
  {
    event->type = evtype_LineInput;
    event->win = (winid_t)this;
    event->val1 = charLen;
    event->val2 = 0;
  }

  if (len > 0)
  {
    wchar_t* readLine = (wchar_t*)alloca(len);
    readReturnData(readLine,len);

    if (m_lineBuffer != NULL)
    {
      for (int i = 0; i < charLen; i++)
      {
        if (readLine[i] > 255)
          m_lineBuffer[i] = '?';
        else
          m_lineBuffer[i] = (char)readLine[i];
      }
    }
    if (m_lineUBuffer != NULL)
    {
      for (int i = 0; i < charLen; i++)
        m_lineUBuffer[i] = readLine[i];
    }
  }

  if (cancel == false)
  {
    if (m_echo != NULL)
    {
      if (m_lineBuffer != NULL)
        m_echo->putStr(m_lineBuffer,charLen);
      if (m_lineUBuffer != NULL)
        m_echo->putStr(m_lineUBuffer,charLen);
    }
    m_stream->putStr("\n",1);
  }

  if (unregisterArrFn)
  {
    if (m_lineBuffer != NULL)
      (*unregisterArrFn)(m_lineBuffer,m_lineLength,"&+#!Cn",m_arrayRock);
    if (m_lineUBuffer != NULL)
      (*unregisterArrFn)(m_lineUBuffer,m_lineLength,"&+#!Iu",m_arrayRock);
  }

  m_lineBuffer = NULL;
  m_lineUBuffer = NULL;
  m_lineLength = 0;

  if (cancel)
  {
    int data[1];
    data[0] = m_id;
    sendCommand(Command_CancelLine,sizeof data,data);
  }
}

void I7GlkTextWindow::requestKey(ReadKey readKey)
{
  m_readKey = readKey;
  sendCommand(Command_ReadKey,0,NULL);
}

void I7GlkTextWindow::requestLink(void)
{
  m_readLink = true;
}

void I7GlkTextWindow::clear(void)
{
  m_stream->flush();

  int data[2];
  data[0] = m_id;
  data[1] = 0;
  sendCommand(Command_Clear,sizeof data,data);
}

glui32 I7GlkTextWindow::draw(glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height)
{
  m_stream->flush();

  switch (val1)
  {
  case imagealign_InlineUp:
  case imagealign_InlineDown:
  case imagealign_InlineCenter:
    {
      int data[6];
      data[0] = m_id;
      data[1] = image;
      data[2] = val1;
      data[3] = val2;
      data[4] = width;
      data[5] = height;
      sendCommand(Command_Draw,sizeof data,data);
    }
    return 1;
  }
  return 0;
}

void I7GlkTextWindow::layout(const I7Rect& r)
{
  m_rect = r;
}

void I7GlkTextWindow::getNeededSize(int size, int& w, int& h, const I7Rect& r)
{
  w = charWidth * size;
  h = charHeight * size;
}

void I7GlkTextWindow::getSize(glui32* w, glui32* h)
{
  if (w != NULL)
    *w = m_rect.width() / charWidth;
  if (h != NULL)
    *h = m_rect.height() / charHeight;
}
