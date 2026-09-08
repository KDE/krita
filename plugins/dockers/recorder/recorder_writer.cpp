/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_writer.h"
#include "recorder_image_copying_job.h"
#include "kis_async_action_feedback.h"
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
#include <QtMath>
#include <qobjectdefs.h>

namespace
{
    const QStringList forceBlacklistedTools = {
        "KisToolTransform",
        "KisToolPolyline",
        "KisToolPolygon",
    }; // disable recorder when toggled to one of these tools.
    const QStringList activateBlacklistedTools = {
        "KritaShape/KisToolLine",
        "KritaShape/KisToolRectangle",
        "KritaShape/KisToolEllipse",
    }; // disable recorder when toggled to one of these tools and activated tool(left button pressed on canvas).

    static QString filePathForIndex(int index, QString directory, RecorderFormat format )
    {
        QString fileName = QString("%1").arg(index, 7, 10, QLatin1Char('0'));

        return QString("%1%2.%3").arg(directory,
                                      fileName,
                                      RecorderFormatInfo::fileExtension(format));
    }
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
    bool isActive = false;
    const RecorderWriterSettings* settings;
    const QDir* outputDir;

    const KoColorSpace *targetCs =
        KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id(),
                                                     KoColorSpaceRegistry::instance()->p709SRGBProfile());

    void captureImage(QRect area, KisPaintDevice* device)
    {
        const bool needSrgbConversion = [&]() {
            if (device->colorSpace()->colorDepthId() != Integer8BitsColorDepthID
                || device->colorSpace()->colorModelId() != RGBAColorModelID) {
                return true;
            }
            const bool hasPrimaries = device->colorSpace()->profile()->hasColorants();
            const TransferCharacteristics gamma = device->colorSpace()->profile()->getTransferCharacteristics();
            if (hasPrimaries) {
                const ColorPrimaries primaries = device->colorSpace()->profile()->getColorPrimaries();
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
        const quint32 width = area.width() & bitmask;
        const quint32 height = area.height() & bitmask;
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

        device->readBytes(reinterpret_cast<quint8 *>(imageBuffer.data()), area.x(), area.y(), width, height);

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
        const QString &filePath = QString("%1%2.%3").arg(settings->outputDirectory,
                                                         fileName,
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
        if (!result) {
            QFile(filePath).remove(); // remove corrupted frame
            return false;
        }

        return true;
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

bool RecorderWriter::onCaptureImageImpl(int index, QRect region, KisPaintDevice *device)
{
    d->captureImage(region, device);

    // downscale image buffer
    for (int res = 0; res < d->settings->resolution; ++res)
        d->halfSizeImageBuffer();

    d->removeFrameTransparency();

    d->partIndex = index;

    return d->writeFrame();
}

bool RecorderWriter::isActive()
{
    return d->isActive;
}

void RecorderWriter::onCaptureImage(int writerId, int index, QVector<QRect> regions, KisPaintDeviceSP device)
{
    if (static_cast<int>(id) != writerId)
        return;

    d->isActive = true;

    // Sanity check
    KIS_ASSERT(!regions.isEmpty());

    bool framesWrittens = true;

    Q_FOREACH (QRect region, regions) {
        framesWrittens &= onCaptureImageImpl(index, region, device.data());
        index++;
    }

    d->isActive = false;

    Q_EMIT capturingDone(id, framesWrittens);
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
    bool captureTail = false;
    QRect oldRegion = QRect(-1, -1, -1, -1);

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

        for (auto &el : writerPool) {
            if (el.inUse && !el.writer->isActive()) {
                // There's a pending job, but the writer didn't recieve the signal yet

                // Wait until all the jobs processes
                canvas->image()->waitForDone();

                // Make sure the signals are delivered
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                //we can exit the loop, all the signals have been delivered and we don't need to check further
                break;
            }
        }

        for (auto &el : writerPool) {
            el.thread->quit();
            el.thread->wait(RecorderConst::waitThreadTimeoutMs);
            disconnect(q, SIGNAL(startCapturing(int, int, QVector<QRect>, KisPaintDeviceSP)), el.writer.get(), SLOT(onCaptureImage(int, int, QVector<QRect>, KisPaintDeviceSP)));
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
            connect(q, SIGNAL(startCapturing(int, int,  QVector<QRect>, KisPaintDeviceSP)), writerPtr.get(), SLOT(onCaptureImage(int, int, QVector<QRect>, KisPaintDeviceSP)));
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

    bool canStartCapture() // skip capture when use some tools.
    {
        if (isForceBlackTool)
            return false;
        if (isActivateBlackTool && toolActivated)
            return false;
        return true;
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
    //Saving previous region as a precaution, so that we don't have to externally manage its state
    if (d->canvas) {
        saveRegion();
    }

    // Restart writers if canvas changes
    bool restart = d->timer.isActive();
    if (restart) {
        stop(false, false);
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
        start(false, false);
    }
}

void RecorderWriterManager::setup(const RecorderWriterSettings &settings)
{
    // Restart writers if setup changes
    bool restart = d->timer.isActive();
    if (restart) {
        stop(false, false);
    }

    d->settings = settings;
    d->outputDir.setPath(settings.outputDirectory);

    d->partIndex = d->findLastIndex(d->settings.outputDirectory);

    if (restart) {
        start(false, false);
    }

    //Settings change whenever a different document is about to be recorder, so instead of externally tracking old region, we can just read it from a file
    tryRestoreRegion();
}

void RecorderWriterManager::start(bool toggleEnabled, bool restoreState)
{
    // This function is called in a lot of places, so we need to restore state very slectively to avoid huge performance
    // losses. Even tho restoring state is a lot cheaper than saving it, it's still better to be careful
    if (restoreState) {
        this->restoreState();
    }

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

bool RecorderWriterManager::stop(bool toggleEnabled, bool saveState)
{
    //Check if the timer is active, so that we don't try to clear the writer pool multiple times
    if (!d->timer.isActive())
        return true;

    // This function is called in a lot of places, so we need to save state very slectively to avoid huge performance
    // losses
    if (saveState) {
      this->saveState();
    }

    d->timer.stop();

    bool result = false;

    // Not sure if this is necessary, but, this may prevent the message from popping up when switching tabs
    if (saveState) {
        KisAsyncActionFeedback f(i18n("Stopping recording"), nullptr);
        result = f.runAction([&] {
            return d->clearWriterPool();
        });
    } else {
        result = d->clearWriterPool();
    }

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

void RecorderWriterManager::onTimer()
{
    if ((!d->enabled && !d->captureTail) || !d->canvas) {
        return;
    }

    // take snapshots only if main window is active
    // else some dialogs like filters may disappear when canvas->image()->lock() is called
    if (qobject_cast<KisMainWindow *>(QApplication::activeWindow()) == nullptr && !d->captureTail) {
        return;
    }

    if ((!d->settings.recordIsolateLayerMode) &&
        (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup())) {
        return;
    }

    if (!d->imageModified && !d->captureTail)
        return;

    d->imageModified = false;

    if (!d->canStartCapture())
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

    QRect fullSize = d->canvas->image()->bounds();

    QRect region = d->settings.focusedRecord && !d->captureTail ? d->canvas->regionOfInterest() : fullSize;

    // Handle initialisation
    if (d->oldRegion.width() == -1 || d->oldRegion.height() == 1) {
        d->oldRegion = region;
    }

    bool interpolate = region != d->oldRegion;

    int totalFrames = 1;
    // Change in the region, let's generate some frames
    if (interpolate) {
        float diffX = abs(region.x() - d->oldRegion.x()) / (float)fullSize.width();
        float diffY = abs(region.y() - d->oldRegion.y()) / (float)fullSize.height();
        float diffW = abs(region.width() - d->oldRegion.width()) / (float)fullSize.width();
        float diffH = abs(region.height() - d->oldRegion.height()) / (float)fullSize.height();

        // Find the highest diff
        float maxDiff = std::max({diffX, diffY, diffW, diffH});

        // Maximum amount of interpolation frames to generate
        const int maxFrames = 10;

        // TODO: Potentially change the number of frames function from linear to smth else
        int framesToGenerate = std::ceil(maxFrames * maxDiff);

        totalFrames += framesToGenerate;
    }

    auto lerp = [](QRect a, QRect b, qreal t) {

        KIS_SAFE_ASSERT_RECOVER(t <= 1);
        QRect out;

        out.setX(a.x() + t * (b.x() - a.x()));
        out.setY(a.y() + t * (b.y() - a.y()));
        out.setWidth(a.width() + t * (b.width() - a.width()));
        out.setHeight(a.height() + t * (b.height() - a.height()));

        return out;
    };

    //TODO: try other easing functions
    auto easeOutCubic = [](qreal t) {
        return 1 - qPow(1 - t, 3);
    };

    QVector<QRect> rects;

    for (int frame = 0; frame < totalFrames; frame++) {
        //Linear animations look really bad, so do easing
        QRect interpolatedRect = lerp(d->oldRegion, region, easeOutCubic((qreal)(frame + 1) / (qreal)totalFrames));
        rects.push_back(interpolatedRect);
    };

    QRect copyArea = rects.first();

    if (rects.length() > 1) {
        QRect united = rects.first();

        Q_FOREACH (QRect r, rects) {
            united = united.united(r);
        }

        copyArea = united;
    }

    RecorderImageCopyingJob *job =
        new RecorderImageCopyingJob(d->canvas->image()->projection(), copyArea, d->freeWriterId, ++d->partIndex, rects);

    connect(job,
            SIGNAL(sigImageCopyingDoneWithData(int, int, QVector<QRect>, KisPaintDeviceSP)),
            this,
            SIGNAL(startCapturing(int, int, QVector<QRect>, KisPaintDeviceSP)));

    d->canvas->image()->addSpontaneousJob(job);

    d->partIndex += totalFrames - 1;

    if (!d->captureTail) {
        d->oldRegion = region;
    }

    d->captureTail = false;
}

void RecorderWriterManager::onCapturingDone(int workerId, bool success)
{
    if (workerId >= d->writerPool.size())
        return;
    d->writerPool[workerId].inUse = false;
    d->writerPool[workerId].thread->setPriority(QThread::IdlePriority);
    recorderThreads.decUsedAndNotify();
    if (!success) {
        stop(true, true);
        Q_EMIT frameWriteFailed();
    }
}

void RecorderWriterManager::onImageModified()
{
    if (!d->enabled || !d->canStartCapture())
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
            (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup()))
        return;

    d->imageModified = true;
}

void RecorderWriterManager::onToolChanged(const QString &toolId)
{
    d->isForceBlackTool = forceBlacklistedTools.contains(toolId);
    d->isActivateBlackTool = activateBlacklistedTools.contains(toolId);
}

void RecorderWriterManager::onToolPrimaryActionActivated(bool activated)
{
    d->toolActivated = activated;
}

//Need to save the region, so that there are no jumps when stopping and resuming the recording
void RecorderWriterManager::saveRegion()
{
    const QString filename = "region";
    QDir dir(d->outputDir);

    QFile file(dir.filePath(filename));

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream out(&file);
    out << d->oldRegion;

    file.close();
}

void RecorderWriterManager::tryRestoreRegion()
{
    const QString filename = "region";
    QDir dir(d->outputDir);

    QFile file(dir.filePath(filename));

    if (!file.open(QIODevice::ReadOnly)) {
        d->oldRegion = QRect(-1, -1, -1, -1);
        return;
    }

    QDataStream out(&file);
    out >> d->oldRegion;

    file.close();
}

void RecorderWriterManager::saveState()
{
    saveRegion();

    // Capture some tail frames, but only if we're zoomed in, and if some frames were captured
    const QString filename = "tail";

    if (d->oldRegion == d->canvas->image()->bounds() || d->oldRegion == QRect(-1, -1, -1, -1)) {
        return;
    }

    int beginIndex = d->partIndex + 1;

    d->captureTail = true;
    onTimer();

    int endIndex = d->partIndex;

    QStringList fileNames;

    for (int i = beginIndex; i <= endIndex; i++) {
        fileNames.push_back(filePathForIndex(i, "", d->settings.format));
    }

    QFile file(d->outputDir.filePath(filename));

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QDataStream out(&file);
    out << fileNames;

    file.close();
}

void RecorderWriterManager::restoreState()
{
    tryRestoreRegion();

    // Remove all tail frames
    const QString filename = "tail";

    QStringList fileNames;

    QFile file(d->outputDir.filePath(filename));

    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream out(&file);
    out >> fileNames;

    file.close();
    d->outputDir.remove(filename);

    Q_FOREACH (QString name, fileNames) {
        d->outputDir.remove(name);
    }

    // Move the frame index back
    QRegularExpression fileRegex =
        RecorderConst::snapshotFilePatternFor(RecorderFormatInfo::fileExtension(d->settings.format));

    QRegularExpressionMatch match = fileRegex.match(fileNames.first());

    if (match.hasMatch()) {
        int index = match.captured(1).toInt();
        d->partIndex = index - 1;
    }
}
