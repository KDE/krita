/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidMediaEncoderRunnable.h"

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QImage>
#include <QSpinBox>
#include <QTemporaryFile>
#include <memory>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QJniEnvironment>
#include <QJniObject>
#else
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
using QJniEnvironment = QAndroidJniEnvironment;
using QJniObject = QAndroidJniObject;
#endif

#include <klocalizedstring.h>

#include <KisAndroidUtils.h>
#include <kis_debug.h>

#include <mlt++/Mlt.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

#include "KisLibavEncoderContext.h"

struct KisAndroidMediaEncoderRunnable::EncoderImage {
    uint8_t *bufferY;
    uint8_t *bufferU;
    uint8_t *bufferV;
    int rowStrideY;
    int rowStrideU;
    int rowStrideV;
    int pixelStrideU;
    int pixelStrideV;
};

class KisAndroidMediaEncoderRunnable::Format : public KisMediaEncoderFormat
{
public:
    struct Encoder {
        QString name;
        bool hardware;
    };

    Format(int formatId, const QVector<Encoder> &videoEncoders, const QVector<Encoder> &audioEncoders)
        : KisMediaEncoderFormat()
        , m_formatId(formatId)
    {
        fillEncoders(m_videoEncoders, videoEncoders);
        fillEncoders(m_audioEncoders, audioEncoders);
    }

    int formatId() const
    {
        return m_formatId;
    }

    Type type() const override
    {
        return Type::AndroidMediaEncoder;
    }

    QString key() const override
    {
        return keyForFormatId(m_formatId);
    }

    QString title() const override
    {
        return i18n("Android: %1", titleForFormatId(m_formatId));
    }

    QString extension() const override
    {
        return extensionForFormatId(m_formatId);
    }

    bool supportsAudio() const override
    {
        return !m_audioEncoders.isEmpty();
    }

    QWidget *createPreferencesWidget(const QVariantMap &preferences) const override
    {
        KisAndroidMediaEncoderPreferencesWidget *pw = new KisAndroidMediaEncoderPreferencesWidget(this);

        for (const Encoder &videoEncoder : m_videoEncoders) {
            pw->addVideoEncoderOption(makeEncoderTitle(videoEncoder), videoEncoder.name);
        }

        for (const Encoder &audioEncoder : m_audioEncoders) {
            pw->addAudioEncoderOption(makeEncoderTitle(audioEncoder), audioEncoder.name);
        }

        applyPreferencesToWidget(pw, preferences);
        return pw;
    }

    void resetPreferencesWidget(QWidget *widget) const override
    {
        KisAndroidMediaEncoderPreferencesWidget *pw = qobject_cast<KisAndroidMediaEncoderPreferencesWidget *>(widget);
        KIS_SAFE_ASSERT_RECOVER_RETURN(pw);
        applyPreferencesToWidget(pw, QVariantMap());
    }

    QVariantMap getPreferencesFromWidget(QWidget *widget) const override
    {
        QVariantMap preferences;
        KisAndroidMediaEncoderPreferencesWidget *pw = qobject_cast<KisAndroidMediaEncoderPreferencesWidget *>(widget);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(pw, preferences);
        preferences.insert(QStringLiteral("encoder"), pw->videoEncoder());
        preferences.insert(QStringLiteral("bitrate"), pw->videoBitrate());
        if (supportsAudio()) {
            preferences.insert(QStringLiteral("aencoder"), pw->audioEncoder());
            preferences.insert(QStringLiteral("abitrate"), pw->audioBitrate());
        }
        return preferences;
    }

    QString getVideoEncoderPreference(const QVariantMap &preferences) const
    {
        QString videoEncoder = preferences.value(QStringLiteral("encoder")).toString();
        if (videoEncoder.isEmpty() && !m_videoEncoders.isEmpty()) {
            return m_videoEncoders.constFirst().name;
        } else {
            return videoEncoder;
        }
    }

    int getVideoBitratePreference(const QVariantMap &preferences) const
    {
        int videoBitrate = preferences.value(QStringLiteral("bitrate")).toInt();
        if (videoBitrate <= 0) {
            return defaultVideoBitrateForFormatId(m_formatId);
        } else {
            return videoBitrate;
        }
    }

    QString getAudioEncoderPreference(const QVariantMap &preferences) const
    {
        QString audioEncoder = preferences.value(QStringLiteral("aencoder")).toString();
        if (audioEncoder.isEmpty() && !m_videoEncoders.isEmpty()) {
            return m_audioEncoders.constFirst().name;
        } else {
            return audioEncoder;
        }
    }

    int getAudioBitratePreference(const QVariantMap &preferences) const
    {
        int audioBitrate = preferences.value(QStringLiteral("abitrate")).toInt();
        if (audioBitrate <= 0) {
            return defaultAudioBitrateForFormatId(m_formatId);
        } else {
            return audioBitrate;
        }
    }

    int audioSampleRate() const
    {
        // While creating an audio track on Android lets you pass it a sample
        // rate, that doesn't actually give you an audio track with the given
        // sample rate. For WEBM with Vorbis, you always get a 48kHz track. MP4
        // with AAC seems more forgiving, but let's go with known good values.
        switch (m_formatId) {
        case FORMAT_MP4_H264:
        case FORMAT_MP4_AV1:
            return 44100;
        case FORMAT_WEBM_VP8:
            return 48000;
        }
        return 44100;
    }

private:
    static void fillEncoders(QVector<Encoder> &outEncoders, const QVector<Encoder> &encoders)
    {
        // Prefer software encoders, because hardware encoders are often busted.
        outEncoders.reserve(encoders.size());
        for (const Encoder &encoder : encoders) {
            if (!encoder.hardware) {
                outEncoders.append(encoder);
            }
        }
        for (const Encoder &encoder : encoders) {
            if (encoder.hardware) {
                outEncoders.append(encoder);
            }
        }
    }

