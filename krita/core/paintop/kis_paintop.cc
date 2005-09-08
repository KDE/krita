/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_alpha_mask.h"
#include "kis_point.h"
#include "kis_abstract_colorspace.h"
#include "kis_global.h"
#include "kis_iterators_pixel.h"
#include "kis_color.h"

KisPaintOp::KisPaintOp(KisPainter * painter)
{
    m_painter = painter;
    setSource(painter->device());
}

KisPaintOp::~KisPaintOp()
{
}

KisLayerSP KisPaintOp::computeDab(KisAlphaMaskSP mask)
{
    // XXX: According to the SeaShore source, the Gimp uses a
    // temporary layer the size of the layer that is being painted
    // on. This layer is cleared between painting actions. Our
    // temporary layer, dab, is for every paintAt, composited with
    // the target layer. We only use a real temporary layer for things
    // like filter tools.

    KisLayerSP dab = new KisLayer(m_painter -> device() -> colorSpace(), "dab");
    Q_CHECK_PTR(dab);

    // XXX: Quick hack: we should use the correct color instead of going via QColor
    KisProfileSP profile = m_painter -> device() -> profile();
    KisColor kc = m_painter -> paintColor();
    
    KisAbstractColorSpace * colorSpace = dab -> colorSpace();

    Q_INT32 pixelSize = colorSpace->pixelSize();
    
    Q_INT32 maskWidth = mask -> width();
    Q_INT32 maskHeight = mask -> height();
    
    // Convert the kiscolor to the right colorspace.
    kc.convertTo(colorSpace, profile);

    for (int y = 0; y < maskHeight; y++)
    {
        KisHLineIteratorPixel hiter = dab->createHLineIterator(0, y, maskWidth, true);
        int x=0;
        while(! hiter.isDone())
        {
            // XXX: Set mask
            colorSpace->setAlpha(kc.data(), mask->alphaAt(x++, y), 1);
            memcpy(hiter.rawData(), kc.data(), pixelSize);
//             colorSpace -> fromQColor(c,
//                              mask -> alphaAt(x++, y),
//                              hiter.rawData(),
//                              profile);
            ++hiter;
        }
    }

    return dab;
}

void KisPaintOp::splitCoordinate(double coordinate, Q_INT32 *whole, double *fraction)
{
    Q_INT32 i = static_cast<Q_INT32>(coordinate);

    if (coordinate < 0) {
        // We always want the fractional part to be positive.
        // E.g. -1.25 becomes -2 and +0.75
        i--;
    }

    double f = coordinate - i;

    *whole = i;
    *fraction = f;
}

void KisPaintOp::setSource(KisPaintDeviceImplSP p) {
    m_source = p;
}
