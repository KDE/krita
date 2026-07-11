/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisLibavMediaEncoderRunnable.h"

#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QVBoxLayout>
#include <memory>

#include <klocalizedstring.h>

#include <kis_debug.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "KisLibavEncoderContext.h"

namespace KisLibav
{
// Same as IO_BUFFER_SIZE in ffmpeg's avio.c.
static constexpr int BUFFER_SIZE = 32768;

// GIF palette sizes, always 16x16 images in BGRA format.
static constexpr int GIF_PALETTE_DIMENSION = 16;
static constexpr int GIF_PALETTE_BYTES = 1024;

// FFMPEG API breakage. They added constness to a callback and changed
// their mind on which macro you use to check for it or something.
#if defined(ff_const59)
using WriteAvioPacketBuf = ff_const59 uint8_t *;
#elif FF_API_AVIO_WRITE_NONCONST
using WriteAvioPacketBuf = uint8_t *;
#else
using WriteAvioPacketBuf = const uint8_t *;
#endif
} // namespace KisLibav

class KisLibavMediaEncoderRunnable::Format : public KisMediaEncoderFormat
{
public:
    Format(int formatId)
        : KisMediaEncoderFormat()
        , m_formatId(formatId)
    {
    }

    int formatId() const
    {
        return m_formatId;
    }

    Type type() const override
    {
        return Type::LibavMediaEncoder;
    }

    QString key() const override
    {
        return keyForFormatId(m_formatId);
    }

    QString title() const override
    {
        return i18n("LibAV: %1", titleForFormatId(m_formatId));
    }

    QString extension() const override
    {
        return extensionForFormatId(m_formatId);
    }

    bool supportsAudio() const override
    {
        return false;
    }

    QString formatName() const
    {
        switch (m_formatId) {
        case FORMAT_GIF_PALETTE:
            return QStringLiteral("rawvideo");
        case FORMAT_GIF:
            return QStringLiteral("gif");
        case FORMAT_WEBP:
            return QStringLiteral("webp");
        case FORMAT_APNG:
            return QStringLiteral("apng");
        }
        return QString();
    }

    AVCodecID codecId() const
    {
        return codecIdForFormatId(m_formatId);
    }

    static AVCodecID codecIdForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_GIF_PALETTE:
            return AV_CODEC_ID_RAWVIDEO;
        case FORMAT_GIF:
            return AV_CODEC_ID_GIF;
        case FORMAT_WEBP:
            return AV_CODEC_ID_WEBP;
        case FORMAT_APNG:
            return AV_CODEC_ID_APNG;
        }
        return AV_CODEC_ID_NONE;
    }

    AVPixelFormat pixelFormat(bool frame) const
    {
        switch (m_formatId) {
        case FORMAT_GIF_PALETTE:
        case FORMAT_WEBP:
            return AV_PIX_FMT_BGRA;
        case FORMAT_GIF:
            return frame ? AV_PIX_FMT_BGRA : AV_PIX_FMT_PAL8;
        case FORMAT_APNG:
            return AV_PIX_FMT_RGBA;
        }
        return AV_PIX_FMT_NONE;
    }

    void setCodecParams(AVCodecContext *codecContext) const
    {
        switch (m_formatId) {
        case FORMAT_GIF_PALETTE:
        case FORMAT_GIF:
        case FORMAT_APNG:
            break;
        case FORMAT_WEBP:
            setOption(codecContext->priv_data, "lossless", "1");
            setOption(codecContext->priv_data, "preset", "drawing");
            setOption(codecContext->priv_data, "quality", "100");
            break;
        }
    }

    void setOutputParams(AVFormatContext *formatContext) const
    {
        switch (m_formatId) {
        case FORMAT_GIF_PALETTE:
        case FORMAT_GIF:
            break;
        case FORMAT_WEBP:
            setOption(formatContext->priv_data, "loop", "0");
            break;
        case FORMAT_APNG:
            setOption(formatContext->priv_data, "plays", "0");
            break;
        }
    }

    QByteArray filterDesc() const
    {
        switch (m_formatId) {
        case FORMAT_WEBP:
        case FORMAT_APNG:
            break;
        case FORMAT_GIF_PALETTE:
            return QByteArrayLiteral("palettegen=stats_mode=full");
        case FORMAT_GIF:
            return QByteArrayLiteral("[in][palette]paletteuse[out]");
        }
        return QByteArray();
    }

    QVector<AVPixelFormat> filterPixelFormats() const
    {
        switch (m_formatId) {
        case FORMAT_GIF:
            return {AV_PIX_FMT_PAL8, AV_PIX_FMT_NONE};
        default:
            return {AV_PIX_FMT_BGRA, AV_PIX_FMT_NONE};
        }
    }

    QWidget *createPreferencesWidget(const QVariantMap &preferences) const override
    {
        Q_UNUSED(preferences);
        return nullptr;
    }

    void resetPreferencesWidget(QWidget *widget) const override
    {
        Q_UNUSED(widget);
    }

    QVariantMap getPreferencesFromWidget(QWidget *widget) const override
    {
        Q_UNUSED(widget);
        return QVariantMap();
    }

