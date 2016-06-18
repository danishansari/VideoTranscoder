#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <string>

// ffmpeg header files.
extern "C" {
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

/** 
 * @brief: structure to define input video file properties.
 */
struct VideoInfo
{
    // input video file name
    std::string videoFileName; 

    // input video codec name
    std::string videoCodecName; 

    // input video width
    int width;

    // input video height
    int height;

    // no of channels in video frame
    int nChannels;

    // total frame in video
    int totalFrame;

    // frame rate of input video 
    int frameRate;

    // total duration of input video
    double duration;

    /**
     * @brief: constructor to initialize member data
     */
    VideoInfo()
    {
        // vidoe filename
        videoFileName = "none";

        // video codec name
        videoCodecName = "none";

        // vidoe width
        width = -1;

        // video height
        height = -1;

        // video num channles
        nChannels = 0;

        // total frames in video
        totalFrame = -1;

        // frame rate of video
        frameRate = -1;

        // total duration
        duration = 0.0;
    }
};

/**
 * @brief: VideoDecoder class 
 *          decode frames from video into rgb24
 */
class VideoDecoder
{
    // input video filename
    std::string m_inpFile;

    // format context 
    AVFormatContext *m_avFmtCtx;

    // codec context
    AVCodecContext *m_avCodecCtx;

    // codec
    AVCodec *m_avCodec;

    // stream
    AVStream *m_avStream;

    // frame
    AVFrame *m_avFrame;

    // converted rgb frame
    AVFrame m_avFrameRGB;

    // avpkt
    AVPacket m_avPkt;

    // width of input video
    int m_width;

    // height of input video
    int m_height;

    // stream index
    int m_streamIndex;

    // total no of frames
    int m_totalFrames;

    // total duration of video
    int m_totalDuration;

    // frame rate of input video
    int m_frameRate;

    // function to initialize private member data
    void initLocals();

    // function to read one frame from video and decode
    int readAndDecodeFrame();

    public:
        // default constructor for videodecoder
        VideoDecoder();

        // constructor for videodecoder
        VideoDecoder(std::string inpVideoFilePath);

        // destructor for videodecoder
        ~VideoDecoder();

        // function to open input video
        int openVideo(std::string inpVideoFilePath="");

        // function to close video if opened
        void closeVideo();

        // function to fetch one decoded frame from input video
        int getNewFrame(unsigned char *frameArray);

        // function to fetch video information of the input video
        int getVideoInfo(VideoInfo &videoInfo);
};

#endif // VIDEO_DECODER_H
