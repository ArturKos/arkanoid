#ifndef SETTINGS_H
#define SETTINGS_H

/** Persisted user preferences, stored as a single binary record in
 *  user_data_path(SETTINGS_FILE). */
struct game_settings {
    float master_volume; /**< Audio gain applied to all samples, clamped to [0,1]. Default 1.0. */
    int   fullscreen;    /**< Non-zero enables ALLEGRO_FULLSCREEN_WINDOW on startup. Default 0. */
};

/** Load settings from the user data dir.
 *  Returns default values (volume=1.0, fullscreen=0) when the file is absent or
 *  corrupt. master_volume is clamped to [0,1] on load. */
game_settings load_settings();

/** Persist @p s to the user data dir as a binary record (overwrites). */
void save_settings(const game_settings &s);

#endif
