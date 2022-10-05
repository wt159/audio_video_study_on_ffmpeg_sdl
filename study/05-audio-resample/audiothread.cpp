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

void showAudioSpec(const ResampleAudioSpec& ras)
{
    qDebug() << "file: " << ras.filename;
    qDebug("rate:%d,fmt:%d,chanLay:0x%x, addr:%p", ras.sampleRate, ras.sampleFmt, ras.chLayout, &ras);
    qDebug("--------------------------------------------");
}

//void freep(void **ptr) {
//    free(*ptr);
//    *ptr = nullptr;
//}
#define PUB_PATH "../../../samples/"
void AudioThread::run() {
    // 44100_s16le_2 -> 48000_f32le_2 -> 48000_s32le_1 -> 44100_s16le_2

     ResampleAudioSpec ras1;
    ras1.filename = PUB_PATH"1-44100_s16le_2.pcm";
    ras1.sampleFmt = AV_SAMPLE_FMT_S16;
    ras1.sampleRate = 44100;
    ras1.chLayout = AV_CH_LAYOUT_STEREO;

     ResampleAudioSpec ras2;
    ras2.filename = PUB_PATH"2-48000_f32le_1.pcm";
    ras2.sampleFmt = AV_SAMPLE_FMT_FLTP;
    ras2.sampleRate = 48000;
    ras2.chLayout = AV_CH_LAYOUT_MONO;

     ResampleAudioSpec ras3;
    ras3.filename = PUB_PATH"3-44100_s32le_2.pcm";
    ras3.sampleFmt = AV_SAMPLE_FMT_S32;
    ras3.sampleRate = 44100;
    ras3.chLayout = AV_CH_LAYOUT_STEREO;

     ResampleAudioSpec ras4;
    ras4.filename = PUB_PATH"4-48000_f32le_1.pcm";
    ras4.sampleFmt = AV_SAMPLE_FMT_FLTP;
    ras4.sampleRate = 48000;
    ras4.chLayout = AV_CH_LAYOUT_MONO;

    showAudioSpec(ras1);
    showAudioSpec(ras2);
    showAudioSpec(ras3);
    showAudioSpec(ras4);

    // int nbChan = 2;
    // int nbSamples = 960;
    // AVSampleFormat nbFmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    // qDebug() << "2:960:16: " << av_samples_get_buffer_size(nullptr, 2, 960, AVSampleFormat::AV_SAMPLE_FMT_S16, 0);

    // uint8_t fmtBuf[3840+1] = {0};
    // int fmtBufSize = 3840;
    // qDebug() << "fmt name:" << av_get_sample_fmt_name(AVSampleFormat::AV_SAMPLE_FMT_S16);
    // qDebug() << av_get_sample_fmt_string((char*)fmtBuf, fmtBufSize, AVSampleFormat::AV_SAMPLE_FMT_S16);
    // qDebug() << fmtBuf;

    // uint8_t *audioDataPtr = nullptr;
    // int linesize = 0;

    //! failed
    // qDebug() << "fill array ret:" << av_samples_fill_arrays((uint8_t**)audioDataPtr, nullptr, (uint8_t*)fmtBuf, nbChan, nbSamples, nbFmt, 0);
    // if(audioDataPtr) {
    //     qDebug() << "audio data ptr not is nullptr";
    // } else {
    //     qDebug() << "audio data ptr is nullptr" ;
    // }

    // qDebug() << av_samples_alloc(&audioDataPtr, &linesize, nbChan, nbSamples, nbFmt, 0);
    // qDebug() << "linesize:" << linesize;
    // av_freep(audioDataPtr);
    // audioDataPtr = nullptr;
    // linesize = 0;

    // uint8_t **audioDataArr = nullptr;

    // qDebug() << av_samples_alloc_array_and_samples(&audioDataArr, &linesize, nbChan, nbSamples, nbFmt, 0);
    // qDebug() << "linesize:" << linesize;
    // audioDataArr[0] = 0;
    // audioDataArr[3839] = 0;
    // qDebug("audioDataArr:&[%p][%p]*[%p]", &audioDataArr, audioDataArr, *audioDataArr);
    // av_freep(audioDataArr);

    // qDebug() << " -------av sample copy-------";
    // uint8_t dstBuf[3840+1];
    // uint8_t *dstPtr = dstBuf;
    // uint8_t* const*srcPtr = (uint8_t**)(&fmtBuf);
    // av_samples_copy(&dstPtr, srcPtr, 0, 0, nbSamples, nbChan, nbFmt);
    // qDebug() << " copy finished";
    FFmpegs::resampleAudio(ras1, ras2);
    FFmpegs::resampleAudio(ras2, ras3);
    FFmpegs::resampleAudio(ras3, ras4);
}
