#ifndef AUDIO_H
#define AUDIO_H

enum sound_id {
  SND_START,
  SND_PADDLE,
  SND_WALL,
  SND_HIT,
  SND_DESTROY,
  SND_POWERUP,
  SND_LIFE_LOST,
  SND_LEVEL,
  SND_GAMEOVER,
  SND_COUNT
};

// Initialize the audio system and load samples from sounds/. Safe to call even
// if audio hardware or files are unavailable — playback then silently no-ops.
void init_audio();
void destroy_audio();
void play_sound(int id);

/** Set the gain applied to every subsequent al_play_sample call.
 *  @param v Desired volume; clamped to [0,1]. */
void set_master_volume(float v);

#endif
