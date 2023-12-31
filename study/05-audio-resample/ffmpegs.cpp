#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs() {

}

void FFmpegs::resampleAudio(const ResampleAudioSpec &in,
                            const ResampleAudioSpec &out) {
    qDebug() << "inFile: " << in.filename;
    qDebug("rate:%d,fmt:%d,chan:%x", in.sampleRate, in.sampleFmt, in.chLayout);
    qDebug() << "outFile:" << out.filename;
    qDebug("rate:%d,fmt:%d,chan:%x", out.sampleRate, out.sampleFmt, out.chLayout);
    qDebug("-----------");
    resampleAudio(in.filename, in.sampleRate, in.sampleFmt, in.chLayout,
                  out.filename, out.sampleRate, out.sampleFmt, out.chLayout);
}

void FFmpegs::resampleAudio(const char *inFilename,
                            const int inSampleRate,
                            const int inSampleFmt,
                            const uint64_t inChLayout,

                            const char *outFilename,
                            const int outSampleRate,
                            const int outSampleFmt,
                            const uint64_t outChLayout) {
    // 向下取整，AV_ROUND_DOWN(2.66) = 2
    // qDebug() << av_rescale_rnd(8, 1, 3, AV_ROUND_DOWN);

    // 向上取整，AV_ROUND_UP(1.25) = 2
    // qDebug() << av_rescale_rnd(5, 1, 4, AV_ROUND_UP);

    // 文件名
    QFile inFile(inFilename);
    QFile outFile(outFilename);
    qDebug() << "inFile: " << inFilename;
    qDebug("rate:%d,fmt:%d,chan:%x", inSampleRate, inSampleFmt, inChLayout);
    qDebug() << "outFile:" << outFilename;
    qDebug("rate:%d,fmt:%d,chan:%x", outSampleRate, outSampleFmt, outChLayout);

    //
    

    // 输入缓冲区
    // 指向缓冲区的指针
    uint8_t **inData = nullptr;
    // 缓冲区的大小
    int inLinesize = 0;
    // 声道数
    int inChs = av_get_channel_layout_nb_channels(inChLayout);
    // 一个样本的大小
    int inBytesPerSample = inChs * av_get_bytes_per_sample((AVSampleFormat)inSampleFmt);
    // 缓冲区的样本数量
    int inSamples = 1024;
    // 读取文件数据的大小
    int len = 0;

    // 输出缓冲区
    // 指向缓冲区的指针
    uint8_t **outData = nullptr;
    // 缓冲区的大小
    int outLinesize = 0;
    // 声道数
    int outChs = av_get_channel_layout_nb_channels(outChLayout);
    // 一个样本的大小
    int outBytesPerSample = outChs * av_get_bytes_per_sample((AVSampleFormat)outSampleFmt);
    // 缓冲区的样本数量
    // int outSamples = av_rescale_rnd(outSampleRate, inSamples, inSampleRate, AV_ROUND_UP);
    int outSamples = 0;
    /*
     inSampleRate     inSamples
     ------------- = -----------
     outSampleRate    outSamples

     outSamples = outSampleRate * inSamples / inSampleRate
     */
     // 返回结果
    int ret = 0;

    // 创建重采样上下文
    SwrContext *ctx = swr_alloc_set_opts(nullptr,
                                         // 输出参数
                                         outChLayout, (AVSampleFormat)outSampleFmt, outSampleRate,
                                         // 输入参数
                                         inChLayout, (AVSampleFormat)inSampleFmt, inSampleRate,
                                         0, nullptr);
    if (!ctx) {
        qDebug() << "swr_alloc_set_opts error";
        goto end;
    }

    // 初始化重采样上下文
    ret = swr_init(ctx);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "swr_init error:" << errbuf;
        goto end;
    }

    outSamples = swr_get_out_samples(ctx, inSamples);
    qDebug() << "输入缓冲区" << inSampleRate << inSamples << inChs;
    qDebug() << "输出缓冲区" << outSampleRate << outSamples << outChs;

    

    /* 指针类型（64bit，8个字节）
    int *;
    double *;
    void *;
    int **;
    int ***;
    int ******;
    */

    // int *p;
    // *(p + i) == p[i]
    // *(p + 0) == p[0]
    // *p == p[0]

    // int *p = new int[15];
    // int *p = av_calloc(15, sizeof (int));
    // int **pp = av_calloc(7, sizeof (int *));

    // uint8_t **inData = av_calloc(1, sizeof(uint8_t *));

    // 创建输入缓冲区
    ret = av_samples_alloc_array_and_samples(
              &inData,
              &inLinesize,
              inChs,
              inSamples,
              (AVSampleFormat)inSampleFmt,
              1);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }

    // 创建输出缓冲区
    ret = av_samples_alloc_array_and_samples(
              &outData,
              &outLinesize,
              outChs,
              outSamples,
              (AVSampleFormat)outSampleFmt,
              1);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_samples_alloc_array_and_samples error:" << errbuf;
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << outFilename;
        goto end;
    }

    // 读取文件数据
    // inData[0] == *inData
    while ((len = inFile.read((char *) inData[0], inLinesize)) > 0) {
        // 读取的样本数量
        inSamples = len / inBytesPerSample;

        // 重采样(返回值转换后的样本数量)
        ret = swr_convert(ctx,
                          outData, outSamples,
                          (const uint8_t **) inData, inSamples
                         );

        if (ret < 0) {
            ERROR_BUF(ret);
            qDebug() << "swr_convert error:" << errbuf;
            goto end;
        }

//        int size = av_samples_get_buffer_size(nullptr, outChs, ret, outSampleFmt, 1);
//        outFile.write((char *) outData[0], size);

        // 将转换后的数据写入到输出文件中
        // outData[0] == *outData
        outFile.write((char *) outData[0], ret * outBytesPerSample);
    }

    // 检查一下输出缓冲区是否还有残留的样本（已经重采样过的，转换过的）
    while ((ret = swr_convert(ctx,
                              outData, outSamples,
                              nullptr, 0)) > 0) {
        outFile.write((char *) outData[0], ret * outBytesPerSample);
    }

