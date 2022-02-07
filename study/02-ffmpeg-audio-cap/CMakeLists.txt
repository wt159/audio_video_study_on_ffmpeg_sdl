cmake_minimum_required(VERSION 3.5)

project(02-ffmpeg-audio-cap LANGUAGES CXX)

# add_compile_definitions(-static)
add_compile_options(-static)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(FFMEPG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../dist/win/ffmpeg-4.3.2-2021-02-27-full_build-shared)
set(INC_DIR ${FFMEPG_PATH}/include)
set(LINK_DIR ${FFMEPG_PATH}/bin ${FFMEPG_PATH}/lib)

include_directories("${INC_DIR}")
link_directories("${LINK_DIR}")

if(ANDROID)
  add_library(02-ffmpeg-audio-cap SHARED
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )
else()
  add_executable(02-ffmpeg-audio-cap
    main.cpp
    mainwindow.cpp
    audio_device_ffmpeg.cpp
    mainwindow.h
    mainwindow.ui
  )
endif()

target_link_libraries(02-ffmpeg-audio-cap PRIVATE Qt5::Widgets
  avutil avdevice
)
