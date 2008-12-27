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


void KisConvolutionPainter::applyMatrix(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, qint32 h, KisConvolutionBorderOp borderOp)
{
    
#if 0
    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
    case BORDER_DEFAULT_FILL :
        break;
    case BORDER_REPEAT:
        applyMatrixRepeat(kernel, src, x, y, w, h);
        return;
    case BORDER_WRAP:
    case BORDER_AVOID:
    default :
        x += khalfWidth;
        y += khalfHeight;
        w -= kw - 1;
        h -= kh - 1;
    }
#endif
    applyMatrixImpl<StandardIteratorFactory>(  kernel, src, x, y, w, h );
}
