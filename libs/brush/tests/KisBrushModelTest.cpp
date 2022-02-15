/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushModelTest.h"

#include "KisBrushModel.h"
#include "KisGlobalResourcesInterface.h"

#define TESTBRUSH
#include <kistest.h>

#include "testutil.h"
#include "KisResourceModel.h"
#include <KisResourceCacheDb.h>
#include "KisResourceLoaderRegistry.h"
#include "KisResourceLoader.h"
#include "KisResourceLocator.h"
#include "kis_png_brush.h"
#include "KoResourcePaths.h"

void KisBrushModelTest::initTestCase()
{
    {
        QString fullFileName = TestUtil::fetchDataFileLazy("kritaTransparent.png");
        KIS_ASSERT(!fullFileName.isEmpty());
        KIS_ASSERT(QFileInfo(fullFileName).exists());

        KisResourceModel model(ResourceType::Brushes);
        KoResourceSP result = model.importResourceFile(fullFileName, false);
        QVERIFY(result);
    }
}

void KisBrushModelTest::testAutoBrush()
{
    KisBrushModel::BrushData data;

    data.type = KisBrushModel::Auto;
    data.common.angle = 3;
    data.common.spacing = 0.5;
    data.common.autoSpacingCoeff = 1.1;
    data.common.useAutoSpacing = true;

    data.autoBrush.density = 0.9;
    data.autoBrush.randomness = 0.1;
    data.autoBrush.generator.type = KisBrushModel::Gaussian;
    data.autoBrush.generator.shape = KisBrushModel::Circle;
    data.autoBrush.generator.diameter = 77;
    data.autoBrush.generator.horizontalFade = 0.5;
    data.autoBrush.generator.verticalFade = 0.6;
    data.autoBrush.generator.ratio = 0.8;
    data.autoBrush.generator.antialiasEdges = false;
    data.autoBrush.generator.spikes = 4;

    QCOMPARE(data, data);

    KisPropertiesConfiguration config;

    data.write(&config);

    std::optional<KisBrushModel::BrushData> newData =
        KisBrushModel::BrushData::read(&config, KisGlobalResourcesInterface::instance());

    QVERIFY(newData);
    QCOMPARE(*newData, data);
}

#include <KisResourceTypes.h>

void KisBrushModelTest::testPredefinedBrush()
{
    auto source = KisGlobalResourcesInterface::instance()->source<KisBrush>(ResourceType::Brushes);

    KisBrushSP fallbackBrush = source.fallbackResource();
    QVERIFY(fallbackBrush);

    KisBrushModel::BrushData data;

    data.type = KisBrushModel::Predefined;
    data.common.angle = 3;
    data.common.spacing = 0.5;
    data.common.autoSpacingCoeff = 1.1;
    data.common.useAutoSpacing = true;

    data.predefinedBrush.resourceSignature = fallbackBrush->signature();
    data.predefinedBrush.subtype = "png_brush";
    data.predefinedBrush.brushType = fallbackBrush->brushType();
    data.predefinedBrush.baseSize = { fallbackBrush->width(), fallbackBrush->height()};
    data.predefinedBrush.scale = 0.7;
    data.predefinedBrush.application = LIGHTNESSMAP;
    data.predefinedBrush.hasColorAndTransparency = true;
    data.predefinedBrush.autoAdjustMidPoint = true;
    data.predefinedBrush.adjustmentMidPoint = 120;
    data.predefinedBrush.brightnessAdjustment = 0.3;
    data.predefinedBrush.contrastAdjustment = 0.2;

    QCOMPARE(data, data);

    KisPropertiesConfiguration config;

    data.write(&config);

    std::optional<KisBrushModel::BrushData> newData =
        KisBrushModel::BrushData::read(&config, KisGlobalResourcesInterface::instance());

    QVERIFY(newData);
    QCOMPARE(*newData, data);
}

void KisBrushModelTest::testTextBrush()
{
    KisBrushModel::BrushData data;

    data.type = KisBrushModel::Text;
    data.common.spacing = 0.5;

    // FIXME: angle and auto-spacing are not saved yet!
    // data.common.angle = 3;
    // data.common.autoSpacingCoeff = 1.1;
    // data.common.useAutoSpacing = true;

    // TODO: implement calculation of the size from the font
    // data.textBrush.baseSize = {10, 11};

    data.textBrush.font = "Font String";

    // FIXME: scale is not saved yet!
    //data.textBrush.scale = 0.3;

    data.textBrush.text = "Test text";
    data.textBrush.usePipeMode = true;

    QCOMPARE(data, data);

    KisPropertiesConfiguration config;

    data.write(&config);

    config.dump();

    std::optional<KisBrushModel::BrushData> newData =
        KisBrushModel::BrushData::read(&config, KisGlobalResourcesInterface::instance());

    QVERIFY(newData);
    QCOMPARE(*newData, data);
}

KISTEST_MAIN(KisBrushModelTest);
