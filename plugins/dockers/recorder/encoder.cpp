#include "encoder.h"
#include <kis_debug.h>

void Encoder::init(const std::string& filename, int width, int height)
{
    av_register_all();
    avcodec_register_all();

    m_filename = filename;

    m_outputFormat = av_guess_format(nullptr, m_filename.c_str(), nullptr);
    if (!m_outputFormat) {
        errPlugins << "can't create output format";
        return;
    }

    int err = avformat_alloc_output_context2(&m_formatContext, m_outputFormat, nullptr, filename.c_str());

    if (err) {
        errPlugins << "can't create output context";
        return;
    }

    AVCodec* codec = nullptr;

    codec = avcodec_find_encoder(m_outputFormat->video_codec);
    if (!codec) {
        errPlugins << "can't create codec";
        return;
    }

    m_stream = avformat_new_stream(m_formatContext, codec);

    if (!m_stream) {
        errPlugins << "can't find format";
        return;
    }

    m_codecContext = avcodec_alloc_context3(codec);

    if (!m_codecContext) {
        errPlugins << "can't create codec context";
        return;
    }

    m_stream->codecpar->codec_id = m_outputFormat->video_codec;
    m_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    m_stream->codecpar->width = width;
    m_stream->codecpar->height = height;
    m_stream->codecpar->format = AV_PIX_FMT_YUV420P;
    m_stream->codecpar->bit_rate = m_bitrate * 1000;
    m_stream->avg_frame_rate = (AVRational){m_fps, 1};
    avcodec_parameters_to_context(m_codecContext, m_stream->codecpar);
    m_codecContext->time_base = (AVRational){1, 1};
    m_codecContext->max_b_frames = 2;
    m_codecContext->gop_size = 12;
    m_codecContext->framerate = (AVRational){m_fps, 1};

    av_opt_set_int(m_codecContext, "lossless", 1, 0);

    avcodec_parameters_from_context(m_stream->codecpar, m_codecContext);

    if ((err = avcodec_open2(m_codecContext, codec, NULL)) < 0) {
        errPlugins << "Failed to open codec: " << err;
        return;
    }

    if (!(m_outputFormat->flags & AVFMT_NOFILE)) {
        if ((err = avio_open(&m_formatContext->pb, m_filename.c_str(), AVIO_FLAG_WRITE)) < 0) {
            errPlugins << "Failed to open file: " << err;
            return;
        }
    }

    if ((err = avformat_write_header(m_formatContext, NULL)) < 0) {
        errPlugins << "Failed to write header" << err;
        return;
    }

    av_dump_format(m_formatContext, 0, m_filename.c_str(), 1);
    m_frameCount = 0;
}

void Encoder::pushFrame(uint8_t* data)
{
    int err;
    if (!m_frame) {
        m_frame = av_frame_alloc();
        m_frame->format = AV_PIX_FMT_YUV420P;
        m_frame->width = m_codecContext->width;
        m_frame->height = m_codecContext->height;

        if ((err = av_frame_get_buffer(m_frame, 32)) < 0) {
            errPlugins << "Failed to allocate picture" << err;
            return;
        }
    }

    if (!m_swsContext) {
        m_swsContext =
            sws_getContext(m_codecContext->width, m_codecContext->height, AV_PIX_FMT_BGRA, m_codecContext->width,
                           m_codecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
    }

    int inLinesize[1] = {4 * m_codecContext->width};

    // From RGB to YUV
    sws_scale(m_swsContext, (const uint8_t* const*)&data, inLinesize, 0, m_codecContext->height, m_frame->data,
              m_frame->linesize);

    m_frame->pts = (m_frameCount++) * m_stream->time_base.den / (m_stream->time_base.num * m_fps);

    if ((err = avcodec_send_frame(m_codecContext, m_frame)) < 0) {
        errPlugins << "Failed to send frame" << err;
        return;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
    pkt.flags |= AV_PKT_FLAG_KEY;
    int ret = 0;
    if ((ret = avcodec_receive_packet(m_codecContext, &pkt)) == 0) {
        av_interleaved_write_frame(m_formatContext, &pkt);
    //    av_packet_unref(&pkt);
        infoPlugins << "Write frame: " << m_frameCount;
        qDebug() << "Write frame: " << m_frameCount;
    }
    qDebug() << "Write push: " << m_frameCount << ret;
    av_packet_unref(&pkt);
}

void Encoder::finish()
{
    qDebug() << "Encoder finish";
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    for (;;) {
        avcodec_send_frame(m_codecContext, NULL);
        if (avcodec_receive_packet(m_codecContext, &pkt) == 0) {
            av_interleaved_write_frame(m_formatContext, &pkt);
            qDebug() << "final push" ;
        } else {
            break;
        }
    }

    av_packet_unref(&pkt);

    av_write_trailer(m_formatContext);
    if (!(m_outputFormat->flags & AVFMT_NOFILE)) {
        int err = avio_close(m_formatContext->pb);
        if (err < 0) {
            errPlugins << "Failed to close file" << err;
        }
    }
    infoPlugins << "finished: " << QString::fromStdString(m_filename);
    m_frameCount = 0;

    if (m_frame) {
        av_frame_free(&m_frame);
        m_frame = nullptr;
    }
    if (m_codecContext) {
        avcodec_close(m_codecContext);
        avcodec_free_context(&m_codecContext);
        m_codecContext = nullptr;
        m_stream = nullptr;
    }
    if (m_formatContext) {
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
    }
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    if (m_outputFormat) {
        // no need to free this.
        m_outputFormat = nullptr;
    }
}
