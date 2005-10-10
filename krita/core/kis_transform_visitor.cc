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
#include <kdebug.h>
#include <klocale.h>

#include "kis_paint_device_impl.h"
#include "kis_selection.h"
#include "kis_transform_visitor.h"
#include "kis_progress_display_interface.h"
#include "kis_iterators_pixel.h"
#include "kis_filter_strategy.h"

KisTransformVisitor::KisTransformVisitor(double xscale, double yscale,
                    double xshear, double yshear, double rotation,
                    Q_INT32 xtranslate, Q_INT32 ytranslate,
                    KisProgressDisplayInterface *progress, KisFilterStrategy *filter)
{
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

void KisTransformVisitor::rotateRight90(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst)
{
    KisSelectionSP dstSelection;
    Q_INT32 pixelSize = src -> pixelSize();
    QRect r;
    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
    }

    for (Q_INT32 y = r.bottom(); y >= r.top(); --y) {
        KisHLineIteratorPixel hit = src -> createHLineIterator(r.x(), y, r.width(), true);
        KisVLineIterator vit = dst -> createVLineIterator(-y, r.x(), r.width(), true);
        KisVLineIterator dstSelIt = dstSelection->createVLineIterator(-y, r.x(), r.width(), true);

            while (!hit.isDone()) {
            if (hit.isSelected())  {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);

                // XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
                // but for now this will do
                *(hit.rawData()+3) = 0;
            }
            *(dstSelIt.rawData()) = hit.selectedness();
            ++hit;
            ++vit;
            ++dstSelIt;
        }
    }
}

void KisTransformVisitor::rotateLeft90(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst)
{
    KisSelectionSP dstSelection;
    Q_INT32 pixelSize = src -> pixelSize();
    QRect r;
    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
    }
    Q_INT32 x = 0;

    for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {
        // Read the horizontal line from back to front, write onto the vertical column
        KisHLineIteratorPixel hit = src -> createHLineIterator(r.x(), y, r.width(), true);
        KisVLineIterator vit = dst -> createVLineIterator(y, -r.x() - r.width(), r.width(), true);
        KisVLineIterator dstSelIt = dstSelection -> createVLineIterator(y, -r.x() - r.width(), r.width(), true);

        hit += r.width() - 1;
        while (!vit.isDone()) {
            if (hit.isSelected()) {
                memcpy(vit.rawData(), hit.rawData(), pixelSize);

                // XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
                // but for now this will do
                *(hit.rawData()+3) = 0;
            }
            *(dstSelIt.rawData()) = hit.selectedness();
            --hit;
            ++vit;
            ++dstSelIt;
        }
        ++x;
    }
}

void KisTransformVisitor::rotate180(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst)
{
    KisSelectionSP dstSelection;
    Q_INT32 pixelSize = src -> pixelSize();
    QRect r;
    if(src->hasSelection())
    {
        r = src->selection()->selectedExactRect();
        dstSelection = dst->selection();
    }
    else
    {
        r = src->exactBounds();
        dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted
    }

    for (Q_INT32 y = r.top(); y <= r.bottom(); ++y) {
        KisHLineIteratorPixel srcIt = src -> createHLineIterator(r.x(), y, r.width(), false);
        KisHLineIterator dstIt = dst -> createHLineIterator(-r.x() - r.width(), -y, r.width(), true);
        KisHLineIterator dstSelIt = dstSelection -> createHLineIterator(-r.x() - r.width(), -y, r.width(), true);

        srcIt += r.width() - 1;
        while (!dstIt.isDone()) {
            if (srcIt.isSelected())  {
                memcpy(dstIt.rawData(), srcIt.rawData(), pixelSize);

                // XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
                // but for now this will do
                *(srcIt.rawData()+3) = 0;
            }
            *(dstSelIt.rawData()) = srcIt.selectedness();
            --srcIt;
            ++dstIt;
            ++dstSelIt;
        }
    }
}

template <class iter> iter createIterator(KisPaintDeviceImpl *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len);

template <> KisHLineIteratorPixel createIterator <KisHLineIteratorPixel>
(KisPaintDeviceImpl *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len)
{
    return dev->createHLineIterator(start, lineNum, len, true);
}

template <> KisVLineIteratorPixel createIterator <KisVLineIteratorPixel>
(KisPaintDeviceImpl *dev, Q_INT32 start, Q_INT32 lineNum, Q_INT32 len)
{
    return dev->createVLineIterator(lineNum, start, len, true);
}

template <class iter> void calcDimensions (KisPaintDeviceImpl *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines);

