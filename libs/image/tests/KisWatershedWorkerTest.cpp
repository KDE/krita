/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisWatershedWorkerTest.h"

#include <QTest>

#include "kis_paint_device.h"
#include "kis_painter.h"

#include "kis_paint_device_debug_utils.h"

#include "kis_gaussian_kernel.h"
#include "krita_utils.h"

#include "lazybrush/kis_lazy_fill_tools.h"
#include "testutil.h"


#include <lazybrush/KisWatershedWorker.h>

inline KisPaintDeviceSP loadTestImage(const QString &name, bool convertToAlpha)
{
    QImage image(TestUtil::fetchDataFileLazy(name));
    KisPaintDeviceSP dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    dev->convertFromQImage(image, 0);

    if (convertToAlpha) {
        dev = KisPainter::convertToAlphaAsAlpha(dev);
    }

    return dev;
}

void KisWatershedWorkerTest::testWorker()
{
    KisPaintDeviceSP mainDev = loadTestImage("fill1_main.png", false);
    KisPaintDeviceSP aLabelDev = loadTestImage("fill1_a_extra.png", true);
    KisPaintDeviceSP bLabelDev = loadTestImage("fill1_b.png", true);
    KisPaintDeviceSP resultColoring = new KisPaintDevice(mainDev->colorSpace());

    KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsGray(mainDev);
    const QRect filterRect = filteredMainDev->exactBounds();
    KisGaussianKernel::applyLoG(filteredMainDev,
                                filterRect,
                                2,
                                QBitArray(), 0);

    KisLazyFillTools::normalizeAlpha8Device(filteredMainDev, filterRect);

    KIS_DUMP_DEVICE_2(filteredMainDev, filterRect, "main", "dd");
    KIS_DUMP_DEVICE_2(aLabelDev, filterRect, "alabel", "dd");

    KisWatershedWorker worker(filteredMainDev, resultColoring, filterRect);
    worker.addKeyStroke(aLabelDev, KoColor(Qt::red, mainDev->colorSpace()));
    worker.addKeyStroke(bLabelDev, KoColor(Qt::blue, mainDev->colorSpace()));
    worker.run();

}

QTEST_MAIN(KisWatershedWorkerTest)
