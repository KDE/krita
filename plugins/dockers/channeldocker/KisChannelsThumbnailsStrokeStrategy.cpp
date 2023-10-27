/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisChannelsThumbnailsStrokeStrategy.h"

#include "kis_sequential_iterator.h"
#include "KoColorModelStandardIds.h"
#include "KoColorSpace.h"
#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<QVector<QImage>>("QVector<QImage>");
    QMetaType::registerEqualsComparator<QVector<QImage>>();
}

void KisChannelsThumbnailsStrokeStrategy::reportThumbnailGenerationCompleted(KisPaintDeviceSP device, const QRect &rect)
{
    const KoColorSpace* cs = device->colorSpace();
    const int channelCount = cs->channelCount();
    const QSize thumbnailSize = rect.size();
    const bool invert = (cs->colorModelId() == CMYKAColorModelID);

    QVector<QImage> thumbnails;
    thumbnails.reserve(channelCount);
    for (int i = 0; i < channelCount; i++) {
        thumbnails.push_back(QImage(thumbnailSize, QImage::Format_Grayscale8));
    }

    KisSequentialConstIterator it(device, QRect(0, 0, thumbnailSize.width(), thumbnailSize.height()));

    for (int y = 0; y < thumbnailSize.height(); y++) {
        for (int x = 0; x < thumbnailSize.width(); x++) {
            it.nextPixel();
            const quint8* pixel = it.rawDataConst();
            for (int chan = 0; chan < channelCount; ++chan) {
                QImage &img = thumbnails[chan];
                if (invert) {
                    *(img.scanLine(y) + x) = 255 - cs->scaleToU8(pixel, chan);
                }
                else {
                    *(img.scanLine(y) + x) = cs->scaleToU8(pixel, chan);
                }
            }
        }
    }

    Q_EMIT thumbnailsUpdated(thumbnails, cs);
}
