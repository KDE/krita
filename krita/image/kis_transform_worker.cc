/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de> filters
 *  Copyright (c) 2005-2007 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2005, 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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

#include "kis_transform_worker.h"

#include <qmath.h>
#include <klocale.h>

#include <QTransform>

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoColor.h>

#include "kis_paint_device.h"
#include "kis_debug.h"
#include "kis_selection.h"
#include "kis_datamanager.h"
#include "kis_global.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"
#include "kis_filter_strategy.h"
#include "kis_layer.h"
#include "kis_painter.h"

KisTransformWorker::KisTransformWorker(KisPaintDeviceSP dev,
                                       double xscale, double yscale,
                                       double xshear, double yshear,
                                       double xshearOrigin, double yshearOrigin,
                                       double rotation,
                                       qint32 xtranslate, qint32 ytranslate,
                                       KoUpdaterPtr progress,
                                       KisFilterStrategy *filter,
                                       bool fixBorderAlpha)
{
    m_dev = dev;
    m_xscale = xscale;
    m_yscale = yscale;
    m_xshear = xshear;
    m_yshear = yshear;
    m_xshearOrigin = xshearOrigin;
    m_yshearOrigin = yshearOrigin;
    m_rotation = rotation,
    m_xtranslate = xtranslate;
    m_ytranslate = ytranslate;
    m_progressUpdater = progress;
    m_filter = filter;
    m_fixBorderAlpha = fixBorderAlpha;
}

KisTransformWorker::~KisTransformWorker()
{
}

QTransform KisTransformWorker::transform() const
{
    QTransform TS = QTransform::fromTranslate(m_xshearOrigin, m_yshearOrigin);
    QTransform S; S.shear(0, m_yshear); S.shear(m_xshear, 0);
    QTransform SC = QTransform::fromScale(m_xscale, m_yscale);
    QTransform R; R.rotateRadians(m_rotation);
    QTransform T = QTransform::fromTranslate(m_xtranslate, m_ytranslate);

    return TS.inverted() * S * TS * SC * R * T;
}

QRect KisTransformWorker::rotateNone(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                                     QRect boundRect,
                                     KoUpdaterPtr progressUpdater,
                                     int &lastProgressReport,
                                     int &progressTotalSteps,
                                     int &progressStep)
{
    qint32 pixelSize = src->pixelSize();
    QRect r(boundRect);
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);

    KisHLineIteratorSP hit = src->createHLineIteratorNG(r.x(), r.top(), r.width());
    KisHLineIteratorSP vit = dst->createHLineIteratorNG(r.x(), r.top(), r.width());

    for (qint32 i = 0; i < r.height(); ++i) {
        do {
            memcpy(vit->rawData(), hit->rawData(), pixelSize);
        } while (hit->nextPixel() && vit->nextPixel());
        hit->nextRow();
        vit->nextRow();

        if (!progressUpdater.isNull()) {
            //progress info
            progressStep += r.width();
            if (lastProgressReport != (progressStep * 100) / progressTotalSteps) {
                lastProgressReport = (progressStep * 100) / progressTotalSteps;
                if (!progressUpdater.isNull()) progressUpdater->setProgress(lastProgressReport);
            }
            if (progressUpdater->interrupted()) {
                break;
            }
        }
    }
    return r;
}

QRect rotateWithTf(int rotation, KisPaintDeviceSP src, KisPaintDeviceSP dst,
                   QRect boundRect,
                   KoUpdaterPtr progressUpdater,
                   int &lastProgressReport,
                   int &progressTotalSteps,
                   int &progressStep)
{
    qint32 pixelSize = src->pixelSize();
    QRect r(boundRect);
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);

    KisRandomAccessorSP srcAcc = src->createRandomAccessorNG(0, 0);
    KisRandomAccessorSP dstAcc = dst->createRandomAccessorNG(0, 0);

    QTransform tf;
    tf = tf.rotate(rotation);

    int ty = 0;
    int tx = 0;

    for (qint32 y = r.y(); y <= r.height() + r.y(); ++y) {
        for (qint32 x = r.x(); x <= r.width() + r.x(); ++x) {
            tf.map(x, y, &tx, &ty);
            srcAcc->moveTo(x, y);
            dstAcc->moveTo(tx, ty);
            memcpy(dstAcc->rawData(), srcAcc->oldRawData(), pixelSize);
        }

        if (!progressUpdater.isNull()) {
            // Progress info
            progressStep += r.width();
            if (lastProgressReport != (progressStep * 100) / progressTotalSteps) {
                lastProgressReport = (progressStep * 100) / progressTotalSteps;
                if (progressUpdater) progressUpdater->setProgress(lastProgressReport);
            }
            if (progressUpdater->interrupted()) {
                break;
            }
        }
    }

    return r;
}

