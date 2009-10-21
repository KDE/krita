/*
 *  Copyright (c) 2005, 2008 Cyrille Berger <cberger@cberger.net>
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

struct StandardIteratorFactory {
    typedef KisHLineIteratorPixel HLineIterator;
    typedef KisHLineConstIteratorPixel HLineConstIterator;
    typedef KisVLineConstIteratorPixel VLineConstIterator;
    inline static KisHLineIteratorPixel createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIterator(x, y, w);
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
    typedef KisRepeatHLineConstIteratorPixel HLineConstIterator;
    typedef KisRepeatVLineConstIteratorPixel VLineConstIterator;
    inline static KisHLineIteratorPixel createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIterator(x, y, w);
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
    dbgImage << *kernel;
    // Make the area we cover as small as possible
    if (selection()) {

        QRect r = selection()->selectedRect().intersect(QRect(srcPos, areaSize));
        dstPos += r.topLeft() - srcPos;
        srcPos = r.topLeft();
        areaSize = r.size();
    }

    if (areaSize.width() == 0 && areaSize.height() == 0) return;
    // Determine the kernel's extent from the center pixel
    qint32 kw, kh, khalfWidth, khalfHeight, xLastMinuskhw, yLastMinuskhh;
    kw = kernel->width();
    kh = kernel->height();
    khalfWidth = (kw - 1) / 2;
    khalfHeight = (kh - 1) / 2;

    xLastMinuskhw = srcPos.x() + (areaSize.width() - khalfWidth);
    yLastMinuskhh = srcPos.y() + (areaSize.height() - khalfHeight);

    // Don't try to convolve on an area smaller than the kernel, or with a kernel that is not square or has no center pixel.
    if (areaSize.width() < kw || areaSize.height() < kh || (kw&1) == 0 || (kh&1) == 0 || kernel->factor() == 0) return;

    int lastProgressPercent = 0;
    if (progressUpdater()) progressUpdater()->setProgress(0);
    const KoColorSpace * cs = device()->colorSpace();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorspace.

    int cacheSize = kw * kh;
    int cdepth = cs -> pixelSize();
    quint8** pixelPtrCache = new quint8*[cacheSize];
    for (int i = 0; i < cacheSize; i++)
        pixelPtrCache[i] = new quint8[cdepth];
//     pixelPtrCache.fill(0);

    // row == the y position of the pixel we want to change in the paint device
    int row = srcPos.y();

    typename _IteratorFactory_::HLineIterator hit = _IteratorFactory_::createHLineIterator(device(), dstPos.x(), dstPos.y(), areaSize.width(), _dataRect);

    for (; row < srcPos.y() + areaSize.height(); ++row) {

        // col = the x position of the pixel we want to change
        int col = srcPos.x();


        bool needFull = true;
        while (!hit.isDone()) {

            // Iterate over all contributing pixels that are covered by the kernel
            // krow = the y position in the kernel matrix
            if (needFull) {
                qint32 i = 0;
                for (qint32 krow = 0; krow <  kh; ++krow) {

                    // col - khalfWidth = the left starting point of the kernel as centered on our pixel
                    // krow - khalfHeight = the offset for the top of the kernel as centered on our pixel
                    // kw = the width of the kernel

                    // Fill the cache with pointers to the pixels under the kernel
                    typename _IteratorFactory_::HLineConstIterator kitSrc = _IteratorFactory_::createHLineConstIterator(src, col - khalfWidth, (row - khalfHeight) + krow, kw, _dataRect);
                    while (!kitSrc.isDone()) {
                        memcpy(pixelPtrCache[i], kitSrc.oldRawData(), cdepth);
                        ++kitSrc;
                        ++i;
                    }
                }
                needFull = false;
                Q_ASSERT(i == kw*kh);
            } else {
                for (qint32 krow = 0; krow <  kh; ++krow) { // shift the cache to the left
                    quint8** d = pixelPtrCache + krow * kw;
                    /*memmove( d, d + 1, (kw-1)*sizeof(quint8*));
                    for (int i = 0; i < (kw - 1); i++) {
                        memcpy(d[i], d[i+1], cdepth);
                    }*/
                    quint8* first = *d;
                    memmove(d, d + 1, (kw - 1)*sizeof(quint8*));
                    *(d + kw - 1) = first;
                }
                qint32 i = kw - 1;
                typename _IteratorFactory_::VLineConstIterator kitSrc = _IteratorFactory_::createVLineConstIterator(src, col + khalfWidth, row - khalfHeight, kh, _dataRect);
                while (!kitSrc.isDone()) {
                    memcpy(pixelPtrCache[i], kitSrc.oldRawData(), cdepth);
                    ++kitSrc;
                    i += kw;
                }
            }
            if (hit.isSelected()) {
                convolutionOp->convolveColors(pixelPtrCache, kernel->data(), hit.rawData(), kernel->factor(), kernel->offset(), kw * kh, channelFlags());
//                 pixelPtrCache.fill(0);
            }
            ++col;
            ++hit;
        }

        hit.nextRow();

        int progressPercent = 100 - ((((dstPos.y() + areaSize.height()) - row) * 100) / areaSize.height());

        if (progressUpdater() && progressPercent > lastProgressPercent) {
            progressUpdater()->setProgress(progressPercent);
            lastProgressPercent = progressPercent;

            if (progressUpdater()->interrupted()) {
                for (int i = 0; i < cacheSize; i++)
                    delete[] pixelPtrCache[i];
                delete[] pixelPtrCache;

                return;
            }
        }

    }

    addDirtyRect(QRect(dstPos.x(), dstPos.y(), areaSize.width(), areaSize.height()));

    if (progressUpdater()) progressUpdater()->setProgress(100);

    for (int i = 0; i < cacheSize; i++)
        delete[] pixelPtrCache[i];
    delete[] pixelPtrCache;
}

#endif
