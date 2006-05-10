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
#include <QWidget>

#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_alpha_mask.h"
#include "kis_point.h"
#include "kis_colorspace.h"
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

KisPaintDeviceSP KisPaintOp::computeDab(KisAlphaMaskSP mask) {
    return computeDab(mask, m_painter->device()->colorSpace());
}

KisPaintDeviceSP KisPaintOp::computeDab(KisAlphaMaskSP mask, KisColorSpace *cs)
{
    // XXX: According to the SeaShore source, the Gimp uses a
    // temporary layer the size of the layer that is being painted
    // on. This layer is cleared between painting actions. Our
    // temporary layer, dab, is for every paintAt, composited with
    // the target layer. We only use a real temporary layer for things
    // like filter tools.

    KisPaintDeviceSP dab = KisPaintDeviceSP(new KisPaintDevice(cs, "dab"));
    Q_CHECK_PTR(dab);

    KisColor kc = m_painter->paintColor();

    KisColorSpace * colorSpace = dab->colorSpace();

    qint32 pixelSize = colorSpace->pixelSize();

    qint32 maskWidth = mask->width();
    qint32 maskHeight = mask->height();

    // Convert the kiscolor to the right colorspace.
    kc.convertTo(colorSpace);

    for (int y = 0; y < maskHeight; y++)
    {
        KisHLineIteratorPixel hiter = dab->createHLineIterator(0, y, maskWidth, true);
        int x=0;
        while(! hiter.isDone())
        {
            // XXX: Set mask
            colorSpace->setAlpha(kc.data(), mask->alphaAt(x++, y), 1);
            memcpy(hiter.rawData(), kc.data(), pixelSize);
            ++hiter;
        }
    }

    return dab;
}

void KisPaintOp::splitCoordinate(double coordinate, qint32 *whole, double *fraction)
{
    qint32 i = static_cast<qint32>(coordinate);

    if (coordinate < 0) {
        // We always want the fractional part to be positive.
        // E.g. -1.25 becomes -2 and +0.75
        i--;
    }

    double f = coordinate - i;

    *whole = i;
    *fraction = f;
}

void KisPaintOp::setSource(KisPaintDeviceSP p) {
    Q_ASSERT(p);
    m_source = p;
}


KisPaintOpSettings* KisPaintOpFactory::settings(QWidget* /*parent*/, const KisInputDevice& /*inputDevice*/) { return 0; }

