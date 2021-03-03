/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_async_merger_test.h"

#include "kis_merge_walker.h"
#include "kis_full_refresh_walker.h"
#include "kis_async_merger.h"

#include <simpletest.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_filter_mask.h"
#include "kis_selection.h"
#include "kis_paint_device_debug_utils.h"
#include <KisGlobalResourcesInterface.h>

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"

#include "../../sdk/tests/testutil.h"

#include "kis_image_config.h"
#include "KisImageConfigNotifier.h"

void KisAsyncMergerTest::init()
{
    KisImageConfig::resetConfig();
}


    /*
      +-----------+
      |root       |
      | group     |
      |  blur 1   |
      |  paint 2  |
      | paint 1   |
      +-----------+
     */

void KisAsyncMergerTest::testMerger()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 640, 441, colorSpace, "merger test");

    QImage sourceImage1(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage sourceImage2(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    QImage referenceProjection(QString(FILES_DATA_DIR) + '/' + "merged_hakonepa.png");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device1->convertFromQImage(sourceImage1, 0, 0, 0);
    device2->convertFromQImage(sourceImage2, 0, 0, 0);

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(configuration);

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);
    KisLayerSP groupLayer = new KisGroupLayer(image, "group", 200/*OPACITY_OPAQUE*/);
    KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration->cloneWithResourcesSnapshot(), 0);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());

    image->addNode(paintLayer2, groupLayer);
    image->addNode(blur1, groupLayer);

    QRect testRect1(0,0,100,441);
    QRect testRect2(100,0,400,441);
    QRect testRect3(500,0,140,441);
    QRect testRect4(580,381,40,40);

    QRect cropRect(image->bounds());

    KisMergeWalker walker(cropRect);
    KisAsyncMerger merger;

    walker.collectRects(paintLayer2, testRect1);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect2);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect3);
    merger.startMerge(walker);

    walker.collectRects(paintLayer2, testRect4);
    merger.startMerge(walker);

    // Old style merging: has artifacts at x=100 and x=500
    // And should be turned on inside KisLayer
/*    paintLayer2->setDirty(testRect1);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect2);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect3);
    QTest::qSleep(3000);
    paintLayer2->setDirty(testRect4);
    QTest::qSleep(3000);
*/

    KisLayerSP rootLayer = image->rootLayer();
    QVERIFY(rootLayer->exactBounds() == image->bounds());

    QImage resultProjection = rootLayer->projection()->convertToQImage(0);
    resultProjection.save(QString(FILES_OUTPUT_DIR) + '/' + "actual_merge_result.png");
    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, resultProjection, referenceProjection, 5, 0, 0));
}


/**
 * This in not fully automated test for child obliging in KisAsyncMerger.
 * It just checks whether devices are shared. To check if the merger
 * touches originals you can add a debug message to the merger
 * and take a look.
 */

    /*
      +-----------+
      |root       |
      | group     |
      |  paint 1  |
      +-----------+
     */

void KisAsyncMergerTest::debugObligeChild()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 640, 441, colorSpace, "merger test");

    QImage sourceImage1(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->convertFromQImage(sourceImage1, 0, 0, 0);

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);

    image->addNode(groupLayer, image->rootLayer());
    image->addNode(paintLayer1, groupLayer);

    QRect testRect1(0,0,640,441);
    QRect cropRect(image->bounds());

    KisMergeWalker walker(cropRect);
    KisAsyncMerger merger;

    walker.collectRects(paintLayer1, testRect1);
    merger.startMerge(walker);

    KisLayerSP rootLayer = image->rootLayer();
    QVERIFY(rootLayer->original() == groupLayer->projection());
    QVERIFY(groupLayer->original() == paintLayer1->projection());
}

    /*
      +--------------+
      |root          |
      | paint 1      |
      |  invert_mask |
      | clone_of_1   |
      +--------------+
     */

