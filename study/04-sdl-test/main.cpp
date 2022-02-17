#include "mainwindow.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char* argv[])
{
    qDebug() << "this is a sdl test code\n";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
