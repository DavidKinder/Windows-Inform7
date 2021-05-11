#pragma once

extern "C" {
#include "glk.h"
#include "gi_dispa.h"
}

#include <set>

class I7GlkChannel
{
public:
  I7GlkChannel(glui32 rock);
  virtual ~I7GlkChannel();

  int getId(void) { return m_id; }
  glui32 getRock(void) { return m_rock; }
  void setDispRock(const gidispatch_rock_t& rock) { m_dispRock = rock; }
  gidispatch_rock_t& getDispRock(void) { return m_dispRock; }

  glui32 play(glui32 snd, glui32 repeats, glui32 notify);
  glui32 multiPlay(glui32 snd, glui32 repeats, glui32 notify, int* data);
  void stop(bool check);
  void setVolume(glui32 volume, glui32 duration, glui32 notify);
  void pause(bool pause);
  void getSoundNotify(event_t& event);
  void getVolumeNotify(event_t& event);

protected:
  int m_id;
  glui32 m_rock;
  gidispatch_rock_t m_dispRock;

  glui32 m_sound;
  glui32 m_soundNotify;
  glui32 m_volumeNotify;
};

extern std::set<I7GlkChannel*> glkChannels;
