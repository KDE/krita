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
#include "kis_paint_device_impl.h"
#include "kis_painter.h"
#include "kis_pattern.h"
#include "kis_rect.h"
#include "kis_colorspace.h"
#include "kis_types.h"
#include "kis_vec.h"
#include "kis_selection.h"
#include "kis_convolution_painter.h"



KisConvolutionPainter::KisConvolutionPainter()
    : super()
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceImplSP device) : super(device)
{
}

void KisConvolutionPainter::applyMatrix(KisKernel * kernel, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h,
                    KisConvolutionBorderOp borderOp,
                    enumChannelFlags  channelFlags )
{
    // Make the area we cover as small as possible
    if (m_device -> hasSelection()) {

        if (m_device -> selection() -> isTotallyUnselected(QRect(x, y, w, h))) {
            return;
        }

        QRect r = m_device -> selection() -> extent().intersect(QRect(x, y, w, h));
        x = r.x();
        y = r.y();
        w = r.width();
        h = r.height();

    }

    // Determine the kernel's extent from the center pixel
    Q_INT32 kw, kh, kd;
    kw = kernel->width;
    kh = kernel->height;
    kd = (kw - 1) / 2;

    // Don't try to convolve on an area smaller than the kernel, or with a kernel that is not square or has no center pixel.
    if (w < kw || h < kh || kw != kh || kw&1 == 0 ) return;

    m_cancelRequested = false;
    int lastProgressPercent = 0;
    emit notifyProgress(this, 0);

    KisColorSpace * cs = m_device->colorSpace();

    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
        case BORDER_DEFAULT_FILL :
            break;
        case BORDER_WRAP:
        case BORDER_REPEAT:
        case BORDER_AVOID:
        default :
            x += (kw - 1) / 2;
            y += (kh - 1) / 2;
            w -= kw - 1;
            h -= kh - 1;
    }

    // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them in the colorstrategy.

    QMemArray<Q_UINT8 *> pixelPtrCache(kw * kh);
    pixelPtrCache.fill(0);

    // row == the y position of the pixel we want to change in the paint device
    for (int row = y; row < y + h; ++row) {

        // col = the x position of the pixel we want to change
        int col = x;

        KisHLineIteratorPixel hit = m_device -> createHLineIterator(x, row, w, true);

        while (!hit.isDone()) {
            if (hit.isSelected()) {

                // Iterate over all contributing pixels that are covered by the kernel
                // krow = the y position in the kernel matrix
                Q_INT32 i = 0;
                for (Q_INT32 krow = 0; krow <  kh; ++krow) {

                    // col - kd = the left starting point of the kernel as centered on our pixel
                    // krow - kd = the offset for the top of the kernel as centered on our pixel
                    // kw = the width of the kernel

                    // Fill the cache with pointers to the pixels under the kernel
                    KisHLineIteratorPixel kit = m_device -> createHLineIterator(col - kd, (row - kd) + krow, kw, false);
                    while (!kit.isDone()) {
                        pixelPtrCache[i] = const_cast<Q_UINT8 *>(kit.oldRawData());
                        ++kit;
                        ++i;
                    }
                }
                Q_ASSERT (i==kw*kh);
                cs->convolveColors(pixelPtrCache.data(), kernel->data, channelFlags, hit.rawData(), kernel->factor, kernel->offset, kw * kh);
                pixelPtrCache.fill(0);
            }
            ++col;
            ++hit;
        }

        int progressPercent = 100 - ((((y + h) - row) * 100) / h);

        if (progressPercent > lastProgressPercent) {
            emit notifyProgress(this, progressPercent);
            lastProgressPercent = progressPercent;

            if (m_cancelRequested) {
                return;
            }
        }

    }

    addDirtyRect(QRect(x, y, w, h));

    emit notifyProgressDone(this);
}
