#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include <string>

// ffmpeg header files.
extern "C" {
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

/**
 * @brief: Video Encoder structure
 */
struct VideoEncoderContext 
{
    // output video filename
    std::string outputVideoFile;

    // output video codec
    std::string codecStr;

    // output video width
    int width;

    // output video height
    int height;

    // output framerate
    int frameRate;

    // output video quality
    int quality;
    
    /**
     * @brief: constructor to initialize member data
     */
    VideoEncoderContext() 
    {
        // output video file
        outputVideoFile = "";

        // output video codec 
        codecStr = "";

        // output video width
        width = -1;

        // output video height
        height = -1;

        // output video framerate
        frameRate = 0;

        // output video quality
        quality = 2;
    }
};

/**
 * @brief: VideoEncoder class
 *          encode frames to video
 */
class VideoEncoder 
{
    // output format
    AVOutputFormat *m_avOutFmt;

    // format context
    AVFormatContext *m_avFmtCtx;

    // stream 
    AVStream *m_avStream;

    // codec context
    AVCodecContext *m_avCodecCtx;

    // frame
    AVFrame *m_avFrame;

    // stream index
    int m_streamIdx;

    // frame count
    int m_frameCount;

    // input picture buffer
    uint8_t *m_pictureInpBuf;

    // output picture buffer
    uint8_t *m_pictureOutBuf;

    // output picture buffer size
    int m_pictureOutBufSize;
    
    // Video Encoder Context member data
    struct VideoEncoderContext m_encoderContext;

    // flag to check for encoder context set
    int m_encoderCtxSet;
 
    // function to clean encoder
    void cleanEncoder();

    // function to initialize video
    int initializeVideo();

    // function to finalize video
    void finalizeVideo();

    // function to initialize encoder
    int initEncoder();

    // function to add a frame after conversion
    int addFrame();

    // function to add stream 
    AVStream* addStream();

    // function to open video
    int openVideo();

    // function to allocate frame
    AVFrame* allocFrame();

    // function to initialize member data
    void initLocals();

    public:
        // constructor for videoencoder
        VideoEncoder();

        // constructor for videoencoder
        VideoEncoder(const struct VideoEncoderContext encoderCtx);

        // destructor for videoencoder
        ~VideoEncoder();

        // function to set encoder context
        void setEncoderContext(const struct VideoEncoderContext encoderCtx);

        // function to start video encoding
        int startVideoEncode();

        // function to stop video encoding
        int stopVideoEncode();

        // function to add new frame
        int addNewFrame(unsigned char *frameArr);
    
        // function to check status of encoder context
        int encoderCtxSet();
};

#endif // VIDEO_ENCODER_H
