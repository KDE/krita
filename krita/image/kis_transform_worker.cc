/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de> filters
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#include <klocale.h>

#include "kis_debug_areas.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_transform_worker.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"
#include "kis_filter_strategy.h"
#include "kis_layer.h"
#include "kis_painter.h"

KisTransformWorker::KisTransformWorker(KisPaintDeviceSP dev, double xscale, double yscale,
                    double xshear, double yshear, double rotation,
                    qint32 xtranslate, qint32 ytranslate,
                    KisProgressDisplayInterface *progress, KisFilterStrategy *filter)
{
    m_dev= dev;
    m_xscale = xscale;
    m_yscale = yscale;
    m_xshear = xshear;
    m_yshear = yshear;
    m_rotation = rotation,
    m_xtranslate = xtranslate;
    m_ytranslate = ytranslate;
    m_progress = progress;
    m_filter = filter;
}

void KisTransformWorker::rotateNone(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    KisSelectionSP dstSelection;
    Q_INT32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();

    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst); // essentially a dummy to be deleted
    }

    KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), r.top(), r.width());
    KisHLineIterator vit = dst->createHLineIterator(r.x(), r.top(), r.width());
    KisHLineIterator dstSelIt = dstSelection->createHLineIterator(r.x(), r.top(), r.width());
    for (Q_INT32 i = 0; i < r.height(); ++i) {
            while (!hit.isDone()) {
            if (hit.isSelected())  {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);

                // XXX: Should set alpha = alpha*(1-selectedness)
                cs->setAlpha(hit.rawData(), 0, 1);
            }
            *(dstSelIt.rawData()) = hit.selectedness();
            ++hit;
            ++vit;
            ++dstSelIt;
        }
        hit.nextRow();
        vit.nextRow();
        dstSelIt.nextRow();

        //progress info
        m_progressStep += r.width();
        if(m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps)
        {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            emit notifyProgress(m_lastProgressReport);
        }
        if (m_cancelRequested) {
            break;
        }
    }
}

void KisTransformWorker::rotateRight90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    KisSelectionSP dstSelection;
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();

    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst); // essentially a dummy to be deleted
    }

    for (qint32 y = r.bottom(); y >= r.top(); --y) {
        KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), y, r.width());
        KisVLineIterator vit = dst->createVLineIterator(-y, r.x(), r.width());
        KisVLineIterator dstSelIt = dstSelection->createVLineIterator(-y, r.x(), r.width());

            while (!hit.isDone()) {
            if (hit.isSelected())  {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);

                // XXX: Should set alpha = alpha*(1-selectedness)
                cs->setAlpha(hit.rawData(), 0, 1);
            }
            *(dstSelIt.rawData()) = hit.selectedness();
            ++hit;
            ++vit;
            ++dstSelIt;
        }

        //progress info
        m_progressStep += r.width();
        if(m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps)
        {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            emit notifyProgress(m_lastProgressReport);
        }
        if (m_cancelRequested) {
            break;
        }
    }
}

void KisTransformWorker::rotateLeft90(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    KisSelectionSP dstSelection;
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();

    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst); // essentially a dummy to be deleted
    }
    qint32 x = 0;

    KisHLineIteratorPixel hit = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        // Read the horizontal line from back to front, write onto the vertical column
        KisVLineIterator vit = dst->createVLineIterator(y, -r.x() - r.width(), r.width());
        KisVLineIterator dstSelIt = dstSelection->createVLineIterator(y, -r.x() - r.width(), r.width());

        hit += r.width() - 1;
        while (!vit.isDone()) {
            if (hit.isSelected()) {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);

                // XXX: Should set alpha = alpha*(1-selectedness)
                cs->setAlpha(hit.rawData(), 0, 1);
            }
            *(dstSelIt.rawData()) = hit.selectedness();
            --hit;
            ++vit;
            ++dstSelIt;
        }
        hit.nextRow();
        ++x;

        //progress info
        m_progressStep += r.width();
        if(m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps)
        {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            emit notifyProgress(m_lastProgressReport);
        }
        if (m_cancelRequested) {
            break;
        }
    }
}

