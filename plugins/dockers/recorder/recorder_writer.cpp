/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "recorder_writer.h"

#include <QFileInfo>
#include <kis_canvas2.h>
#include <kis_image.h>
#include <KisDocument.h>

#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QImage>
#include <QRegularExpression>

namespace
{
constexpr int waitThreadTimeoutMs = 5000;
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
    bool paused = false;
    volatile bool imageModified = false;


    int findLastIndex(const QString &directory)
    {
        QElapsedTimer timer;
        timer.start();

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

        // we don't want image->barrierLock() because it will wait until the full stroke is finished
        image->lock();
        device->readBytes(reinterpret_cast<quint8 *>(imageBuffer.data()), 0, 0, width, height);
        image->unlock();

        imageBufferWidth = width;
        imageBufferHeight = height;
    }

    // Calculate ARGB average value using carry save adder:
    //   https://www.qt.io/blog/2009/01/20/50-scaling-of-argb32-image
    inline quint32 avg(quint32 c1, quint32 c2) {
        return (((c1 ^ c2) & 0xfefefefeUL) >> 1) + (c1 & c2);
    }

    void halfSizeImageBuffer()
    {
        // make width and height even
        const int width = imageBufferWidth & ~1;
        const int height = imageBufferHeight & ~1;

        quint32 *buffer = reinterpret_cast<quint32 *>(imageBuffer.data());
        quint32 *out = buffer;

        for (int y = 0; y < height; y += 2) {
            const quint32 *in1 = buffer + y * imageBufferWidth;
            const quint32 *in2 = in1 + imageBufferWidth;

            for (int x = 0; x < width; x += 2) {
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

    bool writeFrame()
    {
        const QString &fileName = QString("%1%2.jpg")
                                  .arg(settings.outputDirectory)
                                  .arg(partIndex, 7, 10, QLatin1Char('0'));
        return frame.save(fileName, "JPEG", settings.quality);
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
    if (d->canvas)
        disconnect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()));

    d->canvas = canvas;

    if (d->canvas)
        connect(d->canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(onImageModified()),
                Qt::DirectConnection); // because it spams
}

void RecorderWriter::setup(const RecorderWriterSettings &settings)
{
    d->settings = settings;

    QDir dir(d->settings.outputDirectory);
    if (!dir.exists() && !dir.mkpath(d->settings.outputDirectory))
        return;

    d->partIndex = d->findLastIndex(d->settings.outputDirectory);
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

    // downscale image buffer
    for (int res = 0; res < d->settings.resolution; ++res)
        d->halfSizeImageBuffer();

    ++d->partIndex;
    bool isFrameWritten = d->writeFrame();
    if (!isFrameWritten)
        quit();
}

void RecorderWriter::onImageModified()
{
    if (!d->imageModified)
        emit pausedChanged(false);
    d->imageModified = true;
}

void RecorderWriter::run()
{
    if (!d->canvas)
        return;

    d->paused = true;
    d->imageModified = false;
    emit pausedChanged(d->paused);

    const int interval = qMax(d->settings.captureInterval, 1);
    const int timerId = startTimer(interval * 1000);

    QThread::run();

    killTimer(timerId);
}
