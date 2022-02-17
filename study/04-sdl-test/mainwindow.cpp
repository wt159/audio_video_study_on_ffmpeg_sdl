#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <SDL.h>
#include <chrono>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _playThread(nullptr)
{
    SDL_version v;
    SDL_VERSION(&v);
    // 2 0 14
    qDebug() << v.major << v.minor << v.patch;
    ui->setupUi(this);
    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked()
{
    if (_playThread) { // 停止播放
        _playThread->requestInterruption();
        _playThread = nullptr;
        ui->playButton->setText("开始播放");
    } else { // 开始播放
        _playThread = new PlayThread(this);
        _playThread->start();
        // 监听线程的结束
        connect(_playThread, &PlayThread::finished,
            [this]() {
                _playThread = nullptr;
                ui->playButton->setText("开始播放");
            });
        ui->playButton->setText("播放中");
        // SDL_PauseAudio(0);
        // std::this_thread::sleep_for(std::chrono::seconds(3));
        // SDL_PauseAudio(1);
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        // SDL_PauseAudio(0);
    }
}
