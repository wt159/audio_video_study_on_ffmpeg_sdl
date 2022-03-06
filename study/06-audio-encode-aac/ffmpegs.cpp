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

//ADTS头
static void get_adts_header_f(AVCodecContext *ctx, uint8_t *adts_header, int aac_length){
    uint8_t freq_idx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (ctx->sample_rate) {
        case 96000: freq_idx = 0; break;
        case 88200: freq_idx = 1; break;
        case 64000: freq_idx = 2; break;
        case 48000: freq_idx = 3; break;
        case 44100: freq_idx = 4; break;
        case 32000: freq_idx = 5; break;
        case 24000: freq_idx = 6; break;
        case 22050: freq_idx = 7; break;
        case 16000: freq_idx = 8; break;
        case 12000: freq_idx = 9; break;
        case 11025: freq_idx = 10; break;
        case 8000: freq_idx = 11; break;
        case 7350: freq_idx = 12; break;
        default: freq_idx = 4; break;
    }
 
    uint8_t chanCfg = ctx->channels;
    uint32_t frame_length = aac_length + 7;
    adts_header[0] = 0xFF;
    adts_header[1] = 0xF1;
    adts_header[2] = ((ctx->profile) << 6) | (freq_idx << 2) | (chanCfg >> 2);
    adts_header[3] = (((chanCfg & 3) << 6) | (frame_length  >> 11));
    adts_header[4] = ((frame_length & 0x7FF) >> 3);
    adts_header[5] = (((frame_length & 7) << 5) | 0x1F);
    adts_header[6] = 0xFC;
}

static int get_aac_header_info_by_adts(AVCodecContext* ctx, uint8_t* buf, size_t buf_size)
{
    uint8_t* adts_header = nullptr;
    for(size_t i=0; i<buf_size; i++) {
        if(*(buf+0) = 0xFF && *(buf+1) == 0xF1 && *(buf+7) == 0xFC) {
            adts_header = &buf[0];
            break;
        } else {
            buf++;
        }
    }

    if(!adts_header) {
        return -1;
    }

    uint8_t freq_index = (*(buf + 2) >> 2) & 0x0f;
    switch (freq_index) {
        case 0: ctx->sample_rate = 96000; break;
        case 1: ctx->sample_rate = 88200; break;
        case 2: ctx->sample_rate = 64000; break;
        case 3: ctx->sample_rate = 48000; break;
        case 4: ctx->sample_rate = 44100; break;
        case 5: ctx->sample_rate = 32000; break;
        case 6: ctx->sample_rate = 24000; break;
        case 7: ctx->sample_rate = 22050; break;
        case 8: ctx->sample_rate = 16000; break;
        case 9: ctx->sample_rate = 12000; break;
        case 10: ctx->sample_rate = 11025; break;
        case 11: ctx->sample_rate = 8000; break;
        case 12: ctx->sample_rate = 7350; break;
        default: ctx->sample_rate = 44100; break;
    }
    qDebug() << "sample rate:" << ctx->sample_rate;

    ctx->channels = (*(buf + 3) >> 6) | (*(buf + 2) & 1) << 2; 
    qDebug() << "sample chan:" << ctx->channels;
}

// static int adts_write_frame_header(ADTSContext *ctx,
//                                    uint8_t *buf, int size, int pce_size)
// {
//     PutBitContext pb;

//     unsigned full_frame_size = (unsigned)ADTS_HEADER_SIZE + size + pce_size;
//     if (full_frame_size > ADTS_MAX_FRAME_BYTES) {
//         av_log(NULL, AV_LOG_ERROR, "ADTS frame size too large: %u (max %d)\n",
//                full_frame_size, ADTS_MAX_FRAME_BYTES);
//         return AVERROR_INVALIDDATA;
//     }

//     init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

//     /* adts_fixed_header */
//     put_bits(&pb, 12, 0xfff);   /* syncword */
//     put_bits(&pb, 1, 0);        /* ID */
//     put_bits(&pb, 2, 0);        /* layer */
//     put_bits(&pb, 1, 1);        /* protection_absent */

//     put_bits(&pb, 2, ctx->objecttype); /* profile_objecttype */
//     put_bits(&pb, 4, ctx->sample_rate_index);
//     put_bits(&pb, 1, 0);        /* private_bit */
//     put_bits(&pb, 3, ctx->channel_conf); /* channel_configuration */
//     put_bits(&pb, 1, 0);        /* original_copy */
//     put_bits(&pb, 1, 0);        /* home */

//     /* adts_variable_header */
//     put_bits(&pb, 1, 0);        /* copyright_identification_bit */
//     put_bits(&pb, 1, 0);        /* copyright_identification_start */
//     put_bits(&pb, 13, full_frame_size); /* aac_frame_length */
//     put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
//     put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */

//     flush_put_bits(&pb);

//     return 0;
// }

// 音频编码
// 返回负数：中途出现了错误
// 返回0：编码操作正常完成
static int encode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {
    static size_t num = 0;
    // 发送数据到编码器
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // 不断从编码器中取出编码后的数据
    // while (ret >= 0)
    qDebug("frame size:%d,chan:%d", frame->nb_samples, frame->channels);
    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) { // 其他错误
            return ret;
        }

        qDebug("pkt size:%d,num:%lu", pkt->size, ++num);

        uint8_t aac_header[7];
        get_adts_header_f(ctx, aac_header, pkt->size);
        outFile.write((char*)aac_header, 7);

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
    int num = 0;

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
    ctx->channels = av_get_channel_layout_nb_channels(in.chLayout);
    // 比特率
    ctx->bit_rate = 128000;
    // 规格
    ctx->profile = FF_PROFILE_AAC_LOW;
    ctx->flags = AV_CODEC_FLAG_GLOBAL_HEADER;
    ctx->codec_id = AV_CODEC_ID_AAC;
    ctx->codec_type = AVMEDIA_TYPE_AUDIO;

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
        qDebug() << "encoding... " << num++;

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