template <> void calcDimensions <KisHLineIteratorPixel>
(KisPaintDeviceImpl *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines)
{
    if(dev->hasSelection())
    {
        QRect r = dev->selection()->selectedExactRect();
        r.rect(&srcStart, &firstLine, &srcLen, &numLines);
    }
    else
        dev->exactBounds(srcStart, firstLine, srcLen, numLines);
}

template <> void calcDimensions <KisVLineIteratorPixel>
(KisPaintDeviceImpl *dev, Q_INT32 &srcStart, Q_INT32 &srcLen, Q_INT32 &firstLine, Q_INT32 &numLines)
{
    if(dev->hasSelection())
    {
        QRect r = dev->selection()->selectedExactRect();
        r.rect(&firstLine, &srcStart, &numLines, &srcLen);
    }
    else
        dev->exactBounds(firstLine, srcStart, numLines, srcLen);
}


template <class T> void KisTransformVisitor::transformPass(KisPaintDeviceImpl *src, KisPaintDeviceImpl *dst, double floatscale, double shear, Q_INT32 dx, KisFilterStrategy *filterStrategy)
{
    Q_INT32 lineNum,srcStart,firstLine,srcLen,numLines;
        Q_INT32 center, begin, end;    /* filter calculation variables */
    Q_UINT8 *data;
    Q_UINT8 pixelSize = src->pixelSize();
    KisSelectionSP dstSelection;
    KisColorSpace * cs = src->colorSpace();
    Q_INT32 scale;
    Q_INT32 scaleDenom;

    if(src->hasSelection())
        dstSelection = dst->selection();
    else
        dstSelection = new KisSelection(dst, "dummy"); // essentially a dummy to be deleted

    calcDimensions <T>(src, srcStart, srcLen, firstLine, numLines);

    scale = int(floatscale*srcLen);
    scaleDenom = srcLen;

    Q_UINT8 *weight = new Q_UINT8[100];
    const Q_UINT8 *colors[100];
    Q_INT32 support = filterStrategy->intSupport();
    Q_INT32  dstLen, dstStart;
    Q_INT32 invfscale = 256;

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
        dstLen = -srcLen * scale / scaleDenom;
    else
        dstLen = srcLen * scale / scaleDenom;


    // Calculate extra length (in each side) needed due to shear
    Q_INT32 extraLen = (support+256)>>8;

    Q_UINT8 *tmpLine = new Q_UINT8[(srcLen +2*extraLen)* pixelSize];
    Q_CHECK_PTR(tmpLine);

    Q_UINT8 *tmpSel = new Q_UINT8[srcLen+2*extraLen];
    Q_CHECK_PTR(tmpSel);

    kdDebug(DBG_AREA_CORE) << "srcLen=" << srcLen << " dstLen" << dstLen << " scale=" << scale << " sDenom=" <<scaleDenom << endl;
    kdDebug(DBG_AREA_CORE) << "srcStart="<< srcStart << ",dx=" << dx << endl;
    kdDebug(DBG_AREA_CORE) << "extraLen="<< extraLen << endl;

    for(lineNum = firstLine; lineNum < firstLine+numLines; lineNum++)
    {
        if(scale < 0)
            dstStart = srcStart * scale / scaleDenom - dstLen + dx;
        else
            dstStart = (srcStart) * scale / scaleDenom + dx;

        dstStart += int( lineNum * shear +0.5);

        // Build a temporary line
        T srcIt = createIterator <T>(src, srcStart - extraLen, lineNum, srcLen+2*extraLen);
        int i = 0;
        while(!srcIt.isDone())
        {
            Q_UINT8 *data;

            if(srcIt.isSelected())
            {
                data = srcIt.rawData();
                memcpy(&tmpLine[i*pixelSize], data, pixelSize);

                // XXX: Find a way to colorstrategy independently set alpha = alpha*(1-selectedness)
                // but for now this will do
                *(data+3) = 0;

                tmpSel[i] = 255;
            }
            else
                tmpSel[i] = 0;
            ++srcIt;
            i++;
        }

        T dstIt = createIterator <T>(dst, dstStart, lineNum, dstLen);
        T dstSelIt = createIterator <T>(dstSelection, dstStart, lineNum, dstLen);

        i=0;
        while(!dstIt.isDone())
        {
            if(scale < 0)
                center = (srcLen<<8) + (((i * scaleDenom))<<8) / scale;
            else
                center = ((i * scaleDenom)<<8) / scale;

            center += (extraLen<<8);

            // find contributing pixels
            begin = (255 + center - support)>>8; // takes ceiling by adding 255
            end = (center + support)>>8; // takes floor

printf("sup=%d begin=%d end=%d",support,begin,end);
            Q_UINT8 selectedness = tmpSel[center>>8];
            if(selectedness)
            {
                // calculate weights
                int num = 0;
                int sum = 0;
                Q_INT32 t = ((center - (begin<<8)) * invfscale)>>8;
                Q_INT32 dt = -(256 * invfscale)>>8;
                for(int srcpos = begin; srcpos <= end; srcpos++)
                {
                    Q_UINT32 tmpw = filterStrategy->intValueAt(t) * invfscale;

                    tmpw >>=8;
                    sum += tmpw;
printf(" %d=%d",t,tmpw);
                    weight[num] = tmpw;
                    colors[num] = &tmpLine[srcpos*pixelSize];
                    num++;
                    t += dt;
                }
printf(" )=%d\n",sum);
                data = dstIt.rawData();
                cs->mixColors(colors, weight, num, data);
                data = dstSelIt.rawData();
                *data = selectedness;
            }

            ++dstSelIt;
            ++dstIt;
            i++;
        }

        //progress info
        m_progressStep += dstLen;
        emit notifyProgress((m_progressStep * 100) / m_progressTotalSteps);
        if (m_cancelRequested) {
            break;
        }
    }
    delete [] tmpLine;
    delete [] tmpSel;
    delete [] weight;
}

