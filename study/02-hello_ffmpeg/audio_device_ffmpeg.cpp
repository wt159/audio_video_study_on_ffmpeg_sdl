#include "audio_device_ffmpeg.h"

#include <QDebug>

void show()
{
    // avdevice_register_all();
    qDebug() << "audio device ffmpeg info:\n";
    qDebug() << "av_version      :" << av_version_info();
    qDebug() << "avdevice_version:" << avdevice_version();
    qDebug() << "avdevice_config: " << avdevice_configuration();
    qDebug() << "avdevice_license: " << avdevice_license();
}