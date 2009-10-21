/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de> filters
 *  Copyright (c) 2005-2007 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org> right angle rotators
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
#include <klocale.h>

#include <KoProgressUpdater.h>
#include <KoColorSpace.h>

#include "kis_paint_device.h"
#include "kis_debug.h"
#include "kis_selection.h"
#include "kis_datamanager.h"

#include "kis_iterators_pixel.h"
#include "kis_filter_strategy.h"
#include "kis_layer.h"
#include "kis_painter.h"

KisTransformWorker::KisTransformWorker(KisPaintDeviceSP dev,
                                       double xscale, double yscale,
                                       double xshear, double yshear,
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

void KisTransformWorker::rotateNone(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);

    r = src->exactBounds();

    KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), r.top(), r.width());
    KisHLineIterator vit = dst->createHLineIterator(r.x(), r.top(), r.width());
    for (qint32 i = 0; i < r.height(); ++i) {
        while (!hit.isDone()) {
            memcpy(vit.rawData(), hit.rawData(), pixelSize);

            ++hit;
            ++vit;
        }
        hit.nextRow();
        vit.nextRow();

        //progress info
        m_progressStep += r.width();
        if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(m_lastProgressReport);
        }
        if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
            break;
        }
    }
}

void KisTransformWorker::rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);
    r = src->exactBounds();

    for (qint32 y = r.bottom(); y >= r.top(); --y) {
        KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), y, r.width());
        KisVLineIterator vit = dst->createVLineIterator(-y, r.x(), r.width());

        while (!hit.isDone()) {
            memcpy(vit.rawData(), hit.rawData(), pixelSize);

            ++hit;
            ++vit;
        }

        //progress info
        m_progressStep += r.width();
        if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            if (m_progressUpdater) m_progressUpdater->setProgress(m_lastProgressReport);
        }
        if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
            break;
        }
    }
}

void KisTransformWorker::rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);
    r = src->exactBounds();

    KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        // Read the horizontal line from back to front, write onto the vertical column
        KisVLineIterator vit = dst->createVLineIterator(y, -r.x() - r.width(), r.width());

        hit += r.width() - 1;
        while (!vit.isDone()) {
            memcpy(vit.rawData(), hit.rawData(), pixelSize);

            --hit;
            ++vit;
        }
        hit.nextRow();

        //progress info
        m_progressStep += r.width();
        if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            if (m_progressUpdater) m_progressUpdater->setProgress(m_lastProgressReport);
        }
        if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
            break;
        }
    }
}

void KisTransformWorker::rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();
    Q_UNUSED(cs);
    r = src->exactBounds();

    KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIterator dstIt = dst->createHLineIterator(-r.x() - r.width(), -y, r.width());

        srcIt += r.width() - 1;
        while (!dstIt.isDone()) {
            memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);
            --srcIt;
            ++dstIt;
        }
        srcIt.nextRow();

        //progress info
        m_progressStep += r.width();
        if (m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps) {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            if (m_progressUpdater) m_progressUpdater->setProgress(m_lastProgressReport);
        }
        if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
            break;
        }
    }
}

template <class iter> iter createIterator(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len);

template <> KisHLineIteratorPixel createIterator <KisHLineIteratorPixel>
(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len)
{
    return dev->createHLineIterator(start, lineNum, len);
}

template <> KisVLineIteratorPixel createIterator <KisVLineIteratorPixel>
(KisPaintDevice *dev, qint32 start, qint32 lineNum, qint32 len)
{
    return dev->createVLineIterator(lineNum, start, len);
}

template <class iter> void calcDimensions(KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines);

template <> void calcDimensions <KisHLineIteratorPixel>
(KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    QRect rc = dev->exactBounds();
    srcStart = rc.x();
    firstLine = rc.y();
    srcLen = rc.width();
    numLines = rc.height();
}

