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
#include <math.h>
#include <qapplication.h>
#include <qmatrix.h>
#include <qrect.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device.h"
#include "kis_rotate_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"
#include "kis_selection.h"
#include "kis_painter.h"

void KisRotateVisitor::rotate(double angle, bool rotateAboutImageCentre, KisProgressDisplayInterface *progress)
{
    KisPoint centreOfRotation;

    if (rotateAboutImageCentre) {
        centreOfRotation = KisPoint(m_dev->image()->width() / 2.0,  m_dev->image()->height() / 2.0);
    } else {
        QRect r = m_dev->exactBounds();
        centreOfRotation = KisPoint(r.x() + (r.width() / 2.0), r.y() + (r.height() / 2.0));
    }

    m_progress = progress;

    KisPaintDeviceSP rotated = rotate(m_dev, angle, centreOfRotation);

    if (!m_dev->hasSelection()) {
        // Clear everything
        m_dev->clear();
    } else {
        // Clear selected pixels
        m_dev->clearSelection();
    }

    KisPainter p(m_dev);
    QRect r = rotated->extent();

    // OVER ipv COPY
    p.bitBlt(r.x(), r.y(), COMPOSITE_OVER, rotated, OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    p.end();
}

void KisRotateVisitor::shear(double angleX, double angleY, KisProgressDisplayInterface *progress)
{
    const double pi=3.1415926535897932385;
    double thetaX = angleX * pi / 180;
    double shearX = tan(thetaX);
    double thetaY = angleY * pi / 180;
    double shearY = tan(thetaY);

    QRect r = m_dev->exactBounds();

    const int xShearSteps = r.height();
    const int yShearSteps = r.width();

    m_progress = progress;
    initProgress(xShearSteps + yShearSteps);


    KisPaintDeviceSP sheared;

    if (m_dev->hasSelection()) {
        sheared = new KisPaintDevice(m_dev->colorSpace(), "sheared");
        KisPainter p1(sheared);
        p1.bltSelection(r.x(), r.y(), COMPOSITE_OVER, m_dev, OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
        p1.end();
         sheared = xShear(sheared, shearX);
    }
    else {
        sheared = xShear(m_dev, shearX);
    }

     sheared = yShear(sheared, shearY);

     if (!m_dev->hasSelection()) {
        m_dev->clear();
     } else {
         // Clear selected pixels
         m_dev->clearSelection();
     }

    KisPainter p2(m_dev);
    r = sheared->extent();

    p2.bitBlt(r.x(), r.y(), COMPOSITE_OVER, sheared, OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    p2.end();

    setProgressDone();
}

KisPaintDeviceSP KisRotateVisitor::rotateRight90(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "rotateright90"));
    dst->setX(src->getX());
    dst->setY(src->getY());

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();
    qint32 x = 0;

    for (qint32 y = r.bottom(); y >= r.top(); --y) {
        KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), y, r.width(), false);
        KisVLineIterator vit = dst->createVLineIterator(-y, r.x(), r.width(), true);

            while (!hit.isDone()) {
            if (hit.isSelected())  {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);
            }
            ++hit;
            ++vit;
        }
        ++x;
        incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotateLeft90(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "rotateleft90"));

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();
    qint32 x = 0;

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        // Read the horizontal line from back to front, write onto the vertical column
        KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), y, r.width(), false);
        KisVLineIterator vit = dst->createVLineIterator(y, -r.x() - r.width(), r.width(), true);

        hit += r.width() - 1;
        while (!vit.isDone()) {
            if (hit.isSelected()) {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);
            }
            --hit;
            ++vit;
        }
        ++x;
        incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotate180(KisPaintDeviceSP src)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "rotate180"));
    dst->setX(src->getX());
    dst->setY(src->getY());

    qint32 pixelSize = src->pixelSize();
    QRect r = src->exactBounds();

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), y, r.width(), false);
        KisHLineIterator dstIt = dst->createHLineIterator( -r.x() - r.width(), -y, r.width(), true);

        srcIt += r.width() - 1;
        while (!dstIt.isDone()) {
            if (srcIt.isSelected())  {
                memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);
            }
            --srcIt;
            ++dstIt;
        }
        incrementProgress();
    }

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::rotate(KisPaintDeviceSP src, double angle, KisPoint centreOfRotation)
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
        }
        else if (angle >= 45 && angle < 135) {

            initProgress(fixedRotateSteps + yShearSteps + (2 * xShearSteps));
            dst = rotateRight90(src);
            theta = (angle - 90) * pi / 180;
        }
        else if (angle >= 135 && angle < 225) {

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

    KisPoint rotatedCentreOfRotation(
                                centreOfRotation.x() * cosAngle - centreOfRotation.y() * sinAngle,
                                centreOfRotation.x() * sinAngle + centreOfRotation.y() * cosAngle);

    dst->setX((qint32)(dst->getX() + centreOfRotation.x() - rotatedCentreOfRotation.x()));
    dst->setY((qint32)(dst->getY() + centreOfRotation.y() - rotatedCentreOfRotation.y()));

    setProgressDone();

    return dst;
}