    static QString makeEncoderTitle(const Encoder &encoder)
    {
        if (encoder.hardware) {
            return i18n("%1 (hardware)", encoder.name);
        } else {
            return i18n("%1 (software)", encoder.name);
        }
    }

    void applyPreferencesToWidget(KisAndroidMediaEncoderPreferencesWidget *pw, const QVariantMap &preferences) const
    {
        pw->setVideoEncoder(getVideoEncoderPreference(preferences));
        pw->setVideoBitrate(getVideoBitratePreference(preferences));
        if (supportsAudio()) {
            pw->setAudioEncoder(getAudioEncoderPreference(preferences));
            pw->setAudioBitrate(getAudioBitratePreference(preferences));
        }
    }

    static QString keyForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_MP4_H264:
            return QStringLiteral("android:mp4:h264");
        case FORMAT_WEBM_VP8:
            return QStringLiteral("android:webm:vp8");
        case FORMAT_MP4_AV1:
            return QStringLiteral("android:mp4:av1");
        }
        return QString();
    }

    static QString titleForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_MP4_H264:
            return QStringLiteral("MP4/H.264");
        case FORMAT_WEBM_VP8:
            return QStringLiteral("WEBM/VP8");
        case FORMAT_MP4_AV1:
            return QStringLiteral("MP4/AV1");
        }
        return QString();
    }

    static QString extensionForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_MP4_H264:
        case FORMAT_MP4_AV1:
            return QStringLiteral("mp4");
        case FORMAT_WEBM_VP8:
            return QStringLiteral("webm");
        }
        return QString();
    }

    static int defaultVideoBitrateForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_MP4_H264:
            return 6000000;
        case FORMAT_WEBM_VP8:
            return 7000000;
        case FORMAT_MP4_AV1:
            return 3200000;
        }
        return 6000000;
    }

    static int defaultAudioBitrateForFormatId(int formatId)
    {
        switch (formatId) {
        case FORMAT_MP4_H264:
        case FORMAT_MP4_AV1:
            return 128000;
        case FORMAT_WEBM_VP8:
            return 96000;
        }
        return 128000;
    }

    QVector<Encoder> m_videoEncoders;
    QVector<Encoder> m_audioEncoders;
    int m_formatId;
};

class KisAndroidMediaEncoderRunnable::Context : public KisLibavEncoderContext
{
public:
    explicit Context(QString *outErrorMessage = nullptr)
        : KisLibavEncoderContext(outErrorMessage)
    {
    }

    ~Context() override
    {
        clearEncoder();
    }

    QJniEnvironment &env()
    {
        return m_env;
    }

    QJniObject &encoder()
    {
        return m_encoder;
    }

    void setEncoder(const QJniObject &encoder)
    {
        m_encoder = encoder;
    }

    void clearEncoder()
    {
        if (m_encoder.isValid()) {
            m_encoder.callMethod<void>("cancel", "()V");
            m_encoder = QJniObject();
            if (m_env->ExceptionCheck()) {
                warnFile << "JNI exception occurred cancelling encoder";
                m_env->ExceptionDescribe();
                m_env->ExceptionClear();
            }
        }
    }

    bool checkObject(const QString &title, QJniObject &obj)
    {
        if (checkException(title)) {
            return true;
        } else if (!obj.isValid()) {
            setInternalErrorMessage(QStringLiteral("JNI object %1 invalid").arg(title));
            return true;
        } else {
            return false;
        }
    }

    bool checkResult(const QString &title, int result)
    {
        if (checkException(title)) {
            return true;
        } else if (result == STATUS_ERROR_START_VIDEO_ENCODER) {
            warnFile << "Start video encoder error" << result;
            setErrorMessage(i18n("Unsupported video parameters, try lowering the video FPS or size"));
            return true;
        } else if (result == STATUS_ERROR_START_AUDIO_FORMAT || result == STATUS_ERROR_START_AUDIO_ENCODER) {
            warnFile << "Start audio encoder error" << result;
            setErrorMessage(
                i18n("Unsupported audio parameters, try to re-encode your audio file with a more common sample format, "
                     "sampling rate and channel count"));
            return true;
        } else if (result == STATUS_ERROR_DRAIN_VIDEO_MUXER_ADD_TRACK
                   || result == STATUS_ERROR_DRAIN_AUDIO_MUXER_ADD_TRACK) {
            warnFile << "Muxer track error" << result;
            setErrorMessage(i18n("Unsupported format"));
            return true;
        } else if (isErrorResult(result)) {
            setInternalErrorMessage(QStringLiteral("%1 failed with code %2").arg(title).arg(result));
            return true;
        } else {
            return false;
        }
    }

    bool checkException(const QString &title)
    {
        if (m_env->ExceptionCheck()) {
            setInternalErrorMessage(QStringLiteral("JNI exception in %1").arg(title));
            m_env->ExceptionDescribe();
            m_env->ExceptionClear();
            return true;
        } else {
            return false;
        }
    }

    int imageFormat() const
    {
        return m_imageFormat;
    }

    uint8_t **imageBuffers()
    {
        return m_imageBuffers;
    }

    int *imageLinesizes()
    {
        return m_imageLinesizes;
    }

