/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_node_manager_test.h"

#include <qtest_kde.h>

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

    void checkUndoWait() {
        undoStore->undo();
        QTest::qWait(1000);
        image->waitForDone();
        QVERIFY(checkLayers("initial"));
    }

    KisNodeManager *nodeManager;
};

void testRotateNode(bool useShapeLayer, const QString &name)
{
    NodeManagerTester t;
    if(useShapeLayer) {
        t.activateShapeLayer();
    }

    t.nodeManager->rotate(M_PI / 6.0);
    QTest::qWait(1000);
    t.image->waitForDone();
    QVERIFY(t.checkLayers(name));

    t.checkUndoWait();
    t.startConcurrentTask();

    t.nodeManager->rotate(M_PI / 6.0);
    QTest::qWait(1000);
    t.image->waitForDone();

    if (!useShapeLayer) {
        QEXPECT_FAIL("", "The user may run Rotate Layer concurrently. It will cause wrong image/selection size fetched for the crop. There is some barrier needed. At least it doesn't crash.", Continue);
    }
    QVERIFY(t.checkLayers(name));
}

void testShearNode(bool useShapeLayer, const QString &name)
{
    NodeManagerTester t;
    if(useShapeLayer) {
        t.activateShapeLayer();
    }

    t.nodeManager->shear(30, 0);
    QTest::qWait(1000);
    t.image->waitForDone();
    QVERIFY(t.checkLayers(name));

    t.checkUndoWait();
    t.startConcurrentTask();

    t.nodeManager->shear(30, 0);
    QTest::qWait(1000);
    t.image->waitForDone();

    QEXPECT_FAIL("", "The user may run Shear Layer concurrently. It will cause wrong image/selection size fetched for the crop. There is some barrier needed. At least it doesn't crash.", Continue);
    QVERIFY(t.checkLayers(name));
}

void testScaleNode(bool useShapeLayer, const QString &name)
{
    KisFilterStrategy *strategy = new KisBicubicFilterStrategy();

    NodeManagerTester t;
    if(useShapeLayer) {
        t.activateShapeLayer();
    }

    t.nodeManager->scale(0.5, 0.5, strategy);
    QTest::qWait(1000);
    t.image->waitForDone();
    QVERIFY(t.checkLayers(name));

    t.checkUndoWait();
    t.startConcurrentTask();

    t.nodeManager->scale(0.5, 0.5, strategy);
    QTest::qWait(1000);
    t.image->waitForDone();

    QVERIFY(t.checkLayers(name));

    delete strategy;
}

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
    QVERIFY(t.checkLayers(name));

    t.checkUndoWait();
    t.startConcurrentTask();

    if (mirrorX) {
        t.nodeManager->mirrorNodeX();
    } else {
        t.nodeManager->mirrorNodeY();
    }
    QTest::qWait(1000);
    t.image->waitForDone();

    if (mirrorX && !useShapeLayer) {
        QEXPECT_FAIL("", "The user may run Mirror Layer concurrently, but it is not ported to strokes yet. At least it doesn't crash.", Continue);
    }
    QVERIFY(t.checkLayers(name));
}

void KisNodeManagerTest::testRotatePaintNode()
{
    testRotateNode(false, "paint_rotated_30");
}

void KisNodeManagerTest::testShearPaintNode()
{
    testShearNode(false, "paint_shear_30");
}

void KisNodeManagerTest::testScalePaintNode()
{
    testScaleNode(false, "paint_scale_0.5");
}

void KisNodeManagerTest::testMirrorXPaintNode()
{
    testMirrorNode(false, "paint_mirrorX", true);
}

void KisNodeManagerTest::testMirrorYPaintNode()
{
    testMirrorNode(false, "paint_mirrorY", false);
}

void KisNodeManagerTest::testRotateShapeNode()
{
    testRotateNode(true, "shape_rotated_30");
}

void KisNodeManagerTest::testShearShapeNode()
{
    testShearNode(true, "shape_shear_30");
}

void KisNodeManagerTest::testScaleShapeNode()
{
    testScaleNode(true, "shape_scale_0.5");
}

void KisNodeManagerTest::testMirrorShapeNode()
{
    testMirrorNode(true, "shape_mirrorX", true);
}

QTEST_KDEMAIN(KisNodeManagerTest, GUI)