QRect KisTransformWorker::rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                                        QRect boundRect,
                                        KoUpdaterPtr progressUpdater,
                                        int &lastProgressReport,
                                        int &progressTotalSteps,
                                        int &progressStep)
{
    QRect r = rotateWithTf(90, src, dst, boundRect, progressUpdater, lastProgressReport, progressTotalSteps, progressStep);
    dst->move(dst->x() - 1, dst->y());
    return QRect(- r.top() - r.height(), r.x(), r.height(), r.width());
}

QRect KisTransformWorker::rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                                       QRect boundRect,
                                       KoUpdaterPtr progressUpdater,
                                       int &lastProgressReport,
                                       int &progressTotalSteps,
                                       int &progressStep)
{
    QRect r = rotateWithTf(270, src, dst, boundRect, progressUpdater, lastProgressReport, progressTotalSteps, progressStep);
    dst->move(dst->x(), dst->y() -1);
    return QRect(r.top(), - r.x() - r.width(), r.height(), r.width());
}

QRect KisTransformWorker::rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst,
                                    QRect boundRect,
                                    KoUpdaterPtr progressUpdater,
                                    int &lastProgressReport,
                                    int &progressTotalSteps,
                                    int &progressStep)
{
    QRect r = rotateWithTf(180, src, dst, boundRect, progressUpdater, lastProgressReport, progressTotalSteps, progressStep);
    dst->move(dst->x() - 1, dst->y() -1);
    return QRect(- r.x() - r.width(), - r.top() - r.height(), r.width(), r.height());
}

template <class iter> iter createIterator(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len);

template <> KisHLineIteratorSP createIterator <KisHLineIteratorSP>
(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len)
{
    return dev->createHLineIteratorNG(start, lineNum, len);
}

template <> KisVLineIteratorSP createIterator <KisVLineIteratorSP>
(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len)
{
    return dev->createVLineIteratorNG(lineNum, start, len);
}

template <class iter> void calcDimensions(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines);

template <> void calcDimensions <KisHLineIteratorSP>
(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    srcStart = rc.x();
    firstLine = rc.y();
    srcLen = rc.width();
    numLines = rc.height();
}

template <> void calcDimensions <KisVLineIteratorSP>
(QRect rc, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    firstLine = rc.x();
    srcStart = rc.y();
    numLines = rc.width();
    srcLen = rc.height();
}

template <class iter> void updateBounds(QRect &boundRect, double floatscale, double shear, qint32 dx);

template <> void updateBounds <KisHLineIteratorSP>
(QRect &boundRect, double floatscale, double shear, qint32 dx)
{
    // Do not use QRect::right()! Never! See Qt doc for info...

    qreal x0 = boundRect.left() * floatscale + boundRect.top() * shear + dx;
    qreal x1 = boundRect.left() * floatscale + (boundRect.top() + boundRect.height()) * shear + dx;
    qreal x2 = (boundRect.left() + boundRect.width()) * floatscale + boundRect.top() * shear + dx;
    qreal x3 = (boundRect.left() + boundRect.width()) * floatscale + (boundRect.top() + boundRect.height()) * shear + dx;

    qreal min = qMin(qMin(qMin(x0, x1), x2), x3);
    qreal max = qMax(qMax(qMax(x0, x1), x2), x3);

    // working around Qt's "history reasons"
    QRectF newRect(boundRect);
    newRect.setLeft(min);
    newRect.setRight(max);

    boundRect = newRect.toAlignedRect();
}

