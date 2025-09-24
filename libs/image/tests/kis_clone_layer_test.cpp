/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_clone_layer_test.h"

#include <simpletest.h>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_image.h"

#include "kistest.h"
#include "testutil.h"

// for updates on visibility change tests
#include "kis_layer_properties_icons.h"


void KisCloneLayerTest::testCreation()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    KisLayerSP layer = new KisPaintLayer(image, "clone test", OPACITY_OPAQUE_U8);

    KisCloneLayer test(layer, image, "clonetest", OPACITY_OPAQUE_U8);
}

KisImageSP createImage()
{
    /*
      +-----------------+
      |root             |
      | group 1 <-----+ |
      |  paint 3      | |
      |  paint 1      | |
      | group 2       | |
      |  clone_of_g1 -+ |
      |  paint 2        |
      +-----------------+
     */

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    QRect fillRect(10,10,100,100);
    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(fillRect, KoColor( Qt::white, colorSpace));

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    KisLayerSP groupLayer1 = new KisGroupLayer(image, "group1", OPACITY_OPAQUE_U8);

    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);

    KisPaintDeviceSP device3 = new KisPaintDevice(colorSpace);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "paint3", OPACITY_OPAQUE_U8, device3);

    KisLayerSP cloneLayer1 = new KisCloneLayer(groupLayer1, image, "clone_of_g1", OPACITY_OPAQUE_U8);
    cloneLayer1->setX(10);
    cloneLayer1->setY(10);

    KisLayerSP groupLayer2 = new KisGroupLayer(image, "group2", OPACITY_OPAQUE_U8);

    image->addNode(groupLayer2, image->rootLayer());
    image->addNode(groupLayer1, image->rootLayer());

    image->addNode(paintLayer2, groupLayer2);
    image->addNode(cloneLayer1, groupLayer2);
    image->addNode(paintLayer1, groupLayer1);
    image->addNode(paintLayer3, groupLayer1);

    return image;
}

KisNodeSP groupLayer1(KisImageSP image) {
    KisNodeSP node = image->root()->lastChild();
    Q_ASSERT(node->name() == "group1");
    return node;
}

KisNodeSP paintLayer1(KisImageSP image) {
    KisNodeSP node = groupLayer1(image)->firstChild();
    Q_ASSERT(node->name() == "paint1");
    return node;
}

void KisCloneLayerTest::testOriginalUpdates()
{
    const QRect nullRect(QPoint(0, 0), QPoint(-1, -1));
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    QCOMPARE(root->projection()->exactBounds(), nullRect);

    paintLayer1(image)->setDirty(image->bounds());
    image->waitForDone();

    const QRect expectedRect(10,10,110,110);
    QCOMPARE(root->projection()->exactBounds(), expectedRect);
}

void KisCloneLayerTest::testOriginalUpdatesOutOfBounds()
{
    const QRect nullRect(QPoint(0, 0), QPoint(-1, -1));
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    QCOMPARE(root->projection()->exactBounds(), nullRect);

    QRect fillRect(-10,-10,10,10);
    paintLayer1(image)->paintDevice()->fill(fillRect, KoColor(Qt::white, image->colorSpace()));
    paintLayer1(image)->setDirty(fillRect);
    image->waitForDone();

    const QRect expectedRect(0,0,10,10);
    QCOMPARE(root->projection()->exactBounds(), expectedRect);

    QCOMPARE(groupLayer1(image)->projection()->exactBounds(), fillRect);
}

void KisCloneLayerTest::testOriginalRefresh()
{
    const QRect nullRect(QPoint(0, 0), QPoint(-1, -1));
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    QCOMPARE(root->projection()->exactBounds(), nullRect);

    image->refreshGraphAsync();
    image->waitForDone();

    const QRect expectedRect(10,10,110,110);
    QCOMPARE(root->projection()->exactBounds(), expectedRect);
}

#include "commands/kis_image_commands.h"

