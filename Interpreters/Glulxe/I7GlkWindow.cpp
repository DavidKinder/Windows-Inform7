#include "I7GlkStream.h"
#include "I7GlkPairWindow.h"
#include "../../Inform7/InterpreterCommands.h"

std::set<I7GlkWindow*> glkWindows;
extern I7GlkWindow* mainWindow;

static int counterId = 0;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);

void sendCommand(int command, int dataLength, const void* data);
void readReturnData(void* data, int length);

I7Rect::I7Rect()
{
  left = 0;
  top = 0;
  right = 0;
  bottom = 0;
}

I7Rect::I7Rect(int l, int t, int r, int b)
{
  left = l;
  top = t;
  right = r;
  bottom = b;
}

int I7Rect::width() const
{
  return right-left;
}

int I7Rect::height() const
{
  return bottom-top;
}

I7GlkStyle::I7GlkStyle()
{
  m_justify = stylehint_just_LeftFlush;
  m_size = 0;
  m_weight = 0;
  m_italic = false;
  m_proportional = true;
  m_textColour = 0x00000000;
  m_backColour = 0x00FFFFFF;
  m_reverse = false;
}

I7GlkStyle::I7GlkStyle(int weight, bool proportional)
{
  m_justify = stylehint_just_LeftFlush;
  m_size = 0;
  m_weight = weight;
  m_italic = false;
  m_proportional = proportional;
  m_textColour = 0x00000000;
  m_backColour = 0x00FFFFFF;
  m_reverse = false;
}

bool I7GlkStyle::operator==(const I7GlkStyle& style)
{
  if (m_justify != style.m_justify)
    return false;
  if (m_size != style.m_size)
    return false;
  if (m_weight != style.m_weight)
    return false;
  if (m_italic != style.m_italic)
    return false;
  if (m_proportional != style.m_proportional)
    return false;
  if (m_textColour != style.m_textColour)
    return false;
  if (m_backColour != style.m_backColour)
    return false;
  if (m_reverse != style.m_reverse)
    return false;
  return true;
}

I7GlkWindow::I7GlkWindow(glui32 rock)
{
  m_id = counterId++;
  m_rock = rock;
  m_parent = NULL;
  m_readKey = ReadKeyNone;
  m_readMouse = false;
  m_readLink = false;
  m_stream = new I7GlkWinStream(this,0);
  m_echo = NULL;

  glkWindows.insert(this);

  if (registerObjFn)
    setDispRock((*registerObjFn)(this,gidisp_Class_Window));
  else
    m_dispRock.num = 0;
}

I7GlkWindow::~I7GlkWindow()
{
  if (unregisterObjFn)
    (*unregisterObjFn)(this,gidisp_Class_Window,getDispRock());

  glkWindows.erase(this);

  if (mainWindow == this)
    mainWindow = NULL;
}

void I7GlkWindow::endLine(event_t* event, glui32 len, bool cancel)
{
  if (event != NULL)
    event->type = evtype_None;
}

void I7GlkWindow::endKey(event_t* event, glui32 len, bool cancel)
{
  if (m_readKey == ReadKeyNone)
  {
    if (event != NULL)
      event->type = evtype_None;
    return;
  }

  glui32 glkKey = 0;
  if (len > 0)
  {
    int key = 0;
    readReturnData(&key,sizeof key);

    switch (key)
    {
    case '\r':
      glkKey = keycode_Return;
      break;
    case '\033':
      glkKey = keycode_Escape;
      break;
    case Key_Left:
      glkKey = keycode_Left;
      break;
    case Key_Right:
      glkKey = keycode_Right;
      break;
    case Key_Up:
      glkKey = keycode_Up;
      break;
    case Key_Down:
      glkKey = keycode_Down;
      break;
    default:
      if ((key > 255) && (m_readKey == ReadKeyAscii))
        glkKey = '?';
      else
        glkKey = key;
      break;
    }
  }

  if (event != NULL)
  {
    event->type = evtype_CharInput;
    event->win = (winid_t)this;
    event->val1 = glkKey;
    event->val2 = 0;
  }

  m_readKey = ReadKeyNone;

  if (cancel)
    sendCommand(Command_CancelKey,0,NULL);
}

void I7GlkWindow::endMouse(event_t* event, int x, int y)
{
  if (m_readMouse == false)
  {
    if (event != NULL)
      event->type = evtype_None;
    return;
  }

  if (event != NULL)
  {
    event->type = evtype_MouseInput;
    event->win = (winid_t)this;
    event->val1 = x;
    event->val2 = y;
  }
  m_readMouse = false;
}

void I7GlkWindow::endLink(event_t* event, int link)
{
  if (m_readLink == false)
  {
    if (event != NULL)
      event->type = evtype_None;
    return;
  }

  if (event != NULL)
  {
    event->type = evtype_Hyperlink;
    event->win = (winid_t)this;
    event->val1 = link;
  }
  m_readLink = false;
}

I7GlkStyle I7GlkWindow::getStyle(int style)
{
  if ((style >= 0) && (style < style_NUMSTYLES))
    return m_styles[style];
  return I7GlkStyle();
}

void I7GlkWindow::closeWindow(void)
{
  int data[1];
  data[0] = m_id;
  sendCommand(Command_DestroyWindow,sizeof data,data);

  // Remove this window from its parent, if any
  I7GlkPairWindow* parent1 = getParent();
  if (parent1 != NULL)
  {
    I7GlkWindow* otherChild = NULL;
    if (parent1->getChild1() == this)
      otherChild = parent1->getChild2();
    else
      otherChild = parent1->getChild1();

    I7GlkPairWindow* parent2 = parent1->getParent();
    if (parent2 != NULL)
    {
      otherChild->setParent(parent2);
      parent2->replace(parent1,otherChild);
    }
    else
    {
      otherChild->setParent(NULL);
      mainWindow = otherChild;
    }
    parent1->removeChildren();
    delete parent1;
  }

  // Make sure that no key window points to this window
  std::set<I7GlkWindow*>::iterator it;
  for (it = glkWindows.begin(); it != glkWindows.end(); ++it)
  {
    I7GlkPairWindow* pwin = dynamic_cast<I7GlkPairWindow*>(*it);
    if (pwin != NULL)
      pwin->removeKey(this);
  }

  // Delete this window
  delete this;
}