void KisAsyncMergerTest::testFullRefreshWithClones()
{
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(image->bounds(), KoColor( Qt::white, colorSpace));

    KisFilterSP filter = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(configuration);

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    KisFilterMaskSP invertMask1 = new KisFilterMask(image, "invert_mask");
    invertMask1->initSelection(paintLayer1);
    invertMask1->setFilter(configuration->cloneWithResourcesSnapshot());

    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer1, image, "clone_of_1", OPACITY_OPAQUE_U8);
    /**
     * The clone layer must have a projection to allow us
     * to read what it got from its source. Just shift it.
     */
    cloneLayer1->setX(10);
    cloneLayer1->setY(10);

    image->addNode(cloneLayer1, image->rootLayer());
    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(invertMask1, paintLayer1);

    QRect cropRect(image->bounds());

    KisFullRefreshWalker walker(cropRect);
    KisAsyncMerger merger;

    walker.collectRects(image->rootLayer(), image->bounds());
    merger.startMerge(walker);

    // Wait for additional jobs generated by the clone are finished
    image->waitForDone();

    QRect filledRect(10, 10,
                     image->width() - cloneLayer1->x(),
                     image->height() - cloneLayer1->y());

    const int pixelSize = device1->pixelSize();
    const int numPixels = filledRect.width() * filledRect.height();

    QByteArray bytes(numPixels * pixelSize, 13);
    cloneLayer1->projection()->readBytes((quint8*)bytes.data(), filledRect);

    KoColor desiredPixel(Qt::black, colorSpace);
    quint8 *srcPtr = (quint8*)bytes.data();
    quint8 *dstPtr = desiredPixel.data();
    for(int i = 0; i < numPixels; i++) {
        if(memcmp(srcPtr, dstPtr, pixelSize)) {
            dbgKrita << "expected:" << dstPtr[0] << dstPtr[1] << dstPtr[2] << dstPtr[3];
            dbgKrita << "result:  " << srcPtr[0] << srcPtr[1] << srcPtr[2] << srcPtr[3];
            QFAIL("Failed to compare pixels");
        }
        srcPtr += pixelSize;
    }
}

    /*
      +--------------+
      |root          |
      | paint 2      |
      | paint 1      |
      +--------------+
     */

void KisAsyncMergerTest::testSubgraphingWithoutUpdatingParent()
{
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(image->bounds(), KoColor(Qt::white, colorSpace));
    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device2->fill(image->bounds(), KoColor(Qt::black, colorSpace));
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", 128, device2);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    image->initialRefreshGraph();

    QImage refImage(QString(FILES_DATA_DIR) + '/' + "subgraphing_without_updating.png");

    {
        QImage resultImage = image->projection()->convertToQImage(0);
        QCOMPARE(resultImage, refImage);
    }

    QRect cropRect(image->bounds());

    KisRefreshSubtreeWalker walker(cropRect);
    KisAsyncMerger merger;

    walker.collectRects(paintLayer2, image->bounds());
    merger.startMerge(walker);

    {
        QImage resultImage = image->projection()->convertToQImage(0);
        QCOMPARE(resultImage, refImage);
    }
}


#include <KoCompositeOpRegistry.h>

enum DependentNodeType {
    GROUP_LAYER,
    ADJUSTMENT_LAYER
};

/*
  +--------------+
  |root          |
  | (blur1)      |
  | group        |
  |  blur_mask   |
  |  paint 2     |
  |  paint 3     |
  | paint 1      |
  +--------------+
 */
void testFullRefreshForDependentNodes(const DependentNodeType dependentNode,
                                      const bool useFilterMask,
                                      const bool useLayerStyle)
{
    const QRect imageRect = QRect(0, 0, 128, 128);
    const QRect fillRect = QRect(32, 32, 64, 64);
    const QRect smallRect = QRect(60, 60, 8, 8);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, imageRect.width(), imageRect.height(), colorSpace, "blur test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);

    if (dependentNode == GROUP_LAYER && !useLayerStyle) {
        device1->fill(imageRect, KoColor(Qt::black, colorSpace));
    }

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);

    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device2->fill(fillRect, KoColor(Qt::white, colorSpace));
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);

    device2->fill(QRect(63, 0, 1, 64), KoColor(Qt::green, colorSpace));

    KisPaintDeviceSP device3 = new KisPaintDevice(colorSpace);
    device3->fill(smallRect, KoColor(Qt::red, colorSpace));
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8, device3);

    KisLayerSP groupLayer = new KisGroupLayer(image, "group", OPACITY_OPAQUE_U8);


    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(groupLayer, image->rootLayer());
    image->addNode(paintLayer2, groupLayer);
    image->addNode(paintLayer3, groupLayer);

    KisLayerSP testingLayer = groupLayer;

    if (dependentNode == ADJUSTMENT_LAYER) {
        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        KIS_ASSERT(filter);
        KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
        KIS_ASSERT(configuration);

        KisLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration->cloneWithResourcesSnapshot(), 0);
        blur1->setCompositeOpId(COMPOSITE_OVER);

        image->addNode(blur1, image->rootLayer());
    }

    if (useFilterMask) {
        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        KIS_ASSERT(filter);
        KisFilterConfigurationSP configuration = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
        KIS_ASSERT(configuration);

        KisFilterMaskSP blurMask1 = new KisFilterMask(image, "blur_mask");
        blurMask1->initSelection(groupLayer);
        blurMask1->setFilter(configuration->cloneWithResourcesSnapshot());
        image->addNode(blurMask1, testingLayer);
    }

    if (useLayerStyle) {
        KisPSDLayerStyleSP style(new KisPSDLayerStyle());

        style->context()->keep_original = true;
        style->dropShadow()->setEffectEnabled(true);
        style->dropShadow()->setDistance(9);
        style->dropShadow()->setSpread(100);
        style->dropShadow()->setSize(9);
        style->dropShadow()->setNoise(0);
        style->dropShadow()->setKnocksOut(false);
        style->dropShadow()->setOpacity(90);
        style->dropShadow()->setAngle(0);

        testingLayer->setLayerStyle(style);
    }


    QRect cropRect(image->bounds());

    KisFullRefreshWalker walker(cropRect);
    KisAsyncMerger merger;


    QVector<QRect> patchRects;
    patchRects << QRect(0,0,64,64);
    patchRects << QRect(64,0,64,64);
    patchRects << QRect(0,64,64,64);
    patchRects << QRect(64,64,64,64);

    Q_FOREACH(const QRect &rc, patchRects) {
        walker.collectRects(image->rootLayer(), rc);
        merger.startMerge(walker);
    }

    // Wait for additional jobs generated by the clone are finished
    image->waitForDone();

    QString testName = dependentNode == GROUP_LAYER ? "group" : "adj";

    if (useFilterMask) {
        testName += "_mask";
    }

    if (useLayerStyle) {
        testName += "_style";
    }

    TestUtil::checkQImage(image->projection()->convertToQImage(0, imageRect),
                          "async_merger_test", "dependent", testName, 3);
}

