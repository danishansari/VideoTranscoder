
/**
 * Description:
 *
 * Author: Md Danish
 *
 * Date: 2016-06-02 11:19:02 
**/

#include "VideoDecoder.h"

VideoDecoder::VideoDecoder() 
{ 
    initLocals();
}

VideoDecoder::VideoDecoder(string inpVideoFilePath) 
{ 
    initLocals();

    openVideo(inpVideoFilePath);
}

VideoDecoder::~VideoDecoder() 
{
    // free packet if any
    if (m_avPkt.data)
        av_free_packet(&m_avPkt);

    if (m_avFrame)
    {
        av_free(m_avFrame);
        m_avFrame = NULL;
    }

    closeVideo();
}

int VideoDecoder::getVideoInfo(VideoInfo &videoInfo)
{
    videoInfo.videoFileName = m_inpFile;
    videoInfo.width = m_width;
    videoInfo.height = m_height;
}

int VideoDecoder::getNewFrame(unsigned char *frameArray)
{
    if (readAndDecodeFrame() < 0)
        return -1;

    if (frameArray)
    {
        m_avFrameRGB.data[0] = (unsigned char *)frameArray;
        m_avFrameRGB.linesize[0] = m_width * 3;

        struct SwsContext *swsContext = sws_getContext(m_width, m_height,
                           m_avStream->codec->pix_fmt, m_width, m_height,
                           PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

        if (!swsContext)
            return -1;

        sws_scale(swsContext, m_avFrame->data, m_avFrame->linesize, 0,
                    m_avStream->codec->height, m_avFrameRGB.data, m_avFrameRGB.linesize);
        
        if (swsContext)
           sws_freeContext(swsContext);
    }

    return m_avPkt.size;
}

int VideoDecoder::readAndDecodeFrame()
{
    if (m_avStream == NULL)
        return -1;

    if (m_avPkt.data != NULL)
    {
        av_free_packet(&m_avPkt);
        m_avPkt.data = NULL;
    }

    int frameFinished = 0;

    while (!frameFinished && (av_read_frame(m_avFmtCtx, &m_avPkt) >= 0))
    {
        if (m_avPkt.stream_index == m_streamIndex)
        {
//#if 1 //LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
#ifdef FFMPEG_2_7_6

            avcodec_decode_video2(m_avCodecCtx, m_avFrame, &frameFinished, 
                                    &m_avPkt);
#else
            avcodec_decode_video(m_avCodecCtx, m_avFrame, &frameFinished, 
                                    m_avPkt.data, m_avPkt.size);
#endif
        }
    }
}

void VideoDecoder::initLocals()
{
    m_inpFile = "";
    m_avFmtCtx = NULL;
    m_avCodecCtx = NULL;
    m_avCodec = NULL;
    m_avStream = NULL;
    m_avFrame = NULL;
    m_avPkt.data = NULL;
    m_avPkt.size = -1;
    m_width = -1;
    m_height = -1;
    m_streamIndex = -1;
    m_frameRate = -1;
    m_totalFrames = -1;
    m_totalDuration = -1;
}

int VideoDecoder::openVideo(string inpVideoFilePath)
{
    int retStatus = -1;

    // set video file-path
    if (!inpVideoFilePath.empty())
        m_inpFile = inpVideoFilePath;

    av_register_all();

    fprintf(stderr, "\x1b[33m" "VideoDecoder:: Opening video: %s\n" "\x1b[0m", m_inpFile.c_str());
#ifdef FFMPEG_2_7_6 
    //AVDictionary *opts = 0;

    if (avformat_open_input(&m_avFmtCtx, (char *)m_inpFile.c_str(), NULL, NULL) != 0)
#else
    // open video file
    if (av_open_input_file(&m_avFmtCtx, (char *)m_inpFile.c_str(), NULL, 0, NULL) != 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not open video: %s\n" "\x1b[0m", 
                                                            m_inpFile.c_str());
        return retStatus;
    }

#ifdef FFMPEG_2_7_6 // find stream info
    if (avformat_find_stream_info(m_avFmtCtx, NULL) < 0)
#else
    if (av_find_stream_info(m_avFmtCtx) < 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not find stream!!" "\x1b[0m");
        return retStatus;
    }

    // find stream index
    for (int i = 0; i < m_avFmtCtx->nb_streams; i++)
    {
#ifdef FFMPEG_2_7_6
        if (m_avFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
#else
        if (m_avFmtCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
#endif
        {
            m_streamIndex = i;
            break;
        }
    }

    if (m_streamIndex == -1)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not find stream index!!" "\x1b[0m");
        return retStatus;
    }

    m_avStream = m_avFmtCtx->streams[m_streamIndex];

    m_avCodecCtx = m_avFmtCtx->streams[m_streamIndex]->codec;

    m_avCodec = avcodec_find_decoder(m_avCodecCtx->codec_id);

    if (m_avCodec == NULL)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Codec not supposted!!" "\x1b[0m");
        return retStatus;
    }
#ifdef FFMPEG_2_7_6
    if (avcodec_open2(m_avCodecCtx, m_avCodec, NULL) < 0)
#else
    if (avcodec_open(m_avCodecCtx, m_avCodec) < 0)
#endif
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not open codec!!" "\x1b[0m");
        return retStatus;
    }

    m_totalFrames = m_avStream->nb_frames;
    
    m_avFrame = avcodec_alloc_frame();
    
    if (m_avFrame == NULL)
    {
        fprintf(stderr, "\x1b[31m" "VideoDecoder:: Could not alloc memory to frame" "\x1b[0m");
        return retStatus;
    }

    m_width = m_avCodecCtx->width;
    m_height = m_avCodecCtx->height;

    m_totalDuration = m_avFmtCtx->duration;
    
    retStatus = 0;
        
    fprintf(stderr, "\x1b[32m" "VideoDecoder:: Video open success!!\n" "\x1b[0m");

    return retStatus;
}

void VideoDecoder::closeVideo()
{
    if (m_avStream)
    {
        avcodec_close(m_avStream->codec);
        m_avStream = NULL;
    }

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