template <> void calcDimensions <KisVLineIteratorPixel>
(KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines)
{
    QRect rc = dev->exactBounds();
    firstLine = rc.x();
    srcStart = rc.y();
    numLines = rc.width();
    srcLen = rc.height();
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
    qint32 lineNum, srcStart, firstLine, srcLen, numLines;
    qint32 center, begin, end;    /* filter calculation variables */
    quint8 *data;
    quint8 pixelSize = src->pixelSize();
    KoColorSpace * cs = src->colorSpace();
    KoMixColorsOp * mixOp = cs->mixColorsOp();
    qint32 scale;
    qint32 scaleDenom;
    qint32 shearFracOffset;


    calcDimensions <T>(src, srcStart, srcLen, firstLine, numLines);

    scale = int(floatscale * srcLen + 0.5);
    scaleDenom = srcLen;

    if (scaleDenom == 0)
        return;

    qint32 support = filterStrategy->intSupport();
    qint32 dstLen, dstStart;
    qint32 invfscale = 256;

    // handle magnification/minification
    if (abs(scale) < scaleDenom) {
        support *= scaleDenom;
        support /= scale;

        invfscale *= scale;
        invfscale /= scaleDenom;
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
        if (scale < 0)
            dstStart = srcStart * scale / scaleDenom - dstLen + dx;
        else
            dstStart = (srcStart) * scale / scaleDenom + dx;

        shearFracOffset = -int(256 * (lineNum * shear - floor(lineNum * shear)));
        dstStart += int(floor(lineNum * shear));

        // Build a temporary line
        T srcIt = createIterator <T>(src, srcStart, lineNum, srcLen);
        quint8 *data;
        qint32 i = 0;
        data = srcIt.rawData(); // take the first pixel as source - in effect duplicating the pixel
        while (i < extraLen) {
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            i++;
        }

        while (i < srcLen + extraLen) {
            data = srcIt.rawData();
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            cs->setAlpha(data, 0, 1);
            if (i < srcLen + extraLen - 1) // duplicate pixels along edge
                ++srcIt;
            i++;
        }

        data = &tmpLine[(i-1)*pixelSize]; // take the last pixel as source - in effect duplicating the pixel
        while (i < srcLen + 2*extraLen) {
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);
            i++;
        }

        T dstIt = createIterator <T>(dst, dstStart, lineNum, dstLen);

        i = 0;
        while (!dstIt.isDone()) {
            if (scaleDenom < 2500)
                center = ((i << 8) * scaleDenom) / scale;
            else {
                if (scaleDenom < 46000) // real limit is actually 46340 pixels
                    center = ((i * scaleDenom) / scale) << 8;
                else
                    center = ((i << 8) / scale * scaleDenom) / scale; // XXX fails for sizes over 2^23 pixels src width
            }

            if (scale < 0)
                center += srcLen << 8;

            // Since the above gives us the position at the left of the pixels we need to advance by one half dst pixel
            center += 128 * scaleDenom / scale;//xxx doesn't work for scale<0;
            center += (extraLen << 8) + shearFracOffset;

            // find contributing pixels
            begin = (center - support) >> 8; // find first pixel to sample
            end = (center + support) >> 8; // find last pixel to sample

            int num = 0;
            for (int srcpos = begin; srcpos <= end; ++srcpos) {
                colors[num] = &tmpLine[srcpos*pixelSize];
                num++;
            }
            data = dstIt.rawData();
            mixOp->mixColors(colors, filterWeights[center&255].weight, filterWeights[center&255].numWeights, data);

            /*
                        //possibly fix the alpha of the border if user wants it
                        if (fixBorderAlpha && (i == 0 || i == dstLen - 1))
                            cs->setAlpha(data, cs->alpha(&tmpLine[(center>>8)*pixelSize]), 1);
            */
            ++dstIt;
            i++;
        }

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
}

bool KisTransformWorker::run()
{
//return false;
    //progress info
    m_progressTotalSteps = 0;
    m_progressStep = 0;
    QRect r = m_dev->exactBounds();

    KisPaintDeviceSP tmpdev1 = KisPaintDeviceSP(new KisPaintDevice(m_dev->colorSpace()));
    KisPaintDeviceSP srcdev = m_dev;

    double xscale = m_xscale;
    double yscale = m_yscale;
    double xshear = m_xshear;
    double yshear = m_yshear;
    double rotation = m_rotation;
    qint32 xtranslate = m_xtranslate;
    qint32 ytranslate = m_ytranslate;

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
        m_progressTotalSteps = 0;
        break;
    case 1:
        rotation -= M_PI / 2;
        tmp = xscale;
        xscale = yscale;
        yscale = tmp;
        m_progressTotalSteps = r.width() * r.height();
        break;
    case 2:
        rotation -= M_PI;
        m_progressTotalSteps = r.width() * r.height();
        break;
    case 3:
        rotation -= -M_PI / 2 + 2 * M_PI;
        tmp = xscale;
        xscale = yscale;
        yscale = tmp;
        m_progressTotalSteps = r.width() * r.height();
        break;
    }

    // Calculate some auxillary values
    yshear = sin(rotation);
    xshear = -tan(rotation / 2);
    xtranslate -= int(xshear * ytranslate);

    // Calculate progress steps
    m_progressTotalSteps += int(yscale * r.width() * r.height());
    m_progressTotalSteps += int(xscale * r.width() * (r.height() * yscale + r.width() * yshear));

    m_lastProgressReport = 0;

    // Now that we have everything in place it's time to do the actual right angle rotations
    switch (rotQuadrant) {
    default: // just to shut up the compiler
    case 0:
        break;
    case 1:
        rotateRight90(srcdev, tmpdev1);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    case 2:
        rotate180(srcdev, tmpdev1);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    case 3:
        rotateLeft90(srcdev, tmpdev1);
        srcdev->clear();
        srcdev = tmpdev1;
        break;
    }

    // Handle simple move case possibly with rotation of 90,180,270
    if (rotation == 0.0 && xscale == 1.0 && yscale == 1.0) {
        if (rotQuadrant == 0) {
            // When we didn't move the m_dev to a temp device we can simply just move its coords
            srcdev->move(srcdev->x() + xtranslate, srcdev->y() + ytranslate);
        } else {
            srcdev->move(srcdev->x() + xtranslate, srcdev->y() + ytranslate);
            rotateNone(srcdev, m_dev); //copy it back
        }
        //progress info
        if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(100);
        return false;

    }

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(100);
        return false;
    }

    transformPass <KisHLineIteratorPixel>(srcdev.data(), srcdev.data(), xscale, yscale*xshear, 0, m_filter, m_fixBorderAlpha);

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        m_progressUpdater->setProgress(100);
        return false;
    }

    // Now do the second pass
    transformPass <KisVLineIteratorPixel>(srcdev.data(), srcdev.data(), yscale, yshear, ytranslate, m_filter, m_fixBorderAlpha);

    if (!m_progressUpdater.isNull() && m_progressUpdater->interrupted()) {
        m_progressUpdater->setProgress(100);
        return false;
    }

    if (xshear != 0.0)
        transformPass <KisHLineIteratorPixel>(srcdev.data(), m_dev.data(), 1.0, xshear, xtranslate, m_filter, m_fixBorderAlpha);
    else {
        // No need to filter again when we are only scaling
        srcdev->move(srcdev->x() + xtranslate, srcdev->y());
        if (rotQuadrant != 0)  // no need to copy back if we have not copied the device in the first place
            rotateNone(srcdev, m_dev);
    }

    //CBRm_dev->setDirty();

    //progress info
    if (!m_progressUpdater.isNull()) m_progressUpdater->setProgress(100);

    if (!m_progressUpdater.isNull()) return m_progressUpdater->interrupted();
    return true;
}

