/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include "qbrush.h"
#include "qcolor.h"
#include "qfontinfo.h"
#include "qfontmetrics.h"
#include "qpen.h"
#include "qregion.h"
#include "qmatrix.h"
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QVector>

#include <kdebug.h>
#include <klocale.h>

#include <QColor>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "KoColorSpace.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_convolution_painter.h"


KisKernelSP KisKernel::fromQImage(const QImage& img)
{
    KisKernelSP k = KisKernelSP(new KisKernel);
    k->width = img.width();
    k->height = img.height();
    k->offset = 0;
    uint count = k->width * k->height;
    k->data = new qint32[count];
    qint32* itData = k->data;
    const quint8* itImg = img.bits();
    k->factor = 0;
    for(uint i = 0; i < count; ++i , ++itData, itImg+=4)
    {
        *itData = 255 - ( *itImg + *(itImg+1) + *(itImg+2) ) / 3;
        k->factor += *itData;
    }
    return k;
}


KisConvolutionPainter::KisConvolutionPainter()
    : super()
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device) : super(device)
{
}

void KisConvolutionPainter::applyMatrix(KisKernelSP kernel, qint32 x, qint32 y, qint32 w, qint32 h,
                                        KisConvolutionBorderOp borderOp )
{
    // Make the area we cover as small as possible
    if (m_device->hasSelection()) {

        QRect r = m_device->selection()->selectedRect().intersect(QRect(x, y, w, h));
        x = r.x();
        y = r.y();
        w = r.width();
        h = r.height();

    }

    if ( w == 0 && h == 0 ) return;
    // Determine the kernel's extent from the center pixel
    qint32 kw, kh, khalfWidth, khalfHeight, xLastMinuskhw, yLastMinuskhh;
    kw = kernel->width;
    kh = kernel->height;
    khalfWidth = (kw - 1) / 2;
    khalfHeight = (kh - 1) / 2;

    xLastMinuskhw = x + (w - khalfWidth);
    yLastMinuskhh = y + (h - khalfHeight);

    // Don't try to convolve on an area smaller than the kernel, or with a kernel that is not square or has no center pixel.
    if (w < kw || h < kh || (kw&1) == 0 || (kh&1) == 0 || kernel->factor == 0 ) return;

    m_cancelRequested = false;
    int lastProgressPercent = 0;
    emit notifyProgress(0);

    KoColorSpace * cs = m_device->colorSpace();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();

    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
    case BORDER_DEFAULT_FILL :
        break;
    case BORDER_REPEAT:
        applyMatrixRepeat(kernel, x, y, w, h);
        return;
    case BORDER_WRAP:
    case BORDER_AVOID:
    default :
        x += khalfWidth;
        y += khalfHeight;
        w -= kw - 1;
        h -= kh - 1;
    }

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorspace.

    int cacheSize = kw * kh;
    int cdepth = cs -> pixelSize();
    quint8** pixelPtrCache = new quint8*[cacheSize];
    for (int i = 0; i < cacheSize; i++)
        pixelPtrCache[i] = new quint8[cdepth];