private:
    static QString keyForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_GIF:
            return QStringLiteral("libav:gif");
        case FORMAT_WEBP:
            return QStringLiteral("libav:webp");
        case FORMAT_APNG:
            return QStringLiteral("libav:apng");
        }
        return QString();
    }

    static QString titleForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_GIF:
            return QStringLiteral("GIF");
        case FORMAT_WEBP:
            return QStringLiteral("WEBP");
        case FORMAT_APNG:
            return QStringLiteral("APNG");
        }
        return QString();
    }

    static QString extensionForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_GIF:
            return QStringLiteral("gif");
        case FORMAT_WEBP:
            return QStringLiteral("webp");
        case FORMAT_APNG:
            return QStringLiteral("apng");
        }
        return QString();
    }

    static void setOption(void *obj, const char *name, const char *value)
    {
        int result = av_opt_set(obj, name, value, AV_OPT_SEARCH_CHILDREN);
        if (result != 0) {
            warnFile.nospace() << "Error setting option " << name << " to " << value << ": " << av_err2str(result)
                               << " (" << result << ")";
        }
    }

    int m_formatId;
};

class KisLibavMediaEncoderRunnable::Context : public KisLibavEncoderContext
{
public:
    Context(KisLibavMediaEncoderRunnable *runnable, QString &outErrorMessage)
        : KisLibavEncoderContext(&outErrorMessage)
        , m_runnable(runnable)
    {
    }

    ~Context() override
    {
        if (m_formatContext && m_formatContext->pb) {
            avio_context_free(&m_formatContext->pb);
        }
        avfilter_graph_free(&m_graphContext);
        avfilter_inout_free(&m_filterInputs);
        avfilter_inout_free(&m_filterOutputs);
        avfilter_inout_free(&m_filterPaletteOutputs);
        av_packet_free(&m_packet);
        av_frame_free(&m_frame);
        av_frame_free(&m_filteredFrame);
        av_frame_free(&m_paletteFrame);
        avcodec_parameters_free(&m_codecParameters);
        avformat_free_context(m_formatContext);
        avcodec_free_context(&m_codecContext);
    }

    EncodeResult encode(const KisMediaEncoderWrapperSettings &settings,
                        const Format &format,
                        QByteArray &paletteData,
                        bool writePalette)
    {
        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        QString formatName = format.formatName();
        if (formatName.isEmpty()) {
            setInternalErrorMessage(QStringLiteral("Unknown format name"));
            return EncodeResult::Failed;
        }

        AVCodecID codecId = format.codecId();
        if (codecId == AV_CODEC_ID_NONE) {
            setInternalErrorMessage(QStringLiteral("Unknown codec"));
            return EncodeResult::Failed;
        }

        const AVOutputFormat *outputFormat = av_guess_format(formatName.toUtf8().constData(), nullptr, nullptr);
        if (!outputFormat) {
            setInternalErrorMessage(QStringLiteral("av_guess_format failed for %1").arg(formatName));
            return EncodeResult::Failed;
        }

        const AVCodec *codec = avcodec_find_encoder(codecId);
        if (!codec) {
            setInternalErrorMessage(QStringLiteral("avcodec_find_encoder failed for %1").arg(int(codecId)));
            return EncodeResult::Failed;
        }

        m_codecContext = avcodec_alloc_context3(codec);
        if (!m_codecContext) {
            setInternalErrorMessage(QStringLiteral("avcodec_alloc_context3 failed for %1").arg(int(codecId)));
            return EncodeResult::Failed;
        }

        int outputWidth = settings.outputSize.width();
        int outputHeight = settings.outputSize.height();
        double framerate = settings.outputFps;

        m_codecContext->width = outputWidth;
        m_codecContext->height = outputHeight;
        m_codecContext->pix_fmt = format.pixelFormat(false);
        m_codecContext->framerate = av_d2q(framerate, 1000000000);
        m_codecContext->time_base = av_inv_q(m_codecContext->framerate);
        format.setCodecParams(m_codecContext);

        int err = avformat_alloc_output_context2(&m_formatContext, outputFormat, nullptr, nullptr);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avformat_alloc_output_context2 failed"), err);
            return EncodeResult::Failed;
        }

