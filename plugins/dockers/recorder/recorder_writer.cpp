/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_writer.h"
#include "recorder_const.h"
#include "recorder_export_settings.h"

#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisDocument.h>
#include <KoToolProxy.h>
#include <KisMainWindow.h>

#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QImage>
#include <QRegularExpression>
#include <QApplication>
#include <QMutexLocker>
#include <QPointer>
#include <QTimer>
#include <QVector>
#include <QSharedPointer>
#include <atomic>

namespace
{
const QStringList blacklistedTools = { "KritaTransform/KisToolMove", "KisToolTransform", "KritaShape/KisToolLine" };
}

bool ThreadCounter::set(int value)
{
    auto oldValue = threads;
    threads = static_cast<unsigned int>(
        qBound(1, value, static_cast<int>(ThreadSystemValue::MaxThreadCount))
    );
    return oldValue != threads;
}

void ThreadCounter::setAndNotify(int value)
{
    auto oldValue = get();
    if (set(value)) {
        // Emit signal to GUI that the value has been changed
        Q_EMIT notifyValueChange(oldValue < get());
    }
}

unsigned int ThreadCounter::get() const
{
    return threads;
}

bool ThreadCounter::setUsed(int value)
{
    QMutexLocker lock(&inUseMutex);
    return setUsedImpl(value);
}

void ThreadCounter::setUsedAndNotify(int value)
{
    QMutexLocker lock(&inUseMutex);
    auto oldValue = getUsed();
    if (setUsedImpl(value)) {
        // Emit signal to GUI that the value has been changed
        Q_EMIT notifyInUseChange(oldValue < getUsed());
    }
}

void ThreadCounter::incUsedAndNotify()
{
   QMutexLocker lock(&inUseMutex);
   auto oldValue = getUsed();
   if (setUsedImpl(inUse + 1)) {
        // Emit signal to GUI that the value has been changed
        Q_EMIT notifyInUseChange(oldValue < getUsed());
    }
}
void ThreadCounter::decUsedAndNotify()
{
    QMutexLocker lock(&inUseMutex);
    if (setUsedImpl(inUse - 1)) {
        // Emit signal to GUI that the value has been changed
        Q_EMIT notifyInUseChange(false);
    }
}

unsigned int ThreadCounter::getUsed() const
{
    return inUse;
}

bool ThreadCounter::setUsedImpl(int value)
{
    auto oldValue = inUse;
    inUse = static_cast<unsigned int>(
        qBound(0, value, static_cast<int>(threads))
    );
    return oldValue != inUse;
}

class RecorderWriter::Private
{
public:
    Private(QPointer<KisCanvas2> c, const RecorderWriterSettings& s, const QDir& d)
        : canvas(c)
        , settings(&s)
        , outputDir(&d)
    {}
    Private() = delete;
    Private(const Private&) = default;
    Private(Private&&) = delete;
    Private& operator=(const Private&) = default;
    Private& operator=(Private&&) = delete;

    QPointer<KisCanvas2> canvas;
    QByteArray imageBuffer;
    int imageBufferWidth = 0;
    int imageBufferHeight = 0;
    QImage frame;
    int frameResolution = -1;
    int partIndex = 0;                                     // Consecutive file number
    const RecorderWriterSettings* settings;
    const QDir* outputDir;

