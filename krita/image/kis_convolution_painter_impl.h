/*
 *  Copyright (c) 2005, 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#ifndef KIS_CONVOLUTION_PAINTER_IMPL_H
#define KIS_CONVOLUTION_PAINTER_IMPL_H

#include "kis_convolution_painter.h"

#include "kis_repeat_iterators_pixel.h"

#include <KoUpdater.h>
#include <QTime>

struct StandardIteratorFactory {
    typedef KisHLineIteratorPixel HLineIterator;
    typedef KisVLineIteratorPixel VLineIterator;
    typedef KisHLineConstIteratorPixel HLineConstIterator;
    typedef KisVLineConstIteratorPixel VLineConstIterator;
    inline static KisHLineIteratorPixel createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIterator(x, y, w);
    }
    inline static KisVLineIteratorPixel createVLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineIterator(x, y, h);
    }
    inline static KisHLineConstIteratorPixel createHLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineConstIterator(x, y, w);
    }
    inline static KisVLineConstIteratorPixel createVLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineConstIterator(x, y, h);
    }
};

struct RepeatIteratorFactory {
    typedef KisHLineIteratorPixel HLineIterator;
    typedef KisVLineIteratorPixel VLineIterator;
    typedef KisRepeatHLineConstIteratorPixel HLineConstIterator;
    typedef KisRepeatVLineConstIteratorPixel VLineConstIterator;
    inline static KisHLineIteratorPixel createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIterator(x, y, w);
    }
    inline static KisVLineIteratorPixel createVLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineIterator(x, y, h);
    }
    inline static KisRepeatHLineConstIteratorPixel createHLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect& _dataRect) {
        return src->createRepeatHLineConstIterator(x, y, w, _dataRect);
    }
    inline static KisRepeatVLineConstIteratorPixel createVLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect& _dataRect) {
        return src->createRepeatVLineConstIterator(x, y, h, _dataRect);
    }
};
template< class _IteratorFactory_>
void KisConvolutionPainter::applyMatrixImpl(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& _dataRect)
{
    quint32 kernelSize = kernel->width() * kernel->height();
    qreal *kernelData = new qreal[kernelSize];
    qreal *kernelDataPtr = kernelData;

    // fill in data
    for (quint32 r = 0; r < kernel->height(); r++) {
        for (quint32 c = 0; c < kernel->width(); c++) {
            *kernelDataPtr = (*(kernel->data()))(r, c);
            kernelDataPtr++;
        }
    }

    dbgImage << *kernel;

    // Make the area we cover as small as possible
    if (selection()) {
        QRect r = selection()->selectedRect().intersect(QRect(srcPos, areaSize));
        dstPos += r.topLeft() - srcPos;
        srcPos = r.topLeft();
        areaSize = r.size();
    }

    if (areaSize.width() == 0 && areaSize.height() == 0)
        return;

    // store some kernel characteristics
    m_kw = kernel->width();
    m_kh = kernel->height();
    m_khalfWidth = (m_kw - 1) / 2;
    m_khalfHeight = (m_kh - 1) / 2;

    // Don't do with an even sized kernel
    Q_ASSERT((m_kw & 0x01) == 1 || (m_kh & 0x01) == 1 || kernel->factor() != 0);

    bool hasProgressUpdater = progressUpdater();
    if (hasProgressUpdater)
        progressUpdater()->setProgress(0);

    const KoColorSpace * cs = device()->colorSpace();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorspace.
    m_cacheSize = m_kw * m_kh;
    m_cdepth = cs->pixelSize();

    quint8** pixelPtrCache = new quint8*[m_cacheSize];
    quint8** pixelPtrCacheCopy = new quint8*[m_cacheSize];
    for (quint32 i = 0; i < m_cacheSize; i++)
    {
        pixelPtrCache[i] = new quint8[m_cdepth];
        pixelPtrCacheCopy[i] = new quint8[m_cdepth];
    }

    // decide caching strategy
    enum TraversingDirection { Horizontal, Vertical };
    TraversingDirection traversingDirection = Vertical;
    if (m_kw > m_kh) 
       traversingDirection = Horizontal;

    int row = srcPos.y();
    int col = srcPos.x();

    // populate pixelPtrCacheCopy for starting position (0, 0)
    qint32 i = 0;
    for (quint32 krow = 0; krow < m_kh; ++krow) {
        typename _IteratorFactory_::HLineConstIterator kitSrc = _IteratorFactory_::createHLineConstIterator(src, col - m_khalfWidth, row - m_khalfHeight + krow, m_kw, _dataRect);
        while (!kitSrc.isDone()) {
            memcpy(pixelPtrCacheCopy[i], kitSrc.oldRawData(), m_cdepth);
            ++kitSrc;
            ++i;
        }
    }

    if (traversingDirection == Horizontal)
    {
        const float maxProgressValue = 1.0 / (areaSize.width() - 1) * 100;
        typename _IteratorFactory_::HLineIterator hit = _IteratorFactory_::createHLineIterator(device(), dstPos.x(), dstPos.y(), areaSize.width(), _dataRect);

        for (int prow = 0; prow < areaSize.height(); prow++)
        {
            for (quint32 i = 0; i < m_cacheSize; i++)
                memcpy(pixelPtrCache[i], pixelPtrCacheCopy[i], m_cdepth);

            for (int pcol = 0; pcol < areaSize.width(); pcol++)
            {
                convolutionOp->convolveColors(pixelPtrCache, kernelData, hit.rawData(), kernel->factor(), kernel->offset(), m_cacheSize, channelFlags());

                col++;
                ++hit;
                moveKernelRight<_IteratorFactory_>(src, _dataRect, pixelPtrCache, col, row);
            }

            row++;
            hit.nextRow();
            col = srcPos.x();

            moveKernelDown<_IteratorFactory_>(src, _dataRect, pixelPtrCacheCopy, col, row);

            if (hasProgressUpdater) {
                progressUpdater()->setProgress(prow * maxProgressValue);

                if (progressUpdater()->interrupted()) {
                    cleanUp(pixelPtrCache, pixelPtrCacheCopy);
                    return;
                }
            }
        }
    }
    else if (traversingDirection == Vertical)
    {
        const float maxProgressValue = 1.0 / (areaSize.height() - 1) * 100;
        typename _IteratorFactory_::VLineIterator hit = _IteratorFactory_::createVLineIterator(device(), dstPos.x(), dstPos.y(), areaSize.height(), _dataRect);
        
        for (int pcol = 0; pcol < areaSize.width(); pcol++)
        {
            for (quint32 i = 0; i < m_cacheSize; i++)
                memcpy(pixelPtrCache[i], pixelPtrCacheCopy[i], m_cdepth);

            for (int prow = 0; prow < areaSize.height(); prow++)
            {
                convolutionOp->convolveColors(pixelPtrCache, kernelData, hit.rawData(), kernel->factor(), kernel->offset(), m_cacheSize, channelFlags());

                row++;
                ++hit;
                moveKernelDown<_IteratorFactory_>(src, _dataRect, pixelPtrCache, col, row);
            }

            col++;
            hit.nextCol();
            row = srcPos.y();

            moveKernelRight<_IteratorFactory_>(src, _dataRect, pixelPtrCacheCopy, col, row);

            if (hasProgressUpdater) {
                progressUpdater()->setProgress(pcol * maxProgressValue);

                if (progressUpdater()->interrupted()) {
                    cleanUp(pixelPtrCache, pixelPtrCacheCopy);
                    return;
                }
            }
        }
    }

    addDirtyRect(QRect(dstPos.x(), dstPos.y(), areaSize.width(), areaSize.height()));
    cleanUp(pixelPtrCache, pixelPtrCacheCopy);
}

template< class _IteratorFactory_>
void KisConvolutionPainter::moveKernelRight(const KisPaintDeviceSP src, const QRect& _dataRect, quint8 **pixelPtrCache, quint32 col, quint32 row)
{
    quint8** d = pixelPtrCache;

    for (quint32 krow = 0; krow < m_kh; ++krow) {
        quint8* first = *d;
        memmove(d, d + 1, (m_kw - 1) * sizeof(quint8 *));
        *(d + m_kw - 1) = first;
        d += m_kw;
    }

    qint32 i = m_kw - 1;
    typename _IteratorFactory_::VLineConstIterator kitSrc = _IteratorFactory_::createVLineConstIterator(src, col + m_khalfWidth, row - m_khalfHeight, m_kh, _dataRect);
    while (!kitSrc.isDone()) {
        memcpy(pixelPtrCache[i], kitSrc.oldRawData(), m_cdepth);
        ++kitSrc;
        i += m_kw;
    }
}

template< class _IteratorFactory_>
void KisConvolutionPainter::moveKernelDown(const KisPaintDeviceSP src, const QRect& _dataRect, quint8 **pixelPtrCache, quint32 col, quint32 row)
{
    quint8 **tmp = new quint8*[m_kw];
    memcpy(tmp, pixelPtrCache, m_kw * sizeof(quint8 *));
    memmove(pixelPtrCache, pixelPtrCache + m_kw, (m_kw * m_kh - m_kw) * sizeof(quint8 *));
    memcpy(pixelPtrCache + m_kw * (m_kh - 1), tmp, m_kw * sizeof(quint8 *));
    delete[] tmp;

    qint32 i = m_kw * (m_kh - 1);
    typename _IteratorFactory_::HLineConstIterator kitSrc = _IteratorFactory_::createHLineConstIterator(src, col - m_khalfWidth, row + m_khalfHeight, m_kw, _dataRect);
    while (!kitSrc.isDone()) {
        memcpy(pixelPtrCache[i], kitSrc.oldRawData(), m_cdepth);
        ++kitSrc;
        i++;
    }
}

void KisConvolutionPainter::cleanUp(quint8 **p1, quint8 **p2)
{
    for (quint32 i = 0; i < m_cacheSize; i++) {
        delete[] p1[i];
        delete[] p2[i];
    }

    delete[] p1;
    delete[] p2;
}

#endif
