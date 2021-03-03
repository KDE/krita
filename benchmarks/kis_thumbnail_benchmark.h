/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman geneing at gmail dot com
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_THUMBNAIL_BENCHMARK_H
#define KIS_THUMBNAIL_BENCHMARK_H

#include <simpletest.h>
#include "kis_paint_device.h"

class KoColor;
class KoColorSpace;

class KisThumbnailBenchmark : public QObject
{
    Q_OBJECT

private:
    const KoColorSpace * m_colorSpace;
    KisPaintDeviceSP m_dev;
    QVector<QImage> m_thumbnails;
    QSize m_thumbnailSizeLimit;
    int m_oversampleRatio;
    int m_skipCount;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void benchmarkCreateThumbnail();
    void benchmarkCreateThumbnailCached();
    void benchmarkCreateThumbnailHiQ();

    void benchmarkCreateThumbnailHiQcreateThumbOversample2x();
    void benchmarkCreateThumbnailHiQcreateThumbOversample3x();
    void benchmarkCreateThumbnailHiQcreateThumbOversample4x();

};


#endif