void KisCloneLayerTest::testRemoveSourceLayer()
{
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    KisNodeSP group1 = groupLayer1(image);

    /**
     * We are checking that no one keeps a pointer to the
     * source layer after it is deleted. Uncomment the first
     * line to see how perfectly it crashed if removing the
     * source directly through the node facade
     */
    //image->removeNode(group1);

    KUndo2Command *cmd = new KisImageLayerRemoveCommand(image, group1);
    cmd->redo();
    delete cmd;

    // We are veeeery bad! Never do like this! >:)
    dbgKrita << "Ref. count:" << group1->refCount();
    KisNodeWSP group1_wsp = group1;
    KisNode *group1_ptr = group1.data();
    group1 = 0;
    if(group1_wsp.isValid()) {
        group1_wsp = 0;
        while(group1_ptr->refCount()) group1_ptr->deref();
        delete group1_ptr;
    }

    // Are we crashing?
    image->refreshGraphAsync();
    image->waitForDone();
}

void KisCloneLayerTest::testRemoveSourceLayerParent()
{
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    KisNodeSP group1 = TestUtil::findNode(root, "group1");
    KisNodeSP group2 = TestUtil::findNode(root, "group2");
    KisNodeSP cloneLayer1 = TestUtil::findNode(root, "clone_of_g1");


    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP cloneLayer4 = new KisCloneLayer(paintLayer4, image, "clone_of_p4", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer4, group1);
    image->addNode(cloneLayer4, group2);

    Q_ASSERT(group1->lastChild() == KisNodeSP(paintLayer4));
    Q_ASSERT(group2->lastChild() == KisNodeSP(cloneLayer4));
    Q_ASSERT(TestUtil::findNode(group2, "clone_of_g1") == KisNodeSP(cloneLayer1));
    Q_ASSERT(TestUtil::findNode(group2, "clone_of_p4") == KisNodeSP(cloneLayer4));

    /**
     * This test checks if the node reincarnates when the parent of
     * the clone's source layer is removed.
     *
     * Here both group1 and paint4 are removed.
     */
    KUndo2Command *cmd = new KisImageLayerRemoveCommand(image, group1);
    cmd->redo();
    delete cmd;

    KisNodeSP newCloneLayer1 = TestUtil::findNode(group2, "clone_of_g1");
    KisNodeSP newCloneLayer4 = TestUtil::findNode(group2, "clone_of_p4");

    QVERIFY(newCloneLayer1 != KisNodeSP(cloneLayer1));
    QVERIFY(dynamic_cast<KisPaintLayer*>(newCloneLayer1.data()));

    QVERIFY(newCloneLayer4 != KisNodeSP(cloneLayer4));
    QVERIFY(dynamic_cast<KisPaintLayer*>(newCloneLayer4.data()));
}

void KisCloneLayerTest::testUndoingRemovingSource()
{
    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer1, image, "clone_of_p1", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1);
    image->addNode(cloneLayer1);

    QCOMPARE(image->root()->lastChild(), KisNodeSP(cloneLayer1));
    QCOMPARE(image->root()->lastChild()->name(), QString("clone_of_p1"));
    QCOMPARE(image->root()->firstChild()->name(), QString("paint1"));

    QScopedPointer<KUndo2Command> cmd1(new KisImageLayerRemoveCommand(image, paintLayer1));

    image->barrierLock();
    cmd1->redo();
    image->unlock();

    image->waitForDone();

    QCOMPARE(image->root()->lastChild()->name(), QString("clone_of_p1"));
    QVERIFY(image->root()->lastChild() != KisNodeSP(cloneLayer1));

    KisNodeSP reincarnatedLayer = image->root()->lastChild();

    QScopedPointer<KUndo2Command> cmd2(new KisImageLayerRemoveCommand(image, reincarnatedLayer));

    image->barrierLock();
    cmd2->redo();
    image->unlock();

    image->barrierLock();
    cmd2->undo();
    image->unlock();

    image->barrierLock();
    cmd1->undo();
    image->unlock();

    image->barrierLock();
    cmd1->redo();
    image->unlock();

    image->waitForDone();

    QCOMPARE(image->root()->lastChild()->name(), QString("clone_of_p1"));
    QCOMPARE(image->root()->lastChild(), reincarnatedLayer);
    QVERIFY(image->root()->lastChild() != KisNodeSP(cloneLayer1));

    image->barrierLock();
    cmd2->redo();
    image->unlock();
}

