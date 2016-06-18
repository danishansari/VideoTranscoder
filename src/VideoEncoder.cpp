/**
 * Description: VideoEncoder Class
 *              Encode Frames to video file
 *
 * Author: Md Danish
 *
 * Date: 2016-06-05 11:19:02 
 */

#include "VideoEncoder.h"

/**
 * @brief: Default constructor for video encoder
 *          Initialize all the member data
 */
VideoEncoder::VideoEncoder() 
{
    // function call to initlialize all data member
    initLocals();
}

/**
 * @brief: Parametarized constructor for video encoder
 * 
 * @params: VideoEncoderContext
 */
VideoEncoder::VideoEncoder(const struct VideoEncoderContext encoderContext) 
{
    // function call to initlialize all data member 
    initLocals();
    
    // function call to set encoder context
    setEncoderContext(encoderContext);
}

/**
 * @brief: destructor for video encoder
 *          clean allocated member data
 */
VideoEncoder::~VideoEncoder()
{
    // function call to clean allocated member data
    cleanEncoder();
}

/**
 * @brief: Function to set encoder context
 *
 * @params: VideoEncoderContext
 */
void VideoEncoder::setEncoderContext(const struct VideoEncoderContext encoderContext) 
{
    // setting encoder context
    m_encoderContext = encoderContext;

    // setting flag for encoder context
    m_encoderCtxSet = 1;
}

/**
 * @brief: Function to check if encoder context is set
 *
 * @return: returns encoder context flag
 */
int VideoEncoder::encoderCtxSet() 
{
    return m_encoderCtxSet;
}

/**
 * @brief: function to startVideoEncode
 *
 * @return:  return status returned from initialize video
 */
int VideoEncoder::startVideoEncode() 
{
    // function call to initialize video
    return initializeVideo();
}

/**
 * @brief: function to stop video encode
 *
 * @return: 0
 */
int VideoEncoder::stopVideoEncode() 
{
    // function call to finalize video
    finalizeVideo();
    return 0;
}

/**
 * @brief: Function to add new frames to video
 *
 * @params: frame array to add to video
 *
 * @return: returns-1 on failure, 0 on success
 */
int VideoEncoder::addNewFrame(unsigned char *frameArr) 
{
    // allocate memory to frame
    AVFrame *avFrame = avcodec_alloc_frame();

    // check if allocation was success and stream was initialized
    if (!avFrame || !m_avStream)
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Could not initialize frame/stream!!\n" "\x1b[0m");
        return -1; // return failure
    }

    // set width and height
    int width = m_encoderContext.width;
    int height = m_encoderContext.height;

    // fill data to frame
    avpicture_fill((AVPicture *)avFrame, (unsigned char*)frameArr,
            PIX_FMT_RGB24, width, height);

    // check if member frame is initialized
    if (!m_avFrame) 
        m_avFrame = allocFrame(); // allocate frame

    // set conversion context for conversion from rgb
    struct SwsContext* swsContext = sws_getContext(width, height,
                            (::PixelFormat) PIX_FMT_RGB24, width, height,
                            (::PixelFormat)m_avStream->codec->pix_fmt, 
                            SWS_BICUBIC, NULL, NULL, NULL);

    // check if sws context was set
    if (!swsContext) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Could not get scale context\n" "\x1b[0m");
        return -1;
    }

    // convert rgb to required format
    sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, 
            m_avStream->codec->height, m_avFrame->data, m_avFrame->linesize);

    // release conversion context
    sws_freeContext(swsContext);

    // function to add new frame
    return addFrame();
}

int VideoEncoder::addFrame() 
{
    int retStatus = -1;

    if (!m_avStream || !m_avFrame) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Frame/Stream not initialized!!\n" "\x1b[0m");
        return retStatus;
    }

    if (m_avStream->index != 0 || m_avStream->id != 0) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Invalid Video Stream index/id \n" "\x1b[0m");
        return retStatus;
    }

    AVCodecContext* avCodecCtx = m_avStream->codec;

    if (m_avFmtCtx->oformat->flags & AVFMT_RAWPICTURE) 
    {
        AVPacket avPkt;
        av_init_packet(&avPkt);

        //avPkt.flags |= PKT_FLAG_KEY;
        avPkt.stream_index = m_avStream->index;
        avPkt.data = (uint8_t *)m_avFrame;
        avPkt.size = sizeof(AVPicture);

        retStatus = av_write_frame(m_avFmtCtx, &avPkt);
    } 
    else 
    {
        if (avCodecCtx && avCodecCtx->width == m_encoderContext.width &&
                          avCodecCtx->height == m_encoderContext.height) 
        {
            m_avFrame->quality = avCodecCtx->global_quality;

            int size = avcodec_encode_video(avCodecCtx, m_pictureOutBuf, 
                                            m_pictureOutBufSize, m_avFrame);

            if (size >= 0) 
            {
                AVPacket avPkt;
                av_init_packet(&avPkt);

                avPkt.stream_index = m_avStream->index;
                avPkt.data = m_pictureOutBuf;
                avPkt.size = size;

                retStatus = av_write_frame(m_avFmtCtx, &avPkt);

                if (retStatus >= 0)
                    retStatus = size;
            } 
            else 
            {
                fprintf(stderr, "\x1b[31m" "VideoEncoder:: Could not encode frame!!\n" "\x1b[0m");
                retStatus = 0;
            }
        } 
        else
        {
            fprintf(stderr, "\x1b[31m" "VideoEncoder:: Error \n" "\x1b[0m");
        }
    }

    if (retStatus < 0) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Error in encoding frame..\n" "\x1b[0m");
    }

    m_frameCount++;

    return retStatus;
}

