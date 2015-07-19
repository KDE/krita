/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_image_test.h"
#include <QApplication>

#include <qtest_kde.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_selection.h"
#include <kis_debug.h>
#include <kis_layer_composition.h>

#define IMAGE_WIDTH 128
#define IMAGE_HEIGHT 128

void KisImageTest::layerTests()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    image->addNode(layer);

    QVERIFY(image->rootLayer()->firstChild()->objectName() == layer->objectName());
}

void KisImageTest::testConvertImageColorSpace()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    KisPaintDeviceSP device1 = new KisPaintDevice(cs8);
    KisLayerSP paint1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfiguration *configuration = filter->defaultConfiguration(0);
    Q_ASSERT(configuration);

    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);

    image->addNode(paint1, image->root());
    image->addNode(blur1, image->root());

    image->refreshGraph();

    const KoColorSpace *cs16 = KoColorSpaceRegistry::instance()->rgb16();
    image->lock();
    image->convertImageColorSpace(cs16,
                                  KoColorConversionTransformation::InternalRenderingIntent,
                                  KoColorConversionTransformation::InternalConversionFlags);
    image->unlock();

    QVERIFY(*cs16 == *image->colorSpace());
    QVERIFY(*cs16 == *image->root()->colorSpace());
    QVERIFY(*cs16 == *paint1->colorSpace());
    QVERIFY(*cs16 == *blur1->colorSpace());

    QVERIFY(!image->root()->compositeOp());
    QVERIFY(*cs16 == *paint1->compositeOp()->colorSpace());
    QVERIFY(*cs16 == *blur1->compositeOp()->colorSpace());

    image->refreshGraph();
}

void KisImageTest::testGlobalSelection()
{
    const KoColorSpace *cs8 = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 1000, 1000, cs8, "stest");

    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 0U);

    KisSelectionSP selection1 = new KisSelection(new KisDefaultBounds(image));
    KisSelectionSP selection2 = new KisSelection(new KisDefaultBounds(image));

    image->setGlobalSelection(selection1);
    QCOMPARE(image->globalSelection(), selection1);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->setGlobalSelection(selection2);
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    image->deselectGlobalSelection();
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->reselectGlobalSelection();
    QCOMPARE(image->globalSelection(), selection2);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);

    // mixed deselecting/setting/reselecting

    image->deselectGlobalSelection();
    QCOMPARE(image->globalSelection(), KisSelectionSP(0));
    QCOMPARE(image->canReselectGlobalSelection(), true);
    QCOMPARE(image->root()->childCount(), 0U);

    image->setGlobalSelection(selection1);
    QCOMPARE(image->globalSelection(), selection1);
    QCOMPARE(image->canReselectGlobalSelection(), false);
    QCOMPARE(image->root()->childCount(), 1U);
}

void KisImageTest::testLayerComposition()
{
    KisImageSP image = new KisImage(0, IMAGE_WIDTH, IMAGE_WIDTH, 0, "layer tests");
    QVERIFY(image->rootLayer() != 0);
    QVERIFY(image->rootLayer()->firstChild() == 0);

    KisLayerSP layer = new KisPaintLayer(image, "layer 1", OPACITY_OPAQUE_U8);
    image->addNode(layer);
    KisLayerSP layer2 = new KisPaintLayer(image, "layer 2", OPACITY_OPAQUE_U8);
    image->addNode(layer2);

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    KisLayerComposition comp(image, "comp 1");
    comp.store();

    layer2->setVisible(false);

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());

    KisLayerComposition comp2(image, "comp 2");
    comp2.store();

    comp.apply();

    QVERIFY(layer->visible());
    QVERIFY(layer2->visible());

    comp2.apply();

    QVERIFY(layer->visible());
    QVERIFY(!layer2->visible());
}

#include "testutil.h"
#include "kis_group_layer.h"
#include "kis_transparency_mask.h"
#include "kis_psd_layer_style.h"

struct FlattenTestImage
{
    FlattenTestImage()
        : refRect(0,0,512,512)
        , p(refRect)
    {

        image = p.image;

        layer1 = p.layer;

        layer5 = new KisPaintLayer(p.image, "paint5", 0.4 * OPACITY_OPAQUE_U8);
        layer5->disableAlphaChannel(true);

        layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
        tmask = new KisTransparencyMask();

        // check channel flags
        // make addition composite op
        group1 = new KisGroupLayer(p.image, "group1", OPACITY_OPAQUE_U8);
        layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8);
        layer4 = new KisPaintLayer(p.image, "paint4", OPACITY_OPAQUE_U8);

        layer6 = new KisPaintLayer(p.image, "paint6", OPACITY_OPAQUE_U8);

        layer7 = new KisPaintLayer(p.image, "paint7", OPACITY_OPAQUE_U8);
        layer8 = new KisPaintLayer(p.image, "paint8", OPACITY_OPAQUE_U8);
        layer7->setCompositeOp(COMPOSITE_ADD);
        layer8->setCompositeOp(COMPOSITE_ADD);

        QRect rect1(100, 100, 100, 100);
        QRect rect2(150, 150, 150, 150);
        QRect tmaskRect(200,200,100,100);

        QRect rect3(400, 100, 100, 100);
        QRect rect4(500, 100, 100, 100);

        QRect rect5(50, 50, 100, 100);

        QRect rect6(50, 250, 100, 100);

        QRect rect7(50, 350, 50, 50);
        QRect rect8(50, 400, 50, 50);

        layer1->paintDevice()->fill(rect1, KoColor(Qt::red, p.image->colorSpace()));

        layer2->paintDevice()->fill(rect2, KoColor(Qt::green, p.image->colorSpace()));
        tmask->testingInitSelection(tmaskRect);

