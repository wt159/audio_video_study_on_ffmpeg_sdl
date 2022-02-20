#ifndef FFMPEGS_H
#define FFMPEGS_H

extern "C" {
#include <libavformat/avformat.h>
}

typedef struct {
    const char *filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} ResampleAudioSpec;

typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int channels;
} MyResampleAudioSpec;

class FFmpegs {
public:
    FFmpegs();
    static void resampleAudio(ResampleAudioSpec &in,
                              ResampleAudioSpec &out);

    static void resampleAudio(const char *inFilename,
                              int inSampleRate,
                              AVSampleFormat inSampleFmt,
                              int inChLayout,

                              const char *outFilename,
                              int outSampleRate,
                              AVSampleFormat outSampleFmt,
                              int outChLayout);

    static void myResampleAudio(MyResampleAudioSpec& in, MyResampleAudioSpec& out);
    static void myResampleAudio(const char *inFileName, int inSampleRate, AVSampleFormat inSampleFmt, int inChannels
                            , const char *outFileName, int outSampleRate, AVSampleFormat outFmt, int outChannels);
};

#endif // FFMPEGS_H