//     pixelPtrCache.fill(0);

    // row == the y position of the pixel we want to change in the paint device
    int row = y;

    KisHLineIteratorPixel hit = m_device->createHLineIterator(x, y, w);

    for (; row < y + h; ++row) {

        // col = the x position of the pixel we want to change
        int col = x;


        bool needFull = true;
        while (!hit.isDone()) {

            // Iterate over all contributing pixels that are covered by the kernel
            // krow = the y position in the kernel matrix
            if(needFull)
            {
                qint32 i = 0;
                for (qint32 krow = 0; krow <  kh; ++krow) {

                    // col - khalfWidth = the left starting point of the kernel as centered on our pixel
                    // krow - khalfHeight = the offset for the top of the kernel as centered on our pixel
                    // kw = the width of the kernel

                    // Fill the cache with pointers to the pixels under the kernel
                    KisHLineConstIteratorPixel kit = m_device->createHLineIterator(col - khalfWidth, (row - khalfHeight) + krow, kw);
                    while (!kit.isDone()) {
                        memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                        ++kit;
                        ++i;
                    }
                }
                needFull = false;
                Q_ASSERT (i==kw*kh);
            } else {
                for (qint32 krow = 0; krow <  kh; ++krow) { // shift the cache to the left
                    quint8** d = pixelPtrCache + krow * kw;
                    //memmove( d, d + 1, (kw-1)*sizeof(quint8*));
                    for (int i = 0; i < (kw-1); i++) {
                        memcpy(d[i], d[i+1], cdepth);
                    }
                }
                qint32 i = kw - 1;
                KisVLineConstIteratorPixel kit = m_device->createVLineIterator(col + khalfWidth, row - khalfHeight, kh);
                while (!kit.isDone()) {
                    memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                    ++kit;
                    i += kw;
                }
            }
            if (hit.isSelected()) {
                convolutionOp->convolveColors(pixelPtrCache, kernel->data, hit.rawData(), kernel->factor, kernel->offset, kw * kh, m_channelFlags);
//                 pixelPtrCache.fill(0);
            }
            ++col;
            ++hit;
        }

        hit.nextRow();

        int progressPercent = 100 - ((((y + h) - row) * 100) / h);

        if (progressPercent > lastProgressPercent) {
            emit notifyProgress(progressPercent);
            lastProgressPercent = progressPercent;

            if (m_cancelRequested) {
                for (int i = 0; i < cacheSize; i++)
                    delete[] pixelPtrCache[i];
                delete[] pixelPtrCache;

                return;
            }
        }

    }

    addDirtyRect(QRect(x, y, w, h));

    emit notifyProgressDone();

    for (int i = 0; i < cacheSize; i++)
        delete[] pixelPtrCache[i];
    delete[] pixelPtrCache;
}