template <> void updateBounds <KisVLineIteratorSP>
(QRect &boundRect, double floatscale, double shear, qint32 dx)
{
    // Do not use QRect::bottom() as well! Never! See Qt doc for info...

    qreal y0 = boundRect.top() * floatscale + boundRect.left() * shear + dx;
    qreal y1 = boundRect.top() * floatscale + (boundRect.left() + boundRect.width()) * shear + dx;
    qreal y2 = (boundRect.top() + boundRect.height()) * floatscale + boundRect.left() * shear + dx;
    qreal y3 = (boundRect.top() + boundRect.height()) * floatscale + (boundRect.left() + boundRect.width()) * shear + dx;

    qreal min = qMin(qMin(qMin(y0, y1), y2), y3);
    qreal max = qMax(qMax(qMax(y0, y1), y2), y3);

    // working around Qt's "history reasons"
    QRectF newRect(boundRect);
    newRect.setTop(min);
    newRect.setBottom(max);

    boundRect = newRect.toAlignedRect();
}

struct FilterValues {
    quint8 numWeights;
    qint16 *weight;
    ~FilterValues() {
        delete [] weight;
    }
};

template <class T>
void KisTransformWorker::transformPass(KisPaintDevice *src, KisPaintDevice *dst,
                                       double floatscale, double shear, qint32 dx,
                                       KisFilterStrategy *filterStrategy, bool fixBorderAlpha)
{
    m_fixBorderAlpha = fixBorderAlpha;

    qint32 lineNum, srcStart, firstLine, srcLen, numLines;
    qint32 center, begin, end;    /* filter calculation variables */
    quint8 pixelSize = src->pixelSize();
    KoColorSpace * cs = src->colorSpace();
    KoMixColorsOp * mixOp = cs->mixColorsOp();
    qint32 scale;
    qint32 shearFracOffset;


    calcDimensions <T>(m_boundRect, srcStart, srcLen, firstLine, numLines);

    scale = int(floatscale * srcLen + 0.5);
    if (scale == 0) {
        dst->clear();
        return;
    }

    if (srcLen == 0) {
        updateBounds <T>(m_boundRect, floatscale, shear, dx);
        return;
    }

    qint32 support = filterStrategy->intSupport();
    qint32 dstLen, dstStart;
    qint32 invfscale = 256;

    // handle magnification/minification
    if (abs(scale) < srcLen) {
        support *= srcLen;
        support /= scale;

        invfscale *= scale;
        invfscale /= srcLen;
        if (scale < 0) { // handle mirroring
            support = -support;
            invfscale = -invfscale;
        }
    }

    // handle mirroring
    if (scale < 0)
        dstLen = -scale;
    else
        dstLen = scale;

    // Calculate extra length (in each side) needed due to shear
    qint32 extraLen = ((support + 256) >> 8)  + 1;

    quint8 *tmpLine = new quint8[(srcLen +2*extraLen)* pixelSize];
    Q_CHECK_PTR(tmpLine);

    //allocate space for colors
    const quint8 **colors = new const quint8 *[2*support+1];

    // Precalculate weights
    FilterValues *filterWeights = new FilterValues[256];

    for (int center = 0; center < 256; ++center) {
        qint32 begin = (center - support) >> 8; // find pixel at beginning
        qint32 span = ((center + support) >> 8) - begin + 1; // find pixel at end. Then subtract begin and +1 to get span
        qint32 t = (((begin << 8) + 128 - center) * invfscale) >> 8; // calculate position from center of sample to center
        qint32 dt = invfscale;
        filterWeights[center].weight = new qint16[span];

        qint32 sum = 0;
        for (int num = 0; num < span; ++num) {
            qint32 tmpw = filterStrategy->intValueAt(t) * invfscale;
            tmpw >>= 8;
            filterWeights[center].weight[num] = tmpw;
            t += dt;
            sum += tmpw;
        }

        if (sum != 255) {
            double fixfactor = 255.0 / sum;
            sum = 0;
            for (int num = 0; num < span; ++num) {
                filterWeights[center].weight[num] = int(filterWeights[center].weight[num] * fixfactor);
                sum += filterWeights[center].weight[num];
            }
        }

        int num = 0;
        while (sum < 255 && num*2 < span) {
            filterWeights[center].weight[span/2 + num]++;
            ++sum;
            if (sum < 255 && num < span / 2) {
                filterWeights[center].weight[span/2 - num - 1]++;
                ++sum;
            }
            ++num;
        }
        filterWeights[center].numWeights = span;
    }

    // Now time to do the actual scaling and shearing
    for (lineNum = firstLine; lineNum < firstLine + numLines; lineNum++) {
        if (scale < 0) {
            dstStart = srcStart * scale / srcLen - dstLen + dx;
        } else {
            dstStart = (srcStart) * scale / srcLen + dx;
        }

        shearFracOffset = -int(256 * (lineNum * shear - floor(lineNum * shear)));
        dstStart += int(floor(lineNum * shear));

        // Build a temporary line
        T srcIt = createIterator <T>(src, srcStart, lineNum, srcLen);
        quint8 *data;
        qint32 i = 0;
        data = srcIt->rawData(); // take the first pixel as source - in effect duplicating the pixel
        while (i < extraLen) {
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            i++;
        }


        while (i < srcLen + extraLen) {
            data = srcIt->rawData();
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            memcpy(data, src->defaultPixel(), pixelSize);
            if (i < srcLen + extraLen - 1) { // duplicate pixels along edge
                srcIt->nextPixel();
            }
            i++;
        }



        data = &tmpLine[(i-1)*pixelSize]; // take the last pixel as source - in effect duplicating the pixel
        while (i < srcLen + 2*extraLen) {
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            i++;
        }
        // line is done

        T dstIt = createIterator <T>(dst, dstStart, lineNum, dstLen);

        i = 0;

        do {
            // NOTE: 2500 was selected by Boemann as limit to avoid overflow
            // but the overflow happens when dstLen is bigger then 3356, thus
            // new condition to avoid the overflow.
            // How did I caluculated the magic number? (qint32_max / 256) / (2500-1) = ~3356
            if ((srcLen < 2500) && (dstLen < 3300)) {
                center = ((i << 8) * srcLen) / scale;
            } else {
                if (srcLen < 46000) { // real limit is actually 46340 pixels
                    center = ((i * srcLen) / scale) << 8;
                } else {
                    center = ((i << 8) / scale * srcLen) / scale; // XXX fails for sizes over 2^23 pixels src width
                }
            }

            if (scale < 0) {
                center += srcLen << 8;
            }

            // Since the above gives us the position at the left of the pixels we need to advance by one half dst pixel
            center += 128 * srcLen / scale;//xxx doesn't work for scale<0;
            center += (extraLen << 8) + shearFracOffset;

            // find contributing pixels
            begin = (center - support) >> 8; // find first pixel to sample
            end = (center + support) >> 8; // find last pixel to sample

            // catch fixed-arithmethics overflows indirectly (variable center can overflow)
            Q_ASSERT(begin >= 0);
            Q_ASSERT(end >= 0);

            int num = 0;
            for (int srcpos = begin; srcpos <= end; ++srcpos) {
                colors[num] = &tmpLine[srcpos*pixelSize];
                num++;
            }
            data = dstIt->rawData();
            mixOp->mixColors(colors, filterWeights[center&255].weight, filterWeights[center&255].numWeights, data);

            /*
                        //possibly fix the alpha of the border if user wants it
                        if (fixBorderAlpha && (i == 0 || i == dstLen - 1))
                            cs->setAlpha(data, cs->alpha(&tmpLine[(center>>8)*pixelSize]), 1);
            */
            i++;
        } while (dstIt->nextPixel());

        //progress info
        m_progressStep += dstLen;
        if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(m_lastProgressReport);
        }
        if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
            break;
        }
    }
    delete [] colors;
    delete [] tmpLine;
    delete [] filterWeights;

    updateBounds <T>(m_boundRect, floatscale, shear, dx);
}

