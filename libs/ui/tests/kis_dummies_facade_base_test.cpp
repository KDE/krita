/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
    QString expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("layer1 __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_effect A_layer4 "
                       "R_layer4 R_effect R_layer3 R_layer2 R_layer1 R_root");
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
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("__null layer1 layer2 layer3 layer4 effect __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_layer4 A_effect "
                       "R_layer4 R_effect R_layer3 R_layer2 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testRemoveNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->removeNode(m_layer2);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 effect layer4";

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

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_effect A_layer4 "
                       "R_layer2 R_effect R_layer3 R_layer4 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testMoveNodeSameParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer3);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 effect layer2 layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);

    // layer is first removed then added again
    verifyActivatedNodes("layer1 layer2 __null");

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_effect A_layer4 "
                       "R_layer2 A_layer2 "
                       "R_layer4 R_layer2 R_effect R_layer3 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testMoveNodeDifferentParent()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer2, m_image->root(), m_layer4);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer3 effect layer4 layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->moveNode(m_layer3, m_layer4, m_layer4->lastChild());

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer4 layer3 effect layer2";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_dummiesFacade->setImage(0);

    // layer is first removed then added again
    verifyActivatedNodes("layer1 layer2 layer3 __null");

    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_effect A_layer4 "
                       "R_layer2 A_layer2 R_effect R_layer3 A_layer3 A_effect "
                       "R_layer2 R_effect R_layer3 R_layer4 R_layer1 R_root");
}

void KisDummiesFacadeBaseTest::testSubstituteRootNode()
{
    QString actualGraph;
    QString expectedGraph;

    constructImage();

    m_dummiesFacade->setImage(m_image);

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root layer1 layer2 layer3 effect layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 6);

    m_image->flatten();

    actualGraph = collectGraphPatternFull(m_dummiesFacade->rootDummy());
    expectedGraph = "root Layer 1";

    QEXPECT_FAIL("", "Expected 'root Layer 1', got 'root layer1 layer2 layer3 effect layer4'", Continue);
    QCOMPARE(actualGraph, expectedGraph);
    QEXPECT_FAIL("", "Expected 2 dummies, got 6", Continue);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 2);

    m_dummiesFacade->setImage(0);

    verifyActivatedNodes("layer1 __null Layer 1 __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_effect A_layer4 "
                       "R_layer4 R_effect R_layer3 R_layer2 R_layer1 R_root "
                       "A_root A_Layer 1 "
                       "R_Layer 1 R_root");
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
    expectedGraph = "root selection layer1 layer2 selection layer3 effect selection layer4";

    QCOMPARE(actualGraph, expectedGraph);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 9);

    m_dummiesFacade->setImage(0);
    QCOMPARE(m_dummiesFacade->dummiesCount(), 0);

    verifyActivatedNodes("__null layer1 layer2 layer3 layer4 effect __null");
    verifyMovedDummies("A_root A_layer1 A_layer2 A_layer3 A_layer4 A_effect "
                       "A_selection A_selection A_selection "
                       "R_layer4 R_selection R_effect R_layer3 R_selection "
                       "R_layer2 R_layer1 R_selection R_root");
}