    bool allocateImage(int outputWidth, int outputHeight, AVPixelFormat outputFormat)
    {
        // The Android encoder really shouldn't be changing
        // pixel formats along the way, but just in case.
        if (m_imageFormat != AV_PIX_FMT_NONE) {
            m_imageFormat = AV_PIX_FMT_NONE;
            av_freep(&m_imageBuffers[0]);
        }

        int result = av_image_alloc(m_imageBuffers, m_imageLinesizes, outputWidth, outputHeight, outputFormat, 32);
        if (result >= 0) {
            m_imageFormat = outputFormat;
            return true;
        } else {
            setInternalErrorMessage(QStringLiteral("av_image_alloc error %1").arg(result));
            return false;
        }
    }

private:
    static bool isErrorResult(int result)
    {
        return result >= 100;
    }

    QJniEnvironment m_env;
    QJniObject m_encoder;
    uint8_t *m_imageBuffers[4] = {nullptr, nullptr, nullptr, nullptr};
    int m_imageLinesizes[4] = {0, 0, 0, 0};
    AVPixelFormat m_imageFormat = AV_PIX_FMT_NONE;
};

class KisAndroidMediaEncoderRunnable::Audio
{
public:
    static constexpr int PULL_FRAME_OK = 0;
    static constexpr int PULL_FRAME_END_OF_STREAM = 1;
    static constexpr int PULL_FRAME_ERROR = 2;

    Audio(mlt_audio_format format, int sampleRate, int channelCount)
        : m_format(format)
        , m_sampleRate(sampleRate)
        , m_channelCount(channelCount)
        , m_sampleSize(mlt_audio_format_size(format, 1, 1))
    {
    }

    QString formatName() const
    {
        return QString::fromUtf8(mlt_audio_format_name(m_format));
    }

    int sampleRate() const
    {
        return m_sampleRate;
    }

    int channelCount() const
    {
        return m_channelCount;
    }

    bool isFinished() const
    {
        return m_finished;
    }

    bool open(Context &ctx, const KisMediaEncoderWrapperSettings &settings)
    {
        m_audioFileBytes = settings.audioFile.toUtf8();

        m_profile = std::make_unique<Mlt::Profile>();
        m_profile->set_frame_rate(settings.outputFps, 1);

        m_producer = std::make_unique<Mlt::Producer>(*m_profile, "avformat", m_audioFileBytes.constData());
        if (!m_producer->is_valid()) {
            ctx.setInternalErrorMessage(QStringLiteral("failed to open MLT producer for '%1'").arg(settings.audioFile));
            return false;
        }

        m_filter = std::make_unique<Mlt::Filter>(*m_profile, "swresample");
        if (!m_filter->is_valid()) {
            ctx.setInternalErrorMessage(QStringLiteral("failed to create MLT filter"));
            return false;
        }

        int result = m_producer->attach(*m_filter);
        if (result != 0) {
            ctx.setInternalErrorMessage(QStringLiteral("error %1 attaching MLT filter").arg(result));
            return false;
        }

        m_producer->seek(settings.audioSeekFrame);

        return true;
    }

    int pullFrame(Context &ctx)
    {
        if (m_frame) {
            return PULL_FRAME_OK;
        }

        m_frame.reset(m_producer->get_frame());
        if (!m_frame || !m_frame->is_valid()) {
            m_frame.reset();
            return PULL_FRAME_END_OF_STREAM;
        }

        mlt_audio_format format = m_format;
        int sampleRate = m_sampleRate;
        int channelCount = m_channelCount;

        float fps = float(m_profile->fps());
        int64_t position = m_frame->get_position();
        int sampleCount = mlt_audio_calculate_frame_samples(fps, sampleRate, position);
        const void *sampleData = m_frame->get_audio(format, sampleRate, channelCount, sampleCount);
        if (sampleCount <= 0 || !sampleData) {
            m_frame.reset();
            return PULL_FRAME_END_OF_STREAM;
        }

        if (format != m_format || sampleRate != m_sampleRate || channelCount != m_channelCount) {
            ctx.setInternalErrorMessage(QStringLiteral("bad MLT frame format %1:%2:%3 != not %4:%5:%6")
                                            .arg(int(format))
                                            .arg(sampleRate)
                                            .arg(channelCount)
                                            .arg(int(m_format))
                                            .arg(m_sampleRate)
                                            .arg(m_channelCount));
            m_frame.reset();
            return PULL_FRAME_ERROR;
        }

        m_sampleCount = sampleCount;
        m_sampleData = reinterpret_cast<const unsigned char *>(sampleData);
        m_samplePos = 0;
        return PULL_FRAME_OK;
    }

    bool pushSamples(Context &ctx)
    {
        void *buffer;
        int availableSize;
        if (!readAudioInputBuffer(ctx, buffer, availableSize)) {
            return false;
        }

        int availableSamples = sizeToSamples(availableSize);
        if (availableSamples <= 0) {
            ctx.setInternalErrorMessage(QStringLiteral("no available samples from size %1").arg(availableSize));
            return false;
        }

        int remainingSamples = m_sampleCount - m_samplePos;
        int chunkSamples = qMin(availableSamples, remainingSamples);
        int chunkSize = samplesToSize(chunkSamples);
        memcpy(buffer, m_sampleData + samplesToSize(m_samplePos), chunkSize);

        int commitResult =
            int(ctx.encoder().callMethod<jint>("commitAudio", "(II)I", jint(chunkSamples), jint(chunkSize)));
        if (ctx.checkResult(QStringLiteral("commitAudio"), commitResult)) {
            return false;
        }

        m_samplePos += chunkSamples;
        return true;
    }

    bool isSampleDataRemaining() const
    {
        return m_samplePos < m_sampleCount;
    }

    void finishFrame()
    {
        m_frame.reset();
        m_sampleData = nullptr;
        m_sampleCount = 0;
        m_samplePos = 0;
    }

