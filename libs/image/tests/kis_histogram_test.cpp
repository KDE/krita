/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_histogram_test.h"

#include <QTest>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoHistogramProducer.h>
#include "kis_paint_device.h"
#include "kis_histogram.h"
#include "kis_paint_layer.h"
#include "kis_types.h"
#include "kistest.h"

void KisHistogramTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QList<QString> producers = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(cs);
    Q_FOREACH (const QString &id, producers) {
        if (id.contains("YCBCR")) {
            continue;
        }
        KoHistogramProducer *producer = KoHistogramProducerFactoryRegistry::instance()->get(id)->generate();
        if (producer) {
            KisHistogram test(dev, QRect(0, 0, 100, 100), producer, LINEAR);
            Q_UNUSED(test);
        }
        //delete producer;
    }
}


KISTEST_MAIN(KisHistogramTest)