void VideoEncoder::initLocals() 
{
    m_avOutFmt = NULL;
    m_avFmtCtx = NULL;
    m_avStream = NULL;
    m_avFrame = NULL;
    m_pictureInpBuf = NULL;
    m_frameCount = 0;
    m_streamIdx = -1;
    m_pictureOutBuf = NULL;
    m_pictureOutBufSize = 0;
    m_encoderCtxSet = 0;

    av_register_all();
    av_log_set_level(AV_LOG_QUIET);
}

AVStream* VideoEncoder::addStream() 
{
#ifdef FFMPEG_2_7_6
    AVStream *avStream = avformat_new_stream(m_avFmtCtx, NULL);
#else
    AVStream *avStream = av_new_stream(m_avFmtCtx, 0);
#endif
    
    if (!avStream) 
    {
        fprintf(stderr, "Vencoder :: Could not alloc video stream\n");
        return NULL;
    }

    AVCodecContext *avCodecCtx = avStream->codec;

#ifdef FFMPEG_2_7_6
    avCodecCtx->codec_id = (AVCodecID)m_avOutFmt->video_codec;
    avCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    avCodecCtx->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL;
#else
    avCodecCtx->codec_id = (CodecID)m_avOutFmt->video_codec;
    avCodecCtx->codec_type = CODEC_TYPE_VIDEO;
    avCodecCtx->strict_std_compliance = FF_COMPLIANCE_INOFFICIAL;
#endif

    avCodecCtx->bit_rate = 400000;
    avCodecCtx->width = m_encoderContext.width;
    avCodecCtx->height = m_encoderContext.height;

    avCodecCtx->time_base.den = m_encoderContext.frameRate;
    avCodecCtx->time_base.num = 1;
    avCodecCtx->gop_size = 12;   

    avCodecCtx->pix_fmt = PIX_FMT_YUV420P;

    if (m_encoderContext.quality) 
    {
        avCodecCtx->flags |= CODEC_FLAG_QSCALE;
#ifdef FFMPEG_2_7_6
        avCodecCtx->global_quality = FF_QP2LAMBDA * m_encoderContext.quality;
#else
        avCodecCtx->global_quality = avStream->quality = 
            FF_QP2LAMBDA * m_encoderContext.quality; 
#endif
    }

    //if (avCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO) 
    //    avCodecCtx->max_b_frames = 2;

    //if (avCodecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
    //    avCodecCtx->mb_decision=2;

    if (avCodecCtx->codec_id == CODEC_ID_H264)
    {
        avCodecCtx->i_quant_factor = 0.71;
#ifdef FFMPEG_2_7_6
#else
        avCodecCtx->crf = 18; 
#endif
        avCodecCtx->trellis = 1;
        avCodecCtx->qmin = 1;
        avCodecCtx->qmax = 26;
        avCodecCtx->max_qdiff = 4;
        avCodecCtx->max_b_frames = 3;
        avCodecCtx->me_method = ME_HEX;
        avCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
        avCodecCtx->flags |= CODEC_FLAG_LOOP_FILTER;
        avCodecCtx->ticks_per_frame = 2;
    }

    return avStream;
}

void VideoEncoder::openVideo() 
{
    AVCodecContext *avCodecCtx = m_avStream->codec;

    AVCodec *avCodec = avcodec_find_encoder(avCodecCtx->codec_id);

    if (!avCodec) 
    {
        fprintf(stderr, "Vencoder :: video codec not found\n");
        return;
    }
#ifdef FFMPEG_2_7_6
    if (avcodec_open2(avCodecCtx, avCodec, NULL) < 0) 
#else
    if (avcodec_open(avCodecCtx, avCodec) < 0) 
#endif
    {
        fprintf(stderr, "Vencoder :: could not open video codec.\n");
        return;
    }

    m_pictureOutBuf = NULL;

    if (!(m_avFmtCtx->oformat->flags & AVFMT_RAWPICTURE)) 
    {

        m_pictureOutBufSize = 2000000;
        if (!m_pictureOutBuf) 
        {
            m_pictureOutBuf = (uint8_t*)malloc(m_pictureOutBufSize);
        }
    }
}

