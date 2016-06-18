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

/**
 * @brief: Function to write frames to the video
 *
 * @return: return -1 on failure and 0 on success
 */
int VideoEncoder::addFrame() 
{
    // return status of add frame, failure = -1 and 0, success >= 0
    int retStatus = -1;

    // check if stream and frame are initialized
    if (!m_avStream || !m_avFrame) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Frame/Stream not initialized!!\n" "\x1b[0m");
        return retStatus; // return failure
    }

    // check if stream index and id are valid
    if (m_avStream->index != 0 || m_avStream->id != 0) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Invalid Video Stream index/id \n" "\x1b[0m");
        return retStatus; // return failure
    }

    // initialize local codec contex
    AVCodecContext* avCodecCtx = m_avStream->codec;
    
    // check it output format is RAW
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
    else // encode data
    {
        // check if codec contex is initilized and width/height is set
        if (avCodecCtx && avCodecCtx->width == m_encoderContext.width &&
                          avCodecCtx->height == m_encoderContext.height) 
        {
            // set frame quality
            m_avFrame->quality = avCodecCtx->global_quality;

            // encode video into frame
            int size = avcodec_encode_video(avCodecCtx, m_pictureOutBuf, 
                                            m_pictureOutBufSize, m_avFrame);

            // if encoding was success, write data into video file
            if (size >= 0) 
            {
                // initialize pkt
                AVPacket avPkt;
                av_init_packet(&avPkt);

                avPkt.stream_index = m_avStream->index;
                avPkt.data = m_pictureOutBuf;
                avPkt.size = size;

                // write frame
                retStatus = av_write_frame(m_avFmtCtx, &avPkt);

                // check if writting was success
                if (retStatus >= 0)
                    retStatus = size;
            } 
            else // encoding was not successful
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

    // check if writing was success
    if (retStatus < 0) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Error in encoding frame..\n" "\x1b[0m");
    }

    // increament frame count
    m_frameCount++;

    // return write success
    return retStatus;
}

/**
 * @brief: function to initialize all data member
 */
void VideoEncoder::initLocals() 
{
    // output format
    m_avOutFmt = NULL;

    // format context
    m_avFmtCtx = NULL;

    // stream
    m_avStream = NULL;

    // frame
    m_avFrame = NULL;

    // picture input buf
    m_pictureInpBuf = NULL;

    // frame count
    m_frameCount = 0;

    // stream index
    m_streamIdx = -1;

    // picture output buf
    m_pictureOutBuf = NULL;

    // picture out buf size
    m_pictureOutBufSize = 0;

    // encoder context flag
    m_encoderCtxSet = 0;

    // register ffmpeg resources
    av_register_all();

    // remove warning logs
    av_log_set_level(AV_LOG_QUIET);
}

/**
 * @brief: Function to add stream to the video
 *
 * @return: return AVStream on success, NULL on failure
 */
AVStream* VideoEncoder::addStream() 
{
#ifdef FFMPEG_2_7_6
    // allocate stream 
    AVStream *avStream = avformat_new_stream(m_avFmtCtx, NULL);
#else
    // allocate stream
    AVStream *avStream = av_new_stream(m_avFmtCtx, 0);
#endif
    
    // check if stream was initilized
    if (!avStream) 
    {
        fprintf(stderr, "Vencoder :: Could not alloc video stream\n");
        return NULL; // return failure
    }

    // create codec context
    AVCodecContext *avCodecCtx = avStream->codec;

#ifdef FFMPEG_2_7_6
    // set codec id
    avCodecCtx->codec_id = (AVCodecID)m_avOutFmt->video_codec;

    // set codec type
    avCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

    // set compilance
    avCodecCtx->strict_std_compliance = FF_COMPLIANCE_UNOFFICIAL;
#else
    // set codec id
    avCodecCtx->codec_id = (CodecID)m_avOutFmt->video_codec;

    // set codec type
    avCodecCtx->codec_type = CODEC_TYPE_VIDEO;

    // set compilance
    avCodecCtx->strict_std_compliance = FF_COMPLIANCE_INOFFICIAL;
#endif

    // set bit rate
    avCodecCtx->bit_rate = 400000;

    // set codec context width
    avCodecCtx->width = m_encoderContext.width;

    // set codec context height
    avCodecCtx->height = m_encoderContext.height;

    // set codec context denominator
    avCodecCtx->time_base.den = m_encoderContext.frameRate;

    // codec context numerator
    avCodecCtx->time_base.num = 1;

    // set gop size
    avCodecCtx->gop_size = 12;   

    // set pixel format
    avCodecCtx->pix_fmt = PIX_FMT_YUV420P;

    // set flag for encoder quality
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

    // if codec = H264, set required params
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

    // return stream
    return avStream;
}

/**
 * @brief: Function to openvideo
 */
int VideoEncoder::openVideo() 
{
    // check if stream was initialized
    if (!m_avStream)
    {
        fprintf(stderr, "Vencoder :: video stream not found\n");
        return -1; // return failure
    }

    // create codec context
    AVCodecContext *avCodecCtx = m_avStream->codec;

    // create codec
    AVCodec *avCodec = avcodec_find_encoder(avCodecCtx->codec_id);

    // check if codec was initialized 
    if (!avCodec) 
    {
        fprintf(stderr, "Vencoder :: video codec not found\n");
        return -1; // return failure
    }
#ifdef FFMPEG_2_7_6
    // open video with codec context and codec
    if (avcodec_open2(avCodecCtx, avCodec, NULL) < 0) 
#else
    // open video with codec context and codec
    if (avcodec_open(avCodecCtx, avCodec) < 0) 
#endif
    {
        fprintf(stderr, "Vencoder :: could not open video codec.\n");
        return -1; //  return failure
    }

    // reset picture output buff
    m_pictureOutBuf = NULL;

    // check if output format is not RAW
    if (!(m_avFmtCtx->oformat->flags & AVFMT_RAWPICTURE)) 
    {
        // MAX picture out buf size
        m_pictureOutBufSize = 2000000;

        // allocat picture buff if not allocated
        if (!m_pictureOutBuf) 
        {
            m_pictureOutBuf = (uint8_t*)malloc(m_pictureOutBufSize);
        }
    }

    return 0; // return success
}

/**
 * @brief: Function to allocate memory to frame
 *
 * @return: Pointer the frame on succes, NULL on failure
 */
AVFrame* VideoEncoder::allocFrame()
{
    // create frame and allocate memory 
    AVFrame *avFrame = avcodec_alloc_frame();

    // check if initialization was sucess and stream was initialized
    if (!avFrame || !m_avStream)
    {
        fprintf(stderr, "Vencoder :: video frame/stream not initalized!!\n");
        return NULL; // return faiture
    }

    // getting frame size
    int size = avpicture_get_size((::PixelFormat)m_avStream->codec->pix_fmt, 
                                 m_encoderContext.width, m_encoderContext.height);

    // check and clear input picture buf
    if (m_pictureInpBuf) 
    {
        free(m_pictureInpBuf);
        m_pictureInpBuf = NULL;
    }

    // allocate memory to picture buf
    m_pictureInpBuf = (uint8_t*)malloc(size);

    // check if memory was allocated successfully
    if (!m_pictureInpBuf) 
    {
        // free frame
        av_free(avFrame);
        return NULL; // return failure
    }

    // fill frame 
    avpicture_fill((AVPicture *)avFrame, m_pictureInpBuf,
                    (::PixelFormat)m_avStream->codec->pix_fmt,
                    m_encoderContext.width, m_encoderContext.height);

    // return frame
    return avFrame;
}

/**
 * @brief: Function to initialize video encoding
 *
 * @return: return -1 on failure and 0 on success
 */
int VideoEncoder::initializeVideo() 
{
    // init encoder
    if (initEncoder() != 0) 
        return -1; // return failure

    fprintf(stderr, "\x1b[32m" "VideoEncoder:: Video initialize success!!\n" "\x1b[0m");
    return 0; // return success
}

/**
 * @brief: Function to initialize encoder
 *
 * @return: returns -1 on failure and 0 on success
 */
int VideoEncoder::initEncoder() 
{
    // clear allocated spaces in encoder
    cleanEncoder();

    // get output filename
    char *outputFile = (char *)m_encoderContext.outputVideoFile.c_str();

    // encoder started, so reset frame count
    m_frameCount = 0;

#ifdef FFMPEG_2_7_6
    // guess encoder format
    m_avOutFmt = av_guess_format(NULL, outputFile, NULL);
#else
    // guess encoder format
    m_avOutFmt = guess_format(NULL, outputFile, NULL);
#endif

    // check if output format was initialized
    if (!m_avOutFmt) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Output Format not initialized!!\n" "\x1b[0m");
        return -1; // return failure
    }

    // set video codec
    m_avOutFmt->video_codec = CODEC_ID_NONE;

    // check for output video codec. H264 and MPEG-4 are supported
    if (!strcmp(m_encoderContext.codecStr.c_str(), "H264"))
        m_avOutFmt->video_codec = CODEC_ID_H264;
    else if (!strcmp(m_encoderContext.codecStr.c_str(), "MPEG-4"))
        m_avOutFmt->video_codec = CODEC_ID_MSMPEG4V2;
    else
    {
        m_avOutFmt->video_codec = CODEC_ID_H264;
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Codec not supported, Using H264\n" "\x1b[0m");
    }

    // acllocate memory format context member data
    m_avFmtCtx = avformat_alloc_context();

    // check if initialization was success
    if (!m_avFmtCtx) 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Format Context not initialized!!\n" "\x1b[0m");
        return -1; // return failure
    }

    // set output video format
    m_avFmtCtx->oformat = m_avOutFmt;

    // set output video filename
    snprintf(m_avFmtCtx->filename, sizeof(m_avFmtCtx->filename), "%s", 
                                                    outputFile);

    // check and add stream to the video
    if (m_avOutFmt->video_codec != CODEC_ID_NONE) 
    {
        m_avStream = addStream();
    }

    // if stream was set sucess, open video for encoding
    if (m_avStream) 
    {
        // open video for encoding
        openVideo();
    } 
    else // video not opened 
    {
        fprintf(stderr, "\x1b[31m" "VideoEncoder:: Video Stream not initialized!!\n" "\x1b[0m");
    }

    // check output format flag and open video url
    if (!(m_avOutFmt->flags & AVFMT_NOFILE)) 
    {
#ifdef FFMPEG_2_7_6
        // open video url
        if (avio_open(&m_avFmtCtx->pb, outputFile, AVIO_FLAG_WRITE) < 0)
#else
        // open video url
        if (url_fopen(&m_avFmtCtx->pb, outputFile, URL_WRONLY) < 0)
#endif
            fprintf(stderr, "\x1b[31m" "VideoEncoder:: Error in opening url\n" "\x1b[0m");
    }

#ifdef FFMPEG_2_7_6
    // write header information
    avformat_write_header(m_avFmtCtx, NULL);
#else
    // write header information
    av_write_header(m_avFmtCtx);
#endif

    return 0;
}
/**
 * @brief: Function to finalize output video
 */
void VideoEncoder::finalizeVideo() 
{
    // check if video stream was set successfuly
    if (m_avStream && m_avStream->index == 0 && m_avStream->id == 0) 
    {
        // write trailer
        av_write_trailer(m_avFmtCtx);

        // create codec context
        AVCodecContext* avCodecCtx = m_avStream->codec;

        // check if codec context was set succesfuly
        if (avCodecCtx && avCodecCtx->width == m_encoderContext.width &&
                          avCodecCtx->height == m_encoderContext.height) 
        {
            // if codec was set successfuly, close codec
            if (m_avStream->codec->codec) 
            {
                avcodec_close(m_avStream->codec);
            } 
            else 
            {
                // free codec data
                if (m_avStream->codec->priv_data) 
                    av_freep(&(m_avStream->codec->priv_data));

#ifdef FFMPEG_2_7_6
                // free codec
                avcodec_free_context(&m_avStream->codec);
#else
                // free codec
                avcodec_default_free_buffers(m_avStream->codec);
#endif
            }

            // free all stream and codec
            for(int i = 0; i<m_avFmtCtx->nb_streams; i++) 
            {
                av_freep(&m_avFmtCtx->streams[i]->codec);
                av_freep(&m_avFmtCtx->streams[i]);
            }

            // if video url was open, close it
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

/**
 * @brief: Function clean picture buffer
 */
void VideoEncoder::cleanEncoder() 
{
    // if output picture buf was allocated, free and set picture out buffer size to 0
    if (m_pictureOutBuf) 
    {
        av_free(m_pictureOutBuf);
        m_pictureOutBuf = NULL;
        m_pictureOutBufSize = 0;
    }
}