        format.setOutputParams(m_formatContext);

        AVStream *stream = avformat_new_stream(m_formatContext, nullptr);
        if (!stream) {
            setInternalErrorMessage(QStringLiteral("avformat_new_stream failed"));
            return EncodeResult::Failed;
        }

        stream->avg_frame_rate = m_codecContext->framerate;
        stream->time_base = m_codecContext->time_base;
        err = avcodec_open2(m_codecContext, codec, nullptr);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avcodec_open2 failed"), err);
            return EncodeResult::Failed;
        }

        m_codecParameters = avcodec_parameters_alloc();
        if (!m_codecParameters) {
            setInternalErrorMessage(QStringLiteral("avcodec_parameters_alloc failed"));
            return EncodeResult::Failed;
        }

        err = avcodec_parameters_from_context(m_codecParameters, m_codecContext);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avcodec_parameters_from_context failed"), err);
            return EncodeResult::Failed;
        }

        err = avcodec_parameters_copy(stream->codecpar, m_codecParameters);
        avcodec_parameters_free(&m_codecParameters);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avcodec_parameters_copy failed"), err);
            return EncodeResult::Failed;
        }

        if (m_formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            m_formatContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        QByteArray filterDesc = format.filterDesc();
        if (!filterDesc.isEmpty()) {
            const AVFilter *buffersrcFilter = avfilter_get_by_name("buffer");
            if (!buffersrcFilter) {
                setInternalErrorMessage(QStringLiteral("buffer filter not found"));
                return EncodeResult::Failed;
            }

            const AVFilter *buffersinkFilter = avfilter_get_by_name("buffersink");
            if (!buffersinkFilter) {
                setInternalErrorMessage(QStringLiteral("buffersink filter not found"));
                return EncodeResult::Failed;
            }

            m_filteredFrame = av_frame_alloc();
            if (!m_filteredFrame) {
                setInternalErrorMessage(QStringLiteral("filter av_frame_alloc failed"));
                return EncodeResult::Failed;
            }

            if (!paletteData.isEmpty()) {
                m_filterPaletteOutputs = avfilter_inout_alloc();
                if (!m_filterPaletteOutputs) {
                    setInternalErrorMessage(QStringLiteral("palette outputs avfilter_inout_alloc failed"));
                    return EncodeResult::Failed;
                }
            }

            m_filterOutputs = avfilter_inout_alloc();
            if (!m_filterOutputs) {
                setInternalErrorMessage(QStringLiteral("filter outputs avfilter_inout_alloc failed"));
                return EncodeResult::Failed;
            }

            m_filterInputs = avfilter_inout_alloc();
            if (!m_filterInputs) {
                setInternalErrorMessage(QStringLiteral("filter inputs avfilter_inout_alloc failed"));
                return EncodeResult::Failed;
            }

            m_graphContext = avfilter_graph_alloc();
            if (!m_graphContext) {
                setInternalErrorMessage(QStringLiteral("avfilter_graph_alloc failed"));
                return EncodeResult::Failed;
            }

            {
                QByteArray buffersrcArgs =
                    QStringLiteral("video_size=%1x%2:pix_fmt=%3:time_base=%4/%5:pixel_aspect=%6/%7")
                        .arg(outputWidth)
                        .arg(outputHeight)
                        .arg(int(format.pixelFormat(true)))
                        .arg(stream->time_base.num)
                        .arg(stream->time_base.den)
                        .arg(m_codecContext->sample_aspect_ratio.num)
                        .arg(m_codecContext->sample_aspect_ratio.den)
                        .toUtf8();
                err = avfilter_graph_create_filter(&m_buffersrcContext,
                                                   buffersrcFilter,
                                                   "in",
                                                   buffersrcArgs.constData(),
                                                   nullptr,
                                                   m_graphContext);
                if (err < 0) {
                    setInternalErrorMessageAv(QStringLiteral("buffersrc avfilter_graph_create_filter failed"), err);
                    return EncodeResult::Failed;
                }
            }

            if (!paletteData.isEmpty()) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paletteData.size() == KisLibav::GIF_PALETTE_BYTES,
                                                     EncodeResult::Failed);
                QByteArray palettesrcArgs =
                    QStringLiteral("video_size=%1x%1:pix_fmt=%2:time_base=1/24:pixel_aspect=0/1")
                        .arg(KisLibav::GIF_PALETTE_DIMENSION)
                        .arg(int(AV_PIX_FMT_BGRA))
                        .toUtf8();
                err = avfilter_graph_create_filter(&m_palettesrcContext,
                                                   buffersrcFilter,
                                                   "palette",
                                                   palettesrcArgs.constData(),
                                                   nullptr,
                                                   m_graphContext);
                if (err < 0) {
                    setInternalErrorMessageAv(QStringLiteral("palettesrc avfilter_graph_create_filter failed"), err);
                    return EncodeResult::Failed;
                }
            }

            m_buffersinkContext = avfilter_graph_alloc_filter(m_graphContext, buffersinkFilter, "out");
            if (!m_buffersinkContext) {
                setInternalErrorMessageAv(QStringLiteral("buffersink avfilter_graph_alloc_filter failed"), err);
                return EncodeResult::Failed;
            }

            QVector<AVPixelFormat> pixFmts = format.filterPixelFormats();
