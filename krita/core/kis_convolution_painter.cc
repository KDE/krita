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
#include "qwmatrix.h"
#include <qimage.h>
#include <qmap.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpointarray.h>
#include <qrect.h>
#include <qstring.h>

#include <kdebug.h>
#include <klocale.h>

#include <qcolor.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_colorspace.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_convolution_painter.h"


KisKernelSP KisKernel::fromQImage(const QImage& img)
{
    KisKernelSP k = new KisKernel;
    k->width = img.width();
    k->height = img.height();
    k->offset = 0;
    uint count = k->width * k->height;
    k->data = new Q_INT32[count];
    Q_INT32* itData = k->data;
    Q_UINT8* itImg = img.bits();
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

void KisConvolutionPainter::applyMatrix(KisKernelSP kernel, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h,
                    KisConvolutionBorderOp borderOp,
                    KisChannelInfo::enumChannelFlags  channelFlags )
{
    // Make the area we cover as small as possible
    if (m_device->hasSelection()) {

        if (m_device->selection()->isTotallyUnselected(QRect(x, y, w, h))) {
            return;
        }

        QRect r = m_device->selection()->extent().intersect(QRect(x, y, w, h));
        x = r.x();
        y = r.y();
        w = r.width();
        h = r.height();

    }

    // Determine the kernel's extent from the center pixel
    Q_INT32 kw, kh, khalfWidth, khalfHeight, xLastMinuskhw, yLastMinuskhh;
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

    KisColorSpace * cs = m_device->colorSpace();

    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
        case BORDER_DEFAULT_FILL :
            break;
        case BORDER_REPEAT:
            applyMatrixRepeat(kernel, x, y, w, h, channelFlags);
            return;
        case BORDER_WRAP:
        case BORDER_AVOID:
        default :
            x += khalfWidth;
            y += khalfHeight;
            w -= kw - 1;
            h -= kh - 1;
    }

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorstrategy.

    QMemArray<Q_UINT8 *> pixelPtrCache(kw * kh);
//     pixelPtrCache.fill(0);

    // row == the y position of the pixel we want to change in the paint device
    int row = y;

    for (; row < y + h; ++row) {

        // col = the x position of the pixel we want to change
        int col = x;

        KisHLineIteratorPixel hit = m_device->createHLineIterator(x, row, w, true);
        bool needFull = true;
        while (!hit.isDone()) {
            if (hit.isSelected()) {

                // Iterate over all contributing pixels that are covered by the kernel
                // krow = the y position in the kernel matrix
                if(needFull)
                {
                    Q_INT32 i = 0;
                    for (Q_INT32 krow = 0; krow <  kh; ++krow) {

                        // col - khalfWidth = the left starting point of the kernel as centered on our pixel
                        // krow - khalfHeight = the offset for the top of the kernel as centered on our pixel
                        // kw = the width of the kernel

                        // Fill the cache with pointers to the pixels under the kernel
                        KisHLineIteratorPixel kit = m_device->createHLineIterator(col - khalfWidth, (row - khalfHeight) + krow, kw, false);
                        while (!kit.isDone()) {
                            pixelPtrCache[i] = const_cast<Q_UINT8 *>(kit.oldRawData());
                            ++kit;
                            ++i;
                        }
                    }
                    needFull = false;
                    Q_ASSERT (i==kw*kh);
                } else {
                    for (Q_INT32 krow = 0; krow <  kh; ++krow) { // shift the cache to the left
                        Q_UINT8** d = pixelPtrCache.data() + krow * kw;
                        memmove( d, d + 1, (kw-1)*sizeof(Q_UINT8*));
                    }
                    Q_INT32 i = kw - 1;
                    KisVLineIteratorPixel kit = m_device->createVLineIterator(col + khalfWidth, row - khalfHeight, kh, false);
                    while (!kit.isDone()) {
                        pixelPtrCache[i] = const_cast<Q_UINT8 *>(kit.oldRawData());
                        ++kit;
                        i += kw;
                    }
                }
                cs->convolveColors(pixelPtrCache.data(), kernel->data, channelFlags, hit.rawData(), kernel->factor, kernel->offset, kw * kh);
//                 pixelPtrCache.fill(0);
            }
            ++col;
            ++hit;
        }

        int progressPercent = 100 - ((((y + h) - row) * 100) / h);

        if (progressPercent > lastProgressPercent) {
            emit notifyProgress(progressPercent);
            lastProgressPercent = progressPercent;

            if (m_cancelRequested) {
                return;
            }
        }

    }

    addDirtyRect(QRect(x, y, w, h));

    emit notifyProgressDone();
}

