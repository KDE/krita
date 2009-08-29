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

#include "kis_rotate_visitor.h"
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


void KisRotateVisitor::rotate(double angle, KoUpdater *progress)
{
    QRect r = m_dev->exactBounds();
    QPointF centerOfRotation = QPointF(r.x() + (r.width() / 2.0), r.y() + (r.height() / 2.0));
    rotate(angle, centerOfRotation, progress);
}

void KisRotateVisitor::rotate(double angle, qint32 width, qint32 height, KoUpdater *progress)
{
    QPointF centerOfRotation = QPointF(width / 2.0,  height / 2.0);
    rotate(angle, centerOfRotation, progress);
}

void KisRotateVisitor::rotate(double angle, QPointF centerOfRotation, KoUpdater *progress)
{

    m_progressUpdater = progress;

    KisPaintDeviceSP rotated = rotate(m_dev, angle, centerOfRotation);

#if 0 // XXX_SELECTION
    if (!m_dev->hasSelection()) {
#endif
        // Clear everything
        m_dev->clear();
#if 0
    } else {

        // Clear selected pixels
        m_dev->clearSelection();
    }
#endif
    KisPainter p(m_dev);
    QRect r = rotated->extent();

    // OVER ipv COPY
    
    p.bitBlt(r.x(), r.y(), rotated, r.x(), r.y(), r.width(), r.height());
    p.end();
}

void KisRotateVisitor::shear(double angleX, double angleY, KoUpdater *progress)
{
    const double pi = 3.1415926535897932385;
    double thetaX = angleX * pi / 180;
    double shearX = tan(thetaX);
    double thetaY = angleY * pi / 180;
    double shearY = tan(thetaY);

    QRect r = m_dev->exactBounds();

    const int xShearSteps = r.height();
    const int yShearSteps = r.width();

    m_progressUpdater = progress;
    initProgress(xShearSteps + yShearSteps);


    KisPaintDeviceSP sheared;
    sheared = xShear(m_dev, shearX);

    sheared = yShear(sheared, shearY);
    m_dev->clear();

    KisPainter p2(m_dev);
    r = sheared->extent();

    p2.bitBlt(r.x(), r.y(), sheared, r.x(), r.y(), r.width(), r.height());
    p2.end();

    m_progressUpdater->setProgress( m_progressUpdater->maximum() );
}

KisPaintDeviceSP KisRotateVisitor::rotateRight90(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));
    dst->setX(src->x());
    dst->setY(src->y());

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();
    qint32 x = 0;

    for (qint32 y = r.bottom(); y >= r.top(); --y) {
        KisHLineConstIteratorPixel hit = src->createHLineIterator(r.x(), y, r.width());
        KisVLineIterator vit = dst->createVLineIterator(-y, r.x(), r.width());

        while (!hit.isDone()) {
            if (hit.isSelected())  {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);
            }
            ++hit;
            ++vit;
        }
        ++x;
//        incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotateLeft90(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();
    qint32 x = 0;

    KisHLineConstIteratorPixel hit = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        // Read the horizontal line from back to front, write onto the vertical column
        KisVLineIterator vit = dst->createVLineIterator(y, -r.x() - r.width(), r.width());

        hit += r.width() - 1;
        while (!vit.isDone()) {
            if (hit.isSelected()) {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);
            }
            --hit;
            ++vit;
        }
        hit.nextRow();
        ++x;
//         incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotate180(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace()));
    dst->setX(src->x());
    dst->setY(src->y());

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();

    KisHLineConstIteratorPixel srcIt = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIterator dstIt = dst->createHLineIterator(-r.x() - r.width(), -y, r.width());

        srcIt += r.width() - 1;
        while (!dstIt.isDone()) {
            if (srcIt.isSelected())  {
                memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);
            }
            --srcIt;
            ++dstIt;
        }
        srcIt.nextRow();
