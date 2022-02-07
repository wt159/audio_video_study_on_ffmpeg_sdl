#include "mainwindow.h"
#include "audio_device_ffmpeg.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    show();
    return a.exec();
}
