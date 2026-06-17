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

#include <kis_debug.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

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

    Format(int formatId, const QVector<Encoder> &encoders)
        : KisMediaEncoderFormat()
        , m_formatId(formatId)
    {
        // Prefer software encoders, because hardware encoders are often busted.
        m_encoders.reserve(encoders.size());
        for (const Encoder &encoder : encoders) {
            if (!encoder.hardware) {
                m_encoders.append(encoder);
            }
        }
        for (const Encoder &encoder : encoders) {
            if (encoder.hardware) {
                m_encoders.append(encoder);
            }
        }
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

    QWidget *createPreferencesWidget(const QVariantMap &preferences) const override
    {
        KisAndroidMediaEncoderPreferencesWidget *pw = new KisAndroidMediaEncoderPreferencesWidget;

        for (const Encoder &encoder : m_encoders) {
            QString title;
            if (encoder.hardware) {
                title = i18n("%1 (hardware)", encoder.name);
            } else {
                title = i18n("%1 (software)", encoder.name);
            }
            pw->addEncoderOption(title, encoder.name);
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
        preferences.insert(QStringLiteral("encoder"), pw->encoder());
        preferences.insert(QStringLiteral("bitrate"), pw->bitrate());
        return preferences;
    }

    QString getEncoderPreference(const QVariantMap &preferences) const
    {
        QString encoder = preferences.value(QStringLiteral("encoder")).toString();
        if (encoder.isEmpty() && !m_encoders.isEmpty()) {
            return m_encoders.constFirst().name;
        } else {
            return encoder;
        }
    }

    int getBitratePreference(const QVariantMap &preferences) const
    {
        int bitrate = preferences.value(QStringLiteral("bitrate")).toInt();
        if (bitrate <= 0) {
            return defaultBitrateForFormatId(m_formatId);
        } else {
            return bitrate;
        }
    }

private:
    void applyPreferencesToWidget(KisAndroidMediaEncoderPreferencesWidget *pw, const QVariantMap &preferences) const
    {
        pw->setEncoder(getEncoderPreference(preferences));
        pw->setBitrate(getBitratePreference(preferences));
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

    static int defaultBitrateForFormatId(int formatId)
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

    QVector<Encoder> m_encoders;
    int m_formatId;
};

class KisAndroidMediaEncoderRunnable::Context
{
public:
    explicit Context(QString *outErrorMessage = nullptr)
        : m_outErrorMessage(outErrorMessage)
    {
    }

    ~Context()
    {
        if (m_imageFormat != AV_PIX_FMT_NONE) {
            av_freep(&m_imageBuffers[0]);
        }
        sws_freeContext(m_swsContext);
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
        } else if (result == STATUS_ERROR_START_ENCODER) {
            warnFile << "Start encoder error" << result;
            if (m_outErrorMessage) {
                *m_outErrorMessage = i18n("Unsupported video parameters, try lowering the video FPS or size");
            }
            return true;
        } else if (result == STATUS_ERROR_DRAIN_MUXER_ADD_TRACK) {
            warnFile << "Muxer track error" << result;
            if (m_outErrorMessage) {
                *m_outErrorMessage = i18n("Unsupported format");
            }
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

    SwsContext *getSwsContextFor(int inputWidth,
                                 int inputHeight,
                                 AVPixelFormat inputFormat,
                                 int outputWidth,
                                 int outputHeight,
                                 AVPixelFormat outputFormat,
                                 int flags)
    {
        return sws_getCachedContext(m_swsContext,
                                    inputWidth,
                                    inputHeight,
                                    inputFormat,
                                    outputWidth,
                                    outputHeight,
                                    outputFormat,
                                    flags,
                                    nullptr,
                                    nullptr,
                                    nullptr);
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

    void setInternalErrorMessage(const QString &detail)
    {
        warnFile << "Media encoder error:" << detail;
        if (m_outErrorMessage) {
            // Internal encoder errors are only really useful for developers,
            // so there's no point in translating them.
            *m_outErrorMessage = i18n("Internal error (%1)", detail);
        }
    }

private:
    static bool isErrorResult(int result)
    {
        return result >= 100;
    }

    QJniEnvironment m_env;
    QJniObject m_encoder;
    QString *m_outErrorMessage;
    SwsContext *m_swsContext = nullptr;
    uint8_t *m_imageBuffers[4] = {nullptr, nullptr, nullptr, nullptr};
    int m_imageLinesizes[4] = {0, 0, 0, 0};
    AVPixelFormat m_imageFormat = AV_PIX_FMT_NONE;
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

        QJniObject encoderName = QJniObject::fromString(format->getEncoderPreference(settings().formatPreferences));
        ctx.checkException(QStringLiteral("encoderName"));

        ctx.setEncoder(QJniObject("org/krita/android/VideoEncoder",
                                  "(IIIFLjava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V",
                                  jint(format->formatId()),
                                  jint(outputWidth),
                                  jint(outputHeight),
                                  jfloat(settings().outputFps),
                                  outputPath.object<jstring>(),
                                  tempPath.object<jstring>(),
                                  encoderName.object<jstring>(),
                                  jint(format->getBitratePreference(settings().formatPreferences))));
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
    while (nextFrame(frame)) {
        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        // Grab the next frame from disk.
        QImage inputImage;
        AVPixelFormat inputPixelFormat;
        if (frame.readImage(inputImage)) {
            switch (inputImage.format()) {
            case QImage::Format_RGB32:
                inputPixelFormat = AV_PIX_FMT_BGR0;
                break;
            case QImage::Format_ARGB32:
                inputPixelFormat = AV_PIX_FMT_BGRA;
                break;
            default:
                // The above are the only formats I can get the the recorder to
                // produce, so I'm not gonna get experimental with this.
                inputPixelFormat = AV_PIX_FMT_BGRA;
                inputImage = inputImage.convertToFormat(QImage::Format_ARGB32);
                if (inputImage.isNull()) {
                    warnFile << "Frame conversion from" << inputImage.format() << "failed";
                    continue;
                }
                break;
            }
        } else {
            continue; // Keep going, some frames may be corrupted.
        }

        int instances = frame.instances();
        for (int i = 0; i < instances; ++i) {
            // Grab a buffer from the encoder.
            EncodeResult prepareResult = prepare(ctx);
            if (prepareResult != EncodeResult::Completed) {
                return prepareResult;
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
                                                          SWS_FAST_BILINEAR);
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

            int commitResult = int(ctx.encoder().callMethod<jint>("commit", "()I"));
            if (ctx.checkResult(QStringLiteral("commit"), commitResult)) {
                return EncodeResult::Failed;
            }
        }
    }

    if (isCancelled()) {
        return EncodeResult::Cancelled;
    }

    // Finish the encoder stream.
    {
        // Need a buffer from the encoder to tell it that it's done.
        EncodeResult prepareResult = prepare(ctx);
        if (prepareResult != EncodeResult::Completed) {
            return prepareResult;
        }
        // Hand an empty buffer back with the end of stream flag set.
        int finishResult = int(ctx.encoder().callMethod<jint>("finish", "()I"));
        if (ctx.checkResult(QStringLiteral("finish"), finishResult)) {
            return EncodeResult::Failed;
        }
    }

    // Drain all remaining frames out of the encoder.
    while (true) {
        int drainResult = drain(ctx, 1000000LL);
        if (drainResult == DRAIN_END_OF_STREAM) {
            break;
        } else if (drainResult == DRAIN_ERROR) {
            return EncodeResult::Failed;
        } else if (drainResult == DRAIN_CANCELLED) {
            return EncodeResult::Cancelled;
        } else {
            KIS_SAFE_ASSERT_RECOVER_NOOP(drainResult >= 0);
        }
    }

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
            if (!copyTemporaryToOutputFile(ctx, tempFilePath, settings().outputFile)) {
                return EncodeResult::Failed;
            }
        }
    }

    return EncodeResult::Completed;
}

KisMediaEncoderRunnable::EncodeResult KisAndroidMediaEncoderRunnable::prepare(Context &ctx)
{
    while (true) {
        if (isCancelled()) {
            return EncodeResult::Cancelled;
        }

        int prepareResult = int(ctx.encoder().callMethod<jint>("prepare", "(J)I", jlong(100000LL)));
        if (ctx.checkResult(QStringLiteral("prepare"), prepareResult)) {
            return EncodeResult::Failed;

        } else if (prepareResult == STATUS_TIMEOUT) {
            int drainResult = drain(ctx, 0LL);
            if (drainResult == DRAIN_END_OF_STREAM) {
                ctx.setInternalErrorMessage(QStringLiteral("unexpected end of stream"));
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

int KisAndroidMediaEncoderRunnable::drain(Context &ctx, long long initialTimeout)
{
    int count = 0;
    long long timeout = initialTimeout;
    while (true) {
        if (isCancelled()) {
            return DRAIN_CANCELLED;
        }

        int drainResult = int(ctx.encoder().callMethod<jint>("drain", "(J)I", jlong(timeout)));

        if (ctx.checkResult(QStringLiteral("drain"), drainResult)) {
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

bool KisAndroidMediaEncoderRunnable::copyTemporaryToOutputFile(Context &ctx,
                                                               const QString &tempPath,
                                                               const QString &outputPath)
{
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::ReadOnly)) {
        ctx.setInternalErrorMessage(
            QStringLiteral("failed to open temp file '%1': %2").arg(tempPath).arg(tempFile.errorString()));
        return false;
    }

    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        ctx.setInternalErrorMessage(
            QStringLiteral("failed to open output file '%1': %2").arg(outputPath).arg(outputFile.errorString()));
        return false;
    }

    QByteArray buffer;
    buffer.resize(BUFSIZ);
    while (true) {
        qint64 read = tempFile.read(buffer.data(), BUFSIZ);
        if (read < 0) {
            ctx.setInternalErrorMessage(
                QStringLiteral("failed to read from temp file '%1': %2").arg(tempPath).arg(tempFile.errorString()));
            return false;
        } else if (read > 0) {
            qint64 written = outputFile.write(buffer, read);
            if (written < 0) {
                ctx.setInternalErrorMessage(QStringLiteral("failed to write %1 byte(s) to output file '%2': %3")
                                                .arg(read)
                                                .arg(outputPath)
                                                .arg(outputFile.errorString()));
                return false;
            } else if (written != read) {
                ctx.setInternalErrorMessage(
                    QStringLiteral("tried to write %1 byte(s) to output file '%2', but only wrote %3")
                        .arg(read)
                        .arg(outputPath)
                        .arg(written));
                return false;
            }
        } else {
            if (outputFile.flush()) {
                return true;
            } else {
                ctx.setInternalErrorMessage(QStringLiteral("failed to flush output file '%1': %2")
                                                .arg(outputPath)
                                                .arg(outputFile.errorString()));
                return false;
            }
        }
    }
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

void KisAndroidMediaEncoderRunnable::checkFormatSupport(Context &ctx,
                                                        int formatId,
                                                        QVector<KisMediaEncoderFormat *> &outSupportedFormats)
{
    QJniObject supports = QJniObject::callStaticObjectMethod("org/krita/android/VideoEncoder",
                                                             "getSupportsForFormat",
                                                             "(I)Ljava/util/List;",
                                                             jint(formatId));
    if (ctx.checkObject(QStringLiteral("supports"), supports)) {
        return;
    }

    jint count = supports.callMethod<jint>("size", "()I");
    if (ctx.checkException(QStringLiteral("size"))) {
        return;
    }

    QVector<Format::Encoder> encoders;
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

        encoders.append({nameString, hardware});
    }

    if (!encoders.isEmpty()) {
        outSupportedFormats.append(new Format(formatId, encoders));
    }
}

KisAndroidMediaEncoderPreferencesWidget::KisAndroidMediaEncoderPreferencesWidget(QWidget *parent)
    : QWidget(parent)
{
    QFormLayout *form = new QFormLayout(this);

    m_cmbEncoder = new QComboBox;
    form->addRow(i18n("Encoder:"), m_cmbEncoder);

    m_intBitrate = new QSpinBox;
    m_intBitrate->setRange(1, 999999999);
    form->addRow(i18n("Bitrate:"), m_intBitrate);
}

void KisAndroidMediaEncoderPreferencesWidget::addEncoderOption(const QString &title, const QString &key)
{
    m_cmbEncoder->addItem(title, QVariant(key));
}

QString KisAndroidMediaEncoderPreferencesWidget::encoder() const
{
    return m_cmbEncoder->currentData().toString();
}

void KisAndroidMediaEncoderPreferencesWidget::setEncoder(const QString &key)
{
    int index = 0;
    int count = m_cmbEncoder->count();
    for (int i = 0; i < count; ++i) {
        if (m_cmbEncoder->itemData(i).toString() == key) {
            index = i;
            break;
        }
    }
    m_cmbEncoder->setCurrentIndex(index);
}

int KisAndroidMediaEncoderPreferencesWidget::bitrate() const
{
    return m_intBitrate->value();
}

void KisAndroidMediaEncoderPreferencesWidget::setBitrate(int bitrate)
{
    m_intBitrate->setValue(bitrate);
}