//         incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotate(KisPaintDeviceSP src, double angle, QPointF centerOfRotation)
{
    const double pi = 3.1415926535897932385;

    if (angle >= 315 && angle < 360) {
        angle = angle - 360;
    } else if (angle > -360 && angle < -45) {
        angle = angle + 360;
    }

    QRect r = src->exactBounds();

    const int xShearSteps = r.height();
    const int yShearSteps = r.width();
    const int fixedRotateSteps = r.height();

    KisPaintDeviceSP dst;

    if (angle == 90) {
        initProgress(fixedRotateSteps);
        dst = rotateRight90(src);
    } else if (angle == 180) {
        initProgress(fixedRotateSteps);
        dst = rotate180(src);
    } else if (angle == 270) {
        initProgress(fixedRotateSteps);
        dst = rotateLeft90(src);
    } else {
        double theta;

        if (angle >= -45 && angle < 45) {

            theta = angle * pi / 180;
            dst = src;
            initProgress(yShearSteps + (2 * xShearSteps));
        } else if (angle >= 45 && angle < 135) {

            initProgress(fixedRotateSteps + yShearSteps + (2 * xShearSteps));
            dst = rotateRight90(src);
            theta = (angle - 90) * pi / 180;
        } else if (angle >= 135 && angle < 225) {

            initProgress(fixedRotateSteps + yShearSteps + (2 * xShearSteps));
            dst = rotate180(src);
            theta = (angle - 180) * pi / 180;

        } else {

            initProgress(fixedRotateSteps + yShearSteps + (2 * xShearSteps));
            dst = rotateLeft90(src);
            theta = (angle - 270) * pi / 180;
        }

        double shearX = tan(theta / 2);
        double shearY = sin(theta);

        //first perform a shear along the x-axis by tan(theta/2)
        dst = xShear(dst, shearX);
        //next perform a shear along the y-axis by sin(theta)
        dst = yShear(dst, shearY);
        //again perform a shear along the x-axis by tan(theta/2)
        dst = xShear(dst, shearX);
    }

    double sinAngle = sin(angle * pi / 180);
    double cosAngle = cos(angle * pi / 180);

    QPointF rotatedCenterOfRotation(
        centerOfRotation.x() * cosAngle - centerOfRotation.y() * sinAngle,
        centerOfRotation.x() * sinAngle + centerOfRotation.y() * cosAngle);

    dst->setX((qint32)(dst->x() + centerOfRotation.x() - rotatedCenterOfRotation.x()));
    dst->setY((qint32)(dst->y() + centerOfRotation.y() - rotatedCenterOfRotation.y()));

//     setProgressDone();

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::xShear(KisPaintDeviceSP src, double shearX)
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
        incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::yShear(KisPaintDeviceSP src, double shearY)
{
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
//         incrementProgress();
    }

    return dst;
}

void KisRotateVisitor::initProgress(qint32 totalSteps)
{
    if (!m_progressUpdater) return;

    m_progressTotalSteps = totalSteps;
    m_progressStep = 0;
    m_lastProgressPerCent = 0;


    m_progressUpdater->setProgress(0);

}

void KisRotateVisitor::incrementProgress()
{
    if (!m_progressUpdater) return;

    m_progressStep++;
    qint32 progressPerCent = (m_progressStep * 100) / m_progressTotalSteps;

    if (progressPerCent != m_lastProgressPerCent) {
        m_lastProgressPerCent = progressPerCent;
        m_progressUpdater->setProgress(progressPerCent);
    }
}

void KisRotateVisitor::setProgressDone()
{
    if (!m_progressUpdater) return;

    m_progressUpdater->setProgress(100);
}
KisRotateVisitor::KisRotateVisitor(KoUpdater * progressUpdater)
        : m_progressUpdater(progressUpdater)
{
}

KisRotateVisitor::~KisRotateVisitor()
{
}

void KisRotateVisitor::visitKisPaintDevice(KisPaintDevice* dev)
{
    m_dev = dev;
}


