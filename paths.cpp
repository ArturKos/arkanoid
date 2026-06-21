#include "paths.h"

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef BRICKTRON_DATA_DIR
#define BRICKTRON_DATA_DIR ""
#endif

static bool file_exists(const std::string &p) {
  struct stat st;
  return stat(p.c_str(), &st) == 0;
}

// Create every component of a directory path (like `mkdir -p`).
static void mkdirs(const std::string &path) {
  std::string cur;
  for (size_t i = 0; i < path.size(); i++) {
    cur += path[i];
    if (path[i] == '/' && cur.size() > 1) mkdir(cur.c_str(), 0755);
  }
  mkdir(path.c_str(), 0755);
}

std::string data_path(const std::string &rel) {
  if (file_exists(rel)) return rel;
  std::string base = BRICKTRON_DATA_DIR;
  if (!base.empty()) {
    std::string installed = base + "/" + rel;
    if (file_exists(installed)) return installed;
  }
  return rel;  // fall back; caller reports the load failure
}

std::string user_data_path(const std::string &rel) {
  std::string base;
  const char *xdg = getenv("XDG_DATA_HOME");
  if (xdg && xdg[0]) {
    base = std::string(xdg) + "/bricktron";
  } else {
    const char *home = getenv("HOME");
    if (home && home[0]) base = std::string(home) + "/.local/share/bricktron";
  }
  if (base.empty()) return rel;  // no HOME: fall back to the current directory
  mkdirs(base);
  return base + "/" + rel;
}
