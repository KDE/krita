/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

#include "KisScreentoneGeneratorTest.h"

void KisScreentoneGeneratorTest::initTestCase()
{
    KisGeneratorRegistry::instance();
}

void KisScreentoneGeneratorTest::testGenerate01()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("screentone");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration();
    QVERIFY(config);

    KisPaintDeviceSP paintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisProcessingInformation processingInformation(paintDevice, QPoint(0, 0), KisSelectionSP());
    TestUtil::TestProgressBar *testProgressBar = new TestUtil::TestProgressBar();
    KoProgressUpdater *progressUpdater = new KoProgressUpdater(testProgressBar);
    KoUpdaterPtr updater = progressUpdater->startSubtask();

    QSize testImageSize(256, 256);

    generator->generate(processingInformation, testImageSize, config, updater);

    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + "testImage01.png");
    QImage deviceImage = paintDevice->convertToQImage(0, 0, 0, testImageSize.width(), testImageSize.height());

    QPoint differingPoint;
    if (!TestUtil::compareQImages(differingPoint, referenceImage, deviceImage)) {
        deviceImage.save("testImage01_generated.png");
        QFAIL(QString("Test 01: failed to compare images, first different pixel: %1,%2 ").arg(differingPoint.x()).arg(differingPoint.y()).toLatin1());
    }

    delete progressUpdater;
    delete testProgressBar;
}

void KisScreentoneGeneratorTest::testGenerate02()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("screentone");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration();
    QVERIFY(config);

    KisPaintDeviceSP paintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisProcessingInformation processingInformation(paintDevice, QPoint(0, 0), KisSelectionSP());
    TestUtil::TestProgressBar *testProgressBar = new TestUtil::TestProgressBar();
    KoProgressUpdater *progressUpdater = new KoProgressUpdater(testProgressBar);
    KoUpdaterPtr updater = progressUpdater->startSubtask();

    QSize testImageSize(256, 256);

    config->setProperty("pattern", 1);
    config->setProperty("shape", 1);
    config->setProperty("interpolation", 1);

    config->setProperty("keep_size_square", false);
    config->setProperty("size_x", 100.0);
    config->setProperty("rotation", 45.0);

    QVariant v;
    v.setValue(KoColor(QColor(255, 0, 0), paintDevice->colorSpace()));
    config->setProperty("foreground_color", v);
    config->setProperty("background_opacity", 0);
    config->setProperty("brightness", 75.0);
    config->setProperty("contrast", 90.0);

    generator->generate(processingInformation, testImageSize, config, updater);

    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + "testImage02.png");
    QImage deviceImage = paintDevice->convertToQImage(0, 0, 0, testImageSize.width(), testImageSize.height());

    QPoint differingPoint;
    if (!TestUtil::compareQImages(differingPoint, referenceImage, deviceImage)) {
        deviceImage.save("testImage02_generated.png");
        QFAIL(QString("Test 02: failed to compare images, first different pixel: %1,%2 ").arg(differingPoint.x()).arg(differingPoint.y()).toLatin1());
    }

    delete progressUpdater;
    delete testProgressBar;
}

KISTEST_MAIN(KisScreentoneGeneratorTest)