bool KisTransformWorker::run()
{
    /* Check for nonsense and let the user know, this helps debugging.
    Otherwise the program will crash at a later point, in a very obscure way, probably by division by zero */
    Q_ASSERT_X(m_xscale != 0, "KisTransformer::run() validation step", "xscale == 0");
    Q_ASSERT_X(m_yscale != 0, "KisTransformer::run() validation step", "yscale == 0");
    // Fallback safety line in case Krita is compiled without ASSERTS
    if (m_xscale == 0 || m_yscale == 0) return false;

    // Progress info
    m_progressTotalSteps = 1;
    m_progressStep = 0;

    KoColor defaultPixel(m_dev->defaultPixel(), m_dev->colorSpace());
    if (defaultPixel.opacityU8() != OPACITY_TRANSPARENT_U8)
        m_boundRect = m_dev->extent();
    else
        m_boundRect = m_dev->exactBounds();

    if (m_boundRect.isNull()) {
        if (!m_progressUpdater.isNull())
            m_progressUpdater->setProgress(100);
        return true;
    }


    KisPaintDeviceSP tmpdev1 = new KisPaintDevice(m_dev->colorSpace());
    KisPaintDeviceSP srcdev = m_dev;
    tmpdev1->setDefaultPixel(srcdev->defaultPixel());

    double xscale = m_xscale;
    double yscale = m_yscale;
    double xshear = m_xshear;
    double yshear = m_yshear;
    double rotation = m_rotation;
    qint32 xtranslate = m_xtranslate;
    qint32 ytranslate = m_ytranslate;

    // Apply shear X and Y
    if (xshear != 0 || yshear != 0) {
        m_progressTotalSteps += (yscale * m_boundRect.height() * (m_boundRect.width() + m_xshear * m_boundRect.height()));
        m_progressTotalSteps += (xscale * m_boundRect.width() * (m_boundRect.height() + m_yshear * m_boundRect.width()));


        int dx = - qRound(m_yshearOrigin * yscale * m_xshear);
        int dy = - qRound(m_xshearOrigin * xscale * m_yshear);
        transformPass <KisHLineIteratorSP>(srcdev.data(), srcdev.data(), xscale, yscale *  m_xshear, dx, m_filter, m_fixBorderAlpha);
        transformPass <KisVLineIteratorSP>(srcdev.data(), srcdev.data(), yscale, m_yshear, dy, m_filter, m_fixBorderAlpha);
        yscale = 1.;
        xscale = 1.;
    }

    if (rotation < 0.0)
        rotation = -fmod(-rotation, 2 * M_PI) + 2 * M_PI;
    else
        rotation = fmod(rotation, 2 * M_PI);
    int rotQuadrant = int(rotation / (M_PI / 2) + 0.5) & 3;

    // Figure out how we will do the initial right angle rotations
    double tmp;
    switch (rotQuadrant) {
    default: // just to shut up the compiler
    case 0:
        m_progressTotalSteps += 0;
        break;
    case 1:
        rotation -= M_PI / 2;
        tmp = xscale;
        xscale = yscale;
        yscale = tmp;
        m_progressTotalSteps += m_boundRect.width() * m_boundRect.height();
        break;
    case 2:
        rotation -= M_PI;
        m_progressTotalSteps += m_boundRect.width() * m_boundRect.height();
        break;
    case 3:
        rotation -= -M_PI / 2 + 2 * M_PI;
        tmp = xscale;
        xscale = yscale;
        yscale = tmp;
        m_progressTotalSteps += m_boundRect.width() * m_boundRect.height();
        break;
    }

    //// Calculate some auxillary values
    yshear = sin(rotation);
    xshear = -tan(rotation / 2);
    xtranslate -= int(xshear * ytranslate);

    // Calculate progress steps
    m_progressTotalSteps += int(yscale * m_boundRect.width() * m_boundRect.height());
    m_progressTotalSteps += int(xscale * m_boundRect.width() * (m_boundRect.height() * yscale + m_boundRect.width() * yshear));

    // Now that we have everything in place it's time to do the actual right angle rotations
    switch (rotQuadrant) {
    default: // just to shut up the compiler
    case 0:
        break;
    case 1:
        m_boundRect = rotateRight90(srcdev, tmpdev1, m_boundRect, m_progressUpdater, m_lastProgressReport, m_progressTotalSteps, m_progressStep);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    case 2:
        m_boundRect = rotate180(srcdev, tmpdev1, m_boundRect, m_progressUpdater, m_lastProgressReport, m_progressTotalSteps, m_progressStep);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    case 3:
        m_boundRect = rotateLeft90(srcdev, tmpdev1, m_boundRect, m_progressUpdater, m_lastProgressReport, m_progressTotalSteps, m_progressStep);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    }

    //// Handle simple move case possibly with rotation of 90,180,270
    if (rotation == 0.0 && xscale == 1.0 && yscale == 1.0) {

        updateBounds <KisHLineIteratorSP>(m_boundRect, 1.0, 0, xtranslate);
        updateBounds <KisVLineIteratorSP>(m_boundRect, 1.0, 0, ytranslate);

        if (rotQuadrant == 0) {
            // When we didn't move the m_dev to a temp device we can simply just move its coords
            srcdev->move(srcdev->x() + xtranslate, srcdev->y() + ytranslate);
        } else {
            srcdev->move(srcdev->x() + xtranslate, srcdev->y() + ytranslate);
            m_boundRect = rotateNone(srcdev, m_dev, m_boundRect, m_progressUpdater, m_lastProgressReport, m_progressTotalSteps, m_progressStep); //copy it back
        }
        //progress info
        if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(100);
        return true;
    }

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(100);
        return false;
    }

    // First pass
    transformPass <KisHLineIteratorSP>(srcdev.data(), srcdev.data(), xscale, yscale*xshear, 0, m_filter, m_fixBorderAlpha);

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        m_progressUpdater->setProgress(100);
        return false;
    }

    // Now do the second pass
    transformPass <KisVLineIteratorSP>(srcdev.data(), srcdev.data(), yscale, yshear, ytranslate, m_filter, m_fixBorderAlpha);

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        m_progressUpdater->setProgress(100);
        return false;
    }

    if (xshear != 0.0) {
        transformPass <KisHLineIteratorSP>(srcdev.data(), m_dev.data(), 1.0, xshear, xtranslate, m_filter, m_fixBorderAlpha);
    } else {
        // No need to filter again when we are only scaling
        srcdev->move(srcdev->x() + xtranslate, srcdev->y());
        updateBounds <KisHLineIteratorSP>(m_boundRect, 1.0, 0, xtranslate);

        if (rotQuadrant != 0)  // no need to copy back if we have not copied the device in the first place
            m_boundRect = rotateNone(srcdev, m_dev, m_boundRect, m_progressUpdater, m_lastProgressReport, m_progressTotalSteps, m_progressStep);
    }

    //CBRm_dev->setDirty();

    // Progress info
    if (!m_progressUpdater.isNull()) {
        m_progressUpdater->setProgress(100);
        return !m_progressUpdater->interrupted();
    }

    return true;
}