    const KoColorSpace *targetCs =
        KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id(),
                                                     KoColorSpaceRegistry::instance()->p709SRGBProfile());

    void captureImage()
    {
        KisImageSP image = canvas->image();

        // Create detached paint device that can be converted to target colorspace
        KisPaintDeviceSP device = new KisPaintDevice(image->colorSpace());

        // we don't want image->barrierLock() because it will wait until the full stroke is finished
        image->immediateLockForReadOnly();
        device->makeCloneFromRough(image->projection(), image->bounds());
        image->unlock();

        const bool needSrgbConversion = [&]() {
            if (image->colorSpace()->colorDepthId() != Integer8BitsColorDepthID
                || image->colorSpace()->colorModelId() != RGBAColorModelID) {
                return true;
            }
            const bool hasPrimaries = image->colorSpace()->profile()->hasColorants();
            const TransferCharacteristics gamma = image->colorSpace()->profile()->getTransferCharacteristics();
            if (hasPrimaries) {
                const ColorPrimaries primaries = image->colorSpace()->profile()->getColorPrimaries();
                if (gamma == TRC_IEC_61966_2_1 && primaries == PRIMARIES_ITU_R_BT_709_5) {
                    return false;
                }
            }
            return true;
        }();

        if (targetCs && needSrgbConversion) {
            device->convertTo(targetCs);
        }

        // truncate uneven image width/height making it even for subdivided size too
        const quint32 bitmask = ~(0xFFFFFFFFu >> (31 - settings->resolution));
        const quint32 width = image->width() & bitmask;
        const quint32 height = image->height() & bitmask;
        const int bufferSize = device->pixelSize() * width * height;

        bool resize = imageBuffer.size() != bufferSize;
        if (resize)
            imageBuffer.resize(bufferSize);

        if (resize || frameResolution != settings->resolution) {
            const int divider = 1 << settings->resolution;
            const int outWidth = width / divider;
            const int outHeight = height / divider;
            uchar *outData = reinterpret_cast<uchar *>(imageBuffer.data());

            frame = QImage(outData, outWidth, outHeight, QImage::Format_ARGB32);
        }

        device->readBytes(reinterpret_cast<quint8 *>(imageBuffer.data()), 0, 0, width, height);

        imageBufferWidth = width;
        imageBufferHeight = height;
    }

    // Calculate ARGB average value using carry save adder:
    //   https://www.qt.io/blog/2009/01/20/50-scaling-of-argb32-image
    inline quint32 avg(quint32 c1, quint32 c2)
    {
        return (((c1 ^ c2) & 0xfefefefeUL) >> 1) + (c1 & c2);
    }

    void halfSizeImageBuffer()
    {
        quint32 *buffer = reinterpret_cast<quint32 *>(imageBuffer.data());
        quint32 *out = buffer;

        for (int y = 0; y < imageBufferHeight; y += 2) {
            const quint32 *in1 = buffer + y * imageBufferWidth;
            const quint32 *in2 = in1 + imageBufferWidth;

            for (int x = 0; x < imageBufferWidth; x += 2) {
                *out = avg(
                    avg(in1[x], in1[x + 1]),
                    avg(in2[x], in2[x + 1])
                );

                ++out;
            }
        }

        imageBufferWidth /= 2;
        imageBufferHeight /= 2;
    }

    inline quint32 blendSourceOver(const int alpha, const quint32 source, const quint32 destination)
    {
        // co = αs x Cs + αb x Cb x (1 – αs)
        // αo = 1, αb = 1

        const int inverseAlpha = 255 - alpha;
        return qRgb(
            (alpha * qRed(source) + inverseAlpha * qRed(destination)) >> 8,
            (alpha * qGreen(source) + inverseAlpha * qGreen(destination)) >> 8,
            (alpha * qBlue(source) + inverseAlpha * qBlue(destination)) >> 8
        );
    }

    void removeFrameTransparency()
    {
        const quint32 background = 0xFFFFFFFF;
        quint32 *buffer = reinterpret_cast<quint32 *>(imageBuffer.data());
        const quint32 *end = buffer + imageBufferWidth * imageBufferHeight;
        while (buffer != end) {
            const int alpha = qAlpha(*buffer);
            switch (alpha) {
            case 0xFF: // fully opaque
                break;
            case 0x00: // fully transparent - just replace to background
                *buffer = background;
                break;
            default: // partly transparent - do color blending
                *buffer = blendSourceOver(alpha, *buffer, background);
                break;
            }
            ++buffer;
        }
    }

    bool writeFrame()
    {
        if (!outputDir->exists() && !outputDir->mkpath(settings->outputDirectory))
            return false;

        const QString fileName = QString("%1").arg(partIndex, 7, 10, QLatin1Char('0'));
        const QString &filePath = QString("%1%2.%3").arg(settings->outputDirectory, fileName,
                                                         RecorderFormatInfo::fileExtension(settings->format));

        int factor = -1; // default value
        switch (settings->format) {
            case RecorderFormat::JPEG:
                factor = settings->quality; // 0...100
                break;
            case RecorderFormat::PNG:
                factor = qBound(0, 100 - (settings->compression * 10), 100); // 0..10 -> 100..0
                break;
        }

        bool result = frame.save(filePath, RecorderFormatInfo::fileFormat(settings->format).data(), factor);
        if (!result)
            QFile(filePath).remove(); // remove corrupted frame
        return result;
    }

};

RecorderWriter::RecorderWriter(
    unsigned int i,
    QPointer<KisCanvas2> c,
    const RecorderWriterSettings& s,
    const QDir& d)
    : d(new Private(c, s, d))
    , id(i)
{}

RecorderWriter::~RecorderWriter()
{
    delete d;
}