void KisConvolutionPainter::applyMatrixRepeat(KisKernelSP kernel, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h,
                           KisChannelInfo::enumChannelFlags channelFlags)
{
    int lastProgressPercent = 0;
    // Determine the kernel's extent from the center pixel
    Q_INT32 kw, kh, khalfWidth, khalfHeight, xLastMinuskhw, yLastMinuskhh;
    kw = kernel->width;
    kh = kernel->height;
    khalfWidth = (kw - 1) / 2;
    khalfHeight = (kh - 1) / 2;

    xLastMinuskhw = x + (w - khalfWidth);
    yLastMinuskhh = y + (h - khalfHeight);

    KisColorSpace * cs = m_device->colorSpace();

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorstrategy.

    int cacheSize = kw * kh;
    int cdepth = cs -> pixelSize();
    Q_UINT8** pixelPtrCache = new Q_UINT8*[cacheSize];
    for (int i = 0; i < cacheSize; i++)
        pixelPtrCache[i] = new Q_UINT8[cdepth];

    // row == the y position of the pixel we want to change in the paint device
    int row = y;

    for (; row < y + h; ++row) {

        // col = the x position of the pixel we want to change
        int col = x;

        KisHLineIteratorPixel hit = m_device->createHLineIterator(x, row, w, true);
        bool needFull = true;
        
        Q_INT32 itStart = row - khalfHeight;
        Q_INT32 itH = kh;
        if(itStart < 0)
        {
            itH += itStart;
            itStart = 0;
        } else if(itStart + kh > yLastMinuskhh)
        {
            itH -= itStart + kh - yLastMinuskhh;
        }
        KisVLineIteratorPixel kit = m_device->createVLineIterator(col + khalfWidth, itStart, itH, false);
        while (!hit.isDone()) {
            if (hit.isSelected()) {

                // Iterate over all contributing pixels that are covered by the kernel
                // krow = the y position in the kernel matrix
                if(needFull) // The cache has not been fill, so we need to fill it
                {
                    Q_INT32 i = 0;
                    Q_INT32 krow = 0;
                    if( row < khalfHeight )
                    {
                        // We are just outside the layer, all the row in the cache will be identical
                        // so we need to create them only once, and then to copy them
                        if( x < khalfWidth)
                        { // the left pixels are outside of the layer, in the corner
                            Q_INT32 kcol = 0;
                            KisHLineIteratorPixel kit = m_device->createHLineIterator(0, 0, kw, false);
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
                            KisHLineIteratorPixel kit = m_device->createHLineIterator(col - khalfWidth, 0, kw, false);
                            while (!kit.isDone()) {
                                memcpy(pixelPtrCache[kcol], kit.oldRawData(), cdepth);
                                ++kit;
                                ++kcol;
                            }
                        }
                        krow = 1; // we have allready done the first krow
                        for(;krow < (khalfHeight - row); ++krow)
                        {
                            //    Copy the first line in the current line
                            for (int i = 0; i < kw; i++)
                                memcpy(pixelPtrCache[krow * kw + i], pixelPtrCache[i], cdepth);
                        }
                        i = krow * kw;
                    }
                    Q_INT32 itH = kh;
                    if(row + khalfHeight > yLastMinuskhh)
                    {
                        itH += yLastMinuskhh - row - khalfHeight;
                    }
                    for (; krow <  itH; ++krow) {

                        // col - khalfWidth = the left starting point of the kernel as centered on our pixel
                        // krow - khalfHeight = the offset for the top of the kernel as centered on our pixel
                        // kw = the width of the kernel

                        // Fill the cache with pointers to the pixels under the kernel
                        Q_INT32 itHStart = col - khalfWidth;
                        Q_INT32 itW = kw;
                        if(itHStart < 0)
                        {
                            itW += itHStart;
                            itHStart = 0;
                        }
                        KisHLineIteratorPixel kit = m_device->createHLineIterator(itHStart, (row - khalfHeight) + krow, itW, false);
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
                    Q_INT32 lastvalid = i - kw;
                    for(; krow < kh; ++krow) {
                        // Copy the last valid line in the current line
                        for (int i = 0; i < kw; i++)
                            memcpy(pixelPtrCache[krow * kw + i], pixelPtrCache[lastvalid + i],
                                   cdepth);
                    }
                    needFull = false;
                } else {
                    for (Q_INT32 krow = 0; krow <  kh; ++krow) { // shift the cache to the left
                        Q_UINT8** d = pixelPtrCache + krow * kw;
                        //memmove( d, d + 1, (kw-1)*sizeof(Q_UINT8*));
                        for (int i = 0; i < (kw-1); i++) {
                            memcpy(d[i], d[i+1], cdepth);
                        }
                    }
                    if(col < xLastMinuskhw)
                    {
                        Q_INT32 i = kw - 1;
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
                        Q_INT32 lastvalid = i - kw;
                        for(;i < kw*kh; i+=kw)
                        {
                            memcpy(pixelPtrCache[i], pixelPtrCache[lastvalid], cdepth);
                        }
                    }
                }
                cs->convolveColors(pixelPtrCache, kernel->data, channelFlags, hit.rawData(), kernel->factor, kernel->offset, kw * kh);
            }
            ++col;
            ++hit;
        }

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
