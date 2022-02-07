#ifndef AUDIO_DEVICE_FFMPEG
#define AUDIO_DEVICE_FFMPEG

#include <stdint.h>
extern "C" {
    #include <libavutil/avutil.h>
    #include <libavdevice/avdevice.h>
}

void show();

#endif // !AUDIO_DEVICE_FFMPEG