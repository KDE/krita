/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_manager_test.h"

#include <QTest>
#include <kis_filter_configuration.h>

#include <sdk/tests/testutil.h>
#include <sdk/tests/testui.h>

#include "ui_manager_test.h"

class NodeManagerTester : public TestUtil::UiManagerTest
{
public:
    NodeManagerTester()
        : UiManagerTest(false, true,  "node_manager_test")
    {
        nodeManager = view->nodeManager();
    }

    void activateShapeLayer() {
        KisNodeSP shape = findNode(image->root(), "shape");
        Q_ASSERT(shape);
        nodeManager->slotNonUiActivatedNode(shape);
    }

    KisNodeSP findCloneLayer() {
        return findNode(image->root(), "clone1");
    }

    void activateCloneLayer() {
        KisNodeSP node = findCloneLayer();
        Q_ASSERT(node);
        nodeManager->slotNonUiActivatedNode(findCloneLayer());
    }

    KisNodeSP findBlurLayer() {
        return findNode(image->root(), "blur1");
    }

    void activateBlurLayer() {
        KisNodeSP node = findBlurLayer();
        Q_ASSERT(node);
        nodeManager->slotNonUiActivatedNode(findBlurLayer());
    }

    void checkUndoWait() {
        undoStore->undo();
        QTest::qWait(1000);
        image->waitForDone();
        QVERIFY(checkLayersInitial());
    }

    KisNodeManager *nodeManager;
};

void testMirrorNode(bool useShapeLayer, const QString &name, bool mirrorX)
{
    NodeManagerTester t;
    if(useShapeLayer) {
        t.activateShapeLayer();
    }

    if (mirrorX) {
        t.nodeManager->mirrorNodeX();
    } else {
        t.nodeManager->mirrorNodeY();
    }
    QTest::qWait(1000);
    t.image->waitForDone();
    QVERIFY(t.checkLayersFuzzy(name));

    t.checkUndoWait();
    t.startConcurrentTask();

    if (mirrorX) {
        t.nodeManager->mirrorNodeX();
    } else {
        t.nodeManager->mirrorNodeY();
    }
    QTest::qWait(1000);
    t.image->waitForDone();

    QEXPECT_FAIL("", "The user may run Mirror Layer concurrently, but it is not ported to strokes yet. At least it doesn't crash.", Continue);
    QVERIFY(t.checkLayersFuzzy(name));
}

void KisNodeManagerTest::testMirrorXPaintNode()
{
    testMirrorNode(false, "paint_mirrorX", true);
}

void KisNodeManagerTest::testMirrorYPaintNode()
{
    testMirrorNode(false, "paint_mirrorY", false);
}

void KisNodeManagerTest::testMirrorShapeNode()
{
    testMirrorNode(true, "shape_mirrorX", true);
}

void KisNodeManagerTest::testConvertCloneToPaintLayer()
{
    NodeManagerTester t;
    t.activateCloneLayer();

    QVERIFY(t.checkLayersInitial());

    t.nodeManager->convertNode("KisPaintLayer");

    KisNodeSP node = t.findCloneLayer();

    QTest::qWait(1000);
    t.image->waitForDone();

    QVERIFY(dynamic_cast<KisPaintLayer*>(node.data()));

    // No visible change should happen!
    QVERIFY(t.checkLayersInitial());
}

void testConvertToSelectionMask(bool fromClone)
{
    NodeManagerTester t;

    if (fromClone) {
        t.activateCloneLayer();
    } else {
        t.activateBlurLayer();
    }

    QVERIFY(t.checkLayersInitial());
    QVERIFY(!t.image->globalSelection());

    t.nodeManager->convertNode("KisSelectionMask");

    QTest::qWait(1000);
    t.image->waitForDone();

    KisNodeSP node;

    if (fromClone) {
        node = t.findCloneLayer();
    } else {
        node = t.findBlurLayer();
    }

    QVERIFY(!node);

    KisSelectionSP selection = t.image->globalSelection();

    QVERIFY(selection);
    QVERIFY(!selection->outlineCacheValid() ||
            !selection->outlineCache().isEmpty());


    QString testName = fromClone ?
        "selection_from_clone_layer" : "selection_from_blur_layer";

    QVERIFY(t.checkSelectionOnly(testName));
}

void KisNodeManagerTest::testConvertCloneToSelectionMask()
{
    testConvertToSelectionMask(true);
}

void KisNodeManagerTest::testConvertBlurToSelectionMask()
{
    testConvertToSelectionMask(false);
}

KISTEST_MAIN(KisNodeManagerTest)