    bool finish(Context &ctx)
    {
        if (!m_finished) {
            int finishAudioResult = int(ctx.encoder().callMethod<jint>("finishAudio", "()I"));
            if (ctx.checkResult(QStringLiteral("finishAudio"), finishAudioResult)) {
                return false;
            }
            m_finished = true;
        }
        return true;
    }

private:
    int sizeToSamples(int size) const
    {
        return size / (m_channelCount * m_sampleSize);
    }

    int samplesToSize(int samples) const
    {
        return samples * m_channelCount * m_sampleSize;
    }

    QByteArray m_audioFileBytes;
    std::unique_ptr<Mlt::Profile> m_profile;
    std::unique_ptr<Mlt::Producer> m_producer;
    std::unique_ptr<Mlt::Filter> m_filter;
    std::unique_ptr<Mlt::Frame> m_frame;
    const unsigned char *m_sampleData = nullptr;
    const mlt_audio_format m_format;
    const int m_sampleRate;
    const int m_channelCount;
    const int m_sampleSize;
    int m_sampleCount = 0;
    int m_samplePos = 0;
    bool m_finished = false;
};

KisAndroidMediaEncoderRunnable *KisAndroidMediaEncoderRunnable::create(const KisMediaEncoderWrapperSettings &settings,
                                                                       QObject *parent)
{
    if (settings.format->type() == KisMediaEncoderFormat::Type::AndroidMediaEncoder) {
        return new KisAndroidMediaEncoderRunnable(settings, parent);
    } else {
        return nullptr;
    }
}

void KisAndroidMediaEncoderRunnable::getSupportedFormats(QVector<KisMediaEncoderFormat *> &outSupportedFormats)
{
    Context ctx;
    int formatIds[] = {FORMAT_MP4_H264, FORMAT_WEBM_VP8, FORMAT_MP4_AV1};
    for (int formatId : formatIds) {
        checkFormatSupport(ctx, formatId, outSupportedFormats);
    }
}

KisAndroidMediaEncoderRunnable::KisAndroidMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings,
                                                               QObject *parent)
    : KisMediaEncoderRunnable(settings, parent)
{
}

