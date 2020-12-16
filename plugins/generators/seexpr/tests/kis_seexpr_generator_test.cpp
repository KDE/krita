/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisGlobalResourcesInterface.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <QTest>
#include <generator/kis_generator_registry.h>
#include <kis_fill_painter.h>
#include <kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <resources/KisSeExprScript.h>
#include <sdk/tests/testimage.h>
#include <testutil.h>

#include "kis_seexpr_generator_test.h"

#define BASE_SCRIPT                                                                                                                                                                                                                            \
    "$val=voronoi(5*[$u,$v,.5],4,.6,.2); \n \
$color=ccurve($val,\n\
    0.000, [0.141, 0.059, 0.051], 4,\n\
    0.185, [0.302, 0.176, 0.122], 4,\n\
    0.301, [0.651, 0.447, 0.165], 4,\n\
    0.462, [0.976, 0.976, 0.976], 4);\n\
$color\n\
"

void KisSeExprGeneratorTest::initTestCase()
{
    KisGeneratorRegistry::instance();
}

void KisSeExprGeneratorTest::testGenerationFromScript()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("seexpr");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
    QVERIFY(config);

    config->setProperty("script", BASE_SCRIPT);

    QPoint point(0, 0);
    QSize testSize(256, 256);

    KisDefaultBoundsBaseSP bounds(new KisWrapAroundBoundsWrapper(new KisDefaultBounds(), QRect(point.x(), point.y(), testSize.width(), testSize.height())));
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(bounds);

    KisFillPainter fillPainter(dev);
    fillPainter.fillRect(point.x(), point.y(), 256, 256, config);

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "noisecolor2.png");

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, dev->convertToQImage(nullptr, point.x(), point.y(), testSize.width(), testSize.height()), 1)) {
        dev->convertToQImage(nullptr, point.x(), point.y(), testSize.width(), testSize.height()).save("filtertest.png");
        QFAIL(QString("Failed to create image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisSeExprGeneratorTest::testGenerationFromKoResource()
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("seexpr");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
    QVERIFY(config);

    auto resource = new KisSeExprScript(TestUtil::fetchDataFileLazy("Disney_noisecolor2.kse"));
    resource->load(KisGlobalResourcesInterface::instance());
    Q_ASSERT(resource->valid());

    config->setProperty("script", resource->script());

    QPoint point(0, 0);
    QSize testSize(256, 256);

    KisDefaultBoundsBaseSP bounds(new KisWrapAroundBoundsWrapper(new KisDefaultBounds(), QRect(point.x(), point.y(), testSize.width(), testSize.height())));
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(bounds);

    KisFillPainter fillPainter(dev);
    fillPainter.fillRect(point.x(), point.y(), 256, 256, config);

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "noisecolor2.png");

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, dev->convertToQImage(nullptr, point.x(), point.y(), testSize.width(), testSize.height()), 1)) {
        dev->convertToQImage(nullptr, point.x(), point.y(), testSize.width(), testSize.height()).save("filtertest.png");
        QFAIL(QString("Failed to create image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

KISTEST_MAIN(KisSeExprGeneratorTest)
