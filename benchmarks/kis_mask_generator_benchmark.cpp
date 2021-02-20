/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


void benchmarkSIMD(qreal fade) {
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFixedPaintDeviceSP dev = new KisFixedPaintDevice(cs);
    dev->setRect(QRect(0, 0, 1000, 1000));
    dev->initialize();

    MaskProcessingData data(dev, cs, nullptr,
                            0.0, 1.0,
                            500, 500, 0);

    KisCircleMaskGenerator gen(1000, 1.0, fade, fade, 2, false);

    KisBrushMaskApplicatorBase *applicator = gen.applicator();
    applicator->initializeData(&data);

    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(dev->bounds(), QSize(63, 63));

    QBENCHMARK{
        Q_FOREACH (const QRect &rc, rects) {
            applicator->process(rc);
        }
    }
}

void KisMaskGeneratorBenchmark::benchmarkSIMD_SharpBrush()
{
    benchmarkSIMD(1.0);
}

void KisMaskGeneratorBenchmark::benchmarkSIMD_FadedBrush()
{
    benchmarkSIMD(0.5);
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