KisMediaEncoderRunnable::EncodeResult KisAndroidMediaEncoderRunnable::encode(QString &outErrorMessage)
{
    Format *format = static_cast<Format *>(settings().format);

    QTemporaryFile tempFile;
    QString tempFilePath;
    if (tempFile.open()) {
        tempFilePath = tempFile.fileName();
        tempFile.close();
    } else {
        warnFile << "Failed to open temporary file:" << tempFile.errorString();
        // Keep going, we might not actually need a temporary file.
    }

    Context ctx(&outErrorMessage);
    int outputWidth = settings().outputSize.width();
    int outputHeight = settings().outputSize.height();

    // Set up audio if we were given some.
    std::unique_ptr<Audio> audio;
    if (!settings().audioFile.isEmpty()) {
        // We always use these fixed parameters: 16 bit PCM, 2 channels and a
        // sample rate according to the format. The Android media encoder makes
        // it appear like you can create audio tracks with other parameters, but
        // if it doesn't like the parameters it just gives you something else,
        // which obviously just results in nonsense. We'll go with known good
        // parameters instead, they'll also play back consistently everywhere.
        audio = std::make_unique<Audio>(mlt_audio_s16, format->audioSampleRate(), 2);
        if (!audio->open(ctx, settings())) {
            return EncodeResult::Failed;
        }

        switch (audio->pullFrame(ctx)) {
        case Audio::PULL_FRAME_OK:
            break;
        case Audio::PULL_FRAME_END_OF_STREAM:
            // There's no frames, which can happen if our animation range start
            // is beyond the end of the file. Just export without audio then.
            audio.reset();
            break;
        default:
            return EncodeResult::Failed;
        }
    }

    // Set up the encoder.
    {
        QJniObject outputPath = QJniObject::fromString(settings().outputFile);
        if (ctx.checkObject(QStringLiteral("outputPath"), outputPath)) {
            return EncodeResult::Failed;
        }

        QJniObject tempPath = QJniObject::fromString(tempFilePath);
        if (ctx.checkObject(QStringLiteral("tempPath"), tempPath)) {
            return EncodeResult::Failed;
        }

        QJniObject videoEncoderName =
            QJniObject::fromString(format->getVideoEncoderPreference(settings().formatPreferences));
        ctx.checkException(QStringLiteral("videoEncoderName"));

        jint audioSampleRate;
        jint audioChannelCount;
        QJniObject audioEncoderName;
        QJniObject audioFormatName;
        if (audio) {
            audioSampleRate = jint(audio->sampleRate());
            audioChannelCount = jint(audio->channelCount());
            audioEncoderName = QJniObject::fromString(format->getAudioEncoderPreference(settings().formatPreferences));
            ctx.checkException(QStringLiteral("audioEncoderName"));
            audioFormatName = QJniObject::fromString(audio->formatName());
            ctx.checkException(QStringLiteral("audioFormatName"));
        } else {
            audioSampleRate = jint(0);
            audioChannelCount = jint(0);
        }

        ctx.setEncoder(QJniObject(
            "org/krita/android/VideoEncoder",
            "(IIIFLjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;III)V",
            jint(format->formatId()),
            jint(outputWidth),
            jint(outputHeight),
            jfloat(settings().outputFps),
            outputPath.object<jstring>(),
            tempPath.object<jstring>(),
            videoEncoderName.object<jstring>(),
            jint(format->getVideoBitratePreference(settings().formatPreferences)),
            audio ? audioEncoderName.object<jstring>() : nullptr,
            audio ? audioFormatName.object<jstring>() : nullptr,
            jint(audioSampleRate),
            jint(audioChannelCount),
            jint(format->getAudioBitratePreference(settings().formatPreferences))));
        if (ctx.checkObject(QStringLiteral("encoder"), ctx.encoder())) {
            return EncodeResult::Failed;
        }
    }

    if (isCancelled()) {
        return EncodeResult::Cancelled;
    }

    // Start the encoding.
    {
        QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                 "activity",
                                                                 "()Landroid/app/Activity;");
        if (ctx.checkObject(QStringLiteral("activity"), activity)) {
            return EncodeResult::Failed;
        }

        int startResult =
            int(ctx.encoder().callMethod<jint>("start", "(Landroid/content/Context;)I", activity.object<jobject>()));
        if (ctx.checkResult(QStringLiteral("start"), startResult)) {
            return EncodeResult::Failed;
        }
    }

    // Encode the frames.
    Frame frame;
    int swsFlags = ctx.getSwsFlags(settings().scaleFilter);
    while (nextFrame(frame)) {
        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        // Grab the next frame from disk.
        QImage inputImage;
        if (!frame.readImage(inputImage)) {
            // If we don't have audio, we can just skip the frame and hope it's
            // okay. In timelapses it often is, you're not gonna notice a single
            // frame missing in the video. However, if we have an animation with
            // audio, skipping a frame would make a mess and be hard to resync,
            // so in that case we just give up and bail out.
            if (audio) {
                ctx.setInternalErrorMessage(QStringLiteral("failed to read frame %1").arg(frame.path()));
                return EncodeResult::Failed;
            } else {
                continue;
            }
        }

        AVPixelFormat inputPixelFormat;
        if (!ctx.convertFrame(inputImage, inputPixelFormat)) {
            // Dito to the above, try to skip frames we don't understand, don't
            // bother when there's audio involved.
            if (audio) {
                ctx.setInternalErrorMessage(QStringLiteral("failed to convert frame %1").arg(frame.path()));
                return EncodeResult::Failed;
            } else {
                continue;
            }
        }

        int instances = frame.instances();
        for (int i = 0; i < instances; ++i) {
            // Grab a buffer from the encoder.
            EncodeResult prepareVideoResult = prepareVideo(ctx);
            if (prepareVideoResult != EncodeResult::Completed) {
                return prepareVideoResult;
            }

            // Retrieve the buffer layout.
            EncoderImage encoderImage;
            if (!readEncoderImage(ctx, encoderImage)) {
                return EncodeResult::Failed;
            }

            // Map the buffer layout to a libswscale-befitting arrangement. The
            // layout may either have the YUV components in separate buffers or it
            // may have the U and V components combined into a single buffer, where
            // either U or V can come first. Which one we get depends on hardware.
            uint8_t *dstBuffers[4] = {nullptr, nullptr, nullptr, nullptr};
            int dstLinesizes[4] = {0, 0, 0, 0};
            AVPixelFormat outputPixelFormat;
            if (encoderImage.pixelStrideU == 1 && encoderImage.pixelStrideV == 1) {
                // Separate Y, U and V buffers.
                outputPixelFormat = AV_PIX_FMT_YUV420P;
                dstBuffers[0] = encoderImage.bufferY;
                dstBuffers[1] = encoderImage.bufferU;
                dstBuffers[2] = encoderImage.bufferV;
                dstLinesizes[0] = encoderImage.rowStrideY;
                dstLinesizes[1] = encoderImage.rowStrideU;
                dstLinesizes[2] = encoderImage.rowStrideV;

            } else if (encoderImage.pixelStrideU == 2 && encoderImage.pixelStrideV == 2
                       && encoderImage.bufferU + 1 == encoderImage.bufferV) {
                // One Y buffer and one combined UV buffer, U comes first.
                outputPixelFormat = AV_PIX_FMT_NV12;
                dstBuffers[0] = encoderImage.bufferY;
                dstBuffers[1] = encoderImage.bufferU;
                dstLinesizes[0] = encoderImage.rowStrideY;
                dstLinesizes[1] = encoderImage.rowStrideU;

            } else if (encoderImage.pixelStrideU == 2 && encoderImage.pixelStrideV == 2
                       && encoderImage.bufferV + 1 == encoderImage.bufferU) {
                // One Y buffer and one combined UV buffer, V comes first.
                outputPixelFormat = AV_PIX_FMT_NV21;
                dstBuffers[0] = encoderImage.bufferY;
                dstBuffers[1] = encoderImage.bufferV;
                dstLinesizes[0] = encoderImage.rowStrideY;
                dstLinesizes[1] = encoderImage.rowStrideV;

            } else {
                ctx.setInternalErrorMessage(QStringLiteral("unknown buffer format u%1/%2 v%3/%4")
                                                .arg(quintptr(encoderImage.bufferU), 0, 16)
                                                .arg(encoderImage.pixelStrideU)
                                                .arg(quintptr(encoderImage.bufferV), 0, 16)
                                                .arg(encoderImage.pixelStrideV));
                return EncodeResult::Failed;
            }

            SwsContext *swsContext = ctx.getSwsContextFor(inputImage.width(),
                                                          inputImage.height(),
                                                          inputPixelFormat,
                                                          outputWidth,
                                                          outputHeight,
                                                          outputPixelFormat,
                                                          swsFlags);
            if (!swsContext) {
                ctx.setInternalErrorMessage(QStringLiteral("sws_getCachedContext"));
                return EncodeResult::Failed;
            }

            if (instances == 1) {
                // Just a single frame, scale it into the native buffer.
                const uint8_t *srcBuffers[] = {inputImage.bits(), nullptr, nullptr, nullptr};
                const int srcLinesizes[] = {inputImage.bytesPerLine(), 0, 0, 0};
                sws_scale(swsContext, srcBuffers, srcLinesizes, 0, inputImage.height(), dstBuffers, dstLinesizes);

            } else {
                // Repeated frame, scale it into an intermediate buffer, then
                // copy it over to the native one for each instance.
                if (i == 0 || ctx.imageFormat() != outputPixelFormat) {
                    if (ctx.imageFormat() != outputPixelFormat) {
                        if (!ctx.allocateImage(outputWidth, outputHeight, outputPixelFormat)) {
                            warnFile << "Encoder changed pixel format from" << int(ctx.imageFormat()) << "to"
                                     << int(outputPixelFormat);
                            return EncodeResult::Failed;
                        }
                    }

                    const uint8_t *srcBuffers[] = {inputImage.bits(), nullptr, nullptr, nullptr};
                    const int srcLinesizes[] = {inputImage.bytesPerLine(), 0, 0, 0};
                    sws_scale(swsContext,
                              srcBuffers,
                              srcLinesizes,
                              0,
                              inputImage.height(),
                              ctx.imageBuffers(),
                              ctx.imageLinesizes());
                }

                av_image_copy2(dstBuffers,
                               dstLinesizes,
                               ctx.imageBuffers(),
                               ctx.imageLinesizes(),
                               outputPixelFormat,
                               outputWidth,
                               outputHeight);
            }

            int commitResult = int(ctx.encoder().callMethod<jint>("commitVideo", "()I"));
            if (ctx.checkResult(QStringLiteral("commitVideo"), commitResult)) {
                return EncodeResult::Failed;
            }

            if (audio && !audio->isFinished()) {
                int pullFrameResult = audio->pullFrame(ctx);
                if (pullFrameResult == Audio::PULL_FRAME_OK) {
                    do {
                        EncodeResult prepareAudioResult = prepareAudio(ctx);
                        if (prepareAudioResult != EncodeResult::Completed) {
                            return prepareAudioResult;
                        }

                        if (!audio->pushSamples(ctx)) {
                            return EncodeResult::Failed;
                        }
                    } while (audio->isSampleDataRemaining());
                    audio->finishFrame();

                } else if (pullFrameResult == Audio::PULL_FRAME_END_OF_STREAM) {
                    EncodeResult prepareAudioResult = prepareAudio(ctx);
                    if (prepareAudioResult != EncodeResult::Completed) {
                        return prepareAudioResult;
                    }

                    if (!audio->finish(ctx)) {
                        return EncodeResult::Failed;
                    }

                } else {
                    return EncodeResult::Failed;
                }
            }
        }
    }

    if (isCancelled()) {
        return EncodeResult::Cancelled;
    }

    // Finish the encoder streams.
    {
        // Need a buffer from the encoder to tell it that it's done.
        EncodeResult prepareVideoResult = prepareVideo(ctx);
        if (prepareVideoResult != EncodeResult::Completed) {
            return prepareVideoResult;
        }

        // Hand empty buffer back with the end of stream flag set.
        int finishVideoResult = int(ctx.encoder().callMethod<jint>("finishVideo", "()I"));
        if (ctx.checkResult(QStringLiteral("finishVideo"), finishVideoResult)) {
            return EncodeResult::Failed;
        }

        // Same procedure with audio, but it might already have finished.
        if (audio && !audio->isFinished()) {
            EncodeResult prepareAudioResult = prepareAudio(ctx);
            if (prepareAudioResult != EncodeResult::Completed) {
                return prepareAudioResult;
            }

            if (!audio->finish(ctx)) {
                return EncodeResult::Failed;
            }
        }
    }

    // Drain all remaining frames and samples out of the encoders.
    bool videoStreamEnded = false;
    bool audioStreamEnded = !audio;
    do {
        if (!videoStreamEnded) {
            int drainVideoResult = drainVideo(ctx, 1000000LL);
            if (drainVideoResult == DRAIN_END_OF_STREAM) {
                videoStreamEnded = true;
            } else if (drainVideoResult == DRAIN_ERROR) {
                return EncodeResult::Failed;
            } else if (drainVideoResult == DRAIN_CANCELLED) {
                return EncodeResult::Cancelled;
            } else {
                KIS_SAFE_ASSERT_RECOVER_NOOP(drainVideoResult >= 0);
            }
        }
        if (!audioStreamEnded) {
            int drainAudioResult = drainAudio(ctx, 1000000LL);
            if (drainAudioResult == DRAIN_END_OF_STREAM) {
                audioStreamEnded = true;
            } else if (drainAudioResult == DRAIN_ERROR) {
                return EncodeResult::Failed;
            } else if (drainAudioResult == DRAIN_CANCELLED) {
                return EncodeResult::Cancelled;
            } else {
                KIS_SAFE_ASSERT_RECOVER_NOOP(drainAudioResult >= 0);
            }
        }
    } while (!videoStreamEnded || !audioStreamEnded);

    // Close the encoder, copy the temporary file to the output file if needed.
    {
        int closeResult = int(ctx.encoder().callMethod<jint>("close", "()I"));
        if (ctx.checkResult(QStringLiteral("close"), closeResult)) {
            return EncodeResult::Failed;
        }

        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        if (closeResult == STATUS_NEEDS_COPY) {
            QString copyErrorMessage;
            if (!KisAndroidUtils::copyFile(tempFilePath, settings().outputFile, &copyErrorMessage)) {
                ctx.setInternalErrorMessage(copyErrorMessage);
                return EncodeResult::Failed;
            }
        }
    }

    return EncodeResult::Completed;
}

