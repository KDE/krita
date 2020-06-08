/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <generator/kis_generator_registry.h>
#include <kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <QTest>
#include <sdk/tests/kistest.h>
#include <testutil.h>

#include "kis_seexpr_generator_test.h"

void KisSeExprGeneratorTest::initTestCase()
{
    KisGeneratorRegistry::instance();
}

void KisSeExprGeneratorTest::testGeneration()
{

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("seexpr");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration();
    QVERIFY(config);

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisProcessingInformation test(dev, QPoint(0, 0), KisSelectionSP());
    TestUtil::TestProgressBar *bar = new TestUtil::TestProgressBar();
    KoProgressUpdater *pu = new KoProgressUpdater(bar);
    KoUpdaterPtr updater = pu->startSubtask();

    QSize testSize(256, 256);

    generator->generate(test, testSize, config, updater);

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "noisecolor2.png");

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, dev->convertToQImage(0, 0, 0, testSize.width(), testSize.height()))) {
        dev->convertToQImage(0, 0, 0, testSize.width(), testSize.height()).save("filtertest.png");
        QFAIL(QString("Failed to create image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
    delete pu;
    delete bar;
}

KISTEST_MAIN(KisSeExprGeneratorTest)
