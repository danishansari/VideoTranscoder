#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

extern "C" {

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

}

#include <string>

struct VideoEncoderContext 
{
    std::string outputVideoFile;
    int width;
    int height;
    int frameRate;
    int quality;
    std::string codecStr;

    VideoEncoderContext() {
        outputVideoFile = "";
        width = -1;
        height = -1;
        frameRate = 0;
        quality = 2;
        codecStr = "";
    }
};

class VideoEncoder 
{
    AVOutputFormat *m_avOutFmt;

    AVFormatContext *m_avFmtCtx;

    AVStream *m_avStream;

    AVCodecContext *m_avCodecCtx;

    AVFrame *m_avFrame;

    int m_streamIdx;

    int m_frameCount;

    uint8_t *m_pictureInpBuf;

    uint8_t *m_pictureOutBuf;

    int m_pictureOutBufSize;
    
    struct VideoEncoderContext m_encoderContext;

    int m_encoderCtxSet;
 
    void cleanEncoder();

    int initializeVideo();

    void finalizeVideo();

    int initEncoder();

    int addFrame();

    AVStream* addStream();

    void openVideo();

    AVFrame* allocFrame();

    void initLocals();

    public:
        VideoEncoder();

        VideoEncoder(const struct VideoEncoderContext encoderCtx);

        ~VideoEncoder();

        int setEncoderContext(const struct VideoEncoderContext encoderCtx);

        int startVideoEncode();

        int stopVideoEncode();

        int addNewFrame(unsigned char *frameArr);
    
        int encoderCtxSet();
};

#endif // VIDEO_ENCODER_H
