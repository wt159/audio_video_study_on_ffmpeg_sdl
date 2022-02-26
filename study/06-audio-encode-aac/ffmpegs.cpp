#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs() {

}

// 检查采样格式
static int check_sample_fmt(const AVCodec *codec,
                            enum AVSampleFormat sample_fmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
//        qDebug() << av_get_sample_fmt_name(*p);
        if (*p == sample_fmt) return 1;
        qDebug() << "the coder fmt : " << av_get_sample_fmt_name(*p);
        p++;
    }
    return 0;
}

// 音频编码
// 返回负数：中途出现了错误
// 返回0：编码操作正常完成
static int encode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {
    // 发送数据到编码器
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // 不断从编码器中取出编码后的数据
    // while (ret >= 0)
    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) { // 其他错误
            return ret;
        }

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
        outFile.write((char *) pkt->data, pkt->size);

        // 释放pkt内部的资源
        av_packet_unref(pkt);
    }
}

void showAVCodecSupportedInfo(const AVCodec *codec)
{
    qDebug() << __func__ << ":start";
    const AVRational *frameRates = codec->supported_framerates;
    if(frameRates != nullptr) {
        qDebug() << "supported frame rates:";
        for(;(frameRates->num != 0 && frameRates->den != 0); frameRates++) {
            qDebug("num:%d,den:%d\n", frameRates->num, frameRates->den);
        }
    }
    

    const int *sampleRatePtr = codec->supported_samplerates;
    qDebug() << "supported sample rates:";
    for(; *sampleRatePtr != 0; sampleRatePtr++) {
        qDebug() << "rate:" << *sampleRatePtr;
    }

    const enum AVPixelFormat *pixFmtPtr = codec->pix_fmts;
    if(pixFmtPtr != nullptr) {
        qDebug() << "supported pix formats:";
        for(; *pixFmtPtr != AVPixelFormat::AV_PIX_FMT_NONE; pixFmtPtr++) {
            qDebug() << "pix_fmt:" << (int)(*pixFmtPtr);
        }
    }
    
    const enum AVSampleFormat *sampleFmtPtr = codec->sample_fmts;
    if(sampleFmtPtr != nullptr) {
        qDebug() << "supported sample formats:";
        for(; *sampleFmtPtr != AVSampleFormat::AV_SAMPLE_FMT_NONE; sampleFmtPtr++) {
            qDebug() << "fmt: " << av_get_sample_fmt_name(*sampleFmtPtr);
        }
    }

    const uint64_t *chanLayoutPtr = codec->channel_layouts;
    if(chanLayoutPtr != nullptr) {
        qDebug() << "supported channel layouts:";
        for(; *chanLayoutPtr != 0; chanLayoutPtr++) {
            qDebug() << "chanLayout:" << av_get_channel_name(*chanLayoutPtr);
        }
    }

    qDebug() << "supported hardware config:";
    for(int i=0;;i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if(config == nullptr) {
            break;
        }
        qDebug("index:%d, pix_fmt:0x%x, methods:%d, dev_type:0x%x", i, config->pix_fmt, config->methods, config->device_type);
    }
    qDebug() << __func__ << ":end";
}


void FFmpegs::aacEncode(AudioEncodeSpec &in,
                        const char *outFilename) {
    // 文件
    QFile inFile(in.filename);
    QFile outFile(outFilename);

    qDebug() << "codec config:" << avcodec_configuration();

    // 返回结果
    int ret = 0;

    // 编码器
    AVCodec *codec = nullptr;

    // 编码上下文
    AVCodecContext *ctx = nullptr;

    // 存放编码前的数据（pcm）
    AVFrame *frame = nullptr;

    // 存放编码后的数据（aac）
    AVPacket *pkt = nullptr;

    // 获取编码器
//    codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "encoder not found libfdk_aac";
        codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if(!codec) {
            qDebug() << "encoder not found aac";
            return;
        }
        qDebug() << "encoder support aac";
    }

    showAVCodecSupportedInfo(codec);

    // libfdk_aac对输入数据的要求：采样格式必须是16位整数
    // 检查输入数据的采样格式
    if (!check_sample_fmt(codec, in.sampleFmt)) {
        qDebug() << "unsupported sample format"
                 << av_get_sample_fmt_name(in.sampleFmt);
        qDebug("codec name[%s],long_name[%s]", codec->name, codec->long_name);
        return;
    }

    // 创建编码上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    // 设置PCM参数
    ctx->sample_rate = in.sampleRate;
    ctx->sample_fmt = in.sampleFmt;
    ctx->channel_layout = in.chLayout;
    // 比特率
    ctx->bit_rate = 32000;
    // 规格
    ctx->profile = FF_PROFILE_AAC_HE_V2;

    // 打开编码器
//    AVDictionary *options = nullptr;
//    av_dict_set(&options, "vbr", "5", 0);
//    ret = avcodec_open2(ctx, codec, &options);
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // frame缓冲区中的样本帧数量（由ctx->frame_size决定）
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->channel_layout = ctx->channel_layout;

    // 利用nb_samples、format、channel_layout创建缓冲区
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    // 读取数据到frame中
    while ((ret = inFile.read((char *) frame->data[0],
                              frame->linesize[0])) > 0) {
        // 从文件中读取的数据，不足以填满frame缓冲区
        if (ret < frame->linesize[0]) {
            int bytes = av_get_bytes_per_sample((AVSampleFormat) frame->format);
            int ch = av_get_channel_layout_nb_channels(frame->channel_layout);
            // 设置真正有效的样本帧数量
            // 防止编码器编码了一些冗余数据
            frame->nb_samples = ret / (bytes * ch);
        }

        // 进行编码
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }
    }

    // 刷新缓冲区
    encode(ctx, nullptr, pkt, outFile);

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    qDebug() << "线程正常结束";
}
