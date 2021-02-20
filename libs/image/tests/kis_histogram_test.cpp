/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include "testimage.h"

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
