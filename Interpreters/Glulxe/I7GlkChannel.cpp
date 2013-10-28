#include "I7GlkChannel.h"
#include "I7GlkCmd.h"
#include "../../Inform7/InterpreterCommands.h"

#include <deque>

std::set<I7GlkChannel*> glkChannels;

static int counterId = 0;

extern std::deque<event_t> otherEvents;

extern gidispatch_rock_t (*registerObjFn)(void *obj, glui32 objclass);
extern void (*unregisterObjFn)(void *obj, glui32 objclass, gidispatch_rock_t objrock);

I7GlkChannel::I7GlkChannel(glui32 rock)
{
  m_id = counterId++;
  m_rock = rock;

  m_sound = 0;
  m_soundNotify = 0;
  m_volumeNotify = 0;

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
  m_soundNotify = notify;

  int data[3];
  data[0] = m_id;
  data[1] = snd;
  data[2] = repeats;
  sendCommand(Command_PlaySound,sizeof data,data);
  return 1;
}

glui32 I7GlkChannel::multiPlay(glui32 snd, glui32 repeats, glui32 notify, int* data)
{
  m_sound = snd;
  m_soundNotify = notify;

  data[0] = m_id;
  data[1] = snd;
  data[2] = repeats;
  return 1;
}

void I7GlkChannel::stop(bool check)
{
  if (check && (m_sound == 0))
    return;

  m_sound = 0;
  m_soundNotify = 0;

  int data[1];
  data[0] = m_id;
  sendCommand(Command_StopSound,sizeof data,data);
}

void I7GlkChannel::setVolume(glui32 volume, glui32 duration, glui32 notify)
{
  int data[3];
  data[0] = m_id;
  data[1] = volume;
  data[2] = duration;
  sendCommand(Command_SetVolume,sizeof data,data);

  if (duration == 0)
  {
    if (notify != 0)
    {
      event_t notifyEvent;
      notifyEvent.type = evtype_VolumeNotify;
      notifyEvent.win = 0;
      notifyEvent.val1 = 0;
      notifyEvent.val2 = notify;
      otherEvents.push_back(notifyEvent);
    }
    m_volumeNotify = 0;
  }
  else
    m_volumeNotify = notify;
}

void I7GlkChannel::pause(bool pause)
{
  int data[2];
  data[0] = m_id;
  data[1] = pause ? 1 : 0;
  sendCommand(Command_PauseSound,sizeof data,data);
}

void I7GlkChannel::getSoundNotify(event_t& event)
{
  if (m_soundNotify != 0)
  {
    event.type = evtype_SoundNotify;
    event.win = 0;
    event.val1 = m_sound;
    event.val2 = m_soundNotify;
  }
  m_soundNotify = 0;
}

void I7GlkChannel::getVolumeNotify(event_t& event)
{
  if (m_volumeNotify != 0)
  {
    event.type = evtype_VolumeNotify;
    event.win = 0;
    event.val1 = 0;
    event.val2 = m_volumeNotify;
  }
  m_volumeNotify = 0;
}
