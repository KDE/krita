/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "kis_shear_visitor.h"
#include <math.h>
#include <QApplication>
#include <QMatrix>
#include <QRect>

#include <kis_debug.h>
#include <klocale.h>

#include <KoCompositeOp.h>
#include <KoColorSpace.h>

#include "kis_paint_device.h"

#include "KoProgressUpdater.h"
#include "KoUpdater.h"
#include "kis_iterators_pixel.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "krita_export.h"

void KisShearVisitor::shear(KisPaintDeviceSP dev, double angleX, double angleY, KoUpdater *progress)
{
    const double pi = 3.1415926535897932385;
    double thetaX = angleX * pi / 180;
    double shearX = tan(thetaX);
    double thetaY = angleY * pi / 180;
    double shearY = tan(thetaY);

    QRect r = dev->exactBounds();
    progress->setRange(0, r.height() + r.width());

    KisPaintDeviceSP sheared;
    sheared = xShear(dev, shearX, progress);

    sheared = yShear(sheared, shearY, progress);
    dev->clear();

    KisPainter p2(dev);
    r = sheared->extent();

    p2.bitBlt(r.x(), r.y(), sheared, r.x(), r.y(), r.width(), r.height());
    p2.end();

    progress->setProgress(progress->maximum());
}


KisPaintDeviceSP KisShearVisitor::xShear(KisPaintDeviceSP src, double shearX, KoUpdater *progress)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));
    dst->setX(src->x());
    dst->setY(src->y());

    QRect r = src->exactBounds();

    double displacement;
    qint32 displacementInt;
    double weight;

    KoMixColorsOp * mixOp = src->colorSpace()->mixColorsOp();

    for (qint32 y = r.top(); y <= r.bottom(); y++) {

        //calculate displacement
        displacement = -y * shearX;

        displacementInt = (qint32)(floor(displacement));
        weight = displacement - displacementInt;

        qint16 pixelWeights[2];

        pixelWeights[0] = static_cast<quint8>(weight * 255 + 0.5);
        pixelWeights[1] = 255 - pixelWeights[0];

        KisHLineConstIteratorPixel srcIt = src->createHLineIterator(r.x(), y, r.width() + 1);
        KisHLineConstIteratorPixel leftSrcIt = src->createHLineIterator(r.x() - 1, y, r.width() + 1);
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(r.x() + displacementInt, y, r.width() + 1);

        while (!srcIt.isDone()) {

            const quint8 *pixelPtrs[2];

            pixelPtrs[0] = leftSrcIt.rawData();
            pixelPtrs[1] = srcIt.rawData();

            mixOp->mixColors(pixelPtrs, pixelWeights, 2, dstIt.rawData());

            ++srcIt;
            ++leftSrcIt;
            ++dstIt;
        }
        progress->setProgress(y);

    }
    return dst;
}

KisPaintDeviceSP KisShearVisitor::yShear(KisPaintDeviceSP src, double shearY, KoUpdater *progress)
{
    int start = progress->progress();

    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));
    KoMixColorsOp * mixOp = src->colorSpace()->mixColorsOp();

    dst->setX(src->x());
    dst->setY(src->y());

    QRect r = src->exactBounds();

    double displacement;
    qint32 displacementInt;
    double weight;

    for (qint32 x = r.left(); x <= r.right(); x++) {

        //calculate displacement
        displacement = x * shearY;

        displacementInt = (qint32)(floor(displacement));
        weight = displacement - displacementInt;

        qint16 pixelWeights[2];

        pixelWeights[0] = static_cast<quint8>(weight * 255 + 0.5);
        pixelWeights[1] = 255 - pixelWeights[0];

        KisVLineConstIteratorPixel srcIt = src->createVLineIterator(x, r.y(), r.height() + 1);
        KisVLineIteratorPixel leftSrcIt = src->createVLineIterator(x, r.y() - 1, r.height() + 1);
        KisVLineIteratorPixel dstIt = dst->createVLineIterator(x, r.y() + displacementInt, r.height() + 1);

        while (!srcIt.isDone()) {

            const quint8 *pixelPtrs[2];

            pixelPtrs[0] = leftSrcIt.rawData();
            pixelPtrs[1] = srcIt.rawData();

            mixOp->mixColors(pixelPtrs, pixelWeights, 2, dstIt.rawData());

            ++srcIt;
            ++leftSrcIt;
            ++dstIt;
        }
        progress->setProgress(start + x);
    }
    return dst;
}
