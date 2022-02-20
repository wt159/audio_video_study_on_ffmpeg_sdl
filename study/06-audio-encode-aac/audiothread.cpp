#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"

AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void AudioThread::run() {
    AudioEncodeSpec in;
    in.filename = "./../48000_fltp_1.pcm";
    in.sampleRate = 48000;
    in.sampleFmt = AV_SAMPLE_FMT_FLTP;
    in.chLayout = AV_CH_LAYOUT_MONO;

    FFmpegs::aacEncode(in, "./out.aac");
}
