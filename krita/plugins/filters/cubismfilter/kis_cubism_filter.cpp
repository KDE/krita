/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 * ported from Gimp, Copyright (C) 1997 Eiichi Takamori <taka@ma1.seikyou.ne.jp>
 * original pixelize.c for GIMP 0.54 by Tracy Scott
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

#include "kis_cubism_filter.h"

#include <stdlib.h>
#include <time.h>
#include <vector>

#include <QPoint>
#include <QSpinBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <knuminput.h>

#include <kis_painter.h>

#include <KoColorSpaceRegistry.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include <KoProgressUpdater.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoCompositeOp.h>
#include <kis_random_accessor.h>

#include "widgets/kis_multi_integer_filter_widget.h"

#include "kis_polygon.h"


#define RANDOMNESS       5
#define SUPERSAMPLE      4
#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))
#define SQR(x) ((x) * (x))

KisCubismFilter::KisCubismFilter() : KisFilter(id(), KisFilter::categoryArtistic(), i18n("&Cubism..."))
{
    setSupportsPainting(false);
    setSupportsPreview(true);
//     setSupportsThreading(false);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
}

bool KisCubismFilter::workWith(const KoColorSpace* /*cs*/) const
{
    return true;
}


void KisCubismFilter::process(KisConstProcessingInformation srcInfo,
                              KisProcessingInformation dstInfo,
                              const QSize& size,
                              const KisFilterConfiguration* configuration,
                              KoUpdater* progressUpdater
                             ) const
{
    Q_UNUSED(progressUpdater);

    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src);
    Q_ASSERT(dst);
    Q_ASSERT(configuration);

    //read the filter configuration values from the KisFilterConfiguration object
    quint32 tileSize = configuration->getInt("tileSize", 1);
    quint32 tileSaturation = configuration->getInt("tileSaturation");

    if (srcInfo.selection()) {

        KisPaintDeviceSP dev = new KisPaintDevice(src->colorSpace());

        cubism(src, srcTopLeft, dev, dstTopLeft, size, tileSize, tileSaturation);

        KisPainter gc(dst);
        gc.setSelection(srcInfo.selection());
        gc.bitBlt(dstTopLeft.x(), dstTopLeft.y(), dev, dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
        gc.end();
    } else {
        cubism(src, srcTopLeft, dst, dstTopLeft, size, tileSize, tileSaturation);
    }
}

void KisCubismFilter::randomizeIndices(qint32 count, qint32* indices) const
{
    qint32 index1, index2;
    qint32 tmp;

    //initialize random number generator with time
    srand(static_cast<unsigned int>(time(0)));

    for (qint32 i = 0; i < count * RANDOMNESS; i++) {
        index1 = randomIntNumber(0, count);
        index2 = randomIntNumber(0, count);
        tmp = indices[index1];
        indices[index1] = indices[index2];
        indices[index2] = tmp;
    }
}

qint32 KisCubismFilter::randomIntNumber(qint32 lowestNumber, qint32 highestNumber) const
{
    if (lowestNumber > highestNumber) {
        qint32 temp = lowestNumber;
        lowestNumber = highestNumber;
        highestNumber = temp;
    }

    return lowestNumber + ((highestNumber - lowestNumber) * rand()) / RAND_MAX;
}

double KisCubismFilter::randomDoubleNumber(double lowestNumber, double highestNumber) const
{
    if (lowestNumber > highestNumber) {
        double temp = lowestNumber;
        lowestNumber = highestNumber;
        highestNumber = temp;
    }

    double range = highestNumber - lowestNumber;
    return lowestNumber + range * rand() / (double)RAND_MAX;
}

double KisCubismFilter::calcAlphaBlend(double* vec, double  oneOverDist, double  x, double  y) const
{

    if (oneOverDist == 0)
        return 1.0;
    else {
        double r = (vec[0] * x + vec[1] * y) * oneOverDist;
        if (r < 0.2)
            r = 0.2;
        else if (r > 1.0)
            r = 1.0;
        return r;
    }
}

void KisCubismFilter::convertSegment(qint32 x1, qint32 y1, qint32 x2, qint32  y2, qint32 offset, qint32* min, qint32* max, qint32 xmin, qint32 xmax) const
{
    if (y1 > y2) {
        qint32 tmp = y2; y2 = y1; y1 = tmp;
        tmp = x2; x2 = x1; x1 = tmp;
    }
    qint32 ydiff = (y2 - y1);

    if (ydiff) {
        double xinc = static_cast<double>(x2 - x1) / static_cast<double>(ydiff);
        double xstart = x1 + 0.5 * xinc;
        for (qint32 y = y1 ; y < y2; y++) {
            if (xstart >= xmin && xstart <= xmax) {
                if (xstart < min[y - offset]) {
                    min[y-offset] = (int)xstart;
                }
                if (xstart > max[y - offset]) {
                    max[y-offset] = (int)xstart;
                }
                xstart += xinc;
            }
        }
    }
}

#define USE_READABLE_BUT_SLOW_CODE