#if LIBAVUTIL_VERSION_MAJOR >= 60
            // Option int lists and "pix_fmt" got deprecated and replaced with array
            // options and "pixel_formats".
            unsigned int pixFmtsCount = 0;
            for (AVPixelFormat pixFmt : pixFmts) {
                if (pixFmt == AV_PIX_FMT_NONE) {
                    break;
                } else {
                    ++pixFmtsCount;
                }
            }
            err = av_opt_set_array(m_buffersinkContext,
                                   "pixel_formats",
                                   AV_OPT_SEARCH_CHILDREN | AV_OPT_ARRAY_REPLACE,
                                   0,
                                   pixFmtsCount,
                                   AV_OPT_TYPE_PIXEL_FMT,
                                   pixFmts.constData());
#else
            err = av_opt_set_int_list(m_buffersinkContext,
                                      "pix_fmts",
                                      pixFmts.constData(),
                                      AV_PIX_FMT_NONE,
                                      AV_OPT_SEARCH_CHILDREN);
#endif
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("buffersink av_opt_set failed"), err);
                return EncodeResult::Failed;
            }

            err = avfilter_init_str(m_buffersinkContext, nullptr);
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("buffersink avfilter_init_str failed"), err);
                return EncodeResult::Failed;
            }

            if (m_filterPaletteOutputs) {
                m_filterPaletteOutputs->name = av_strdup("palette");
                m_filterPaletteOutputs->filter_ctx = m_palettesrcContext;
                m_filterPaletteOutputs->pad_idx = 0;
                m_filterPaletteOutputs->next = nullptr;
            }

            m_filterOutputs->name = av_strdup("in");
            m_filterOutputs->filter_ctx = m_buffersrcContext;
            m_filterOutputs->pad_idx = 0;
            m_filterOutputs->next = m_filterPaletteOutputs;
            m_filterPaletteOutputs = nullptr;

            m_filterInputs->name = av_strdup("out");
            m_filterInputs->filter_ctx = m_buffersinkContext;
            m_filterInputs->pad_idx = 0;
            m_filterInputs->next = nullptr;

            err = avfilter_graph_parse_ptr(m_graphContext,
                                           filterDesc.constData(),
                                           &m_filterInputs,
                                           &m_filterOutputs,
                                           nullptr);
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("avfilter_graph_parse_ptr failed"), err);
                return EncodeResult::Failed;
            }

            err = avfilter_graph_config(m_graphContext, nullptr);
            if (err < 0) {
                setInternalErrorMessageAv(
                    QStringLiteral("avfilter_graph_config failed").arg(QString::fromUtf8(filterDesc)),
                    err);
                return EncodeResult::Failed;
            }

            avfilter_inout_free(&m_filterOutputs);
            avfilter_inout_free(&m_filterInputs);
        }

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        std::unique_ptr<QIODevice> output;
        if (writePalette) {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paletteData.isEmpty(), EncodeResult::Failed);
            output = std::make_unique<QBuffer>(&paletteData);
            if (!output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                setInternalErrorMessage(
                    QStringLiteral("failed to open palette output buffer: %1").arg(output->errorString()));
                return EncodeResult::Failed;
            }
        } else {
            output = std::make_unique<QFile>(settings.outputFile);
            if (!output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                setInternalErrorMessage(QStringLiteral("failed to open output file '%1': %2")
                                            .arg(settings.outputFile)
                                            .arg(output->errorString()));
                return EncodeResult::Failed;
            }
        }

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        QByteArray buffer;
        buffer.resize(KisLibav::BUFFER_SIZE);
        m_formatContext->pb = avio_alloc_context(reinterpret_cast<unsigned char *>(buffer.data()),
                                                 buffer.size(),
                                                 AVIO_FLAG_WRITE,
                                                 output.get(),
                                                 nullptr,
                                                 &Context::writeAvioPacket,
                                                 &Context::seekAvio);
        if (!m_formatContext->pb) {
            setInternalErrorMessage(QStringLiteral("avio_alloc_context failed"));
            return EncodeResult::Failed;
        }

        m_formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
        m_formatContext->pb->seekable = AVIO_SEEKABLE_NORMAL;

        err = avformat_write_header(m_formatContext, nullptr);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avformat_write_header failed"), err);
            return EncodeResult::Failed;
        }

        m_frame = av_frame_alloc();
        if (!m_frame) {
            setInternalErrorMessage(QStringLiteral("av_frame_alloc failed"));
            return EncodeResult::Failed;
        }

        m_frame->width = outputWidth;
        m_frame->height = outputHeight;
        m_frame->format = format.pixelFormat(true);

        err = av_frame_get_buffer(m_frame, 0);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("av_frame_get_buffer failed"), err);
            return EncodeResult::Failed;
        }

        m_packet = av_packet_alloc();
        if (!m_packet) {
            setInternalErrorMessage(QStringLiteral("av_packet_alloc failed"));
            return EncodeResult::Failed;
        }

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        if (m_palettesrcContext) {
            m_paletteFrame = av_frame_alloc();
            if (!m_paletteFrame) {
                setInternalErrorMessage(QStringLiteral("palette av_frame_alloc failed"));
                return EncodeResult::Failed;
            }

            m_paletteFrame->width = KisLibav::GIF_PALETTE_DIMENSION;
            m_paletteFrame->height = KisLibav::GIF_PALETTE_DIMENSION;
            m_paletteFrame->format = AV_PIX_FMT_BGRA;

            err = av_frame_get_buffer(m_paletteFrame, 0);
            if (err != 0) {
                setInternalErrorMessageAv(QStringLiteral("palette av_frame_get_buffer failed"), err);
                return EncodeResult::Failed;
            }

            err = av_frame_make_writable(m_paletteFrame);
            if (err != 0) {
                setInternalErrorMessageAv(QStringLiteral("palette av_frame_make_writable failed"), err);
                return EncodeResult::Failed;
            }

            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(paletteData.size() == KisLibav::GIF_PALETTE_BYTES,
                                                 EncodeResult::Failed);
            memcpy(m_paletteFrame->data[0], paletteData.constData(), KisLibav::GIF_PALETTE_BYTES);
            err = av_buffersrc_add_frame_flags(m_palettesrcContext, m_paletteFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("palette feed av_buffersrc_add_frame_flags failed"), err);
                return EncodeResult::Failed;
            }

            err = av_buffersrc_add_frame_flags(m_palettesrcContext, nullptr, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("palette flush v_buffersrc_add_frame_flags failed"), err);
                return EncodeResult::Failed;
            }
        }

        m_frame->pts = 0;
        Frame inputFrame;
        int swsFlags = getSwsFlags(settings.scaleFilter);
        while (m_runnable->nextFrame(inputFrame)) {
            if (isCancelled()) {
                return EncodeResult::Cancelled;
            }

            // Grab the next frame from disk.
            QImage inputImage;
            AVPixelFormat inputPixelFormat;
            if (!inputFrame.readImage(inputImage) || !convertFrame(inputImage, inputPixelFormat)) {
                continue; // Keep going, some frames may be corrupted.
            }

            int inputWidth = inputImage.width();
            int inputHeight = inputImage.height();
            if (inputWidth <= 0 || inputHeight <= 0 || inputImage.isNull()) {
                setInternalErrorMessage(QStringLiteral("null frame image"));
                return EncodeResult::Failed;
            }

            int instances = inputFrame.instances();
            if (instances < 1) {
                setInternalErrorMessage(QStringLiteral("frame has %1 instances < 1").arg(instances));
                return EncodeResult::Failed;
            }

            err = av_frame_make_writable(m_frame);
            if (err != 0) {
                setInternalErrorMessageAv(QStringLiteral("frame av_frame_make_writable failed"), err);
                return EncodeResult::Failed;
            }

            SwsContext *swsContext = getSwsContextFor(inputWidth,
                                                      inputHeight,
                                                      inputPixelFormat,
                                                      outputWidth,
                                                      outputHeight,
                                                      AVPixelFormat(m_frame->format),
                                                      swsFlags);
            if (!swsContext) {
                setInternalErrorMessage(QStringLiteral("sws_getCachedContext failed"));
                return EncodeResult::Failed;
            }

            const uint8_t *data = reinterpret_cast<const uint8_t *>(inputImage.constBits());
            const int stride = inputImage.bytesPerLine();
            sws_scale(swsContext, &data, &stride, 0, inputHeight, m_frame->data, m_frame->linesize);

            for (int i = 0; i < instances; ++i) {
                if (!filterFrame(stream, m_frame)) {
                    return EncodeResult::Failed;
                }
                ++m_frame->pts;
            }
        }

        av_frame_free(&m_frame);

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        if (!filterFrame(stream, nullptr)) {
            return EncodeResult::Failed;
        }

        av_packet_free(&m_packet);

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        err = av_write_trailer(m_formatContext);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("av_write_trailer failed"), err);
            return EncodeResult::Failed;
        }

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        avio_flush(m_formatContext->pb);
        avio_context_free(&m_formatContext->pb);

        // QIODevice doesn't have a flush function, so we gotta go via a cast.
        QFile *outputFile = qobject_cast<QFile *>(output.get());
        if (outputFile && !outputFile->flush()) {
            setInternalErrorMessage(QStringLiteral("failed to flush output file '%1': %2")
                                        .arg(settings.outputFile)
                                        .arg(outputFile->errorString()));
            return EncodeResult::Failed;
        }
        output->close();

        return EncodeResult::Completed;
    }

