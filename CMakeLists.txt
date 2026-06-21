cmake_minimum_required(VERSION 3.2)
project(
  bricktron
  VERSION 1.0
  LANGUAGES CXX)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_audio-5 allegro_acodec-5 allegro_font-5 allegro_image-5 allegro_primitives-5)

add_executable(bricktron
  main.cpp game_objects.cpp intro.cpp scores.cpp screen.cpp audio.cpp paths.cpp settings.cpp)
target_link_libraries(bricktron ${ALLEGRO5_LIBRARIES})
target_include_directories(bricktron PUBLIC ${ALLEGRO5_INCLUDE_DIRS})

# Where installed read-only assets live; the game checks the current directory
# first so development builds still run straight from build/.
set(BRICKTRON_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/bricktron")
target_compile_definitions(bricktron PRIVATE BRICKTRON_DATA_DIR="${BRICKTRON_DATA_DIR}")

if(MSVC)
  target_compile_options(bricktron PRIVATE /wall)
else()
  target_compile_options(bricktron PRIVATE -Wall -Wextra -pedantic)
endif()

# --- Installation (for packaging / `make install`) ---
install(TARGETS bricktron RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES build/background.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bricktron)
install(DIRECTORY build/sounds build/levels
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bricktron)
install(FILES bricktron.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(DIRECTORY icons/hicolor
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
install(FILES io.github.arturkos.Bricktron.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