void KisCubismFilter::fillPolyColor(KisPaintDeviceSP src,
                                    const QPoint& srcTopLeft,
                                    KisPaintDeviceSP dst,
                                    const QPoint & dstTopLeft,
                                    const QSize& size,
                                    KisPolygon* poly,
                                    const quint8* col,
                                    quint8* dest) const
// void KisCubismFilter::fillPolyColor (KisPaintDeviceSP src, KisPaintDeviceSP dst, KisPolygon* poly, const quint8* col, quint8* /*s*/, QRect rect) const
{
    Q_UNUSED(srcTopLeft);
    Q_UNUSED(dest);

    qint32         val;
    double         alpha;
    qint32         x, y;
    double          xx, yy;
    double          vec[2];
    QRect rect(dstTopLeft, size);
    qint32         x1 = rect.left(), y1 = rect.top(), x2 = rect.right(), y2 = rect.bottom();
//         qint32         selWidth, selHeight;
    qint32         *vals, *valsIter, *valsEnd;
    qint32         xs, ys, xe, ye;


    qint32 sx = (qint32)(*poly)[0].x();
    qint32 sy = (qint32)(*poly)[0].y();
    qint32 ex = (qint32)(*poly)[1].x();
    qint32 ey = (qint32)(*poly)[1].y();

    double dist = sqrt((double)(SQR(ex - sx) + SQR(ey - sy)));
    double oneOverDist = 0.0;
    if (dist > 0.0) {
        double oneOverDist = 1 / dist;
        vec[0] = (ex - sx) * oneOverDist;
        vec[1] = (ey - sy) * oneOverDist;
    }

    qint32 pixelSize = src->pixelSize();
    //get the extents of the polygon
    double dMinX, dMinY, dMaxX, dMaxY;
    poly->extents(dMinX, dMinY, dMaxX, dMaxY);
    qint32 minX = static_cast<qint32>(dMinX);
    qint32 minY = static_cast<qint32>(dMinY);
    qint32 maxX = static_cast<qint32>(dMaxX);
    qint32 maxY = static_cast<qint32>(dMaxY);
    qint32 sizeX = (maxX - minX) * SUPERSAMPLE;
    qint32 sizeY = (maxY - minY) * SUPERSAMPLE;

    qint32 *minScanlines = new qint32[sizeY];
    qint32 *minScanlinesIter = minScanlines;
    qint32 *maxScanlines = new qint32[sizeY];
    qint32 *maxScanlinesIter = maxScanlines;

    for (qint32 i = 0; i < sizeY; i++) {
        minScanlines[i] = maxX * SUPERSAMPLE;
        maxScanlines[i] = minX * SUPERSAMPLE;
    }

    if (poly->numberOfPoints()) {
        qint32 polyNpts = poly->numberOfPoints();

        xs = static_cast<qint32>((*poly)[polyNpts-1].x());
        ys = static_cast<qint32>((*poly)[polyNpts-1].y());
        xe = static_cast<qint32>((*poly)[0].x());
        ye = static_cast<qint32>((*poly)[0].y());

        xs *= SUPERSAMPLE;
        ys *= SUPERSAMPLE;
        xe *= SUPERSAMPLE;
        ye *= SUPERSAMPLE;

        convertSegment(xs, ys, xe, ye, minY * SUPERSAMPLE, minScanlines, maxScanlines, minX* SUPERSAMPLE, maxX* SUPERSAMPLE);

        KisPolygon::iterator it;

        for (it = poly->begin(); it != poly->end();) {
            xs = static_cast<qint32>((*it).x());
            ys = static_cast<qint32>((*it).y());
            ++it;

            if (it != poly->end()) {
                xe = static_cast<qint32>((*it).x());
                ye = static_cast<qint32>((*it).y());

                xs *= SUPERSAMPLE;
                ys *= SUPERSAMPLE;
                xe *= SUPERSAMPLE;
                ye *= SUPERSAMPLE;

                convertSegment(xs, ys, xe, ye, minY * SUPERSAMPLE, minScanlines, maxScanlines, minX* SUPERSAMPLE, maxX* SUPERSAMPLE);
            }
        }
    }

    KisRandomAccessor dstAccessor = dst->createRandomAccessor(0, 0);

    KoMixColorsOp * mixOp = src->colorSpace()->mixColorsOp();
    quint8* buf = new quint8[pixelSize];

    vals = new qint32[sizeX];
//         x1 = minX; x2 = maxX; y1 = minY; y2 = maxY;
    for (qint32 i = 0; i < sizeY; i++, minScanlinesIter++, maxScanlinesIter++) {
        if (!(i % SUPERSAMPLE)) {
            memset(vals, 0, sizeof(qint32) * sizeX);
        }

        yy = static_cast<double>(i) / static_cast<double>(SUPERSAMPLE) + minY;

        for (qint32 j = *minScanlinesIter; j < *maxScanlinesIter; j++) {
            x = j - minX * SUPERSAMPLE;
            vals[x] += 255;
        }

        if (!((i + 1) % SUPERSAMPLE)) {
            y = (i / SUPERSAMPLE) + minY;
            if (y >= y1 && y <= y2) {
                for (qint32 j = 0; j < sizeX; j += SUPERSAMPLE) {
                    x = (j / SUPERSAMPLE) + minX;

                    if (x >= x1 && x <= x2) {
                        for (val = 0, valsIter = &vals[j], valsEnd = &valsIter[SUPERSAMPLE]; valsIter < valsEnd; valsIter++) {
                            val += *valsIter;
                        }
                        val /= SQR(SUPERSAMPLE);

                        if (val > 0) {
                            xx = static_cast<double>(j) / static_cast<double>(SUPERSAMPLE) + minX;
                            alpha = calcAlphaBlend(vec, oneOverDist, xx - sx, yy - sy);

                            qint16 weights[2];
                            weights[0] = static_cast<quint8>(alpha * 255);
                            weights[1] = 255 - weights[0];

                            dstAccessor.moveTo(x, y);
                            memcpy(buf, dstAccessor.rawData(), sizeof(quint8) * pixelSize);

                            const quint8* colors[2];
                            colors[0] = col;
                            colors[1] = buf;

                            mixOp->mixColors(colors, weights, 2, dstAccessor.rawData());
                        }
                    }
                }
            }
        }
    }
    delete[] buf;
    delete[] vals;
    delete[] minScanlines;
    delete[] maxScanlines;
}

