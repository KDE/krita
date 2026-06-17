/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISMEDIAENCODERWRAPPER
#define KISMEDIAENCODERWRAPPER

#include <QDir>
#include <QObject>
#include <QPointer>
#include <QRunnable>
#include <QSize>
#include <QVariantMap>
#include <QVector>

#include <kritaui_export.h>

class QImage;
class QWidget;

class KRITAUI_EXPORT KisMediaEncoderFormat
{
public:
    enum class Type {
        AndroidMediaEncoder,
    };

    virtual ~KisMediaEncoderFormat() = default;

    virtual Type type() const = 0;
    virtual QString key() const = 0;
    virtual QString title() const = 0;
    virtual QString extension() const = 0;

    virtual QWidget *createPreferencesWidget(const QVariantMap &preferences) const = 0;
    virtual void resetPreferencesWidget(QWidget *widget) const = 0;
    virtual QVariantMap getPreferencesFromWidget(QWidget *widget) const = 0;

protected:
    KisMediaEncoderFormat() = default;
};

struct KRITAUI_EXPORT KisMediaEncoderWrapperSettings {
    QString outputFile;
    QStringList inputFiles;
    KisMediaEncoderFormat *format;
    QVariantMap formatPreferences;
    QSize outputSize;
    int inputFps;
    int outputFps;
    int firstFrameSec;
    int lastFrameSec;
};

// Intentionally not exported, this is only needed internally.
class KisMediaEncoderRunnable : public QObject, public QRunnable
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(KisMediaEncoderRunnable)
public:
    void run() override;

public Q_SLOTS:
    void slotHandleCancelRequested();

Q_SIGNALS:
    void sigStarted();
    void sigCompleted();
    void sigCancelled();
    void sigFailed(const QString &errorMessage);
    void sigProgressUpdated(int frameNo);

protected:
    enum class EncodeResult {
        Completed,
        Cancelled,
        Failed,
    };

    class Frame
    {
    public:
        explicit Frame(const QString &path = QString(), int instances = 0)
            : m_path(path)
            , m_instances(instances)
        {
        }

        int instances() const
        {
            return m_instances;
        }

        bool readImage(QImage &outImage) const;

    private:
        QString m_path;
        int m_instances;
    };

    KisMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings, QObject *parent);

    virtual EncodeResult encode(QString &outErrorMessage) = 0;

    const KisMediaEncoderWrapperSettings settings()
    {
        return m_settings;
    }

    bool isCancelled() const
    {
        return m_cancel;
    }

    bool nextFrame(Frame &outFrame);

private:
    EncodeResult prepareAndEncode(QString &outErrorMessage);

    KisMediaEncoderWrapperSettings m_settings;
    double m_inputTime = 0.0;
    int m_inputFileIndex = 0;
    int m_outputFrameNo = 0;
    bool m_needsPreviewBefore = true;
    bool m_needsLingerAfter = true;
    bool m_cancel = false;
};

class KRITAUI_EXPORT KisMediaEncoderWrapper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(KisMediaEncoderWrapper)
public:
    explicit KisMediaEncoderWrapper(QObject *parent = nullptr);
    ~KisMediaEncoderWrapper() override;

    void startNonBlocking(const KisMediaEncoderWrapperSettings &settings);
    void reset();

    static const QVector<KisMediaEncoderFormat *> &getSupportedFormats();
    static KisMediaEncoderFormat *getFormatByKey(const QString &key);

Q_SIGNALS:
    void sigStarted();
    void sigFinished();
    void sigFinishedWithError(QString message);
    void sigProgressUpdated(int frameNo);
    void sigCancelRequested();

private:
    static KisMediaEncoderRunnable *makeSupportedRunnable(const KisMediaEncoderWrapperSettings &settings);

    QPointer<KisMediaEncoderRunnable> m_runnable;
};

#endif