end:
    // 释放资源
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放输入缓冲区
    if (inData) {
        av_freep(&inData[0]);
    }
    av_freep(&inData);

    // 释放输出缓冲区
    if (outData) {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    // 释放重采样上下文
    swr_free(&ctx);

//    void *ptr = malloc(100);
//    freep(&ptr);
//    free(ptr);
//    ptr = nullptr;
}

#if 0
void myResampleAudio(MyResampleAudioSpec& in, MyResampleAudioSpec& out)
{
    myResampleAudio(in.filename, in.sampleRate, in.sampleFmt, in.channels,
                    out.filename, out.sampleRate, out.sampleFmt, out.channels);
}
void myResampleAudio(const char* inFileName, int inSampleRate, AVSampleFormat inSampleFmt, int inChannels,
                    const char* outFileName, int outSampleRate, AVSampleFormat outFmt, int outChannels)
{
    QFile inFile(inFileName);
    QFile outFile(outFileName);

    uint8_t **inData = nullptr;
    int inLineSize = 0;
    int inChLayout = av_get_default_channel_layout(inChannels);
    int inChs = inChannels;
    int inBytesPerSample = inChs * av_get_bytes_per_sample(inSampleFmt);
    int inSamples = 1024;
    int len = 0;

    uint8_t **outData = nullptr;
    int outLineSize = 0;
    int outChs = outChannels;
    int outBytesPerSample = outChs * av_get_bytes_per_sample(outFmt);
    int outSamples = av_rescale_rnd(outSampleRate, inSamples, inSampleRate, AV_ROUND_UP);

    qDebug() << "输入缓冲区:" << inSampleRate << " " << inSamples;
    qDebug() << "输出缓冲区:" << outSampleRate << " " << outSamples;

    int ret = 0;

    SwrContext *ctx = swr_alloc_set_opts(nullptr,
                                        );
}
#endif