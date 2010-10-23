#include "I7GlkPairWindow.h"
#include "../../Inform7/InterpreterCommands.h"

void sendCommand(int command, int dataLength, const void* data);

I7GlkPairWindow::I7GlkPairWindow(I7GlkWindow* win1, I7GlkWindow* win2, glui32 method, glui32 size)
  : I7GlkWindow(0)
{
  m_win1 = win1;
  m_win1->setParent(this);

  m_win2 = win2;
  m_win2->setParent(this);

  m_method = method;
  m_size = size;
  m_key = win2;
}

I7GlkPairWindow::~I7GlkPairWindow()
{
  if (m_win1 != NULL)
    delete m_win1;
  if (m_win2 != NULL)
    delete m_win2;
}

void I7GlkPairWindow::layout(const I7Rect& r)
{
  I7Rect r1 = r;
  I7Rect r2 = r;

  switch (m_method & winmethod_DivisionMask)
  {
  case winmethod_Proportional:
    switch (m_method & winmethod_DirMask)
    {
    case winmethod_Above:
      r2.bottom = r2.top + (int)(r.height() * m_size * 0.01);
      r1.top = r2.bottom;
      break;
    case winmethod_Below:
      r2.top = r2.bottom - (int)(r.height() * m_size * 0.01);
      r1.bottom = r2.top;
      break;
    case winmethod_Left:
      r2.right = r2.left + (int)(r.width() * m_size * 0.01);
      r1.left = r2.right;
      break;
    case winmethod_Right:
      r2.left = r2.right - (int)(r.width() * m_size * 0.01);
      r1.right = r2.left;
      break;
    }
    break;
  case winmethod_Fixed:
    {
      int w = 0;
      int h = 0;
      if (m_key != NULL)
        m_key->getNeededSize(m_size,w,h,r);

      switch (m_method & winmethod_DirMask)
      {
      case winmethod_Above:
        r2.bottom = r2.top + h;
        r1.top = r2.bottom;
        break;
      case winmethod_Below:
        r2.top = r2.bottom - h;
        r1.bottom = r2.top;
        break;
      case winmethod_Left:
        r2.right = r2.left + w;
        r1.left = r2.right;
        break;
      case winmethod_Right:
        r2.left = r2.right - w;
        r1.right = r2.left;
        break;
      }
    }
    break;
  }

  m_win1->layout(r1);
  m_win2->layout(r2);
}

void I7GlkPairWindow::getNeededSize(int size, int& w, int& h, const I7Rect& r)
{
  w = 0;
  h = 0;
}

void I7GlkPairWindow::getSize(glui32* w, glui32* h)
{
  if (w != NULL)
    *w = 0;
  if (h != NULL)
    *h = 0;
}

void I7GlkPairWindow::setArrangement(glui32 method, glui32 size, I7GlkWindow* keywin)
{
  if (keywin != NULL)
  {
    bool keyIsChild = false;
    I7GlkWindow* wnd = keywin;
    while ((wnd != NULL) && (keyIsChild == false))
    {
      if (wnd == m_win2)
        keyIsChild = true;
      else if (wnd == m_win1)
        keyIsChild = true;
      wnd = wnd->getParent();
    }
    if (keyIsChild == false)
      return;
  }

  bool swap = false;
  glui32 m1 = m_method & winmethod_DirMask;
  glui32 m2 = method & winmethod_DirMask;
  if (m1 != m2)
  {
    if ((m1 == winmethod_Above) && (m2 != winmethod_Below))
      return;
    if ((m1 == winmethod_Below) && (m2 != winmethod_Above))
      return;
    if ((m1 == winmethod_Left) && (m2 != winmethod_Right))
      return;
    if ((m1 == winmethod_Right) && (m2 != winmethod_Left))
      return;
    std::swap(m_win1,m_win2);
    swap = true;
  }

  m_method = method;
  m_size = size;
  if (keywin != NULL)
    m_key = keywin;

  int data[5];
  data[0] = m_id;
  data[1] = method;
  data[2] = size;
  data[3] = (m_key != NULL) ? m_key->getId() : -1;
  data[4] = swap ? 1 : 0;
  sendCommand(Command_ArrangeWindow,sizeof data,data);
}

void I7GlkPairWindow::getArrangement(glui32* method, glui32* size, winid_t* keywin)
{
  if (method != NULL)
    *method = m_method;
  if (size != NULL)
    *size = m_size;
  if (keywin != NULL)
    *keywin = (winid_t)m_key;
}

void I7GlkPairWindow::replace(I7GlkWindow* oldWin, I7GlkWindow* newWin)
{
  if (m_win1 == oldWin)
    m_win1 = newWin;
  if (m_win2 == oldWin)
    m_win2 = newWin;
}

void I7GlkPairWindow::removeChildren(void)
{
  m_win1 = NULL;
  m_win2 = NULL;
}

void I7GlkPairWindow::removeKey(I7GlkWindow* win)
{
  if (m_key == win)
    m_key = NULL;
}

I7GlkWindow* I7GlkPairWindow::getSibling(I7GlkWindow* win)
{
  if (win == m_win1)
    return m_win2;
  return m_win1;
}