KisPaintDeviceSP KisRotateVisitor::xShear(KisPaintDeviceSP src, double shearX)
{
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "xShear"));
    dst->setX(src->getX());
    dst->setY(src->getY());

    QRect r = src->exactBounds();

        double displacement;
        qint32 displacementInt;
        double weight;

    for (qint32 y = r.top(); y <= r.bottom(); y++) {

        //calculate displacement
        displacement = -y * shearX;

        displacementInt = (qint32)(floor(displacement));
        weight = displacement - displacementInt;

        quint8 pixelWeights[2];

        pixelWeights[0] = static_cast<quint8>(weight * 255 + 0.5);
        pixelWeights[1] = 255 - pixelWeights[0];

        KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), y, r.width() + 1, false);
        KisHLineIteratorPixel leftSrcIt = src->createHLineIterator(r.x() - 1, y, r.width() + 1, false);
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(r.x() + displacementInt, y, r.width() + 1, true);

        while (!srcIt.isDone()) {

            const quint8 *pixelPtrs[2];

            pixelPtrs[0] = leftSrcIt.rawData();
            pixelPtrs[1] = srcIt.rawData();

            src->colorSpace()->mixColors(pixelPtrs, pixelWeights, 2, dstIt.rawData());

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
    KisPaintDeviceSP dst = KisPaintDeviceSP(new KisPaintDevice(src->colorSpace(), "yShear"));
    dst->setX(src->getX());
    dst->setY(src->getY());

    QRect r = src->exactBounds();

        double displacement;
        qint32 displacementInt;
        double weight;

    for (qint32 x = r.left(); x <= r.right(); x++) {

        //calculate displacement
        displacement = x * shearY;

        displacementInt = (qint32)(floor(displacement));
        weight = displacement - displacementInt;

        quint8 pixelWeights[2];

        pixelWeights[0] = static_cast<quint8>(weight * 255 + 0.5);
        pixelWeights[1] = 255 - pixelWeights[0];

        KisVLineIteratorPixel srcIt = src->createVLineIterator(x, r.y(), r.height() + 1, false);
        KisVLineIteratorPixel leftSrcIt = src->createVLineIterator(x, r.y() - 1, r.height() + 1, false);
        KisVLineIteratorPixel dstIt = dst->createVLineIterator(x, r.y() + displacementInt, r.height() + 1, true);

        while (!srcIt.isDone()) {

            const quint8 *pixelPtrs[2];

            pixelPtrs[0] = leftSrcIt.rawData();
            pixelPtrs[1] = srcIt.rawData();

            src->colorSpace()->mixColors(pixelPtrs, pixelWeights, 2, dstIt.rawData());

            ++srcIt;
            ++leftSrcIt;
            ++dstIt;
        }
        incrementProgress();
    }

        return dst;
}

void KisRotateVisitor::initProgress(qint32 totalSteps)
{
    if (!m_progress) return;

    m_progressTotalSteps = totalSteps;
    m_progressStep = 0;
    m_lastProgressPerCent = 0;


    m_progress->setSubject(this, true, false);
    emit notifyProgress(0);

}

void KisRotateVisitor::incrementProgress()
{
    if (!m_progress) return;

    m_progressStep++;
    qint32 progressPerCent = (m_progressStep * 100) / m_progressTotalSteps;

    if (progressPerCent != m_lastProgressPerCent) {
        m_lastProgressPerCent = progressPerCent;
        emit notifyProgress(progressPerCent);
    }
}

void KisRotateVisitor::setProgressDone()
{
    if (!m_progress) return;

    emit notifyProgressDone();
}