void KisAsyncMergerTest::testFullRefreshGroupWithMask()
{
    testFullRefreshForDependentNodes(GROUP_LAYER, true, false);
}

void KisAsyncMergerTest::testFullRefreshGroupWithStyle()
{
    testFullRefreshForDependentNodes(GROUP_LAYER, false, true);
}

void KisAsyncMergerTest::testFullRefreshGroupWithMaskAndStyle()
{
    testFullRefreshForDependentNodes(GROUP_LAYER, true, true);
}

void KisAsyncMergerTest::testFullRefreshAdjustmentWithMask()
{
    testFullRefreshForDependentNodes(ADJUSTMENT_LAYER, true, false);
}

void KisAsyncMergerTest::testFullRefreshAdjustmentWithStyle()
{
    testFullRefreshForDependentNodes(ADJUSTMENT_LAYER, false, true);
}

/*
  +---------------------------------+
  |root                             |
  | adj 2 (filter 2, color balance) |
  |  mask 3 (filter 3, blur)        |
  | paint 1                         |
  +---------------------------------+
 */

void KisAsyncMergerTest::testFilterMaskOnFilterLayer()
{
    {
        KisImageConfig cfg(false);
        cfg.setUpdatePatchWidth(64);
        cfg.setUpdatePatchHeight(64);
        cfg.setMaxNumberOfThreads(1);
    }
    KisImageConfigNotifier::instance()->notifyConfigChanged();


    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "masks test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(image->bounds(), KoColor(Qt::yellow, colorSpace));
    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    image->addNode(paintLayer1, image->rootLayer());


    KisFilterSP filter2 = KisFilterRegistry::instance()->value("colorbalance");
    KIS_ASSERT(filter2);
    KisFilterConfigurationSP configuration2 = filter2->defaultConfiguration(KisGlobalResourcesInterface::instance());
    KIS_ASSERT(configuration2);
    KisLayerSP adjLayer2 = new KisAdjustmentLayer(image, "adj2", configuration2->cloneWithResourcesSnapshot(), 0);
    image->addNode(adjLayer2, image->rootLayer());


    KisFilterSP filter3 = KisFilterRegistry::instance()->value("blur");
    KIS_ASSERT(filter3);
    KisFilterConfigurationSP configuration3 = filter3->defaultConfiguration(KisGlobalResourcesInterface::instance());
    KIS_ASSERT(configuration3);
    KisFilterMaskSP mask3 = new KisFilterMask(image, "mask3");
    mask3->initSelection(adjLayer2);
    mask3->setFilter(configuration3->cloneWithResourcesSnapshot());
    image->addNode(mask3, adjLayer2);

    image->initialRefreshGraph();

    QVERIFY(TestUtil::checkQImage(image->projection()->convertToQImage(0),
                                  "async_merger_test", "mask_on_adj", "initial", 3));
}


SIMPLE_TEST_MAIN(KisAsyncMergerTest)