QRect KisTransformWorker::mirrorX(KisPaintDeviceSP dev, qreal axis, const KisSelection* selection)
{

    int pixelSize = dev->pixelSize();
    KisPaintDeviceSP dst = new KisPaintDevice(dev->colorSpace());

    QRect r;

    if (selection) {
        r = selection->selectedExactRect();
    } else {
        KoColor defaultPixel(dev->defaultPixel(), dev->colorSpace());
        if (defaultPixel.opacityU8() != OPACITY_TRANSPARENT_U8)
            r = dev->dataManager()->extent();
        else
            r = dev->exactBounds();

        if (axis > 0) {
            // Extend rect so it has the same width on both sides of the axis
            qreal distanceFromAxis = qMax(fabs((qreal)r.left() - axis), fabs((qreal)r.right() - axis));
            QRect newRect(floor(axis - distanceFromAxis), r.y(), ceil(2*distanceFromAxis), r.height());
            r = newRect.adjusted(-1, 0, 2, 0);
        }
    }

    if (r.width() <= 1) return r;

    {
        quint8 *dstPixels = new quint8[r.width() * pixelSize];

        KisHLineConstIteratorSP srcIt = dev->createHLineConstIteratorNG(r.x(), r.top(), r.width());

        KisHLineConstIteratorSP selIt;
        if (selection) {
            selIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.top(), r.width());
        }

        for (qint32 y = r.top(); y <= r.bottom(); ++y) {

            quint8 *dstIt = dstPixels + (r.width() * pixelSize) - pixelSize;

            do {
                if (selIt) {
                    if (*selIt->oldRawData() > SELECTION_THRESHOLD) {
                        memcpy(dstIt, srcIt->oldRawData(), pixelSize);
                        selIt->nextPixel();
                    }
                }
                else {
                    memcpy(dstIt, srcIt->oldRawData(), pixelSize);
                }
                dstIt -= pixelSize;

            } while (srcIt->nextPixel());

            dst->writeBytes(dstPixels, QRect(r.left(), y, r.width(), 1));
            srcIt->nextRow();
        }

        delete[] dstPixels;
    }
    KisPainter gc(dev);

    if (selection) {
        dev->clearSelection(const_cast<KisSelection*>(selection));
    }
    else {
        dev->clear(r);
    }
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.bitBlt(r.topLeft(), dst, r);

    return r;
}

