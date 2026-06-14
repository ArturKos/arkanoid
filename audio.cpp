#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>

#include "audio.h"
#include "paths.h"

static ALLEGRO_SAMPLE *samples[SND_COUNT] = {0};
static bool audio_ready = false;

static const char *sound_files[SND_COUNT] = {
    "sounds/start.wav",     "sounds/paddle.wav",    "sounds/wall.wav",
    "sounds/hit.wav",       "sounds/destroy.wav",   "sounds/powerup.wav",
    "sounds/life_lost.wav", "sounds/level.wav",     "sounds/gameover.wav",
};

void init_audio() {
  if (!al_install_audio()) {
    fprintf(stderr, "Warning: audio init failed; running without sound.\n");
    return;
  }
  if (!al_init_acodec_addon()) {
    fprintf(stderr, "Warning: audio codec init failed; running without sound.\n");
    al_uninstall_audio();
    return;
  }
  al_reserve_samples(16);
  for (int i = 0; i < SND_COUNT; i++) {
    samples[i] = al_load_sample(data_path(sound_files[i]).c_str());
    if (!samples[i])
      fprintf(stderr, "Warning: failed to load '%s'.\n", sound_files[i]);
  }
  audio_ready = true;
}

void play_sound(int id) {
  if (!audio_ready || id < 0 || id >= SND_COUNT || !samples[id]) return;
  al_play_sample(samples[id], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void destroy_audio() {
  for (int i = 0; i < SND_COUNT; i++) {
    if (samples[i]) {
      al_destroy_sample(samples[i]);
      samples[i] = NULL;
    }
  }
  if (audio_ready) al_uninstall_audio();
  audio_ready = false;
}