void KisTransformWorker::rotate180(KisPaintDeviceSP src, KisPaintDeviceSP dst)
{
    KisSelectionSP dstSelection;
    qint32 pixelSize = src->pixelSize();
    QRect r;
    KoColorSpace *cs = src->colorSpace();

    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst); // essentially a dummy to be deleted
    }

    KisHLineIteratorPixel srcIt = src->createHLineIterator(r.x(), r.top(), r.width());

    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIterator dstIt = dst->createHLineIterator(-r.x() - r.width(), -y, r.width());
        KisHLineIterator dstSelIt = dstSelection->createHLineIterator(-r.x() - r.width(), -y, r.width());

        srcIt += r.width() - 1;
        while (!dstIt.isDone()) {
            if (srcIt.isSelected())  {
                memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);

                // XXX: Should set alpha = alpha*(1-selectedness)
                cs->setAlpha(srcIt.rawData(), 0, 1);
            }
            *(dstSelIt.rawData()) = srcIt.selectedness();
            --srcIt;
            ++dstIt;
            ++dstSelIt;
        }
        srcIt.nextRow();

        //progress info
        m_progressStep += r.width();
        if(m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps)
        {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            emit notifyProgress(m_lastProgressReport);
        }
        if (m_cancelRequested) {
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

template <class iter> void calcDimensions (KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines, qint32 &srcStartData, qint32 &srcLenData);

template <> void calcDimensions <KisHLineIteratorPixel>
(KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines, qint32 &srcStartData, qint32 &srcLenData)
{
    dev->exactBounds(srcStartData, firstLine, srcLenData, numLines);
    if(dev->hasSelection())
    {
        QRect r = dev->selection()->selectedExactRect();
        r.getRect(&srcStart, &firstLine, &srcLen, &numLines);
    }
    else
    {
        srcStart = srcStartData;
        srcLen = srcLenData;
    }
}

template <> void calcDimensions <KisVLineIteratorPixel>
(KisPaintDevice *dev, qint32 &srcStart, qint32 &srcLen, qint32 &firstLine, qint32 &numLines, qint32 &srcStartData, qint32 &srcLenData)
{
dev->exactBounds(firstLine, srcStartData, numLines, srcLenData);    if(dev->hasSelection())
    {
        QRect r = dev->selection()->selectedExactRect();
        r.getRect(&firstLine, &srcStart, &numLines, &srcLen);
    }
    else
    {
        srcStart = srcStartData;
        srcLen = srcLenData;
    }
}

struct FilterValues
{
    quint8 numWeights;
    quint8 *weight;
    ~FilterValues() {delete [] weight;}
};

template <class T> void KisTransformWorker::transformPass(KisPaintDevice *src, KisPaintDevice *dst, double floatscale, double shear, qint32 dx, KisFilterStrategy *filterStrategy)
{
    qint32 lineNum,srcStart,firstLine,srcLen,numLines,srcStartData,srcLenData;
    qint32 center, begin, end;    /* filter calculation variables */
    quint8 *data;
    quint8 pixelSize = src->pixelSize();
    KisSelectionSP dstSelection;
    KoColorSpace * cs = src->colorSpace();
    qint32 scale;
    qint32 scaleDenom;
    qint32 shearFracOffset;

    if(src->hasSelection())
        dstSelection = dst->selection();
    else
        dstSelection = KisSelectionSP(new KisSelection(KisPaintDeviceSP(dst))); // essentially a dummy to be deleted

    calcDimensions <T>(src, srcStart, srcLen, firstLine, numLines,  srcStartData, srcLenData);

    scale = int(floatscale*srcLen);
    scaleDenom = srcLen;

    if(scaleDenom == 0)
        return;

    qint32 support = filterStrategy->intSupport();
    qint32 dstLen, dstStart;
    qint32 invfscale = 256;

    // handle magnification/minification
    if(abs(scale) < scaleDenom)
    {
        support *= scaleDenom;
        support /= scale;

        invfscale *= scale;
        invfscale /= scaleDenom;
        if(scale < 0) // handle mirroring
        {
            support = -support;
            invfscale = -invfscale;
        }
    }

    // handle mirroring
    if(scale < 0)
        dstLen = -scale;
    else
        dstLen = scale;

    // Calculate extra length (in each side) needed due to shear
    qint32 extraLen = (support+256)>>8  + 1;

    quint8 *tmpLine = new quint8[(srcLen +2*extraLen)* pixelSize];
    Q_CHECK_PTR(tmpLine);

    quint8 *tmpSel = new quint8[srcLen+2*extraLen];
    Q_CHECK_PTR(tmpSel);

    //allocate space for colors
    const quint8 **colors = new const quint8 *[2*support+1];

    // Precalculate weights
    FilterValues *filterWeights = new FilterValues[256];

    for(int center = 0; center<256; ++center)
    {
        qint32 begin = (255 + center - support)>>8; // takes ceiling by adding 255
        qint32 span = ((center + support)>>8) - begin + 1; // takes floor to get end. Subtracts begin to get span
        qint32 t = (((begin<<8) - center) * invfscale)>>8;
        qint32 dt = invfscale;
        filterWeights[center].weight = new quint8[span];
//printf("%d (",center);
        quint32 sum=0;
        for(int num = 0; num<span; ++num)
        {
            quint32 tmpw = filterStrategy->intValueAt(t) * invfscale;

            tmpw >>=8;
            filterWeights[center].weight[num] = tmpw;
//printf(" %d=%d,%d",t,filterWeights[center].weight[num],tmpw);
            t += dt;
            sum+=tmpw;
        }
//printf(" )%d sum =%d",span,sum);
        if(sum!=255)
        {
            double fixfactor= 255.0/sum;
            sum=0;
            for(int num = 0; num<span; ++num)
            {
                filterWeights[center].weight[num] = int(filterWeights[center].weight[num] * fixfactor);
                sum+=filterWeights[center].weight[num];
            }
        }

//printf("  sum2 =%d",sum);
        int num = 0;
        while(sum<255 && num*2<span)
        {
            filterWeights[center].weight[span/2 + num]++;
            ++sum;
            if(sum<255 && num<span/2)
            {
                filterWeights[center].weight[span/2 - num - 1]++;
                ++sum;
            }
            ++num;
        }
//printf("  sum3 =%d\n",sum);

        filterWeights[center].numWeights = span;
    }

    for(lineNum = firstLine; lineNum < firstLine+numLines; lineNum++)
    {
        if(scale < 0)
            dstStart = srcStart * scale / scaleDenom - dstLen + dx;
        else
            dstStart = (srcStart) * scale / scaleDenom + dx;

        shearFracOffset = -int( 256 * (lineNum * shear - floor(lineNum * shear)));
        dstStart += int(floor(lineNum * shear));

        // Build a temporary line
        T srcIt = createIterator <T>(src, QMAX(srcStart - extraLen, srcStartData), lineNum, srcLen+2*extraLen);
        qint32 i = 0;
        qint32 x = srcStart - extraLen;

        while(i < srcLen + 2*extraLen)
        {
            quint8 *data;

            data = srcIt.rawData();
            memcpy(&tmpLine[i*pixelSize], data, pixelSize);

            if(srcIt.isSelected())
            {

                tmpSel[i] = 255;
            }
            else
            {
                tmpSel[i] = 0;
            }
             if(x >= srcStartData && x < srcStartData + srcLenData - 1)
             {
                 // XXX: Should set alpha = alpha*(1-selectedness)
                 cs->setAlpha(data, 0, 1);
                 ++srcIt;
             }
            i++;
            x++;
        }

        T dstIt = createIterator <T>(dst, dstStart, lineNum, dstLen);
        T dstSelIt = createIterator <T>(dstSelection.data(), dstStart, lineNum, dstLen);

        i=0;
        while(!dstIt.isDone())
        {
            if(scaleDenom<2500)
                center = ((i<<8) * scaleDenom) / scale;
            else
            {
                if(scaleDenom<46000) // real limit is actually 46340 pixels
                    center = ((i * scaleDenom) / scale)<<8;
                else
                    center = ((i<<8)/scale * scaleDenom) / scale; // XXX fails for sizes over 2^23 pixels src width
            }

            if(scale < 0)
                center += srcLen<<8;

            center += 128*scaleDenom/scale;//xxx doesn't work for scale<0;
            center += (extraLen<<8) + shearFracOffset;

            // find contributing pixels
            begin = (255 + center - support)>>8; // takes ceiling by adding 255
            end = (center + support)>>8; // takes floor

////printf("sup=%d begin=%d end=%d",support,begin,end);
            quint8 selectedness = tmpSel[center>>8];
            if(selectedness)
            {
                int num=0;
                for(int srcpos = begin; srcpos <= end; ++srcpos)
                {
                    colors[num] = &tmpLine[srcpos*pixelSize];
                    num++;
                }
                data = dstIt.rawData();
                cs->mixColors(colors, filterWeights[center&255].weight, filterWeights[center&255].numWeights, data);
                data = dstSelIt.rawData();
                *data = selectedness;
            }

            ++dstSelIt;
            ++dstIt;
            i++;
        }

        //progress info
        m_progressStep += dstLen;
        if(m_lastProgressReport != (m_progressStep * 100) / m_progressTotalSteps)
        {
            m_lastProgressReport = (m_progressStep * 100) / m_progressTotalSteps;
            emit notifyProgress(m_lastProgressReport);
        }
        if (m_cancelRequested) {
            break;
        }
    }
    delete [] colors;
    delete [] tmpLine;
    delete [] tmpSel;
    delete [] filterWeights;
}

bool KisTransformWorker::run()
{
    //progress info
    m_cancelRequested = false;
    if(m_progress)
        m_progress->setSubject(this, true, true);
    m_progressTotalSteps = 0;
    m_progressStep = 0;
    QRect r;
    if(m_dev->hasSelection())
        r = m_dev->selection()->selectedExactRect();
    else
        r = m_dev->exactBounds();

    KisPaintDeviceSP tmpdev1 = KisPaintDeviceSP(new KisPaintDevice(m_dev->colorSpace(),"transform_tmpdev1"));
    KisPaintDeviceSP tmpdev2 = KisPaintDeviceSP(new KisPaintDevice(m_dev->colorSpace(),"transform_tmpdev2"));
    KisPaintDeviceSP tmpdev3 = KisPaintDeviceSP(new KisPaintDevice(m_dev->colorSpace(),"transform_tmpdev2"));
    KisPaintDeviceSP srcdev = m_dev;

    double xscale = m_xscale;
    double yscale = m_yscale;
    double xshear = m_xshear;
    double yshear = m_yshear;
    double rotation = m_rotation;
    qint32 xtranslate = m_xtranslate;
    qint32 ytranslate = m_ytranslate;

    if(rotation < 0.0)
        rotation = -fmod(-rotation, 2*M_PI) + 2*M_PI;
    else
        rotation = fmod(rotation, 2*M_PI);
    int rotQuadrant = int(rotation /(M_PI/2) + 0.5) & 3;

    // Figure out how we will do the initial right angle rotations
    double tmp;
    switch(rotQuadrant)
    {
        default: // just to shut up the compiler
        case 0:
            m_progressTotalSteps = 0;
            break;
        case 1:
            rotation -= M_PI/2;
            tmp = xscale;
            xscale=yscale;
            yscale=tmp;
            m_progressTotalSteps = r.width() * r.height();
            break;
        case 2:
            rotation -= M_PI;
            m_progressTotalSteps = r.width() * r.height();
            break;
        case 3:
            rotation += M_PI/2 + 2*M_PI;
            tmp = xscale;
            xscale = yscale;
            yscale = tmp;
            m_progressTotalSteps = r.width() * r.height();
            break;
    }

    // Calculate some auxillary values
    yshear = sin(rotation);
    xshear = -tan(rotation/2);
    xtranslate -= int(xshear*ytranslate);

    // Calculate progress steps
    m_progressTotalSteps += int(yscale * r.width() * r.height());
    m_progressTotalSteps += int(xscale * r.width() * (r.height() * yscale + r.width()*yshear));

    m_lastProgressReport=0;

    // Now that we have everything in place it's time to do the actual right angle rotations
    switch(rotQuadrant)
    {
        default: // just to shut up the compiler
        case 0:
            break;
        case 1:
            rotateRight90(srcdev, tmpdev1);
            srcdev = tmpdev1;
            break;
        case 2:
            rotate180(srcdev, tmpdev1);
            srcdev = tmpdev1;
            break;
        case 3:
            rotateLeft90(srcdev, tmpdev1);
            srcdev = tmpdev1;
            break;
    }

    // Handle simple move case possibly with rotation of 90,180,270
    if(rotation == 0.0 && xscale == 1.0 && yscale == 1.0)
    {
        if(rotQuadrant==0)
        {
            // Though not nessesay in the general case because we make several passes
            // We need to move (not just copy) the data to a temp dev so we can move them back
            rotateNone(srcdev, tmpdev1);
            srcdev = tmpdev1;
        }
        if(m_dev->hasSelection())
            m_dev->selection()->clear();

        srcdev->move(srcdev->getX() + xtranslate, srcdev->getY() + ytranslate);
        rotateNone(srcdev, m_dev);

        //progress info
        emit notifyProgressDone();
        m_dev->emitSelectionChanged();

        return m_cancelRequested;
    }

    if ( m_cancelRequested) {
        emit notifyProgressDone();
        return false;
    }

    transformPass <KisHLineIteratorPixel>(srcdev.data(), tmpdev2.data(), xscale, yscale*xshear, 0, m_filter);
    if(m_dev->hasSelection())
        m_dev->selection()->clear();

    if ( m_cancelRequested) {
        emit notifyProgressDone();
        return false;
    }

     // Now do the second pass
     transformPass <KisVLineIteratorPixel>(tmpdev2.data(), tmpdev3.data(), yscale, yshear, ytranslate, m_filter);

    if(m_dev->hasSelection())
        m_dev->selection()->clear();

    if ( m_cancelRequested) {
        emit notifyProgressDone();
        return false;
    }

    if(xshear!=0.0)
        transformPass <KisHLineIteratorPixel>(tmpdev3.data(), m_dev.data(), 1.0, xshear, xtranslate, m_filter);
     else
     {
         // No need to filter again when we are only scaling
         tmpdev3->move(tmpdev3->getX() + xtranslate, tmpdev3->getY());
         rotateNone(tmpdev3, m_dev);
     }

    if (m_dev->parentLayer()) {
        m_dev->parentLayer()->setDirty();
    }
    //progress info
    emit notifyProgressDone();
    m_dev->emitSelectionChanged();

    return m_cancelRequested;
}
