/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman geneing at gmail dot com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_thumbnail_benchmark.h"
#include "kis_benchmark_values.h"

#include <simpletest.h>
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
#include "kis_transform_worker.h"



#include <cmath>

#define SAVE_OUTPUT

const int THUMBNAIL_WIDTH = 64;
const int THUMBNAIL_HEIGHT = 64;
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
    m_dev->fill(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, color.data());

    color.fromQColor(Qt::black);

    KisPainter painter(m_dev);
    painter.setPaintColor(color);
    float radius = std::min(IMAGE_WIDTH, IMAGE_HEIGHT);
    const float angle = 2 * 3.1415926 / 360.;
    const float endWidth = 30;

    for (int i = 0; i < 90; i += 5) {
        painter.drawThickLine(QPointF(0, 0), QPointF(radius * std::sin(angle * i), radius * std::cos(angle * i)), 1, endWidth);
        painter.drawThickLine(QPointF(IMAGE_WIDTH, IMAGE_HEIGHT),
                              QPointF(IMAGE_WIDTH - radius * std::sin(angle * i), IMAGE_HEIGHT - radius * std::cos(angle * i)), 1, endWidth);
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
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, QRect() );
        //m_dev->setDirty();
    }

    image.save("createThumbnail.png");
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailCached()
{
    QImage image;

    QBENCHMARK{
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, 2. );
    }
}


void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQ()
{
    QImage image;

    QBENCHMARK{
        image = m_dev->createThumbnail(OVERSAMPLE * THUMBNAIL_WIDTH, OVERSAMPLE * THUMBNAIL_HEIGHT);
        image = image.scaled(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_dev->setDirty();
    }

    image.save("createThumbnailHiQ.png");
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQcreateThumbOversample2x()
{
    QImage image;

    QBENCHMARK{
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, QRect(), 2,
                                       KoColorConversionTransformation::internalRenderingIntent(),
                                       KoColorConversionTransformation::internalConversionFlags());
        m_dev->setDirty();
    }

    image.save("createThumbnailHiQcreateThumbOversample2x.png");
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQcreateThumbOversample3x()
{
    QImage image;

    QBENCHMARK{
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, QRect(), 3,
                                       KoColorConversionTransformation::internalRenderingIntent(),
                                       KoColorConversionTransformation::internalConversionFlags());
        m_dev->setDirty();
    }

    image.save("createThumbnailHiQcreateThumbOversample3x.png");
}

void KisThumbnailBenchmark::benchmarkCreateThumbnailHiQcreateThumbOversample4x()
{
    QImage image;

    QBENCHMARK{
        image = m_dev->createThumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, QRect(), 4,
                                       KoColorConversionTransformation::internalRenderingIntent(),
                                       KoColorConversionTransformation::internalConversionFlags());
        m_dev->setDirty();
    }

    image.save("createThumbnailHiQcreateThumbOversample4x.png");
}


SIMPLE_TEST_MAIN(KisThumbnailBenchmark)