void KisCloneLayerTest::testDuplicateGroup()
{
    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    KisNodeSP group1 = groupLayer1(image);

    KisLayerSP paintLayer4 = new KisPaintLayer(image, "paint4", OPACITY_OPAQUE_U8);
    KisLayerSP cloneLayer4 = new KisCloneLayer(paintLayer4, image, "clone_of_p4", OPACITY_OPAQUE_U8);
    KisLayerSP groupLayer4 = new KisGroupLayer(image, "group4", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer4, group1);
    image->addNode(cloneLayer4, groupLayer4);
    image->addNode(groupLayer4, group1);

    Q_ASSERT(group1->lastChild() == KisNodeSP(groupLayer4));
    Q_ASSERT(group1->lastChild()->lastChild() == KisNodeSP(cloneLayer4));
    Q_ASSERT(group1->lastChild()->prevSibling() == KisNodeSP(paintLayer4));

    KisNodeSP copyGroup1 = group1->clone();

    KisNodeSP copyCloneLayer4 = copyGroup1->lastChild()->lastChild();
    KisNodeSP copyGroupLayer4 = copyGroup1->lastChild();
    KisNodeSP copyPaintLayer4 = copyGroup1->lastChild()->prevSibling();

    QCOMPARE(copyPaintLayer4->name(), QString("paint4"));
    QCOMPARE(copyGroupLayer4->name(), QString("group4"));
    QCOMPARE(copyCloneLayer4->name(), QString("clone_of_p4"));

    QVERIFY(copyPaintLayer4 != KisNodeSP(paintLayer4));
    QVERIFY(copyGroupLayer4 != KisNodeSP(groupLayer4));
    QVERIFY(copyCloneLayer4 != KisNodeSP(cloneLayer4));

    KisCloneLayerSP newClone = dynamic_cast<KisCloneLayer*>(copyCloneLayer4.data());

    /**
     * The newly created clone should now point to the *newly created*
     * paint layer, not to the previous one.
     */
    QCOMPARE(KisNodeSP(newClone->copyFrom()), copyPaintLayer4);
}

struct CyclingTester {
    CyclingTester() {
        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        image = new KisImage(0, 128, 128, colorSpace, "clones test");

        groupLayer1 = new KisGroupLayer(image, "group1", OPACITY_OPAQUE_U8);
        groupLayer2 = new KisGroupLayer(image, "group2", OPACITY_OPAQUE_U8);

        cloneOfGroup1 = new KisCloneLayer(groupLayer1, image, "clone_of_g1", OPACITY_OPAQUE_U8);
        cloneOfGroup2 = new KisCloneLayer(groupLayer2, image, "clone_of_g2", OPACITY_OPAQUE_U8);
    }

    void reset() {
        image->removeNode(groupLayer1);
        image->removeNode(groupLayer2);
        image->removeNode(cloneOfGroup1);
        image->removeNode(cloneOfGroup2);
    }

    KisImageSP image;
    KisLayerSP groupLayer1;
    KisLayerSP groupLayer2;
    KisLayerSP cloneOfGroup1;
    KisLayerSP cloneOfGroup2;
};

inline void addIfNotAnyOf(KisNodeSP node, CyclingTester &t, KisNodeSP group1Child, KisNodeSP group2Child)
{
    if(node != group1Child && node != group2Child) {
        t.image->addNode(node);
    }
}

/**
 * group1 <-- adding \p group1Child here
 * group2  -- has \p group2Child before addition
 */
