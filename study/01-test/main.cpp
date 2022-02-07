#include "mainwindow.h"
#include <QDebug>
#include <QApplication>

extern "C" {
#include <libavutil/avutil.h>
}

int main(int argc, char *argv[])
{
    // 打印版本信息
    qDebug() << av_version_info();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
