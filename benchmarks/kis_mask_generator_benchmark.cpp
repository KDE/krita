/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include <config-vc.h>
#ifdef HAVE_VC
#if defined _MSC_VER
// Lets shut up the "possible loss of data" and "forcing value to bool 'true' or 'false'
#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4800 )
#endif
#include <Vc/Vc>
#include <Vc/IO>
#if defined _MSC_VER
#pragma warning ( pop )
#endif
#endif

#include <QTest>

#include "kis_mask_generator_benchmark.h"

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"

void KisMaskGeneratorBenchmark::benchmarkCircle()
{
    KisCircleMaskGenerator gen(1000, 0.5, 0.5, 0.5, 3, true);
    QBENCHMARK{
        for(int i = -600; i < 600; ++i)
        {
            for(int j = -600; j < 600; ++j)
            {
                gen.valueAt(i, j);
            }
        }
    }
}

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_fixed_paint_device.h"
#include "kis_types.h"
#include "kis_brush_mask_applicator_base.h"
#include "krita_utils.h"


void KisMaskGeneratorBenchmark::benchmarkSIMD()
{
#ifdef HAVE_VC
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->setRect(QRect(0, 0, 100, 100));
    dev->initialize();

    MaskProcessingData data(dev, cs,
                            0.0, 1.0,
                            50, 50, 0);

    KisCircleMaskGenerator gen(100, 0.5, 0.5, 0.5, 2, false);

    KisBrushMaskApplicatorBase *applicator = gen.applicator();
    applicator->initializeData(&data);

    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(dev->bounds(), QSize(63, 63));

    QBENCHMARK{
        Q_FOREACH (const QRect &rc, rects) {
            applicator->process(rc);
        }
    }
#endif
}

void KisMaskGeneratorBenchmark::benchmarkSquare()
{
    KisRectangleMaskGenerator gen(1000, 0.5, 0.5, 0.5, 3, true);
    QBENCHMARK{
        for(int i = -600; i < 600; ++i)
        {
            for(int j = -600; j < 600; ++j)
            {
                gen.valueAt(i, j);
            }
        }
    }
}

QTEST_MAIN(KisMaskGeneratorBenchmark)
