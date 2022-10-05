cmake_minimum_required(VERSION 3.5)

project(vidoe-audio-test LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_COMPILER 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "F:/Qt/Qt5.14.2/5.14.2/mingw73_64")
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(FFMEPG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../dist/win/ffmpeg-4.3.2-2021-02-27-full_build-shared)
set(INC_DIR ${FFMEPG_PATH}/include)
set(LINK_DIR ${FFMEPG_PATH}/bin ${FFMEPG_PATH}/lib)

include_directories("${INC_DIR}")
link_directories("${LINK_DIR}")

message(STATUS "${INC_DIR}")
message(STATUS "${LINK_DIR}")

if(ANDROID)
  add_library(vidoe-audio-test SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
else()
  add_executable(vidoe-audio-test
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
endif()

target_link_libraries(vidoe-audio-test PRIVATE Qt5::Widgets avutil)