KisMediaEncoderRunnable::EncodeResult KisAndroidMediaEncoderRunnable::prepareVideo(Context &ctx)
{
    return prepare(ctx, false);
}

KisMediaEncoderRunnable::EncodeResult KisAndroidMediaEncoderRunnable::prepareAudio(Context &ctx)
{
    return prepare(ctx, true);
}

KisMediaEncoderRunnable::EncodeResult KisAndroidMediaEncoderRunnable::prepare(Context &ctx, bool audio)
{
    const char *methodName = audio ? "prepareAudio" : "prepareVideo";
    QString title = audio ? QStringLiteral("prepareAudio") : QStringLiteral("prepareVideo");

    while (true) {
        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        int prepareResult = int(ctx.encoder().callMethod<jint>(methodName, "(J)I", jlong(100000LL)));
        if (ctx.checkResult(title, prepareResult)) {
            return EncodeResult::Failed;

        } else if (prepareResult == STATUS_TIMEOUT) {
            int drainResult = drain(ctx, 0LL, audio);
            if (drainResult == DRAIN_END_OF_STREAM) {
                ctx.setInternalErrorMessage(QStringLiteral("unexpected end of %1 stream")
                                                .arg(audio ? QStringLiteral("audio") : QStringLiteral("video")));
                return EncodeResult::Failed;
            } else if (drainResult == DRAIN_ERROR) {
                return EncodeResult::Failed;
            } else if (drainResult == DRAIN_CANCELLED) {
                return EncodeResult::Cancelled;
            } else {
                KIS_SAFE_ASSERT_RECOVER_NOOP(drainResult >= 0);
            }

        } else {
            KIS_SAFE_ASSERT_RECOVER_NOOP(prepareResult == STATUS_OK);
            break;
        }
    }
    return EncodeResult::Completed;
}

