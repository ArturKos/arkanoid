#ifndef PATHS_H
#define PATHS_H

#include <string>

// Resolve a read-only data file (background, sounds, levels). Checks the current
// directory first (dev builds run from build/), then the compiled-in install
// data dir (e.g. /usr/share/arkanoid).
std::string data_path(const std::string &rel);

// Resolve a writable per-user file (high scores), creating the directory tree.
// Uses $XDG_DATA_HOME/arkanoid, falling back to ~/.local/share/arkanoid.
std::string user_data_path(const std::string &rel);

#endif
