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

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoHistogramProducer.h>
#include "kis_paint_device.h"
#include "kis_histogram.h"

void KisHistogramTest::testCreation()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    QList<QString> producers = KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(cs);
    foreach (const QString &id, producers) {
        KoHistogramProducerSP producer = KoHistogramProducerFactoryRegistry::instance()->get(id)->generate();
        KisHistogram test(dev, producer, LINEAR);
    }
}


QTEST_KDEMAIN(KisHistogramTest, GUI)
#include "kis_histogram_test.moc"
