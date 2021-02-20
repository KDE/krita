/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dummies_facade_base_test.h"

#include <QTest>

#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade.h"
#include "node_shapes_utils.h"

void KisDummiesFacadeBaseTest::init()
{
    initBase();
    m_dummiesFacade = dummiesFacadeFactory();

    m_activatedNodes.clear();
    m_movedDummies.clear();
    connect(m_dummiesFacade, SIGNAL(sigActivateNode(KisNodeSP)),
            SLOT(slotNodeActivated(KisNodeSP)));
    connect(m_dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
            SLOT(slotEndInsertDummy(KisNodeDummy*)));
    connect(m_dummiesFacade, SIGNAL(sigBeginRemoveDummy(KisNodeDummy*)),
            SLOT(slotBeginRemoveDummy(KisNodeDummy*)));
}

void KisDummiesFacadeBaseTest::cleanup()
{
    destroyDummiesFacade(m_dummiesFacade);
    cleanupBase();
}

void KisDummiesFacadeBaseTest::slotNodeActivated(KisNodeSP node)
{
    QString prefix = m_activatedNodes.isEmpty() ? "" : " ";
    QString name = node ? node->name() : "__null";

    m_activatedNodes += prefix + name;
}

void KisDummiesFacadeBaseTest::slotEndInsertDummy(KisNodeDummy *dummy)
{
    QString prefix = m_movedDummies.isEmpty() ? "" : " ";
    QString name = dummy->node()->name();

    m_movedDummies += prefix + "A_" + name;
}

void KisDummiesFacadeBaseTest::slotBeginRemoveDummy(KisNodeDummy *dummy)
{
    QString prefix = m_movedDummies.isEmpty() ? "" : " ";
    QString name = dummy->node()->name();

    m_movedDummies += prefix + "R_" + name;
}

void KisDummiesFacadeBaseTest::verifyActivatedNodes(const QString &nodes)
{
    if (nodes != m_activatedNodes)
        QEXPECT_FAIL("", "Expected nodes string is not the same as the activated nodes string", Continue);
    QCOMPARE(m_activatedNodes, nodes);
}

void KisDummiesFacadeBaseTest::verifyMovedDummies(const QString &nodes)
{
    if (nodes != m_movedDummies)
        QEXPECT_FAIL("", "Expected nodes string is not the same as the moved dummies", Continue);
    QCOMPARE(m_movedDummies, nodes);
}

void KisDummiesFacadeBaseTest::testSetImage()
{
    constructImage();

    m_dummiesFacade->setImage(m_image);

    QString actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    QString expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("layer1 __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_mask1 A_layer4 "
                       "R_layer4 R_mask1 R_layer3 R_layer2 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testAddNode()
{
    QString actualGraph;
    QString expectedGraph;

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 1);

    constructImage();

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("__null layer1 layer2 layer3 layer4 mask1 __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_layer4 A_mask1 "
                       "R_layer4 R_mask1 R_layer3 R_layer2 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testRemoveNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->removeNode(m_layer2);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 5);

    m_image->removeNode(m_layer3);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 3);

    m_dummiesFacade->setImage(0);

    // we are not expected to handle nodes removal, it is done by Qt
    verifyActivatedNodes("layer1 __null");

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_mask1 A_layer4 "
                       "R_layer2 R_mask1 R_layer3 R_layer4 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testMoveNodeSameParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer3);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 mask1 layer2 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);

    // layer is first removed then added again
    verifyActivatedNodes("layer1 layer2 __null");

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_mask1 A_layer4 "
                       "R_layer2 A_layer2 "
                       "R_layer4 R_layer2 R_mask1 R_layer3 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testMoveNodeDifferentParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer4);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 mask1 layer4 layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer3, m_layer4, m_layer4->lastChild());

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer4 layer3 mask1 layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);

    // layer is first removed then added again
    verifyActivatedNodes("layer1 layer2 layer3 __null");

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_mask1 A_layer4 "
                       "R_layer2 A_layer2 R_mask1 R_layer3 A_layer3 A_mask1 "
                       "R_layer2 R_mask1 R_layer3 R_layer4 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testSubstituteRootNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 mask1 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->flatten(0);
    m_image->waitForDone();
    QTest::qWait(50);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root root Merged"; // "root Merged" is the name of the layer :)

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 2);

    m_dummiesFacade->setImage(0);

    verifyActivatedNodes("layer1 __null root Merged __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_mask1 A_layer4 "
                       "R_layer4 R_mask1 R_layer3 R_layer2 R_layer1 R_root "
                       "A_root A_root Merged "
                       "R_root Merged R_root");
}

void KisDummiesFacadeBaseTest::testAddSelectionMasksNoActivation()
{
    QString actualGraph;
    QString expectedGraph;

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 1);

    constructImage();
    addSelectionMasks();

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root sel1 layer1 layer2 sel2 layer3 mask1 sel3 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 9);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("__null layer1 layer2 layer3 layer4 mask1 __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_layer4 A_mask1 "
                       "A_sel1 A_sel2 A_sel3 "
                       "R_layer4 R_sel3 R_mask1 R_layer3 R_sel2 "
                       "R_layer2 R_layer1 R_sel1 R_root");
}