        layer3->paintDevice()->fill(rect3, KoColor(Qt::blue, p.image->colorSpace()));
        layer4->paintDevice()->fill(rect4, KoColor(Qt::yellow, p.image->colorSpace()));
        layer5->paintDevice()->fill(rect5, KoColor(Qt::green, p.image->colorSpace()));

        layer6->paintDevice()->fill(rect6, KoColor(Qt::cyan, p.image->colorSpace()));

        layer7->paintDevice()->fill(rect7, KoColor(Qt::red, p.image->colorSpace()));
        layer8->paintDevice()->fill(rect8, KoColor(Qt::green, p.image->colorSpace()));

        KisPSDLayerStyleSP style(new KisPSDLayerStyle());
        style->dropShadow()->setEffectEnabled(true);
        style->dropShadow()->setDistance(10.0);
        style->dropShadow()->setSpread(80.0);
        style->dropShadow()->setSize(10);
        style->dropShadow()->setNoise(0);
        style->dropShadow()->setKnocksOut(false);
        style->dropShadow()->setOpacity(80.0);
        layer2->setLayerStyle(style);

        layer2->setCompositeOp(COMPOSITE_ADD);
        group1->setCompositeOp(COMPOSITE_ADD);

        p.image->addNode(layer5);

        p.image->addNode(layer2);
        p.image->addNode(tmask, layer2);

        p.image->addNode(group1);
        p.image->addNode(layer3, group1);
        p.image->addNode(layer4, group1);

        p.image->addNode(layer6);

        p.image->addNode(layer7);
        p.image->addNode(layer8);

        p.image->initialRefreshGraph();

        // qDebug() << ppVar(layer1->exactBounds());
        // qDebug() << ppVar(layer5->exactBounds());
        // qDebug() << ppVar(layer2->exactBounds());
        // qDebug() << ppVar(group1->exactBounds());
        // qDebug() << ppVar(layer3->exactBounds());
        // qDebug() << ppVar(layer4->exactBounds());

        TestUtil::ExternalImageChecker chk("flatten", "imagetest");
        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
    }

    QRect refRect;
    TestUtil::MaskParent p;

    KisImageSP image;
    KisPaintLayerSP layer1;

    KisPaintLayerSP layer2;
    KisTransparencyMaskSP tmask;

    KisGroupLayerSP group1;
    KisPaintLayerSP layer3;
    KisPaintLayerSP layer4;

    KisPaintLayerSP layer5;

    KisPaintLayerSP layer6;

    KisPaintLayerSP layer7;
    KisPaintLayerSP layer8;
};

void KisImageTest::testFlattenLayer()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker chk("flatten", "imagetest");

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);

        KisLayerSP newLayer = p.image->flattenLayer(p.layer2);
        p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
    }

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);

        KisLayerSP newLayer = p.image->flattenLayer(p.group1);
        p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "02_group1_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(newLayer->exactBounds(), QRect(400, 100, 200, 100));
    }

    {
        QCOMPARE(p.layer5->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer5->alphaChannelDisabled(), true);

        KisLayerSP newLayer = p.image->flattenLayer(p.layer5);
        p.image->waitForDone();

        QVERIFY(chk.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "03_layer5_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 50, 100, 100));

        QCOMPARE(newLayer->alphaChannelDisabled(), true);
    }
}

#include <metadata/kis_meta_data_merge_strategy_registry.h>

void KisImageTest::testMergeDown()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergedown_simple", "imagetest");


    {
        QCOMPARE(p.layer5->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer5->alphaChannelDisabled(), true);

        KisLayerSP newLayer = p.image->mergeDown(p.layer5, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer5_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.layer2, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "02_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(100, 100, 213, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.group1, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "03_group1_mergedown_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(100, 100, 500, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationInheritsAlpha()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergedown_dst_inheritsalpha", "imagetest");

    {
        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.layer2, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer2_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50,50, 263, 267));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationCustomCompositeOp()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergedown_dst_customop", "imagetest");

    {
        QCOMPARE(p.layer6->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(p.layer6->alphaChannelDisabled(), false);

        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.layer6, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer6_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 100, 550, 250));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationSameCompositeOpLayerStyle()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergedown_sameop_ls", "imagetest");

    {
        QCOMPARE(p.group1->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.group1->alphaChannelDisabled(), false);

        QCOMPARE(p.layer2->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer2->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.group1, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_group1_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(197, 100, 403, 217));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeDownDestinationSameCompositeOp()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergedown_sameop_fastpath", "imagetest");

    {
        QCOMPARE(p.layer8->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer8->alphaChannelDisabled(), false);

        QCOMPARE(p.layer7->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(p.layer7->alphaChannelDisabled(), false);

        KisLayerSP newLayer = p.image->mergeDown(p.layer8, KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer8_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_ADD);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 350, 50, 100));
        QCOMPARE(newLayer->alphaChannelDisabled(), false);
    }
}

void KisImageTest::testMergeMultiple()
{
    FlattenTestImage p;

    TestUtil::ExternalImageChecker img("flatten", "imagetest");
    TestUtil::ExternalImageChecker chk("mergemultiple", "imagetest");

    QList<KisNodeSP> selectedNodes;

    selectedNodes << p.layer2
                  << p.group1
                  << p.layer6;

    {
        KisNodeSP newLayer = p.image->mergeMultipleLayers(selectedNodes, 0);
        p.image->waitForDone();

        QVERIFY(img.checkDevice(p.image->projection(), p.image, "00_initial"));
        QVERIFY(chk.checkDevice(newLayer->projection(), p.image, "01_layer8_layerproj"));

        QCOMPARE(newLayer->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(newLayer->exactBounds(), QRect(50, 100, 550, 250));
    }
}

QTEST_KDEMAIN(KisImageTest, NoGUI)
#include "kis_image_test.moc"