inline void testCyclingCase(CyclingTester &t, KisNodeSP group1Child, KisNodeSP group2Child, bool expected)
{
    addIfNotAnyOf(t.groupLayer2, t, group1Child, group2Child);
    addIfNotAnyOf(t.cloneOfGroup1, t, group1Child, group2Child);
    addIfNotAnyOf(t.cloneOfGroup2, t, group1Child, group2Child);

    t.image->addNode(t.groupLayer1);

    if(group2Child) {
        t.image->addNode(group2Child, t.groupLayer2);
    }

    QCOMPARE(t.groupLayer1->allowAsChild(group1Child), expected);
    t.reset();
}

void KisCloneLayerTest::testCyclingGroupLayer()
{
    CyclingTester t;

    testCyclingCase(t, t.groupLayer2, 0, true);
    testCyclingCase(t, t.groupLayer2, t.cloneOfGroup1, false);

    testCyclingCase(t, t.cloneOfGroup1, 0, false);

    testCyclingCase(t, t.cloneOfGroup2, 0, true);
    testCyclingCase(t, t.cloneOfGroup2, t.cloneOfGroup1, false);
}

void KisCloneLayerTest::testUpdatesWhileHidden_data()
{
    QTest::addColumn<bool>("changeP1InVisibleArea");
    QTest::addColumn<bool>("makeC1Invisible");
    QTest::addColumn<bool>("makeG2Invisible");
    QTest::addColumn<bool>("makeG1Invisible");


    QTest::newRow("in-all-visible")       << true << false << false << false;
    QTest::newRow("in-dst-hidden")        << true << true  << false << false;
    QTest::newRow("in-dst-parent-hidden") << true << false << true  << false;
    QTest::newRow("in-dst-both-hidden")   << true << true  << true  << false;
    QTest::newRow("in-src-hidden")        << true << false << false << true ;

    QTest::newRow("out-all-visible")       << false << false << false << false;
    QTest::newRow("out-dst-hidden")        << false << true  << false << false;
    QTest::newRow("out-dst-parent-hidden") << false << false << true  << false;
    // TODO: add a test for reversing the order of unhiding g2 and c1
    QTest::newRow("out-dst-both-hidden")   << false << true  << true  << false;
    QTest::newRow("out-src-hidden")        << false << false << false << true ;
}