void  RecorderWriter::onCaptureImage(int writerId, int index)
{
    if (static_cast<int>(id) != writerId)
        return;

    d->captureImage();

    // downscale image buffer
    for (int res = 0; res < d->settings->resolution; ++res)
        d->halfSizeImageBuffer();

    d->removeFrameTransparency();

    d->partIndex = index;

    bool isFrameWritten = d->writeFrame();

    Q_EMIT capturingDone(id, isFrameWritten);
}


struct WriterPoolEl
{
    using QThreadPtr = QSharedPointer<QThread>;
    using RecorderWriterPtr = QSharedPointer<RecorderWriter>;

    WriterPoolEl(
        QObject* threadParent,
        unsigned int i,
        QPointer<KisCanvas2> c,
        const RecorderWriterSettings& s,
        const QDir& d
    )
        : thread(QThreadPtr::create(threadParent))
        , writer(RecorderWriterPtr::create(i, c, s, d))
    {}

    bool    inUse{false};
    QSharedPointer<QThread> thread;
    QSharedPointer<RecorderWriter> writer;
};

using WriterPool = QVector<WriterPoolEl>;

class RecorderWriterManager::Private
{
public:
    Private(RecorderWriterManager* q_ptr, ThreadCounter& rt)
        : q(q_ptr)
        , recorderThreads(rt)
    {}

    RecorderWriterManager* const q;
    ThreadCounter& recorderThreads;
    volatile std::atomic_bool enabled = false;                  // enable recording only for active documents
    volatile std::atomic_bool imageModified = false;
    volatile std::atomic_bool skipCapturing = false;            // set true on move or transform enabled to prevent tool deactivation
    int partIndex = 0;                                          // Consecutive file number
    std::atomic_int freeWriterId = -1;
    int interval = 1;
    QPointer<KisCanvas2> canvas;
    QTimer timer;
    WriterPool writerPool;
    RecorderWriterSettings settings{};
    QDir outputDir;

    int findLastIndex(const QString &directory)
    {
        QElapsedTimer dbgTimer;
        dbgTimer.start();

        QDirIterator dirIterator(directory);
        const QString &extension = RecorderFormatInfo::fileExtension(settings.format);
        const QRegularExpression &snapshotFilePattern = RecorderConst::snapshotFilePatternFor(extension);

        int recordIndex = -1;
        while (dirIterator.hasNext()) {
            dirIterator.next();

            const QString &fileName = dirIterator.fileName();
            const QRegularExpressionMatch &match = snapshotFilePattern.match(fileName);
            if (match.hasMatch()) {
                int index = match.captured(1).toInt();
                if (recordIndex < index)
                    recordIndex = index;
            }
        }
        dbgTools << "findLastPartNumber for" << directory << ": " << dbgTimer.elapsed() << "ms";

        return recordIndex;
    }

    bool clearWriterPool()
    {
        bool result = true;
        bool alreadyWarn = false;
        bool alreadyErr = false;
        for(auto& el: writerPool)
        {
            el.thread->quit();
            el.thread->wait(RecorderConst::waitThreadTimeoutMs);
            disconnect(q, SIGNAL(startCapturing(int, int)), el.writer.get(), SLOT(onCaptureImage(int, int)));
            disconnect(el.writer.get(), SIGNAL(capturingDone(int, bool)), q, SLOT(onCapturingDone(int, bool)));
            if (el.thread->isRunning())
            {
                if (!alreadyWarn) {
                    warnResources << "One of the Recorder WriterPool threads has been blocked and has to be terminated. "
                                  << "Thread Name: " << el.thread->objectName();
                    alreadyWarn = true;
                }
                el.thread->terminate();
                if (!el.thread->wait(RecorderConst::waitThreadTimeoutMs))
                {
                    if (!alreadyErr) {
                        errResources << "Something odd has been happen. Krita was unable to stop one of the Recorder WriterPool Threads. "
                                     << "Thread Name: " << el.thread->objectName();
                        alreadyErr = true;
                    }
                    result = false;
                }
            }
        }

        writerPool.clear();
        freeWriterId = -1;

        if (!result)
            Q_EMIT q->recorderStopWarning();

        return result;
    }

    void enlargeWriterPool()
    {
        writerPool.reserve(recorderThreads.get());
        while (static_cast<int>(recorderThreads.get()) > writerPool.size()) {
            auto newWorkerId = writerPool.size();
            freeWriterId = newWorkerId - 1; // Set the value to the last existing writerEl index ->
                                            // The next call of searchForFreeWriter() will than automatically find newWorkerId

            writerPool.append(WriterPoolEl(q, newWorkerId, canvas, settings, outputDir));

            auto writerPtr = writerPool[newWorkerId].writer;
            auto threadPtr = writerPool[newWorkerId].thread;
            threadPtr->setObjectName(QString("Krita-Recorder-WriterPool#%1").arg(newWorkerId));
            connect(q, SIGNAL(startCapturing(int, int)), writerPtr.get(), SLOT(onCaptureImage(int, int)));
            connect(writerPtr.get(), SIGNAL(capturingDone(int, bool)), q, SLOT(onCapturingDone(int, bool)));
            writerPtr->moveToThread(threadPtr.get());
            threadPtr->start(QThread::IdlePriority);
        }
    }