int KisAndroidMediaEncoderRunnable::drainVideo(Context &ctx, long long initialTimeout)
{
    return drain(ctx, initialTimeout, false);
}

int KisAndroidMediaEncoderRunnable::drainAudio(Context &ctx, long long initialTimeout)
{
    return drain(ctx, initialTimeout, true);
}

int KisAndroidMediaEncoderRunnable::drain(Context &ctx, long long initialTimeout, bool audio)
{
    const char *methodName = audio ? "drainAudio" : "drainVideo";
    QString title = audio ? QStringLiteral("drainAudio") : QStringLiteral("drainVideo");

    int count = 0;
    long long timeout = initialTimeout;
    while (true) {
        if (isCancelled()) {
            return DRAIN_CANCELLED;
        }

        int drainResult = int(ctx.encoder().callMethod<jint>(methodName, "(J)I", jlong(timeout)));

        if (ctx.checkResult(title, drainResult)) {
            return DRAIN_ERROR;

        } else if (drainResult == STATUS_TIMEOUT) {
            break;

        } else if (drainResult == STATUS_END_OF_STREAM) {
            return DRAIN_END_OF_STREAM;

        } else {
            KIS_SAFE_ASSERT_RECOVER_NOOP(drainResult == STATUS_OK);
            ++count;
        }
    }
    return count;
}

bool KisAndroidMediaEncoderRunnable::readEncoderImage(Context &ctx, EncoderImage &outImage)
{
    return readPlaneBuffer(ctx, 0, outImage.bufferY) && readPlaneBuffer(ctx, 1, outImage.bufferU)
        && readPlaneBuffer(ctx, 2, outImage.bufferV) && readPlaneRowStride(ctx, 0, outImage.rowStrideY)
        && readPlaneRowStride(ctx, 1, outImage.rowStrideU) && readPlaneRowStride(ctx, 2, outImage.rowStrideV)
        && readPlanePixelStride(ctx, 1, outImage.pixelStrideU) && readPlanePixelStride(ctx, 2, outImage.pixelStrideV);
}

bool KisAndroidMediaEncoderRunnable::readPlaneBuffer(Context &ctx, int index, uint8_t *&outBuffer)
{
    QJniObject plane =
        ctx.encoder().callObjectMethod("getInputImagePlaneBuffer", "(I)Ljava/nio/ByteBuffer;", jint(index));
    if (ctx.checkObject(QStringLiteral("plane"), plane)) {
        return false;
    }

    uint8_t *buffer = static_cast<uint8_t *>(ctx.env()->GetDirectBufferAddress(plane.object<jobject>()));
    if (!buffer) {
        ctx.setInternalErrorMessage(QStringLiteral("null plane buffer %1").arg(index));
        return false;
    }

    outBuffer = buffer;
    return true;
}

bool KisAndroidMediaEncoderRunnable::readPlaneRowStride(Context &ctx, int index, int &outRowStride)
{
    jint rowStride = ctx.encoder().callMethod<jint>("getInputImagePlaneRowStride", "(I)I", jint(index));
    if (ctx.checkException(QStringLiteral("rowStride"))) {
        return false;
    } else if (rowStride <= 0) {
        ctx.setInternalErrorMessage(QStringLiteral("invalid row stride %1: %2").arg(index).arg(rowStride));
        return false;
    }

    outRowStride = int(rowStride);
    return true;
}

bool KisAndroidMediaEncoderRunnable::readPlanePixelStride(Context &ctx, int index, int &outPixelStride)
{
    jint pixelStride = ctx.encoder().callMethod<jint>("getInputImagePlanePixelStride", "(I)I", jint(index));
    if (ctx.checkException(QStringLiteral("pixelStride"))) {
        return false;
    } else if (pixelStride <= 0) {
        ctx.setInternalErrorMessage(QStringLiteral("invalid pixel stride %1: %2").arg(index).arg(pixelStride));
        return false;
    }

    outPixelStride = int(pixelStride);
    return true;
}