private:
    bool isCancelled() const
    {
        return m_runnable->isCancelled();
    }

    bool filterFrame(AVStream *stream, AVFrame *frame)
    {
        if (m_buffersrcContext) {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_buffersinkContext, false);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_filteredFrame, false);
            int err = av_buffersrc_add_frame_flags(m_buffersrcContext, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (err < 0) {
                setInternalErrorMessageAv(QStringLiteral("feed av_buffersrc_add_frame_flags failed"), err);
                return false;
            }

            while (true) {
                err = av_buffersink_get_frame(m_buffersinkContext, m_filteredFrame);
                if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
                    return true;
                } else if (err != 0) {
                    setInternalErrorMessageAv(QStringLiteral("av_buffersink_get_frame failed"), err);
                    return false;
                }

                bool ok = handleFrame(stream, m_filteredFrame);
                av_frame_unref(m_filteredFrame);
                return ok;
            }
        } else {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_buffersinkContext, false);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_filteredFrame, false);
            return handleFrame(stream, frame);
        }
    }

    bool handleFrame(AVStream *stream, AVFrame *frame)
    {
        int err = avcodec_send_frame(m_codecContext, frame);
        if (err != 0) {
            setInternalErrorMessageAv(QStringLiteral("avcodec_send_frame failed"), err);
            return false;
        }

        while (true) {
            err = avcodec_receive_packet(m_codecContext, m_packet);
            if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
                return true;
            } else if (err != 0) {
                setInternalErrorMessageAv(QStringLiteral("avcodec_receive_packet failed"), err);
                return false;
            }

            av_packet_rescale_ts(m_packet, m_codecContext->time_base, stream->time_base);
            m_packet->stream_index = stream->index;

            err = av_interleaved_write_frame(m_formatContext, m_packet);
            if (err != 0) {
                setInternalErrorMessageAv(QStringLiteral("av_interleaved_write_frame failed"), err);
                return false;
            }
        }
    }

    void setInternalErrorMessageAv(const QString &detail, int err)
    {
        setInternalErrorMessage(
            QStringLiteral("%1: %2 (%3)").arg(detail).arg(QString::fromUtf8(av_err2str(err))).arg(err));
    }

    static int writeAvioPacket(void *opaque, KisLibav::WriteAvioPacketBuf buf, int bufSize)
    {
        QIODevice *output = static_cast<QIODevice *>(opaque);
        if (bufSize > 0) {
            qint64 toWrite = bufSize;
            qint64 written = output->write(reinterpret_cast<const char *>(buf), toWrite);
            if (written != toWrite) {
                return AVERROR(EIO);
            }
        }
        return bufSize;
    }

    static int64_t seekSetAvio(QIODevice *output, qint64 offset)
    {
        if (output->seek(offset)) {
            return offset;
        } else {
            return AVERROR(EIO);
        }
    }

    static int64_t seekCurAvio(QIODevice *output, qint64 offset)
    {
        qint64 pos = output->pos();
        if (pos < 0) {
            return AVERROR(EIO);
        }

        qint64 targetPos = qMax(qint64(0), pos + offset);
        if (targetPos != pos && !output->seek(targetPos)) {
            return AVERROR(EIO);
        }

        return targetPos;
    }

    static int64_t seekAvio(void *opaque, int64_t offset, int whence)
    {
        QIODevice *output = static_cast<QIODevice *>(opaque);
        if (whence & AVSEEK_SIZE) {
            warnFile << "AVSEEK_SIZE unsupported with whence" << whence;
            return 0;
        } else {
            switch (whence & ~AVSEEK_FORCE) {
            case SEEK_SET:
                return seekSetAvio(output, offset);
            case SEEK_CUR:
                return seekCurAvio(output, offset);
            default:
                warnFile << "Unsupported seek whence" << whence;
                return AVERROR(EINVAL);
            }
        }
    }

    KisLibavMediaEncoderRunnable *m_runnable;
    AVCodecContext *m_codecContext = nullptr;
    AVFormatContext *m_formatContext = nullptr;
    AVCodecParameters *m_codecParameters = nullptr;
    AVFilterInOut *m_filterPaletteOutputs = nullptr;
    AVFilterInOut *m_filterOutputs = nullptr;
    AVFilterInOut *m_filterInputs = nullptr;
    AVFilterGraph *m_graphContext = nullptr;
    AVFilterContext *m_buffersrcContext = nullptr;
    AVFilterContext *m_palettesrcContext = nullptr;
    AVFilterContext *m_buffersinkContext = nullptr;
    AVFrame *m_paletteFrame = nullptr;
    AVFrame *m_frame = nullptr;
    AVFrame *m_filteredFrame = nullptr;
    AVPacket *m_packet = nullptr;
};