void KisCloneLayerTest::testUpdatesWhileHidden()
{
    /*
      +-----------------+
      |root             |
      | group 1 <-----+ |
      |  paint 3      | |
      |  paint 1      | |
      | group 2       | |
      |  clone_of_g1 -+ |
      |  paint 2        |
      +-----------------+
     */

    KisImageSP image = createImage();
    KisNodeSP root = image->root();

    auto findNode = [root] (const QString &name) {
        KisNodeSP node = KisLayerUtils::findNodeByName(root, name);
        KIS_ASSERT(node);
        return node;
    };

    KisNodeSP paint1 = findNode("paint1");
    KisNodeSP paint2 = findNode("paint2");
    KisNodeSP paint3 = findNode("paint3");
    KisNodeSP clone1 = findNode("clone_of_g1");
    KisNodeSP group1 = findNode("group1");
    KisNodeSP group2 = findNode("group2");

    paint2->paintDevice()->fill(QRect(0, 5, 10, 10), KoColor( Qt::red, image->colorSpace()));
    paint2->paintDevice()->fill(QRect(100, 5, 10, 10), KoColor( Qt::red, image->colorSpace()));
    paint3->paintDevice()->fill(QRect(5, 0, 10, 10), KoColor( Qt::green, image->colorSpace()));
    paint3->paintDevice()->fill(QRect(5, 100, 10, 10), KoColor( Qt::green, image->colorSpace()));

    image->initialRefreshGraph();

    QFETCH(bool, changeP1InVisibleArea);
    QFETCH(bool, makeC1Invisible);
    QFETCH(bool, makeG2Invisible);
    QFETCH(bool, makeG1Invisible);

    const QString testName =
        QString("hidden-%1-%2-%3-%4")
            .arg(changeP1InVisibleArea ? "p1_in" : "p1_out")
            .arg(makeC1Invisible ? "c1_off" : "c1_on")
            .arg(makeG1Invisible ? "g1_off" : "g1_on")
            .arg(makeG2Invisible ? "g2_off" : "g2_on");

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        testName,
        "00_initial"));


    // TODO: verify no updates after the parent group or layer is hidden

    if (makeC1Invisible) {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(clone1, KisLayerPropertiesIcons::visible, false, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "10_c1_hidden"));
    }

    if (makeG2Invisible) {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(group2, KisLayerPropertiesIcons::visible, false, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "15_g2_hidden"));
    }

    if (makeG1Invisible) {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(group1, KisLayerPropertiesIcons::visible, false, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "17_g1_hidden"));
    }

    const QRect p1ChangeArea = changeP1InVisibleArea ?
        QRect(100, 30, 7, 7) : QRect(-10, 30, 7, 7);
    paint1->paintDevice()->fill(p1ChangeArea, KoColor( Qt::blue, image->colorSpace()));

    paint1->setDirty(p1ChangeArea);
    image->waitForDone();

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        testName,
        "20_p1_changed"));

    if (makeG1Invisible) {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(group1, KisLayerPropertiesIcons::visible, true, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "25_g1_revealed"));
    }

    if (makeG2Invisible) {
        KisLayerPropertiesIcons::setNodePropertyAutoUndo(group2, KisLayerPropertiesIcons::visible, true, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "30_g2_revealed"));
    }

    if (makeC1Invisible) {
        /**
         * TODO: We have some issues with updating a layer outside the image bounds
         * when its clone is hidden. When the clone is made visible again, this hidden
         * area is not vidible. To fix this, we need to perform "change-rect-based"-
         * update instead of "dirty-rect-based"-update. In the current terms, it means
         * that we should do refreshGraphAsync() on any visibility change. But our system
         * is not fully designed to incorporate that. We usually keep the content of the
         * group layers updated, even when the group itself is hidden. So we need to
         * do a significant design change for that.
         */

        QEXPECT_FAIL("out-dst-both-hidden", "Will fix in the next release", Continue);
        QEXPECT_FAIL("out-dst-hidden", "Will fix in the next release", Continue);

        KisLayerPropertiesIcons::setNodePropertyAutoUndo(clone1, KisLayerPropertiesIcons::visible, true, image);
        image->waitForDone();
        QVERIFY(TestUtil::checkQImage(
            image->projection()->convertToQImage(nullptr, image->bounds()),
            "clone_layer_test",
            testName,
            "40_c1_revealed"));
    }
}

#include <kis_transform_mask.h>
#include <KritaTransformMaskStubs.h>
#include <KisDumbTransformMaskParams.h>
#include <KisTransformMaskTestingListener.h>

void KisCloneLayerTest::initTestCase()
{
    TestUtil::registerTransformMaskStubs();
}

