#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}

struct ResampleAudioSpec {
    const char *filename;
    int sampleRate;
    int sampleFmt;
    uint64_t chLayout;
};

typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int channels;
} MyResampleAudioSpec;

class FFmpegs {
public:
    FFmpegs();
    static void resampleAudio(const ResampleAudioSpec &in,
                              const ResampleAudioSpec &out);

    static void resampleAudio(const char *inFilename,
                              const int inSampleRate,
                              const int inSampleFmt,
                              const uint64_t inChLayout,

                              const char *outFilename,
                              const int outSampleRate,
                              const int outSampleFmt,
                              const uint64_t outChLayout);

    // static void myResampleAudio(MyResampleAudioSpec& in, MyResampleAudioSpec& out);
    // static void myResampleAudio(const char *inFileName, int inSampleRate, AVSampleFormat inSampleFmt, int inChannels
    //                         , const char *outFileName, int outSampleRate, AVSampleFormat outFmt, int outChannels);
};

#endif // FFMPEGS_H
