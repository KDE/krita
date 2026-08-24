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
#include "kis_tool_proxy.h"
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
    const QStringList forceBlacklistedTools = {
        "KisToolTransform",
        "KisToolPolyline",
        "KisToolPolygon",
        "KisToolSelectOutline",
        "KisToolSelectPolygonal",
        "KisToolEncloseAndFill",
        "KisToolPath",
        "KisToolCrop",
        "KisToolSelectPath",
        "KisToolSelectMagnetic",
        "SvgTextTool",
    }; // disable recorder when toggled to one of these tools.
    const QStringList activateBlacklistedTools = {
        "KritaTransform/KisToolMove",
        "KritaShape/KisToolLine",
        "KritaShape/KisToolRectangle",
        "KritaShape/KisToolEllipse",
        "KisToolSelectRectangular",
        "KisToolSelectElliptical",
    }; // disable recorder when toggled to one of these tools and activated tool(left button pressed on canvas).
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
    Private(QPointer<KisCanvas2> c, const RecorderWriterSettings &s, const QDir &d, RecorderWriterManager *m)
        : canvas(c)
        , settings(&s)
        , outputDir(&d)
        , manager(m)
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
    RecorderWriterManager *manager;

    const KoColorSpace *targetCs =
        KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id(),
                                                     KoColorSpaceRegistry::instance()->p709SRGBProfile());

    int captureImage()
    {
        KisImageSP image = canvas->image();

        // Make sure we can actually capture something right now
        {
            QMutexLocker lock(manager->captureMutex());
            if (manager->canStartCapture()) {
                // we don't want image->barrierLock() because it will wait until
                // the full stroke is finished
                image->immediateLockForReadOnly();
                // Grab the next index while the capture mutex is held
                partIndex = manager->incrementAndGetIndex();
            } else {
                return STATUS_BLOCKED;
            }
        }

        // Create detached paint device that can be converted to target colorspace
        KisPaintDeviceSP device = new KisPaintDevice(image->colorSpace());
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
        return STATUS_OK;
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

    int writeFrame()
    {
        if (!outputDir->exists() && !outputDir->mkpath(settings->outputDirectory))
            return STATUS_ERROR;

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

        if (!frame.save(filePath, RecorderFormatInfo::fileFormat(settings->format).data(), factor)) {
            QFile(filePath).remove(); // remove corrupted frame
            return STATUS_ERROR;
        }

        return STATUS_OK;
    }

};

RecorderWriter::RecorderWriter(
    unsigned int i,
    QPointer<KisCanvas2> c,
    const RecorderWriterSettings& s,
    const QDir& d,
    RecorderWriterManager *m)
    : d(new Private(c, s, d, m))
    , id(i)
{}

RecorderWriter::~RecorderWriter()
{
    delete d;
}

void  RecorderWriter::onCaptureImage(int writerId)
{
    if (static_cast<int>(id) != writerId)
        return;

    int captureStatus = d->captureImage();
    if (captureStatus != STATUS_OK) {
        Q_EMIT capturingDone(id, captureStatus);
        return;
    }

    // downscale image buffer
    for (int res = 0; res < d->settings->resolution; ++res)
        d->halfSizeImageBuffer();

    d->removeFrameTransparency();

    int writeStatus = d->writeFrame();

    Q_EMIT capturingDone(id, writeStatus);
}


struct WriterPoolEl
{
    using QThreadPtr = QSharedPointer<QThread>;
    using RecorderWriterPtr = QSharedPointer<RecorderWriter>;

    WriterPoolEl(
        RecorderWriterManager* m,
        unsigned int i,
        QPointer<KisCanvas2> c,
        const RecorderWriterSettings& s,
        const QDir& d
    )
        : thread(QThreadPtr::create(m))
        , writer(RecorderWriterPtr::create(i, c, s, d, m))
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
    volatile std::atomic_bool isForceBlackTool = false;
    volatile std::atomic_bool isActivateBlackTool = false; 
    volatile std::atomic_bool toolActivated = false;
    int partIndex = 0;                                          // Consecutive file number
    std::atomic_int freeWriterId = -1;
    int interval = 1;
    QPointer<KisCanvas2> canvas;
    QTimer timer;
    WriterPool writerPool;
    RecorderWriterSettings settings{};
    QDir outputDir;
    QMutex captureMutex;

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
            disconnect(q, SIGNAL(startCapturing(int)), el.writer.get(), SLOT(onCaptureImage(int)));
            disconnect(el.writer.get(), SIGNAL(capturingDone(int, int)), q, SLOT(onCapturingDone(int, int)));
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
            connect(q, SIGNAL(startCapturing(int)), writerPtr.get(), SLOT(onCaptureImage(int)));
            connect(writerPtr.get(), SIGNAL(capturingDone(int, int)), q, SLOT(onCapturingDone(int, int)));
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
    // Restart writers if canvas changes
    bool restart = d->timer.isActive();
    if (restart) {
        stop(false);
    }

