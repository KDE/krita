/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <generator/kis_generator_registry.h>
#include <kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KisGlobalResourcesInterface.h>
#include <simpletest.h>
#include <sdk/tests/testimage.h>
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

    KisFilterConfigurationSP config = generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
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

    KisFilterConfigurationSP config = generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
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
