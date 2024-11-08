/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_writer.h"
#include "recorder_const.h"

#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisDocument.h>
#include "kis_tool_proxy.h"
#include <KisMainWindow.h>

#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QImage>
#include <QRegularExpression>
#include <QApplication>

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

const double lowPerformanceWarningThreshold = 1.25;
const int lowPerformanceWarningMax = 3;
}

class RecorderWriter::Private
{
public:
    QPointer<KisCanvas2> canvas;
    QByteArray imageBuffer;
    int imageBufferWidth = 0;
    int imageBufferHeight = 0;
    QImage frame;
    int frameResolution = -1;
    int partIndex = 0;
    RecorderWriterSettings settings;
    QDir outputDir;
    bool paused = false;
    int interval = 1;
    int lowPerformanceWarningCount = 0;
    volatile bool enabled = false;  // enable recording only for active documents
    volatile bool imageModified = false;
    volatile bool isForceBlackTool = false;
    volatile bool isActivateBlackTool = false;
    volatile bool toolActivated = false;


    const KoColorSpace *targetCs =
        KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                     Integer8BitsColorDepthID.id(),
                                                     KoColorSpaceRegistry::instance()->p709SRGBProfile());

    int findLastIndex(const QString &directory)
    {
        QElapsedTimer timer;
        timer.start();

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
        dbgTools << "findLastPartNumber for" << directory << ": " << timer.elapsed() << "ms";

        return recordIndex;
    }


    void captureImage()
    {
        if (!canvas)
            return;

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
        const quint32 bitmask = ~(0xFFFFFFFFu >> (31 - settings.resolution));
        const quint32 width = image->width() & bitmask;
        const quint32 height = image->height() & bitmask;
        const int bufferSize = device->pixelSize() * width * height;

        bool resize = imageBuffer.size() != bufferSize;
        if (resize)
            imageBuffer.resize(bufferSize);

        if (resize || frameResolution != settings.resolution) {
            const int divider = 1 << settings.resolution;
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
        if (!outputDir.exists() && !outputDir.mkpath(settings.outputDirectory))
            return false;

        const QString fileName = QString("%1").arg(partIndex, 7, 10, QLatin1Char('0'));
        const QString &filePath = QString("%1%2.%3").arg(settings.outputDirectory, fileName,
                                                         RecorderFormatInfo::fileExtension(settings.format));

        int factor = -1; // default value
        switch (settings.format) {
            case RecorderFormat::JPEG:
                factor = settings.quality; // 0...100
                break;
            case RecorderFormat::PNG:
                factor = qBound(0, 100 - (settings.compression * 10), 100); // 0..10 -> 100..0
                break;
        }

        bool result = frame.save(filePath, RecorderFormatInfo::fileFormat(settings.format).data(), factor);
        if (!result)
            QFile(filePath).remove(); // remove corrupted frame
        return result;
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

RecorderWriter::RecorderWriter()
    : d(new Private())
{
    moveToThread(this);
}

RecorderWriter::~RecorderWriter()
{
    delete d;
}

void RecorderWriter::setCanvas(QPointer<KisCanvas2> canvas)
{
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
}

void RecorderWriter::setup(const RecorderWriterSettings &settings)
{
    d->settings = settings;
    d->outputDir.setPath(settings.outputDirectory);

    d->partIndex = d->findLastIndex(d->settings.outputDirectory);
}

bool RecorderWriter::stop()
{
    if (!isRunning())
        return true;

    quit();
    if (!wait(RecorderConst::waitThreadTimeoutMs)) {
        terminate();
        if (!wait(RecorderConst::waitThreadTimeoutMs)) {
            qCritical() << "Unable to stop Writer";
            return false;
        }
    }

    return true;
}

void RecorderWriter::setEnabled(bool enabled)
{
    d->enabled = enabled;
}


void RecorderWriter::timerEvent(QTimerEvent */*event*/)
{
    if (!d->enabled || !d->canvas)
        return;

    // take snapshots only if main window is active
    // else some dialogs like filters may disappear when canvas->image()->lock() is called
    if (qobject_cast<KisMainWindow*>(QApplication::activeWindow()) == nullptr)
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
            (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup())) {
        if (!d->paused) {
            d->paused = true;
            Q_EMIT pausedChanged(d->paused);
        }
        return;
    }

    if (d->imageModified == d->paused) {
        d->paused = !d->imageModified;
        Q_EMIT pausedChanged(d->paused);
    }

    if (!d->imageModified)
        return;

    d->imageModified = false;

    if (!d->canStartCapture())
        return;

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    d->captureImage();

    // downscale image buffer
    for (int res = 0; res < d->settings.resolution; ++res)
        d->halfSizeImageBuffer();

    d->removeFrameTransparency();

    ++d->partIndex;

    bool isFrameWritten = d->writeFrame();
    if (!isFrameWritten) {
        Q_EMIT frameWriteFailed();
        quit();
    }

    qint64 elapsed = elapsedTimer.elapsed();
    if (static_cast<double>(elapsed) > static_cast<double>(d->interval) * lowPerformanceWarningThreshold) {
        ++d->lowPerformanceWarningCount;
        if (d->lowPerformanceWarningCount > lowPerformanceWarningMax) {
            Q_EMIT lowPerformanceWarning();
        }
    } else if (d->lowPerformanceWarningCount != 0) {
        d->lowPerformanceWarningCount = 0;
    }
}

void RecorderWriter::onImageModified()
{
    if (!d->enabled || !d->canStartCapture())
        return;

    if ((!d->settings.recordIsolateLayerMode) &&
            (d->canvas->image()->isIsolatingLayer() || d->canvas->image()->isIsolatingGroup()))
        return;

    if (!d->imageModified)
        Q_EMIT pausedChanged(false);
    d->imageModified = true;
}

void RecorderWriter::onToolChanged(const QString &toolId)
{
    d->isForceBlackTool = forceBlacklistedTools.contains(toolId);
    d->isActivateBlackTool = activateBlacklistedTools.contains(toolId);
}

void RecorderWriter::onToolPrimaryActionActivated(bool activated)
{
    d->toolActivated = activated;
}

void RecorderWriter::run()
{
    if (!d->canvas)
        return;

    d->enabled = true;
    d->paused = true;
    d->imageModified = false;
    Q_EMIT pausedChanged(d->paused);

    d->interval = static_cast<int>(qMax(d->settings.captureInterval, .1) * 1000.);
    const int timerId = startTimer(d->interval);

    QThread::run();

    killTimer(timerId);
}
