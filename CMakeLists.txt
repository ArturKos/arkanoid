cmake_minimum_required(VERSION 3.2)
project(
  arkanoid
  VERSION 1.0
  LANGUAGES CXX)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5)

add_executable(arkanoid
  main.cpp game_objects.cpp intro.cpp scores.cpp screen.cpp audio.cpp paths.cpp)
target_link_libraries(arkanoid ${ALLEGRO5_LIBRARIES})
target_include_directories(arkanoid PUBLIC ${ALLEGRO5_INCLUDE_DIRS})

# Where installed read-only assets live; the game checks the current directory
# first so development builds still run straight from build/.
set(ARKANOID_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/arkanoid")
target_compile_definitions(arkanoid PRIVATE ARKANOID_DATA_DIR="${ARKANOID_DATA_DIR}")

if(MSVC)
  target_compile_options(arkanoid PRIVATE /wall)
else()
  target_compile_options(arkanoid PRIVATE -Wall -Wextra -pedantic)
endif()

# --- Installation (for packaging / `make install`) ---
install(TARGETS arkanoid RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES build/background.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/arkanoid)
install(DIRECTORY build/sounds build/levels
        DESTINATION ${CMAKE_INSTALL_DATADIR}/arkanoid)
install(FILES arkanoid.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(DIRECTORY icons/hicolor
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