void KisCloneLayerTest::testWithSourceUnderTransformMask()
{
    /*
      +-----------------+
      |root             |
      |  clone_of_p1 -+ |
      |  paint 2      | |
      |  paint 1    <-+ |
      |    transf1      |
      +-----------------+
     */

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(QRect(10,10,100,100), KoColor( Qt::white, colorSpace));
    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    image->addNode(paintLayer1, image->root());

    KisTransformMaskTestingListener *listener = new KisTransformMaskTestingListener();

    QTransform transform = QTransform::fromScale(0.1, 0.2);
    KisTransformMaskSP transf1 = new KisTransformMask(image, "transf1");
    image->addNode(transf1, paintLayer1);
    transf1->setTransformParams(KisTransformMaskParamsInterfaceSP(
        new KisDumbTransformMaskParams(transform)));
    transf1->setTestingInterface(listener);

    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device2->fill(QRect(20, 20, 10, 10), KoColor( Qt::red, colorSpace));
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);
    image->addNode(paintLayer2, image->root());

    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer1, image, "clone_of_p1", OPACITY_OPAQUE_U8);
    cloneLayer1->setX(10);
    cloneLayer1->setY(10);
    image->addNode(cloneLayer1, image->root());

    image->initialRefreshGraph();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_source",
        "00_initial"));

    transf1->forceUpdateTimedNode();
    QTest::qWait(50);
    image->waitForDone();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(!transf1->hasPendingTimedUpdates());

    {
        auto stat = listener->stats();
        listener->clear();

        QVERIFY(stat.decorateRectTriggeredStaticImageUpdate > 0);
        QCOMPARE(stat.forceUpdateTimedNode, 1);
        QCOMPARE(stat.slotDelayedStaticImageUpdate, 0);
        QCOMPARE(stat.recalculateStaticImage, 1);
    }

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_source",
        "10_transform_mask_initial_static"));

    cloneLayer1->setDirty();
    QTest::qWait(50);
    image->waitForDone();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(!transf1->hasPendingTimedUpdates());

    {
        auto stat = listener->stats();
        listener->clear();

        QCOMPARE(stat.decorateRectTriggeredStaticImageUpdate, 0);
        QCOMPARE(stat.forceUpdateTimedNode, 0);
        QCOMPARE(stat.slotDelayedStaticImageUpdate, 0);
        QCOMPARE(stat.recalculateStaticImage, 0);
    }

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_source",
        "20_final"));
}

void KisCloneLayerTest::testWithSourceUnderTwoTransformMasks()
{
    /*
      +--------------------+
      |root                |
      |  clone_of_p1 -+    |
      |  paint 2      |    |
      |  paint 1    <-+    |
      |    transf2 (offset)|
      |    transf1 (scale) |
      +--------------------+
     */

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(QRect(10,10,100,100), KoColor( Qt::white, colorSpace));
    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8, device1);
    image->addNode(paintLayer1, image->root());

    KisTransformMaskTestingListener *listener1 = new KisTransformMaskTestingListener();
    QTransform transform1 = QTransform::fromScale(0.1, 0.2);
    KisTransformMaskSP transf1 = new KisTransformMask(image, "transf1");
    image->addNode(transf1, paintLayer1);
    transf1->setTransformParams(KisTransformMaskParamsInterfaceSP(
        new KisDumbTransformMaskParams(transform1)));
    transf1->setTestingInterface(listener1);

    /**
     * An additional offset in `transf2` makes the "hidden area
     * generation" on the clone to cause a dirty request inside
     * the image bounds on the transform mask `transf1`, which
     * causes an async update, and, hence infinite cycle.
     */

    KisTransformMaskTestingListener *listener2 = new KisTransformMaskTestingListener();
    QTransform transform2 = QTransform::fromTranslate(-3, -3);
    KisTransformMaskSP transf2 = new KisTransformMask(image, "transf2");
    image->addNode(transf2, paintLayer1);
    transf2->setTransformParams(KisTransformMaskParamsInterfaceSP(
        new KisDumbTransformMaskParams(transform2)));
    transf2->setTestingInterface(listener2);

    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    device2->fill(QRect(20, 20, 10, 10), KoColor( Qt::red, colorSpace));
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8, device2);
    image->addNode(paintLayer2, image->root());

    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer1, image, "clone_of_p1", OPACITY_OPAQUE_U8);
    cloneLayer1->setX(10);
    cloneLayer1->setY(10);
    image->addNode(cloneLayer1, image->root());

    image->initialRefreshGraph();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_x2_source",
        "00_initial"));

    transf1->forceUpdateTimedNode();
    transf2->forceUpdateTimedNode();
    QTest::qWait(50);
    image->waitForDone();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(!transf1->hasPendingTimedUpdates());
    QVERIFY(!transf2->hasPendingTimedUpdates());

    {
        auto stat = listener1->stats();
        listener1->clear();

        QVERIFY(stat.decorateRectTriggeredStaticImageUpdate > 0);
        QCOMPARE(stat.forceUpdateTimedNode, 1);
        QCOMPARE(stat.slotDelayedStaticImageUpdate, 0);
        QCOMPARE(stat.recalculateStaticImage, 1);
    }

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_x2_source",
        "10_transform_mask_initial_static"));

    cloneLayer1->setDirty();
    QTest::qWait(50);
    image->waitForDone();
    QTest::qWait(50);
    image->waitForDone();

    QVERIFY(!transf1->hasPendingTimedUpdates());
    QVERIFY(!transf2->hasPendingTimedUpdates());

    {
        auto stat = listener1->stats();
        listener1->clear();

        QCOMPARE(stat.decorateRectTriggeredStaticImageUpdate, 0);
        QCOMPARE(stat.forceUpdateTimedNode, 0);
        QCOMPARE(stat.slotDelayedStaticImageUpdate, 0);
        QCOMPARE(stat.recalculateStaticImage, 0);
    }

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        "tmask_x2_source",
        "20_final"));
}