bool KisAndroidMediaEncoderRunnable::readAudioInputBuffer(Context &ctx, void *&outBuffer, int &outSize)
{
    QJniObject audioBuffer = ctx.encoder().callObjectMethod("getInputAudioBuffer", "()Ljava/nio/ByteBuffer;", jint());
    if (ctx.checkObject(QStringLiteral("audioBuffer"), audioBuffer)) {
        return false;
    }

    void *buffer = ctx.env()->GetDirectBufferAddress(audioBuffer.object<jobject>());
    if (!buffer) {
        ctx.setInternalErrorMessage(QStringLiteral("null audio buffer"));
        return false;
    }

    jint size = audioBuffer.callMethod<jint>("remaining", "()I");
    if (ctx.checkException(QStringLiteral("remaining"))) {
        return false;
    }

    if (size <= 0) {
        ctx.setInternalErrorMessage(QStringLiteral("audio buffer with size %1").arg(size));
        return false;
    }

    outBuffer = buffer;
    outSize = size;
    return true;
}

void KisAndroidMediaEncoderRunnable::checkFormatSupport(Context &ctx,
                                                        int formatId,
                                                        QVector<KisMediaEncoderFormat *> &outSupportedFormats)
{
    QVector<Format::Encoder> videoEncoders;
    QVector<Format::Encoder> audioEncoders;

    const QPair<QVector<Format::Encoder> *, const char *> ps[] = {
        {&videoEncoders, "getSupportsForVideoFormat"},
        {&audioEncoders, "getSupportsForAudioFormat"},
    };

    for (const QPair<QVector<Format::Encoder> *, const char *> &p : ps) {
        QJniObject supports = QJniObject::callStaticObjectMethod("org/krita/android/VideoEncoder",
                                                                 p.second,
                                                                 "(I)Ljava/util/List;",
                                                                 jint(formatId));
        if (ctx.checkObject(QStringLiteral("supports"), supports)) {
            return;
        }

        jint count = supports.callMethod<jint>("size", "()I");
        if (ctx.checkException(QStringLiteral("size"))) {
            return;
        }

        for (jint i = 0; i < count; ++i) {
            QJniObject entry = supports.callObjectMethod("get", "(I)Ljava/lang/Object;", i);
            if (ctx.checkObject(QStringLiteral("entry"), entry)) {
                continue;
            }

            QJniObject name = entry.getObjectField("name", "Ljava/lang/String;");
            if (ctx.checkObject(QStringLiteral("name"), name)) {
                continue;
            }

            QString nameString = name.toString();
            if (ctx.checkException(QStringLiteral("nameString")) || nameString.isEmpty()) {
                continue;
            }

            bool hardware = entry.getField<jboolean>("hardware");
            if (ctx.checkException(QStringLiteral("hardware"))) {
                continue;
            }

            p.first->append({nameString, hardware});
        }
    }

    if (!videoEncoders.isEmpty()) {
        outSupportedFormats.append(new Format(formatId, videoEncoders, audioEncoders));
    }
}

KisAndroidMediaEncoderPreferencesWidget::KisAndroidMediaEncoderPreferencesWidget(const KisMediaEncoderFormat *format,
                                                                                 QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *form = new QFormLayout(this);

    m_cmbVideoEncoder = new QComboBox;
    form->addRow(i18n("Video encoder:"), m_cmbVideoEncoder);

    m_intVideoBitrate = new QSpinBox;
    m_intVideoBitrate->setRange(1, 999999999);
    form->addRow(i18n("Video bitrate:"), m_intVideoBitrate);

    if (format->supportsAudio()) {
        m_cmbAudioEncoder = new QComboBox;
        form->addRow(i18n("Audio encoder:"), m_cmbAudioEncoder);

        m_intAudioBitrate = new QSpinBox;
        m_intAudioBitrate->setRange(1, 999999999);
        form->addRow(i18n("Audio bitrate:"), m_intAudioBitrate);
    } else {
        m_cmbAudioEncoder = nullptr;
        m_intAudioBitrate = nullptr;
    }
}

void KisAndroidMediaEncoderPreferencesWidget::addVideoEncoderOption(const QString &title, const QString &key)
{
    m_cmbVideoEncoder->addItem(title, QVariant(key));
}

void KisAndroidMediaEncoderPreferencesWidget::addAudioEncoderOption(const QString &title, const QString &key)
{
    m_cmbAudioEncoder->addItem(title, QVariant(key));
}

QString KisAndroidMediaEncoderPreferencesWidget::videoEncoder() const
{
    return m_cmbVideoEncoder->currentData().toString();
}

void KisAndroidMediaEncoderPreferencesWidget::setVideoEncoder(const QString &key)
{
    int index = 0;
    int count = m_cmbVideoEncoder->count();
    for (int i = 0; i < count; ++i) {
        if (m_cmbVideoEncoder->itemData(i).toString() == key) {
            index = i;
            break;
        }
    }
    m_cmbVideoEncoder->setCurrentIndex(index);
}

int KisAndroidMediaEncoderPreferencesWidget::videoBitrate() const
{
    return m_intVideoBitrate->value();
}

void KisAndroidMediaEncoderPreferencesWidget::setVideoBitrate(int videoBitrate)
{
    m_intVideoBitrate->setValue(videoBitrate);
}

QString KisAndroidMediaEncoderPreferencesWidget::audioEncoder() const
{
    return m_cmbAudioEncoder->currentData().toString();
}

void KisAndroidMediaEncoderPreferencesWidget::setAudioEncoder(const QString &key)
{
    int index = 0;
    int count = m_cmbAudioEncoder->count();
    for (int i = 0; i < count; ++i) {
        if (m_cmbAudioEncoder->itemData(i).toString() == key) {
            index = i;
            break;
        }
    }
    m_cmbAudioEncoder->setCurrentIndex(index);
}

int KisAndroidMediaEncoderPreferencesWidget::audioBitrate() const
{
    return m_intAudioBitrate->value();
}

void KisAndroidMediaEncoderPreferencesWidget::setAudioBitrate(int videoBitrate)
{
    m_intAudioBitrate->setValue(videoBitrate);
}
