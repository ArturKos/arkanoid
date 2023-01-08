cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_COMPILER "g++")
project(
  arkanoid
  VERSION 1.0
  LANGUAGES CXX)

  find_package(PkgConfig REQUIRED)
  add_executable(arkanoid main.cpp)
  pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5 ibus-table opusfile opusurl RapidJSON)
  target_link_libraries(arkanoid  ${ALLEGRO5_LIBRARIES})
  target_include_directories(arkanoid PUBLIC ${ALLEGRO5_INCLUDE_DIRS})


# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
#target_link_libraries(MyExample PRIVATE MyLibExample)