#include <kis_filter_registry.h>
#include <kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <KisGlobalResourcesInterface.h>

void KisCloneLayerTest::testCloneOfGroupWithClones_data()
{
    QTest::addColumn<bool>("useBlurFilter");
    QTest::addRow("no_blur") << false;
    QTest::addRow("with_blur") << true;
}

void KisCloneLayerTest::testCloneOfGroupWithClones()
{
    QFETCH(bool, useBlurFilter);

    /*
      +-------------------+
      |root               |
      |  clone_of_g1 ---+ |
      |    blur_1       | |
      |                 | |
      |  group_1    <---+ |
      |                   |
      |    clone_of_p1 -+ |
      |    paint_1  <---+ |
      +-------------------+
     */

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 128, 128, colorSpace, "clones test");

    KisLayerSP groupLayer1 = new KisGroupLayer(image, "group_1", OPACITY_OPAQUE_U8);
    image->addNode(groupLayer1, image->root());

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    device1->fill(QRect(10,10,100,100), KoColor( Qt::white, colorSpace));
    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint_1", OPACITY_OPAQUE_U8, device1);
    image->addNode(paintLayer1, groupLayer1);

    KisLayerSP cloneLayer1 = new KisCloneLayer(paintLayer1, image, "clone_of_p1", OPACITY_OPAQUE_U8);
    cloneLayer1->setX(10);
    cloneLayer1->setY(10);
    image->addNode(cloneLayer1, groupLayer1);

    KisLayerSP cloneLayer2 = new KisCloneLayer(groupLayer1, image, "clone_of_g1", 200);
    cloneLayer2->setX(-5);
    cloneLayer2->setY(-5);
    cloneLayer2->setCompositeOpId(COMPOSITE_EXCLUSION);
    image->addNode(cloneLayer2, image->root());


    if (useBlurFilter) {
        // add a filter mask to cause out-of-the-bounds updates

        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        KIS_ASSERT(filter);

        KisFilterConfigurationSP configuration1 = filter->defaultConfiguration(KisGlobalResourcesInterface::instance());
        configuration1->setProperty("halfWidth", 7);
        configuration1->setProperty("halfHeight", 7);

        KisFilterMaskSP filterMask1 = new KisFilterMask(image, "blur_1");
        filterMask1->setFilter(configuration1->cloneWithResourcesSnapshot());

        image->addNode(filterMask1, cloneLayer2);
    }

    image->initialRefreshGraph();
    QTest::qWait(50);
    image->waitForDone();

    /**
     * If the image is successfully rendered and **not** stuck in the infinite
     * loop in the clone layers triggering each other, then the test is passed.
     */

    QVERIFY(TestUtil::checkQImage(
        image->projection()->convertToQImage(nullptr, image->bounds()),
        "clone_layer_test",
        QString("clone_of_group_with_clones_%1").arg(QTest::currentDataTag()),
        "00_initial"));
}

KISTEST_MAIN(KisCloneLayerTest)
