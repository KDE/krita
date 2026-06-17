/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISANDROIDMEDIAENCODERRUNNABLE
#define KISANDROIDMEDIAENCODERRUNNABLE

#include "KisMediaEncoderWrapper.h"
#include <QWidget>

class QComboBox;
class QSpinBox;

class KisAndroidMediaEncoderRunnable : public KisMediaEncoderRunnable
{
public:
    static KisAndroidMediaEncoderRunnable *create(const KisMediaEncoderWrapperSettings &settings,
                                                  QObject *parent = nullptr);

    static void getSupportedFormats(QVector<KisMediaEncoderFormat *> &outSupportedFormats);

protected:
    KisAndroidMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings, QObject *parent);

    EncodeResult encode(QString &outErrorMessage) override;

private:
    static constexpr int DRAIN_END_OF_STREAM = -1;
    static constexpr int DRAIN_ERROR = -2;
    static constexpr int DRAIN_CANCELLED = -3;
    // Keep these formats in sync with VideoEncoder.java!
    static constexpr int FORMAT_MP4_H264 = 0;
    static constexpr int FORMAT_WEBM_VP8 = 1;
    static constexpr int FORMAT_MP4_AV1 = 2;
    // These statuses too!
    static constexpr int STATUS_OK = 0;
    static constexpr int STATUS_TIMEOUT = 1;
    static constexpr int STATUS_END_OF_STREAM = 2;
    static constexpr int STATUS_NEEDS_COPY = 3;
    static constexpr int STATUS_ERROR_START_ENCODER = 104;
    static constexpr int STATUS_ERROR_DRAIN_MUXER_ADD_TRACK = 404;

    struct EncoderImage;
    class Format;
    class Context;

    EncodeResult prepare(Context &ctx);

    // Returns number of frames drained or one of the DRAIN_* values above.
    int drain(Context &ctx, long long initialTimeout);

    bool copyTemporaryToOutputFile(Context &ctx, const QString &tempPath, const QString &outputPath);

    static bool readEncoderImage(Context &ctx, EncoderImage &outImage);
    static bool readPlaneBuffer(Context &ctx, int index, uint8_t *&outBuffer);
    static bool readPlaneRowStride(Context &ctx, int index, int &outRowStride);
    static bool readPlanePixelStride(Context &ctx, int index, int &outPixelStride);

    static void checkFormatSupport(Context &ctx, int formatId, QVector<KisMediaEncoderFormat *> &outSupportedFormats);
};

class KisAndroidMediaEncoderPreferencesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KisAndroidMediaEncoderPreferencesWidget(QWidget *parent = nullptr);

    void addEncoderOption(const QString &title, const QString &key);

    QString encoder() const;
    void setEncoder(const QString &key);

    int bitrate() const;
    void setBitrate(int bitrate);

private:
    QComboBox *m_cmbEncoder;
    QSpinBox *m_intBitrate;
};

#endif
