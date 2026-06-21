#include <cstdio>
#include "settings.h"
#include "bricktron.h"
#include "paths.h"

game_settings load_settings() {
    game_settings s;
    s.master_volume = 1.0f;
    s.fullscreen    = 0;

    FILE *f = fopen(user_data_path(SETTINGS_FILE).c_str(), "rb");
    if (!f) return s;

    if (fread(&s, sizeof(game_settings), 1, f) != 1) {
        fclose(f);
        s.master_volume = 1.0f;
        s.fullscreen    = 0;
        return s;
    }
    fclose(f);

    if (s.master_volume < 0.0f) s.master_volume = 0.0f;
    if (s.master_volume > 1.0f) s.master_volume = 1.0f;
    return s;
}

void save_settings(const game_settings &s) {
    FILE *f = fopen(user_data_path(SETTINGS_FILE).c_str(), "wb");
    if (!f) return;
    fwrite(&s, sizeof(game_settings), 1, f);
    fclose(f);
}
