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

#include <qtest_kde.h>

#include <config-vc.h>
#ifdef HAVE_VC
#include <Vc/Vc>
#include <Vc/IO>
#endif


#include "kis_mask_generator_benchmark.h"

#include "kis_circle_mask_generator.h"
#include "kis_rect_mask_generator.h"

void KisMaskGeneratorBenchmark::benchmarkCircle()
{
    KisCircleMaskGenerator gen(1000, 0.5, 0.5, 0.5, 3);
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

void KisMaskGeneratorBenchmark::benchmarkSIMD()
{
#ifdef HAVE_VC
    int width = 1000;
    float *buffer = Vc::malloc<float, Vc::AlignOnVector>(width);

    KisCircleMaskGenerator gen(1000, 0.5, 0.5, 0.5, 2);
    QBENCHMARK{
        for(int y = 0; y < 1000; ++y)
        {
//            gen.processRowFast(buffer, width, y, 0.0f, 1.0f, 500.0f, 500.0f, 0.5f, 0.5f);
        }
    }
    Vc::free(buffer);
#endif
}

void KisMaskGeneratorBenchmark::benchmarkSquare()
{
    KisRectangleMaskGenerator gen(1000, 0.5, 0.5, 0.5, 3);
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

QTEST_KDEMAIN(KisMaskGeneratorBenchmark, GUI)
#include "kis_mask_generator_benchmark.moc"
