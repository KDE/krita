/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisGlobalResourcesInterface.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <generator/kis_generator_registry.h>
#include <kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <kis_selection.h>
#include <testimage.h>
#include <testutil.h>
#include <testing_timed_default_bounds.h>

#include "KisGradientGeneratorTest.h"


void KisGradientGeneratorTest::initTestCase()
{
    KisGeneratorRegistry::instance();
}

void KisGradientGeneratorTest::testDithering_data()
{
    QTest::addColumn<bool>("useLegacyFormat");
    QTest::addColumn<QString>("ditherMode");

    QTest::newRow("old_no-dither") << true << "none";
    QTest::newRow("old_blue") << true << "blue";
}

void KisGradientGeneratorTest::testDithering()
{
    QFETCH(bool, useLegacyFormat);
    QFETCH(QString, ditherMode);

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("gradient");
    QVERIFY(generator);

    KisFilterConfigurationSP config = generator->defaultConfiguration(KisGlobalResourcesInterface::instance());
    QVERIFY(config);

    if (useLegacyFormat) {
        QFile configFile(QString(FILES_DATA_DIR) + QDir::separator() + "gradient_generator_old.filterconfig");
        if (!configFile.open(QFile::ReadOnly)) {
            QFAIL("Faled to open the reference config file");
        }
        config->fromXML(configFile.readAll());
        if (ditherMode == "none") {
            config->setProperty("dither", false);
        } else if (ditherMode == "blue") {
            config->setProperty("dither", true);
        } else {
            qFatal("Dither mode %s is not supported in the legacy format", ditherMode.toLatin1().data());
        }
    } else {
        qFatal("not implemented");
    }

    QSize testImageSize(256, 256);
    KisPaintDeviceSP paintDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    paintDevice->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(QRect(QPoint(), testImageSize)));
    KisProcessingInformation processingInformation(paintDevice, QPoint(0, 0), KisSelectionSP());
    TestUtil::TestProgressBar *testProgressBar = new TestUtil::TestProgressBar();
    KoProgressUpdater *progressUpdater = new KoProgressUpdater(testProgressBar);
    KoUpdaterPtr updater = progressUpdater->startSubtask();

    generator->generate(processingInformation, testImageSize, config, updater);

    const QImage deviceImage = paintDevice->convertToQImage(0, 0, 0, testImageSize.width(), testImageSize.height());
    TestUtil::checkQImage(deviceImage, "test_dithering", "result", QTest::currentDataTag());
}

KISTEST_MAIN(KisGradientGeneratorTest)