    void searchForFreeWriter()
    {
        auto j = freeWriterId + 1;
        for(auto i = 0; i < writerPool.size(); i++, j++)
        {
            freeWriterId = j % writerPool.size();
            if (writerPool[freeWriterId].thread->isRunning() && !writerPool[freeWriterId].inUse)
                return;
        }
        freeWriterId = -1;
    }
};

RecorderWriterManager::RecorderWriterManager(const RecorderExportSettings &es)
    : d(new Private(this, recorderThreads))
    , exporterSettings(es)
{
    d->timer.setTimerType(Qt::PreciseTimer);
}

RecorderWriterManager::~RecorderWriterManager()
{
    delete d;
}

void RecorderWriterManager::setCanvas(QPointer<KisCanvas2> canvas)
{
    // Stop current recording if canvas is about to be changed
    if (d->timer.isActive())
        stop();

    if (d->canvas) {
        disconnect(d->canvas->toolProxy(), SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)));
        disconnect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()));
    }

    d->canvas = canvas;

    if (d->canvas) {
        connect(d->canvas->toolProxy(), SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)),
                Qt::DirectConnection); // need to handle it even if our event loop is not running
        connect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()),
                Qt::DirectConnection); // because it spams
    }
}

void RecorderWriterManager::setup(const RecorderWriterSettings &settings)
{
    // Stop current recording, if setup is called again
    if (d->timer.isActive())
        stop();

    d->settings = settings;
    d->outputDir.setPath(settings.outputDirectory);

    d->partIndex = d->findLastIndex(d->settings.outputDirectory);
}

void RecorderWriterManager::start()
{
    if (d->timer.isActive())
        return;

    if (!d->canvas)
        return;

    d->enabled = true;
    d->imageModified = false;

    connect(&d->timer, SIGNAL (timeout()), this, SLOT (onTimer()));
    if (d->settings.realTimeCaptureMode) {
        d->interval = static_cast<int>(1000.0/static_cast<double>(exporterSettings.fps));
    } else {
        d->interval = static_cast<int>(qMax(d->settings.captureInterval, .1) * 1000.0);
    }
    d->enlargeWriterPool();
    d->timer.start(d->interval);
    Q_EMIT started();
}

bool RecorderWriterManager::stop()
{
    if (!d->timer.isActive())
        return true;

    d->timer.stop();
    auto result = d->clearWriterPool();
    recorderThreads.setUsed(0);
    Q_EMIT stopped();
    return result;
}

void RecorderWriterManager::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

void RecorderWriterManager::onTimer()
{
    if (!d->enabled || !d->canvas)
        return;

    // take snapshots only if main window is active
    // else some dialogs like filters may disappear when canvas->image()->lock() is called
    if (qobject_cast<KisMainWindow*>(QApplication::activeWindow()) == nullptr)
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
        (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup())) {
        return;
    }

    if (!d->imageModified)
        return;

    d->imageModified = false;

    if (d->skipCapturing)
        return;

    d->searchForFreeWriter();

    if (d->freeWriterId == -1)
    {
        Q_EMIT lowPerformanceWarning();
        return;
    }

    d->writerPool[d->freeWriterId].inUse = true;
    d->writerPool[d->freeWriterId].thread->setPriority(QThread::HighPriority);
    recorderThreads.incUsedAndNotify();
    Q_EMIT startCapturing(d->freeWriterId, ++d->partIndex);
}

void RecorderWriterManager::onCapturingDone(int workerId, bool success)
{
    if (workerId >= d->writerPool.size())
        return;
    d->writerPool[workerId].inUse = false;
    d->writerPool[workerId].thread->setPriority(QThread::IdlePriority);
    recorderThreads.decUsedAndNotify();
    if (!success) {
        stop();
        Q_EMIT frameWriteFailed();
    }
}

void RecorderWriterManager::onImageModified()
{
    if (d->skipCapturing || !d->enabled)
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
            (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup()))
        return;

    d->imageModified = true;
}

void RecorderWriterManager::onToolChanged(const QString &toolId)
{
    d->skipCapturing = blacklistedTools.contains(toolId);
}