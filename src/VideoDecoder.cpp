
/**
 * Description: VideoDecoder Class
 *                  decode frames from video
 *
 * Author: Md Danish
 *
 * Date: 2016-06-02 11:19:02 
 */

#include "VideoDecoder.h"

using namespace std;

/**
 * @brief: Defult constructor for VideoDecoder
 *          Initializes all the member data
 */
VideoDecoder::VideoDecoder() 
{ 
    // function call to initlialize member data
    initLocals();
}

/**
 * @brief: Parameterized Construtor for VideoDecoder
 *          Initializes input video filename and all the member data
 * 
 * @params: input video filename, defualt = ""
 */
VideoDecoder::VideoDecoder(string inpVideoFilePath) 
{ 
    // function call to initlialize member data
    initLocals();

    // open video with the input filename
    openVideo(inpVideoFilePath);
}

/**
 * @brief: dectructor
 */
VideoDecoder::~VideoDecoder() 
{
    // free packet if any
    if (m_avPkt.data)
        av_free_packet(&m_avPkt);

    // free frames if any
    if (m_avFrame)
    {
        av_free(m_avFrame);
        m_avFrame = NULL;
    }

    // function to close running video
    closeVideo();
}

/**
 * @brief: function to fetch input video information
 *
 * @params: videoInfo structure as reference
 */
int VideoDecoder::getVideoInfo(VideoInfo &videoInfo)
{
    // fill input filename
    videoInfo.videoFileName = m_inpFile;

    // fill input vidio width
    videoInfo.width = m_width;

    // fill input vidio height
    videoInfo.height = m_height;
}

/**
 * @brief: function to fetch a new frame from the video
 *
 * @params: unsigned char array pointer to fill frame data
 *
 * @return: returns -1 on failure, frame size on success
 */