AVFrame* VideoEncoder::allocFrame()
{
    AVFrame *avFrame = avcodec_alloc_frame();

    if (!avFrame || !m_avStream)
    {
        fprintf(stderr, "Vencoder :: video frame/stream not initalized!!\n");
        return NULL;
    }

    int size = avpicture_get_size((::PixelFormat)m_avStream->codec->pix_fmt, 
                                 m_encoderContext.width, m_encoderContext.height);

    if (m_pictureInpBuf) 
    {
        free(m_pictureInpBuf);
        m_pictureInpBuf = NULL;
    }

    m_pictureInpBuf = (uint8_t*)malloc(size);

    if (!m_pictureInpBuf) 
    {
        av_free(avFrame);
        return NULL;
    }

    avpicture_fill((AVPicture *)avFrame, m_pictureInpBuf,
                    (::PixelFormat)m_avStream->codec->pix_fmt,
                    m_encoderContext.width, m_encoderContext.height);

    return avFrame;
}

int VideoEncoder::initializeVideo() 
{
    if (initEncoder() != 0) 
        return -1;

    fprintf(stderr, "\x1b[32m" "VideoEncoder:: Video initialize success!!\n" "\x1b[0m");
    return 0;
}

int VideoEncoder::initEncoder() 
{
    cleanEncoder();

    char *outputFile = (char *)m_encoderContext.outputVideoFile.c_str();

    m_frameCount = 0;

#ifdef FFMPEG_2_7_6
    m_avOutFmt = av_guess_format(NULL, outputFile, NULL);
#else
    m_avOutFmt = guess_format(NULL, outputFile, NULL);
#endif

    if (!m_avOutFmt) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Output Format not initialized!!\n" "\x1b[0m");
        return -1;
    }

    m_avOutFmt->video_codec = CODEC_ID_NONE;

    if (!strcmp(m_encoderContext.codecStr.c_str(), "H264"))
        m_avOutFmt->video_codec = CODEC_ID_H264;
    else if (!strcmp(m_encoderContext.codecStr.c_str(), "MPEG-4"))
        m_avOutFmt->video_codec = CODEC_ID_MSMPEG4V2;
    else
    {
        m_avOutFmt->video_codec = CODEC_ID_H264;
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Codec not supported, Using H264\n" "\x1b[0m");

        //return -1;
    }

    m_avFmtCtx = avformat_alloc_context();

    if (!m_avFmtCtx) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Format Context not initialized!!\n" "\x1b[0m");
        return -1;
    }

    m_avFmtCtx->oformat = m_avOutFmt;

    snprintf(m_avFmtCtx->filename, sizeof(m_avFmtCtx->filename), "%s", 
            outputFile);

    if (m_avOutFmt->video_codec != CODEC_ID_NONE) 
    {
        m_avStream = addStream();
    }

    if (m_avStream) 
    {
        openVideo();
    } 
    else 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Stream not initialized!!\n" "\x1b[0m");
    }

    if (!(m_avOutFmt->flags & AVFMT_NOFILE)) 
    {
#ifdef FFMPEG_2_7_6
        if (avio_open(&m_avFmtCtx->pb, outputFile, AVIO_FLAG_WRITE) < 0)
#else
        if (url_fopen(&m_avFmtCtx->pb, outputFile, URL_WRONLY) < 0)
#endif
            fprintf(stderr, "\x1b[31m" "VideoEncoder:: Error in opening url\n" "\x1b[0m");
    }

#ifdef FFMPEG_2_7_6
    avformat_write_header(m_avFmtCtx, NULL);
#else
    av_write_header(m_avFmtCtx);
#endif

    return 0;
}

void VideoEncoder::finalizeVideo() 
{
    if (m_avStream && m_avStream->index == 0 && m_avStream->id == 0) 
    {
        av_write_trailer(m_avFmtCtx);

        AVCodecContext* avCodecCtx = m_avStream->codec;

        if (avCodecCtx && avCodecCtx->width == m_encoderContext.width &&
                          avCodecCtx->height == m_encoderContext.height) 
        {
            if (m_avStream->codec->codec) 
            {
                avcodec_close(m_avStream->codec);
            } 
            else 
            {
                if (m_avStream->codec->priv_data) 
                    av_freep(&(m_avStream->codec->priv_data));

#ifdef FFMPEG_2_7_6
                avcodec_free_context(&m_avStream->codec);
#else
                avcodec_default_free_buffers(m_avStream->codec);
#endif
            }

            for(int i = 0; i<m_avFmtCtx->nb_streams; i++) 
            {
                av_freep(&m_avFmtCtx->streams[i]->codec);
                av_freep(&m_avFmtCtx->streams[i]);
            }

            if (!(m_avOutFmt->flags & AVFMT_NOFILE)) 
#ifdef FFMPEG_2_7_6
                 avio_close(m_avFmtCtx->pb);
#else
                url_fclose(m_avFmtCtx->pb);
#endif
        }
    }
    
    fprintf(stderr, "\x1b[32m" "VideoEncoder:: Video finalize success!!\n" "\x1b[0m");
}

void VideoEncoder::cleanEncoder() 
{
    if (m_pictureOutBuf) 
    {
        av_free(m_pictureOutBuf);
        m_pictureOutBuf = NULL;
        m_pictureOutBufSize = 0;
    }
}