    if (d->canvas) {
        KoToolProxy *proxy = d->canvas->toolProxy();
        KisToolProxy *kritaProxy = dynamic_cast<KisToolProxy*>(proxy);

        disconnect(proxy, SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)));
        disconnect(kritaProxy, SIGNAL(toolPrimaryActionActivated(bool)), this, SLOT(onToolPrimaryActionActivated(bool)));
        disconnect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()));
    }

    d->canvas = canvas;

    if (d->canvas) {
        KoToolProxy *proxy = d->canvas->toolProxy();
        KisToolProxy *kritaProxy = dynamic_cast<KisToolProxy*>(proxy);

        connect(proxy, SIGNAL(toolChanged(QString)), this, SLOT(onToolChanged(QString)),
                Qt::DirectConnection); // need to handle it even if our event loop is not running
        connect(kritaProxy, SIGNAL(toolPrimaryActionActivated(bool)), this, SLOT(onToolPrimaryActionActivated(bool)),
                Qt::DirectConnection);
        connect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()),
                Qt::DirectConnection); // because it spams
    }

    if (restart) {
        start(false);
    }
}

void RecorderWriterManager::setup(const RecorderWriterSettings &settings)
{
    // Restart writers if setup changes
    bool restart = d->timer.isActive();
    if (restart) {
        stop(false);
    }

    d->settings = settings;
    d->outputDir.setPath(settings.outputDirectory);

    d->partIndex = d->findLastIndex(d->settings.outputDirectory);

    if (restart) {
        start(false);
    }
}

void RecorderWriterManager::start(bool toggleEnabled)
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
    if (toggleEnabled) {
        Q_EMIT started();
    }
}

bool RecorderWriterManager::stop(bool toggleEnabled)
{
    if (!d->timer.isActive())
        return true;

    d->timer.stop();
    auto result = d->clearWriterPool();
    recorderThreads.setUsed(0);
    if (toggleEnabled) {
        Q_EMIT stopped();
    }
    return result;
}

void RecorderWriterManager::setEnabled(bool enabled = false)
{
    d->enabled = enabled;
}

bool RecorderWriterManager::canStartCapture() const
{
    if (d->isForceBlackTool)
        return false;
    if (d->isActivateBlackTool && d->toolActivated)
        return false;
    return true;
}

int RecorderWriterManager::incrementAndGetIndex()
{
    return ++d->partIndex;
}

QMutex *RecorderWriterManager::captureMutex()
{
    return &d->captureMutex;
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

    if (!canStartCapture())
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
    Q_EMIT startCapturing(d->freeWriterId);
}

void RecorderWriterManager::onCapturingDone(int workerId, int status)
{
    if (workerId >= d->writerPool.size())
        return;
    d->writerPool[workerId].inUse = false;
    d->writerPool[workerId].thread->setPriority(QThread::IdlePriority);
    recorderThreads.decUsedAndNotify();
    if (status == RecorderWriter::STATUS_ERROR) {
        stop();
        Q_EMIT frameWriteFailed();
    }
}

void RecorderWriterManager::onImageModified()
{
    if (!d->enabled || !canStartCapture() )
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
            (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup()))
        return;

    d->imageModified = true;
}

void RecorderWriterManager::onToolChanged(const QString &toolId)
{
    QMutexLocker lock(&d->captureMutex);
    d->isForceBlackTool = forceBlacklistedTools.contains(toolId);
    d->isActivateBlackTool = activateBlacklistedTools.contains(toolId);
}

void RecorderWriterManager::onToolPrimaryActionActivated(bool activated)
{
    QMutexLocker lock(&d->captureMutex);
    d->toolActivated = activated;
}
