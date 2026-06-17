/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QAtomicInt>
#include <QImage>
#include <QImageReader>
#include <QRunnable>
#include <QThreadPool>

#include <functional>

#include <klocalizedstring.h>

#include "KisMediaEncoderWrapper.h"

#include <kis_assert.h>
#include <kis_debug.h>

#ifdef Q_OS_ANDROID
#include "KisAndroidMediaEncoderRunnable.h"
#endif

void KisMediaEncoderRunnable::slotHandleCancelRequested()
{
    m_cancel = true;
}

void KisMediaEncoderRunnable::run()
{
    Q_EMIT sigStarted();
    QString errorMessage;
    EncodeResult result = prepareAndEncode(errorMessage);
    if (result == EncodeResult::Completed) {
        Q_EMIT sigCompleted();
    } else if (result == EncodeResult::Cancelled) {
        Q_EMIT sigCancelled();
    } else {
        Q_EMIT sigFailed(errorMessage);
    }
}

bool KisMediaEncoderRunnable::Frame::readImage(QImage &outImage) const
{
    QImageReader reader(m_path);
    if (reader.read(&outImage)) {
        // Should be non-null on success, but we'll guard against mayhem.
        if (outImage.isNull() || outImage.size().isEmpty()) {
            warnFile.nospace() << "Got null image reading frame from file '" << m_path << "'";
            return false;
        } else {
            return true;
        }
    } else {
        warnFile.nospace() << "Error " << reader.error() << " reading frame from file '" << m_path
                           << "': " << reader.errorString();
        // Again, should be null anyway, but let's be extra sure.
        outImage = QImage();
        return false;
    }
}

KisMediaEncoderRunnable::KisMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

bool KisMediaEncoderRunnable::nextFrame(Frame &outFrame)
{
    int inputFileCount = m_settings.inputFiles.size();
    if (inputFileCount == 0) {
        return false;
    }

    int lastIndex = inputFileCount - 1;
    if (m_needsPreviewBefore) {
        m_needsPreviewBefore = false;
        int seconds = m_settings.firstFrameSec;
        if (seconds > 0) {
            outFrame = Frame(m_settings.inputFiles[lastIndex], seconds * m_settings.outputFps);
            return true;
        }
    }

    double inputFrameDuration = 1.0 / double(m_settings.inputFps);
    double outputFrameDuration = 1.0 / double(m_settings.outputFps);
    while (m_inputFileIndex < inputFileCount) {
        int fileIndex = m_inputFileIndex;
        ++m_inputFileIndex;

        int instances = 0;
        double nextInputTime = double(m_inputFileIndex) * inputFrameDuration;
        while (m_inputTime < nextInputTime) {
            ++instances;
            m_inputTime += outputFrameDuration;
        }

        if (instances != 0) {
            m_outputFrameNo += instances;
            Q_EMIT sigProgressUpdated(m_outputFrameNo);

            // If we're on the last frame, we can append the linger time.
            if (fileIndex == lastIndex && m_needsLingerAfter) {
                m_needsLingerAfter = false;
                instances += qMax(0, m_settings.lastFrameSec) * m_settings.outputFps;
            }

            outFrame = Frame(m_settings.inputFiles[fileIndex], instances);
            return true;
        }
    }

    if (m_needsLingerAfter) {
        m_needsLingerAfter = false;
        int seconds = m_settings.lastFrameSec;
        if (seconds > 0) {
            outFrame = Frame(m_settings.inputFiles[lastIndex], seconds * m_settings.outputFps);
            return true;
        }
    }

    return false;
}

KisMediaEncoderRunnable::EncodeResult KisMediaEncoderRunnable::prepareAndEncode(QString &outErrorMessage)
{
    if (m_cancel) {
        return EncodeResult::Cancelled;
    }

    if (m_settings.inputFiles.isEmpty()) {
        outErrorMessage = i18n("No frame files found");
        return EncodeResult::Failed;
    }

    m_inputFileIndex = 0;
    return encode(outErrorMessage);
}

KisMediaEncoderWrapper::KisMediaEncoderWrapper(QObject *parent)
    : QObject(parent)
{
}

KisMediaEncoderWrapper::~KisMediaEncoderWrapper()
{
    reset();
}

void KisMediaEncoderWrapper::startNonBlocking(const KisMediaEncoderWrapperSettings &settings)
{
    reset();

    KisMediaEncoderRunnable *runnable = makeSupportedRunnable(settings);
    if (runnable) {
        connect(runnable,
                &KisMediaEncoderRunnable::sigStarted,
                this,
                &KisMediaEncoderWrapper::sigStarted,
                Qt::QueuedConnection);
        connect(runnable,
                &KisMediaEncoderRunnable::sigCompleted,
                this,
                &KisMediaEncoderWrapper::sigFinished,
                Qt::QueuedConnection);
        connect(runnable,
                &KisMediaEncoderRunnable::sigCancelled,
                this,
                &KisMediaEncoderWrapper::sigFinished,
                Qt::QueuedConnection);
        connect(runnable,
                &KisMediaEncoderRunnable::sigFailed,
                this,
                &KisMediaEncoderWrapper::sigFinishedWithError,
                Qt::QueuedConnection);
        connect(runnable,
                &KisMediaEncoderRunnable::sigProgressUpdated,
                this,
                &KisMediaEncoderWrapper::sigProgressUpdated,
                Qt::QueuedConnection);
        connect(this,
                &KisMediaEncoderWrapper::sigCancelRequested,
                runnable,
                &KisMediaEncoderRunnable::slotHandleCancelRequested,
                Qt::QueuedConnection);

        m_runnable = runnable;
        runnable->setAutoDelete(true);
        QThreadPool::globalInstance()->start(runnable);

    } else {
        Q_EMIT sigFinishedWithError(i18n("No encoder for the selected format found"));
    }
}

void KisMediaEncoderWrapper::reset()
{
    KisMediaEncoderRunnable *runnable = m_runnable.data();
    if (runnable) {
        m_runnable.clear();
        Q_EMIT sigCancelRequested();
        disconnect(runnable);
    }
}

const QVector<KisMediaEncoderFormat *> &KisMediaEncoderWrapper::getSupportedFormats()
{
    static QVector<KisMediaEncoderFormat *> *supportedFormats;
    if (!supportedFormats) {
        supportedFormats = new QVector<KisMediaEncoderFormat *>();
#ifdef Q_OS_ANDROID
        KisAndroidMediaEncoderRunnable::getSupportedFormats(*supportedFormats);
#endif
    }
    return *supportedFormats;
}

KisMediaEncoderFormat *KisMediaEncoderWrapper::getFormatByKey(const QString &key)
{
    for (KisMediaEncoderFormat *format : getSupportedFormats()) {
        if (format->key() == key) {
            return format;
        }
    }
    return nullptr;
}

KisMediaEncoderRunnable *KisMediaEncoderWrapper::makeSupportedRunnable(const KisMediaEncoderWrapperSettings &settings)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(settings.format, nullptr);

#ifdef Q_OS_ANDROID
    {
        KisAndroidMediaEncoderRunnable *androidRunnable = KisAndroidMediaEncoderRunnable::create(settings);
        if (androidRunnable) {
            return androidRunnable;
        }
    }
#endif

    return nullptr;
}