bool KisTransformVisitor::visit(KisPainter &/*gc*/, KisPaintDeviceImpl *dev)
{
        //progress info
        m_cancelRequested = false;
        m_progress -> setSubject(this, true, true);
    m_progressTotalSteps = 0;
    m_progressStep = 0;
    QRect r;
    if(dev->hasSelection())
        r = dev->selection()->selectedExactRect();
    else
        r = dev->exactBounds();

    KisPaintDeviceImplSP tmpdev1 = new KisPaintDeviceImpl(dev->colorSpace(),"temporary");;
    KisPaintDeviceImplSP tmpdev2 = new KisPaintDeviceImpl(dev->colorSpace(),"temporary");;
    KisPaintDeviceImplSP srcdev = dev;

    double xscale = m_xscale;
    double yscale = m_yscale;
    double xshear = m_xshear;
    double yshear = m_yshear;
    double rotation = m_rotation;
    Q_INT32 xtranslate = m_xtranslate;
    Q_INT32 ytranslate = m_ytranslate;

    int rotQuadrant = int(rotation /(M_PI/2) + 0.5);
    double tmp;
    switch(rotQuadrant)
    {
        case 0:
            break;
        case 1:
            rotateRight90(srcdev, tmpdev1);
            srcdev = tmpdev1;
            rotation -= M_PI/2;
            tmp = xscale;
            xscale=yscale;
            yscale=tmp;
            break;
        case 2:
            rotate180(srcdev, tmpdev1);
            srcdev = tmpdev1;
            rotation += M_PI;
            break;
        case 3:
            rotateLeft90(srcdev, tmpdev1);
            srcdev = tmpdev1;
            rotation += M_PI/2;
            break;
        default:
            break;
    }

    yscale *= cos(rotation);
    yshear = xscale*sin(rotation);
    xscale *= 1/cos(rotation);
    xshear = -tan(rotation);
    xtranslate += int(tan(rotation)*ytranslate);

    m_progressTotalSteps = int(yscale * r.width() * r.height());
    m_progressTotalSteps += int(xscale * r.width() * (r.height() * yscale + r.width()*yshear));

QTime time;
time.start();
    if ( m_cancelRequested) {
        emit notifyProgressDone();
        return false;
    }

    transformPass <KisVLineIteratorPixel>(srcdev, tmpdev2, yscale, yshear, ytranslate, m_filter);
printf("time taken first pass %d\n",time.restart());
    if(dev->hasSelection())
        dev->selection()->clear();
printf("time taken to clear selection %d\n",time.restart());

    if ( m_cancelRequested) {
        emit notifyProgressDone();
        return false;
    }

    transformPass <KisHLineIteratorPixel>(tmpdev2, dev, xscale, xshear, xtranslate, m_filter);

printf("time taken second pass %d\n",time.elapsed());
printf("%d %d\n",xtranslate, ytranslate);
printf("%d %d\n",m_progressStep,m_progressTotalSteps);
printf("%f\n",rotation);

    //progress info
        emit notifyProgressDone();

        return m_cancelRequested;
}