KisLibavMediaEncoderRunnable *KisLibavMediaEncoderRunnable::create(const KisMediaEncoderWrapperSettings &settings,
                                                                   QObject *parent)
{
    if (settings.format->type() == KisMediaEncoderFormat::Type::LibavMediaEncoder) {
        return new KisLibavMediaEncoderRunnable(settings, parent);
    } else {
        return nullptr;
    }
}

void KisLibavMediaEncoderRunnable::getSupportedFormats(QVector<KisMediaEncoderFormat *> &outSupportedFormats)
{
    if (avcodec_find_encoder(Format::codecIdForFormatId(FORMAT_GIF_PALETTE))
        && avcodec_find_encoder(Format::codecIdForFormatId(FORMAT_GIF))) {
        outSupportedFormats.append(new Format(FORMAT_GIF));
    }
}

KisLibavMediaEncoderRunnable::KisLibavMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings,
                                                           QObject *parent)
    : KisMediaEncoderRunnable(settings, parent)
{
}

KisMediaEncoderRunnable::EncodeResult KisLibavMediaEncoderRunnable::encode(QString &outErrorMessage)
{
    Format *format = static_cast<Format *>(settings().format);
    QByteArray paletteData;

    if (format->formatId() == FORMAT_GIF) {
        // GIF needs to generate a palette, so we must do two rendering runs.
        // Setting the multiplier here halves the speed of the progress bar.
        setOutputFrameNoMultiplier(0.5);

        Format paletteFormat(FORMAT_GIF_PALETTE);
        EncodeResult paletteResult =
            Context(this, outErrorMessage).encode(settings(), paletteFormat, paletteData, true);
        if (paletteResult != EncodeResult::Completed) {
            return paletteResult;
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!paletteData.isEmpty(), EncodeResult::Failed);
        rewindFrames(); // Ready the frame iterator for another round.
    }

    return Context(this, outErrorMessage).encode(settings(), *format, paletteData, false);
}