QRect KisTransformWorker::mirrorY(KisPaintDeviceSP dev, qreal axis, const KisSelection* selection)
{
    int pixelSize = dev->pixelSize();
    KisPaintDeviceSP dst = new KisPaintDevice(dev->colorSpace());

    /* Read a line from bottom to top and and from top to bottom and write their values to each other */
    QRect r;
    if (selection) {
        r = selection->selectedExactRect();
    } else {
        KoColor defaultPixel(dev->defaultPixel(), dev->colorSpace());
        if (defaultPixel.opacityU8() != OPACITY_TRANSPARENT_U8)
            r = dev->dataManager()->extent();
        else
            r = dev->exactBounds();

        if (axis > 0) {
            // Extend rect so it has the same height on both sides of the axis
            qreal distanceFromAxis = qMax(fabs((qreal)r.top() - axis), fabs((qreal)r.bottom() - axis));
            QRect newRect(r.x(), floor(axis - distanceFromAxis), r.width(), ceil(2*distanceFromAxis));
            r = newRect.adjusted(0, -1, 0, 2);
        }
    }
    {
        qint32 y1, y2;
        for (y1 = r.top(), y2 = r.bottom(); y1 <= r.bottom(); ++y1, --y2) {

            KisHLineConstIteratorSP itTop = dev->createHLineConstIteratorNG(r.x(), y1, r.width());
            KisHLineIteratorSP itBottom = dst->createHLineIteratorNG(r.x(), y2, r.width());
            KisHLineConstIteratorSP selIt;
            if (selection) {
                selIt = selection->projection()->createHLineConstIteratorNG(r.x(), r.top(), r.width());
            }
            do {
                if (selIt) {
                    if (*selIt->oldRawData() > SELECTION_THRESHOLD) {
                        memcpy(itBottom->rawData(), itTop->oldRawData(), pixelSize);
                        selIt->nextPixel();
                    }
                }
                else {
                    memcpy(itBottom->rawData(), itTop->oldRawData(), pixelSize);
                }
            } while (itTop->nextPixel() && itBottom->nextPixel());
        }
    }
    KisPainter gc(dev);

    if (selection) {
        dev->clearSelection(const_cast<KisSelection*>(selection));
    }
    else {
        dev->clear(r);
    }
    gc.setCompositeOp(COMPOSITE_OVER);
    gc.bitBlt(r.topLeft(), dst, r);

    return r;

}

