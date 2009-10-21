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

#include "kis_convolution_painter.h"

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

#include <kis_debug.h>
#include <klocale.h>

#include <QColor>

#include <KoProgressUpdater.h>

#include "kis_convolution_kernel.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "KoColorSpace.h"
#include "kis_types.h"

#include "kis_selection.h"

#include "kis_convolution_painter_impl.h"


KisConvolutionPainter::KisConvolutionPainter()
        : KisPainter()
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device)
        : KisPainter(device)
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device, KisSelectionSP selection)
        : KisPainter(device, selection)
{
}


void KisConvolutionPainter::applyMatrix(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, KisConvolutionBorderOp borderOp)
{

    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
    case BORDER_REPEAT: {
        QRect dataRect = src->exactBounds();
        applyMatrixImpl<RepeatIteratorFactory>(kernel, src, srcPos, dstPos, areaSize, dataRect);
    }
    return;
    case BORDER_DEFAULT_FILL : {
        applyMatrixImpl<StandardIteratorFactory>(kernel, src, srcPos, dstPos, areaSize, QRect());
        return;
    }
    case BORDER_WRAP: {
        qFatal("Not implemented");
    }
    case BORDER_AVOID:
    default : {
        // TODO should probably be computed from the exactBounds...
        qint32 kw = kernel->width();
        qint32 kh = kernel->height();
        QPoint tr((kw - 1) / 2, (kh - 1) / 2);
        srcPos += tr;
        dstPos += tr;
        areaSize -= QSize(kw - 1, kh - 1);
        applyMatrixImpl<StandardIteratorFactory>(kernel, src, srcPos, dstPos, areaSize, QRect());
    }
    }
}
