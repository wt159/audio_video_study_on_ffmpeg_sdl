cmake_minimum_required(VERSION 3.5)

project(04-sdl-test LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "F:/Qt/Qt5.14.2/5.14.2/mingw73_64")
find_package(Qt5 COMPONENTS Widgets REQUIRED)

SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_SOURCE_DIR}/../../dist/win/SDL2-2.0.16/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)

add_compile_options("-static")

add_executable(04-sdl-test
  main.cpp
  mainwindow.cpp
  playthread.cpp
  mainwindow.h
  mainwindow.ui
)

target_link_libraries(04-sdl-test PRIVATE Qt5::Widgets mingw32 SDL2::SDL2main SDL2::SDL2)