QRect KisTransformWorker::mirrorX(KisPaintDeviceSP dev, const KisSelection* selection)
{
    KisPaintDeviceSP dst = dev;
    bool hasCurrentMemento = dev->dataManager()->hasCurrentMemento();
    int pixelSize = dev->pixelSize();

    if (!hasCurrentMemento) {
        dst = new KisPaintDevice(dev->colorSpace());
    }

    QRect r;
    if (selection) {
        r = selection->selectedExactRect();
    } else {
        r = dev->exactBounds();
    }
    {
        KisHLineConstIteratorPixel srcIt = dev->createHLineConstIterator(r.x(), r.top(), r.width(), selection);
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(r.x(), r.top(), r.width());

        for (qint32 y = r.top(); y <= r.bottom(); ++y) {

            dstIt += r.width() - 1;

            while (!srcIt.isDone()) {
                if (srcIt.isSelected()) {
                    if (hasCurrentMemento)
                        memcpy(dstIt.rawData(), srcIt.oldRawData(), pixelSize);
                    else
                        memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);
                }
                ++srcIt;
                --dstIt;

            }
            srcIt.nextRow();
            dstIt.nextRow();
        }
    }
    if (!hasCurrentMemento) {
        dev->setDataManager(dst->dataManager(), dst->colorSpace());
    }

    return r;
}

QRect KisTransformWorker::mirrorY(KisPaintDeviceSP dev, const KisSelection* selection)
{
    KisPaintDeviceSP dst = dev;
    bool hasCurrentMemento = dev->dataManager()->hasCurrentMemento();

    int pixelSize = dev->pixelSize();

    if (!hasCurrentMemento) {
        dst = new KisPaintDevice(dev->colorSpace());
    }

    /* Read a line from bottom to top and and from top to bottom and write their values to each other */
    QRect r;
    if (selection) {
        r = selection->selectedExactRect();
    } else {
        r = dev->exactBounds();
    }
    {
        qint32 y1, y2;
        for (y1 = r.top(), y2 = r.bottom(); y1 <= r.bottom(); ++y1, --y2) {

            KisHLineIteratorPixel itTop = dst->createHLineIterator(r.x(), y1, r.width(), selection);
            KisHLineConstIteratorPixel itBottom = dev->createHLineConstIterator(r.x(), y2, r.width());

            while (!itTop.isDone() && !itBottom.isDone()) {
                if (itBottom.isSelected()) {
                    if (hasCurrentMemento)
                        memcpy(itTop.rawData(), itBottom.oldRawData(), pixelSize);
                    else
                        memcpy(itTop.rawData(), itBottom.rawData(), pixelSize);
                }
                ++itBottom;
                ++itTop;
            }
        }
    }
    if (!hasCurrentMemento) {
        dev->setDataManager(dst->dataManager(), dst->colorSpace());
    }

    return r;
}