void KisConvolutionPainter::applyMatrixRepeat( KisKernelSP kernel, qint32 x, qint32 y, qint32 w, qint32 h )
{
    int lastProgressPercent = 0;
    // Determine the kernel's extent from the center pixel
    qint32 kw, kh, khalfWidth, khalfHeight, xLastMinuskhw, yLastMinuskhh;
    kw = kernel->width;
    kh = kernel->height;
    khalfWidth = (kw - 1) / 2;
    khalfHeight = (kh - 1) / 2;

    xLastMinuskhw = x + (w - khalfWidth);
    yLastMinuskhh = y + (h - khalfHeight);

    KoColorSpace * cs = m_device->colorSpace();
    KoConvolutionOp * convolutionOp = cs->convolutionOp();

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorspace.

    int cacheSize = kw * kh;
    int cdepth = cs -> pixelSize();
    quint8** pixelPtrCache = new quint8*[cacheSize];
    for (int i = 0; i < cacheSize; i++)
        pixelPtrCache[i] = new quint8[cdepth];

    // row == the y position of the pixel we want to change in the paint device
    int row = y;
    KisHLineIteratorPixel hit = m_device->createHLineIterator(x, y, w);
    for (; row < y + h; ++row) {

        // col = the x position of the pixel we want to change
        int col = x;


        bool needFull = true;

        qint32 itStart = row - khalfHeight;
        qint32 itH = kh;
        if(itStart < 0)
        {
            itH += itStart;
            itStart = 0;
        } else if(itStart + kh > yLastMinuskhh)
        {
            itH -= itStart + kh - yLastMinuskhh;
        }
        KisVLineConstIteratorPixel kit = m_device->createVLineIterator(col + khalfWidth, itStart, itH);
        while (!hit.isDone()) {

            // Iterate over all contributing pixels that are covered by the kernel
            // krow = the y position in the kernel matrix
            if(needFull) // The cache has not been fill, so we need to fill it
            {
                qint32 i = 0;
                qint32 krow = 0;
                if( row < khalfHeight )
                {
                    // We are just outside the layer, all the row in the cache will be identical
                    // so we need to create them only once, and then to copy them
                    if( x < khalfWidth)
                    { // the left pixels are outside of the layer, in the corner
                        qint32 kcol = 0;
                        KisHLineConstIteratorPixel kit = m_device->createHLineIterator(0, 0, kw);
                        for(; kcol < (khalfWidth - x) + 1; ++kcol)
                        { // First copy the address of the topleft pixel
                            memcpy(pixelPtrCache[kcol], kit.oldRawData(), cdepth);
                        }
                        for(; kcol < kw; ++kcol)
                        { // Then copy the address of the rest of the line
                            ++kit;
                            memcpy(pixelPtrCache[kcol], kit.oldRawData(), cdepth);
                        }
                    } else {
                        uint kcol = 0;
                        KisHLineConstIteratorPixel kit = m_device->createHLineIterator(col - khalfWidth, 0, kw);
                        while (!kit.isDone()) {
                            memcpy(pixelPtrCache[kcol], kit.oldRawData(), cdepth);
                            ++kit;
                            ++kcol;
                        }
                    }
                    krow = 1; // we have already done the first krow
                    for(;krow < (khalfHeight - row); ++krow)
                    {
                        //    Copy the first line in the current line
                        for (int i = 0; i < kw; i++)
                            memcpy(pixelPtrCache[krow * kw + i], pixelPtrCache[i], cdepth);
                    }
                    i = krow * kw;
                }
                qint32 itH = kh;
                if(row + khalfHeight > yLastMinuskhh)
                {
                    itH += yLastMinuskhh - row - khalfHeight;
                }
                for (; krow <  itH; ++krow) {

                    // col - khalfWidth = the left starting point of the kernel as centered on our pixel
                    // krow - khalfHeight = the offset for the top of the kernel as centered on our pixel
                    // kw = the width of the kernel

                    // Fill the cache with pointers to the pixels under the kernel
                    qint32 itHStart = col - khalfWidth;
                    qint32 itW = kw;
                    if(itHStart < 0)
                    {
                        itW += itHStart;
                        itHStart = 0;
                    }
                    KisHLineConstIteratorPixel kit = m_device->createHLineIterator(itHStart, (row - khalfHeight) + krow, itW);
                    if( col < khalfWidth )
                    {
                        for(; i <  krow * kw + ( kw - itW ); i+= 1)
                        {
                            memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                        }
                    }
                    while (!kit.isDone()) {
                        memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                        ++kit;
                        ++i;
                    }
                }
                qint32 lastvalid = i - kw;
                for(; krow < kh; ++krow) {
                    // Copy the last valid line in the current line
                    for (int i = 0; i < kw; i++)
                        memcpy(pixelPtrCache[krow * kw + i], pixelPtrCache[lastvalid + i],
                               cdepth);
                }
                needFull = false;
            } else {
                for (qint32 krow = 0; krow <  kh; ++krow) { // shift the cache to the left
                    quint8** d = pixelPtrCache + krow * kw;
                    //memmove( d, d + 1, (kw-1)*sizeof(quint8*));
                    for (int i = 0; i < (kw-1); i++) {
                        memcpy(d[i], d[i+1], cdepth);
                    }
                }
                if(col < xLastMinuskhw)
                {
                    qint32 i = kw - 1;
//                         KisVLineIteratorPixel kit = m_device->createVLineIterator(col + khalfWidth, itStart, itH, false);
                    kit.nextCol();
                    if( row < khalfHeight )
                    {
                        for(; i < (khalfHeight- row ) * kw; i+=kw)
                        {
                            memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                        }
                    }
                    while (!kit.isDone()) {
                        memcpy(pixelPtrCache[i], kit.oldRawData(), cdepth);
                        ++kit;
                        i += kw;
                    }
                    qint32 lastvalid = i - kw;
                    for(;i < kw*kh; i+=kw)
                    {
                        memcpy(pixelPtrCache[i], pixelPtrCache[lastvalid], cdepth);
                    }
                }
            }
            if (hit.isSelected()) {
                convolutionOp->convolveColors(pixelPtrCache, kernel->data, hit.rawData(), kernel->factor, kernel->offset, kw * kh, m_channelFlags);
            }
            ++col;
            ++hit;
        }
        hit.nextRow();

        int progressPercent = 100 - ((((y + h) - row) * 100) / h);

        if (progressPercent > lastProgressPercent) {
            emit notifyProgress(progressPercent);
            lastProgressPercent = progressPercent;

            if (m_cancelRequested) {
                for (int i = 0; i < cacheSize; i++)
                    delete[] pixelPtrCache[i];
                delete[] pixelPtrCache;

                return;
            }
        }

    }

    addDirtyRect(QRect(x, y, w, h));

    emit notifyProgressDone();

    for (int i = 0; i < cacheSize; i++)
        delete[] pixelPtrCache[i];
    delete[] pixelPtrCache;
}