void KisCubismFilter::cubism(KisPaintDeviceSP src,
                             const QPoint& srcTopLeft,
                             KisPaintDeviceSP dst,
                             const QPoint& dstTopLeft,
                             const QSize& size,
                             quint32 tileSize,
                             quint32 tileSaturation) const
{
    Q_ASSERT(src);
    Q_ASSERT(dst);

    //fill the destination image with the background color (black for now)
    KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
    qint32 depth = src->colorSpace()->colorChannelCount();
    while (! dstIt.isDone()) {
        for (qint32 i = 0; i < depth; i++) {
            dstIt.rawData()[i] = 0;
        }
        ++dstIt;
    }

    //compute number of rows and columns
    qint32 cols = (size.width() + tileSize - 1) / tileSize;
    qint32 rows = (size.height() + tileSize - 1) / tileSize;
    qint32 numTiles = (rows + 1) * (cols + 1);

//         setProgressTotalSteps(numTiles);
//         setProgressStage(i18n("Applying cubism filter..."),0);

    qint32* randomIndices = new qint32[numTiles];
    for (qint32 i = 0; i < numTiles; i++) {
        randomIndices[i] = i;
    }
    randomizeIndices(numTiles, randomIndices);

    qint32 count = 0;
    qint32 i, j, ix, iy;
    double x, y, width, height, theta;
    KisPolygon *poly = new KisPolygon();
    qint32 pixelSize = src->pixelSize();
    const quint8 *srcPixel /*= new quint8[ pixelSize ]*/;
    quint8 *dstPixel = 0;
    KisRandomAccessor srcAccessor = src->createRandomAccessor(0, 0);
    while (count < numTiles) {
        i = randomIndices[count] / (cols + 1);
        j = randomIndices[count] % (cols + 1);
        x = j * tileSize + (tileSize / 4.0) - randomDoubleNumber(0, tileSize / 2.0) + dstTopLeft.x();
        y = i * tileSize + (tileSize / 4.0) - randomDoubleNumber(0, tileSize / 2.0) + dstTopLeft.y();
        width = (tileSize + randomDoubleNumber(0, tileSize / 4.0) - tileSize / 8.0) * tileSaturation;
        height = (tileSize + randomDoubleNumber(0, tileSize / 4.0) - tileSize / 8.0) * tileSaturation;
        theta = randomDoubleNumber(0, 2 * M_PI);
        poly->clear();
        poly->addPoint(-width / 2.0, -height / 2.0);
        poly->addPoint(width / 2.0, -height / 2.0);
        poly->addPoint(width / 2.0, height / 2.0);
        poly->addPoint(-width / 2.0, height / 2.0);
        poly->rotate(theta);
        poly->translate(x, y);
        //  bounds check on x, y
        ix = (qint32) CLAMP(x, dstTopLeft.x(), dstTopLeft.x() + size.width() - 1);
        iy = (qint32) CLAMP(y, dstTopLeft.y(), dstTopLeft.y() + size.height() - 1);

        //read the pixel at ix, iy
        srcAccessor.moveTo(ix, iy);
        srcPixel = srcAccessor.rawData();
        if (srcPixel[pixelSize - 1]) {
            fillPolyColor(src, srcTopLeft, dst, dstTopLeft, size, poly, srcPixel, dstPixel);
        }
        count++;
//                 if ((count % 5) == 0) setProgress(count);
    }

}

KisConfigWidget * KisCubismFilter::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP /*dev*/, const KisImageWSP) const
{
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 40, 10, i18n("Tile size"), "tileSize"));
    param.push_back(KisIntegerWidgetParam(2, 40, 10, i18n("Tile saturation"), "tileSaturation"));
    return new KisMultiIntegerFilterWidget(id().id(),  parent,  id().id(),  param);
}

KisFilterConfiguration* KisCubismFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("tileSize", 10);
    config->setProperty("tileSaturation", 10);
    return config;
}