int VideoDecoder::getNewFrame(unsigned char *frameArray)
{
    // function call to read and decoded frame
    if (readAndDecodeFrame() < 0)
        return -1; // read and decode failed

    // check for valid frame array pointer
    if (frameArray)
    {
        // initialize rgb frame array for conversion
        m_avFrameRGB.data[0] = (unsigned char *)frameArray;
        m_avFrameRGB.linesize[0] = m_width * 3;

        // initialize swsScale struct for rgb conversion
        struct SwsContext *swsContext = sws_getContext(m_width, m_height,
                           m_avStream->codec->pix_fmt, m_width, m_height,
                           PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

        // if initalization was success
        if (!swsContext)
            return -1; // initialization failed

        // input format to rgb24 conversion
        sws_scale(swsContext, m_avFrame->data, m_avFrame->linesize, 0,
                    m_avStream->codec->height, m_avFrameRGB.data, m_avFrameRGB.linesize);
       
        // free swscontext 
        if (swsContext)
           sws_freeContext(swsContext);
    }

    // return read frame size
    return m_avPkt.size;
}

/**
 * @brief: function to read and decode frame
 *
 * @params: none
 *
 * @return: return -1 on failure, 0 on success 
 */
int VideoDecoder::readAndDecodeFrame()
{
    // check for valid stream 
    if (m_avStream == NULL)
        return -1; // return failure

    // check for valid frame data and free if already filled
    if (m_avPkt.data != NULL)
    {
        // free packet
        av_free_packet(&m_avPkt);
        m_avPkt.data = NULL;
    }

    // flag to check if a complete frame is read
    int frameFinished = 0;

    // loop until a complete frame is read
    while (!frameFinished && (av_read_frame(m_avFmtCtx, &m_avPkt) >= 0))
    {
        // check for valid stream index to decode frame
        if (m_avPkt.stream_index == m_streamIndex)
        {
#ifdef FFMPEG_2_7_6

            // decode read frame
            avcodec_decode_video2(m_avCodecCtx, m_avFrame, &frameFinished, 
                                    &m_avPkt);
#else
            // decode read frame
            avcodec_decode_video(m_avCodecCtx, m_avFrame, &frameFinished, 
                                    m_avPkt.data, m_avPkt.size);
#endif
        }
    }

    return 0; // return success
}

/**
 * @brief: function to initialize mebmer data
 */
void VideoDecoder::initLocals()
{
    // input video file
    m_inpFile = "";

    // input format context
    m_avFmtCtx = NULL;

    // input codec context
    m_avCodecCtx = NULL;

    // input codec
    m_avCodec = NULL;

    // input stream
    m_avStream = NULL;

    // frame
    m_avFrame = NULL;

    // packet
    m_avPkt.data = NULL;

    // packet size
    m_avPkt.size = -1;

    // input video width
    m_width = -1;

    // input video height
    m_height = -1;

    // stream index
    m_streamIndex = -1;

    // input video frame rate
    m_frameRate = -1;

    // total frame in video
    m_totalFrames = -1;

    // total duration of video
    m_totalDuration = -1;
}

/**
 * @brief: function to open input video
 *
 * @params: input video filepath
 *
 * @return: return -1 on failure, 0 on success
 */
int VideoDecoder::openVideo(string inpVideoFilePath)
{
    // set video file-path
    if (!inpVideoFilePath.empty())
        m_inpFile = inpVideoFilePath;

    // register all the resources required from ffmpeg 
    av_register_all();

    fprintf(stderr, "\x1b[33m" "VideoDecoder:: Opening video: %s\n" "\x1b[0m", m_inpFile.c_str());
#ifdef FFMPEG_2_7_6 
    //AVDictionary *opts = 0;
    
    // if open video with format context and video filename is success
    if (avformat_open_input(&m_avFmtCtx, (char *)m_inpFile.c_str(), NULL, NULL) != 0)
#else
    // if open video with format context and video filename is success
    if (av_open_input_file(&m_avFmtCtx, (char *)m_inpFile.c_str(), NULL, 0, NULL) != 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not open video: %s\n" "\x1b[0m", 
                                                            m_inpFile.c_str());
        return -1; // return failure
    }

#ifdef FFMPEG_2_7_6 
    // find stream info from format context
    if (avformat_find_stream_info(m_avFmtCtx, NULL) < 0)
#else
    // find stream info from format context
    if (av_find_stream_info(m_avFmtCtx) < 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not find stream!!" "\x1b[0m");
        return -1; // return failure
    }

    // loop for all stream
    for (int i = 0; i < m_avFmtCtx->nb_streams; i++)
    {
#ifdef FFMPEG_2_7_6
        // find stream index
        if (m_avFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#else
        // find stream index
        if (m_avFmtCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
#endif
        {
            // stream index found
            m_streamIndex = i;
            break;
        }
    }

    // if stream index not found
    if (m_streamIndex == -1)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not find stream index!!" "\x1b[0m");
        return -1; // return failure
    }

    // set stream, from above found stream index
    m_avStream = m_avFmtCtx->streams[m_streamIndex];

    // set codec context
    m_avCodecCtx = m_avFmtCtx->streams[m_streamIndex]->codec;

    // set codec
    m_avCodec = avcodec_find_decoder(m_avCodecCtx->codec_id);

    // check for codec initialized
    if (m_avCodec == NULL)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Codec not supposted!!" "\x1b[0m");
        return -1; // return failure
    }
#ifdef FFMPEG_2_7_6
    // open video with codec context and codec
    if (avcodec_open2(m_avCodecCtx, m_avCodec, NULL) < 0)
#else
    // open video with codec context and codec
    if (avcodec_open(m_avCodecCtx, m_avCodec) < 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not open codec!!" "\x1b[0m");
        return -1; // return failure
    }

    // set total no of frame in video
    m_totalFrames = m_avStream->nb_frames;
   
    // allocate memory to frame 
    m_avFrame = avcodec_alloc_frame();
    
    // check if allocation was success
    if (m_avFrame == NULL)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not alloc memory to frame" "\x1b[0m");
        return -1; // return failure
    }

    // set video width and height
    m_width = m_avCodecCtx->width;
    m_height = m_avCodecCtx->height;

    // set total duration of video
    m_totalDuration = m_avFmtCtx->duration;
   
        
    fprintf(stderr, "\x1b[32m" "VideoDecoder:: Video open success!!\n" "\x1b[0m");

    return 0; // return success
}

/**
 * @brief: function to close opened video
 */
void VideoDecoder::closeVideo()
{
    // close if stream is valid
    if (m_avStream)
    {
        avcodec_close(m_avStream->codec);
        m_avStream = NULL;
    }

    // close input format context
    if (m_avFmtCtx)
    {
#ifdef FFMPEG_2_7_6
        avformat_close_input(&m_avFmtCtx);
#else
        av_close_input_file(m_avFmtCtx);
#endif
        m_avFmtCtx = NULL;
        fprintf(stderr, "\x1b[32m" "VideoDecoder:: Video close success!!\n" "\x1b[0m");
    }
}
