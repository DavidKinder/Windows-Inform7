#include "I7GlkTextWindow.h"
#include "../../Inform7/InterpreterCommands.h"

#include <deque>
#include <malloc.h>
#include <windows.h>

extern int charWidth;
extern int charHeight;

extern gidispatch_rock_t (*registerArrFn)(void *array, glui32 len, char *typecode);
extern void (*unregisterArrFn)(void *array, glui32 len, char *typecode, gidispatch_rock_t objrock);

struct FrontEndCmd
{
  FrontEndCmd();
  void free(void);
  void read(void);

  int cmd;
  int len;
  void* data;
};
extern std::deque<FrontEndCmd> commands;

void sendCommand(int command, int dataLength, const void* data);
bool readCommand(void);

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
  m_echoInput = true;

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

void I7GlkTextWindow::requestLine(char *buf, glui32 maxlen, glui32 initlen, bool echo)
{
  m_lineBuffer = buf;
  m_lineLength = maxlen;
  m_echoInput = echo;

  if (registerArrFn)
    m_arrayRock = (*registerArrFn)(m_lineBuffer,m_lineLength,"&+#!Cn");

  if (initlen > 0)
  {
    m_stream->putStr(buf,initlen,false);
    m_stream->flush();
  }

  int data[2];
  data[0] = m_id;
  data[1] = initlen;
  sendCommand(echo ? Command_ReadLine : Command_ReadLineSilent,sizeof data,data);
}

void I7GlkTextWindow::requestLine(glui32 *buf, glui32 maxlen, glui32 initlen, bool echo)
{
  m_lineUBuffer = buf;
  m_lineLength = maxlen;
  m_echoInput = echo;

  if (registerArrFn)
    m_arrayRock = (*registerArrFn)(m_lineUBuffer,m_lineLength,"&+#!Iu");

  if (initlen > 0)
  {
    m_stream->putStr(buf,initlen,false);
    m_stream->flush();
  }

  int data[2];
  data[0] = m_id;
  data[1] = initlen;
  sendCommand(echo ? Command_ReadLine : Command_ReadLineSilent,sizeof data,data);
}

static void readCancelLine(int id, wchar_t*& lineData, int& lineLen)
{
  for (;;)
  {
    while (readCommand());

    for (std::deque<FrontEndCmd>::iterator it = commands.begin(); it != commands.end(); ++it)
    {
      if ((it->cmd == Return_ReadLine) && (it->len >= sizeof(int)))
      {
        if (id == ((int*)it->data)[0])
        {
          lineData = (wchar_t*)(((int*)it->data)+1);
          lineLen = (it->len - sizeof(int)) / sizeof(wchar_t);
          it->free();
          commands.erase(it);
          return;
        }
      }
    }

    ::Sleep(50);
  }
}

void I7GlkTextWindow::endLine(event_t* event, bool cancel, wchar_t* lineData, int lineLen)
{
  if ((m_lineBuffer == NULL) && (m_lineUBuffer == NULL))
  {
    if (event != NULL)
      event->type = evtype_None;
    return;
  }

  if (cancel)
  {
    int data[1];
    data[0] = m_id;
    sendCommand(Command_CancelLine,sizeof data,data);

    // Wait for the cancelled line input to be sent back
    readCancelLine(getId(),lineData,lineLen);
  }

  if (event != NULL)
  {
    event->type = evtype_LineInput;
    event->win = (winid_t)this;
    event->val1 = lineLen;
    event->val2 = 0;
  }

  if (lineLen > 0)
  {
    if (m_lineBuffer != NULL)
    {
      for (int i = 0; i < lineLen; i++)
      {
        if (lineData[i] > 255)
          m_lineBuffer[i] = '?';
        else
          m_lineBuffer[i] = (char)lineData[i];
      }
    }
    if (m_lineUBuffer != NULL)
    {
      for (int i = 0; i < lineLen; i++)
        m_lineUBuffer[i] = lineData[i];
    }
  }

  if (m_echoInput)
  {
    if (m_echo != NULL)
    {
      if (m_lineBuffer != NULL)
        m_echo->putStr(m_lineBuffer,lineLen,false);
      if (m_lineUBuffer != NULL)
        m_echo->putStr(m_lineUBuffer,lineLen,false);
    }
    m_stream->putStr("\n",1,false);
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

bool I7GlkTextWindow::inputActive(void)
{
  if ((m_lineBuffer != NULL) || (m_lineUBuffer != NULL))
    return true;
  if (m_readKey != ReadKeyNone)
    return true;
  return false;
}

void I7GlkTextWindow::clear(void)
{
  m_stream->flush();

  int data[8];
  data[0] = m_id;
  for (int i = 1; i < 8; i++)
    data[i] = 0;
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
