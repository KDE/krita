/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_clone_layer_test.h"

#include <QTest>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_group_layer.h"
#include "kis_clone_layer.h"
#include "kis_image.h"

#include "testutil.h"


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

    image->refreshGraph();
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
    image->refreshGraph();
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

    QCOMPARE(image->root()->lastChild()->name(), QString("clone_of_p1"));
    QCOMPARE(image->root()->lastChild(), reincarnatedLayer);
    QVERIFY(image->root()->lastChild() != KisNodeSP(cloneLayer1));

    cmd2->redo();
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

QTEST_MAIN(KisCloneLayerTest)
