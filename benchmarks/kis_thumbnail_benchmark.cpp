/*
 *  Copyright (c) 2016 Eugene Ingerman geneing at gmail dot com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_thumbnail_benchmark.h"
#include "kis_benchmark_values.h"

#include <QTest>
#include <QImage>
#include "kis_iterator_ng.h"

#include "kis_paint_device.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoCompositeOpRegistry.h"
#include "KoColor.h"

#include "kis_image.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_sequential_iterator.h"


#include <cmath>

#define SAVE_OUTPUT
const int THUMBNAIL_WIDTH = 64;
const int THUMBNAIL_HEIGHT =64;
const int IMAGE_WIDTH = 8000;
const int IMAGE_HEIGHT = 6000;
const int OVERSAMPLE = 4;

void KisThumbnailBenchmark::initTestCase()
{
    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();

    m_dev = new KisPaintDevice(m_colorSpace);
    KoColor color(m_colorSpace);
    color.fromQColor(Qt::white);

    m_dev->clear();
    m_dev->fill(0,0,IMAGE_WIDTH,IMAGE_HEIGHT,color.data());

    color.fromQColor(Qt::black);

    KisPainter painter(m_dev);
    painter.setPaintColor(color);
    float radius = std::min(IMAGE_WIDTH,IMAGE_HEIGHT);
    const float angle = 2*3.1415926/360.;
    const float endWidth = 30;

    for (float i = 0; i < 90; i+=5){
        painter.drawThickLine(QPointF(0,0),QPointF(radius*std::sin(angle*i),radius*std::cos(angle*i)),1,endWidth);
        painter.drawThickLine(QPointF(IMAGE_WIDTH,IMAGE_HEIGHT),QPointF(IMAGE_WIDTH-radius*std::sin(angle*i),IMAGE_HEIGHT-radius*std::cos(angle*i)),1,endWidth);
    }
#ifdef SAVE_OUTPUT
    m_dev->convertToQImage(m_colorSpace->profile()).save("ThumbFullImage.png");
#endif
}

void KisThumbnailBenchmark::cleanupTestCase()
{
}

void KisThumbnailBenchmark::benchmarkCreateThumbnail()
{
    QImage image;
    QBENCHMARK{
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT );
        m_dev->setDirty();
    }
    image.save("createThumbnail.png");
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQ()
{
    QImage image;
    QBENCHMARK{
        image = m_dev->createThumbnail(2*THUMBNAIL_WIDTH, 2*THUMBNAIL_HEIGHT );
        image = image.scaled(THUMBNAIL_WIDTH,THUMBNAIL_HEIGHT,Qt::KeepAspectRatio, Qt::SmoothTransformation );
        m_dev->setDirty();
    }
    image.save("createThumbnailHiQ.png");
}


void KisThumbnailBenchmark::initThumbnailImages(QSize size, int nChannels)
{
    m_thumbnails.clear();
    m_thumbnails.resize(nChannels);

    QSize thumbnailSizeOversample = m_thumbnailSizeLimit;

    //We allocate space for image that is oversampleRatio times larger than the end image
    thumbnailSizeOversample *= m_oversampleRatio;

    //compute decimation step size
    m_skipCount = static_cast<int>(std::max( size.width()/thumbnailSizeOversample.width(), size.height()/thumbnailSizeOversample.height() ));
    m_skipCount = std::max(m_skipCount, 1); //skip count of 1, means we are getting back the original image. Can't go lower.

    thumbnailSizeOversample = size;
    thumbnailSizeOversample /= m_skipCount;

    for(int i=0; i<nChannels; ++i){
        //channel thumbnails are grayscale images.
        m_thumbnails[i]=QImage(thumbnailSizeOversample,QImage::Format_Grayscale8);
    }
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQThumbnailIterator()
{
    int channelCount = m_colorSpace->channelCount();
    QVector<QImage> image(channelCount);

    QBENCHMARK{
        QVector<uchar*> image_cache(channelCount);

        auto image_cache_iterator = image_cache.begin();
        for(auto &img: image ){
            img = QImage(QSize(OVERSAMPLE*THUMBNAIL_WIDTH, OVERSAMPLE*THUMBNAIL_HEIGHT),QImage::Format_Grayscale8);
            *(image_cache_iterator++) = img.bits();
        }

        auto thumbnailDev = m_dev->createThumbnailDevice(OVERSAMPLE*THUMBNAIL_WIDTH, OVERSAMPLE*THUMBNAIL_HEIGHT);
        KisSequentialConstIterator it( thumbnailDev, QRect(0,0,OVERSAMPLE*THUMBNAIL_WIDTH, OVERSAMPLE*THUMBNAIL_HEIGHT));

        do{
            const quint8* pixel = it.rawDataConst();
            for(int i=0; i<channelCount; ++i){
                *(image_cache[i]++)=m_colorSpace->scaleToU8(pixel,i);
            }
        }while(it.nextPixel());
        for(auto &img: image ){
            img = img.scaled(THUMBNAIL_WIDTH,THUMBNAIL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        }
    }
    image[1].save("createThumbnailHiQThumbnailIterator.png");
}


QTEST_MAIN(KisThumbnailBenchmark)
