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

void testGenerate(const QString &testName, const QHash<QString, QVariant> &properties)
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

    QHashIterator<QString, QVariant> it(properties);
    while (it.hasNext()) {
        it.next();
        config->setProperty(it.key(), it.value());
    }

    generator->generate(processingInformation, testImageSize, config, updater);

    QImage referenceImage(QString(FILES_DATA_DIR) + QDir::separator() + testName + ".png");
    QImage deviceImage = paintDevice->convertToQImage(0, 0, 0, testImageSize.width(), testImageSize.height());

    QPoint differingPoint;
    if (!TestUtil::compareQImages(differingPoint, referenceImage, deviceImage)) {
        deviceImage.save(testName + "_generated.png");
        QFAIL(QString(testName + ": failed to compare images, first different pixel: %1,%2 ").arg(differingPoint.x()).arg(differingPoint.y()).toLatin1());
    }

    delete progressUpdater;
    delete testProgressBar;
}

void KisScreentoneGeneratorTest::testGenerate01()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 0);
    properties.insert("brightness", 70);

    testGenerate("test01", properties);
}

void KisScreentoneGeneratorTest::testGenerate02()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 1);
    properties.insert("brightness", 70);

    testGenerate("test02", properties);
}

void KisScreentoneGeneratorTest::testGenerate03()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 2);
    properties.insert("brightness", 70);

    testGenerate("test03", properties);
}

void KisScreentoneGeneratorTest::testGenerate04()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 0);
    properties.insert("align_to_pixel_grid", false);
    properties.insert("brightness", 70);

    testGenerate("test04", properties);
}

void KisScreentoneGeneratorTest::testGenerate05()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 1);
    properties.insert("align_to_pixel_grid", false);
    properties.insert("brightness", 70);

    testGenerate("test05", properties);
}

void KisScreentoneGeneratorTest::testGenerate06()
{
    QHash<QString, QVariant> properties;

    properties.insert("equalization_mode", 2);
    properties.insert("align_to_pixel_grid", false);
    properties.insert("brightness", 70);

    testGenerate("test06", properties);
}

void KisScreentoneGeneratorTest::testGenerate07()
{
    QHash<QString, QVariant> properties;

    properties.insert("pattern", 1);
    properties.insert("shape", 1);
    properties.insert("equalization_mode", 0);

    properties.insert("size_mode", 1);
    properties.insert("keep_size_square", false);
    properties.insert("size_x", 100.0);
    properties.insert("rotation", 15.0);

    QVariant v;
    v.setValue(KoColor(QColor(255, 0, 0), KoColorSpaceRegistry::instance()->rgb8()));
    properties.insert("foreground_color", v);
    properties.insert("background_opacity", 0);
    properties.insert("brightness", 75);
    properties.insert("contrast", 90);

    testGenerate("test07", properties);
}

void KisScreentoneGeneratorTest::testGenerate08()
{
    QHash<QString, QVariant> properties;

    properties.insert("pattern", 1);
    properties.insert("shape", 1);
    properties.insert("equalization_mode", 1);

    properties.insert("size_mode", 1);
    properties.insert("keep_size_square", false);
    properties.insert("size_x", 100.0);
    properties.insert("rotation", 15.0);

    QVariant v;
    v.setValue(KoColor(QColor(255, 0, 0), KoColorSpaceRegistry::instance()->rgb8()));
    properties.insert("foreground_color", v);
    properties.insert("background_opacity", 0);
    properties.insert("brightness", 75);
    properties.insert("contrast", 90);

    testGenerate("test08", properties);
}

void KisScreentoneGeneratorTest::testGenerate09()
{
    QHash<QString, QVariant> properties;

    properties.insert("pattern", 1);
    properties.insert("shape", 1);
    properties.insert("equalization_mode", 2);

    properties.insert("size_mode", 1);
    properties.insert("keep_size_square", false);
    properties.insert("size_x", 100.0);
    properties.insert("rotation", 15.0);

    QVariant v;
    v.setValue(KoColor(QColor(255, 0, 0), KoColorSpaceRegistry::instance()->rgb8()));
    properties.insert("foreground_color", v);
    properties.insert("background_opacity", 0);
    properties.insert("brightness", 75);
    properties.insert("contrast", 90);

    testGenerate("test09", properties);
}

KISTEST_MAIN(KisScreentoneGeneratorTest)
