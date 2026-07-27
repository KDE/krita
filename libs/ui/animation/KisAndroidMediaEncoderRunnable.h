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
    static constexpr int STATUS_ERROR_START_VIDEO_ENCODER = 104;
    static constexpr int STATUS_ERROR_START_AUDIO_FORMAT = 107;
    static constexpr int STATUS_ERROR_START_AUDIO_ENCODER = 108;
    static constexpr int STATUS_ERROR_DRAIN_VIDEO_MUXER_ADD_TRACK = 404;
    static constexpr int STATUS_ERROR_DRAIN_AUDIO_MUXER_ADD_TRACK = 704;

    struct EncoderImage;
    class Format;
    class Context;
    class Audio;

    EncodeResult prepareVideo(Context &ctx);
    EncodeResult prepareAudio(Context &ctx);
    EncodeResult prepare(Context &ctx, bool audio);

    // Returns number of frames drained or one of the DRAIN_* values above.
    int drainVideo(Context &ctx, long long initialTimeout);
    int drainAudio(Context &ctx, long long initialTimeout);
    int drain(Context &ctx, long long initialTimeout, bool audio);

    static bool readEncoderImage(Context &ctx, EncoderImage &outImage);
    static bool readPlaneBuffer(Context &ctx, int index, uint8_t *&outBuffer);
    static bool readPlaneRowStride(Context &ctx, int index, int &outRowStride);
    static bool readPlanePixelStride(Context &ctx, int index, int &outPixelStride);
    static bool readAudioInputBuffer(Context &ctx, void *&outBuffer, int &outSize);

    static void checkFormatSupport(Context &ctx, int formatId, QVector<KisMediaEncoderFormat *> &outSupportedFormats);
};

class KisAndroidMediaEncoderPreferencesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KisAndroidMediaEncoderPreferencesWidget(const KisMediaEncoderFormat *format, QWidget *parent = nullptr);

    void addVideoEncoderOption(const QString &title, const QString &key);
    void addAudioEncoderOption(const QString &title, const QString &key);

    QString videoEncoder() const;
    void setVideoEncoder(const QString &key);

    int videoBitrate() const;
    void setVideoBitrate(int videoBitrate);

    QString audioEncoder() const;
    void setAudioEncoder(const QString &key);

    int audioBitrate() const;
    void setAudioBitrate(int audioBitrate);

private:
    QComboBox *m_cmbVideoEncoder;
    QSpinBox *m_intVideoBitrate;
    QComboBox *m_cmbAudioEncoder;
    QSpinBox *m_intAudioBitrate;
};

#endif
