#include "I7GlkChannel.h"
#include "../../Inform7/InterpreterCommands.h"

std::set<I7GlkChannel*> glkChannels;

static int counterId = 0;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);

void sendCommand(int command, int dataLength, const void* data);

I7GlkChannel::I7GlkChannel(glui32 rock)
{
  m_id = counterId++;
  m_rock = rock;

  m_sound = 0;
  m_notify = 0;

  glkChannels.insert(this);

  if (registerObjFn)
    setDispRock((*registerObjFn)(this,gidisp_Class_Schannel));
  else
    m_dispRock.num = 0;
}

I7GlkChannel::~I7GlkChannel()
{
  if (unregisterObjFn)
    (*unregisterObjFn)(this,gidisp_Class_Schannel,getDispRock());

  glkChannels.erase(this);
}

glui32 I7GlkChannel::play(glui32 snd, glui32 repeats, glui32 notify)
{
  m_sound = snd;
  m_notify = notify;

  int data[3];
  data[0] = m_id;
  data[1] = snd;
  data[2] = repeats;
  sendCommand(Command_PlaySound,sizeof data,data);
  return 1;
}

void I7GlkChannel::stop(bool check)
{
  if (check && (m_sound == 0))
    return;

  m_sound = 0;
  m_notify = 0;

  int data[1];
  data[0] = m_id;
  sendCommand(Command_StopSound,sizeof data,data);
}

void I7GlkChannel::setVolume(glui32 volume, glui32 duration)
{
  int data[3];
  data[0] = m_id;
  data[1] = volume;
  data[2] = duration;
  sendCommand(Command_SetVolume,sizeof data,data);
}

void I7GlkChannel::getNotify(event_t& event)
{
  if (m_notify != 0)
  {
    event.type = evtype_SoundNotify;
    event.win = 0;
    event.val1 = m_sound;
    event.val2 = m_notify;
  }
  m_notify = 0;
}
