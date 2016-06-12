#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <string>

using namespace std;

extern "C" {
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

struct VideoInfo
{
    string videoFileName;
    string videoCodecName;
    int width;
    int height;
    int nChannels;
    int totalFrame;
    int frameRate;
    double duration;

    VideoInfo()
    {
        videoFileName = "none";
        videoCodecName = "none";
        width = -1;
        height = -1;
        nChannels = 0;
        totalFrame = -1;
        frameRate = -1;
        duration = 0.0;
    }
};

class VideoDecoder
{
    string m_inpFile;

    AVFormatContext *m_avFmtCtx;

    AVCodecContext *m_avCodecCtx;

    AVCodec *m_avCodec;

    AVStream *m_avStream;

    AVFrame *m_avFrame;

    AVFrame m_avFrameRGB;

    AVPacket m_avPkt;

    int m_width;

    int m_height;

    int m_streamIndex;

    int m_totalFrames;

    int m_totalDuration;

    int m_frameRate;

    void initLocals();

    int readAndDecodeFrame();

    public:
        VideoDecoder();

        VideoDecoder(string inpVideoFilePath);

        ~VideoDecoder();

        int openVideo(string inpVideoFilePath="");

        void closeVideo();

        int getNewFrame(unsigned char *frameArray);

        int getVideoInfo(VideoInfo &videoInfo);
};

#endif // VIDEO_DECODER_H
