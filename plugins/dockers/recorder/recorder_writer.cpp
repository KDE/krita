#include "recorder_writer.h"
#include "recorder_config.h"

#include <QFileInfo>
#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisDocument.h>

#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QImage>
#include <QRegularExpression>

namespace {

constexpr int waitThreadTimeoutMs = 5000;

}

class RecorderWriter::Private {
public:
    QPointer<KisCanvas2> canvas;
    QByteArray buffer;
    QImage frame;

    int partIndex = 0;
    QString outputDirectory;
    int quality = 100;
    int resolution = 0;
    bool paused = false;
    volatile bool imageModified = false;


    void updateOutputDirectory(const RecorderConfig &config) {
        const QString &prefix = config.useDocumentName()
                                    ? canvas->imageView()->document()->uniqueID()
                                    : config.defaultPrefix();
        const QString &outputDirectory = config.snapshotDirectory() % "/" % prefix % "/";

        QDir dir(outputDirectory);
        if (!dir.exists() && !dir.mkpath(outputDirectory))
            return;

        this->outputDirectory = outputDirectory;
        partIndex = findLastIndex(outputDirectory);
    }

    int findLastIndex(const QString &directory)
    {
        QElapsedTimer timer;
        QDirIterator dirIterator(directory);

        int recordIndex = -1;
        QRegularExpression namePattern("^([0-9]{7}).jpg$");
        while (dirIterator.hasNext()) {
            dirIterator.next();

            const QString &fileName = dirIterator.fileName();
            const QRegularExpressionMatch &match = namePattern.match(fileName);
            if (match.hasMatch()) {
                int index = match.captured(1).toInt();
                if (recordIndex < index)
                    recordIndex = index;
            }
        }
        qDebug() << "findLastPartNumber for" << directory << ": " << timer.elapsed() << "ms";

        return recordIndex;
    }


    void readImage()
    {
        if (!canvas)
            return;

        KisImageSP image = canvas->image();

        KisPaintDeviceSP device = image->projection();

        const int width = image->width();
        const int height = image->height();
        const int bufferSize = device->pixelSize() * width * height;

        if (buffer.size() != bufferSize) {
            buffer.resize(bufferSize);

            const int divider = resolution ? (resolution << 2) : 1;
            const int outWidth = width / divider;
            const int outHeight = height / divider;
            uchar *outData = reinterpret_cast<uchar *>(buffer.data());

            // TODO: add second buffer for downscaled version and use it in image
            // downscaledBuffer.resize(bufferSize / (divider * divider));
            // outData = reinterpret_cast<uchar *>(downscaledBuffer.data());

            // FIXME: image format??
            frame = QImage(outData, outWidth, outHeight, QImage::Format_ARGB32);
        }

        // we don't want image->barrierLock() because it will wait until the full stroke is finished
        image->lock();
        device->readBytes(reinterpret_cast<quint8 *>(buffer.data()), frame.rect());
        image->unlock();
    }

    bool writeFrame()
    {
        const QString &fileName = QString("%1%2.jpg")
                                      .arg(outputDirectory)
                                      .arg(partIndex, 7, 10, QLatin1Char('0'));
qDebug() << "WRITE FRAME" << fileName;
        return frame.save(fileName, "JPEG", quality);
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
    if (d->canvas == canvas)
        return;

    if (d->canvas)
        disconnect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()));

    d->canvas = canvas;

    if (d->canvas) {
        d->updateOutputDirectory(RecorderConfig(true));

        connect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()),
            Qt::DirectConnection); // because it spams
    }
}

bool RecorderWriter::stop()
{
    if (!isRunning())
        return true;

    quit();
    if (!wait(waitThreadTimeoutMs)) {
        terminate();
        if (!wait(waitThreadTimeoutMs)) {
            qCritical() << "Unable to stop Writer";
            return false;
        }
    }

    return true;
}

void RecorderWriter::timerEvent(QTimerEvent */*event*/)
{
    if (d->imageModified == d->paused) {
        d->paused = !d->imageModified;
        emit pausedChanged(d->paused);
    }

    if (!d->imageModified)
        return;

    d->imageModified = false;

    d->readImage();

    // TODO: downscale buffer using fast algorightm
    //   https://www.qt.io/blog/2009/01/20/50-scaling-of-argb32-image
    //quint32 avg = (((c1 ^ c2) & 0xfefefefeUL) >> 1) + (c1 & c2);


    ++d->partIndex;
    bool isFrameWritten = d->writeFrame();
    if (!isFrameWritten)
        quit();
}

void RecorderWriter::onImageModified()
{
    d->imageModified = true;
}

void RecorderWriter::run()
{
    if (!d->canvas)
        return;

    d->paused = true;
    d->imageModified = false;
    emit pausedChanged(d->paused);

    RecorderConfig config(true);

    d->updateOutputDirectory(config);

    d->quality = config.quality();
    d->resolution = config.resolution();

    const int intervalSeconds = qMax(config.captureInterval(), 1);
    const int timerId = startTimer(intervalSeconds * 1000);

    QThread::run();

    killTimer(timerId);
}
