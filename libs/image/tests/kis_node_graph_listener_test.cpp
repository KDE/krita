/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_graph_listener_test.h"

#include <QTest>
#include "kis_node_graph_listener.h"
#include "kis_node_facade.h"
#include <testutil.h>

void KisNodeGraphListenerTest::testUpdateOfListener()
{
    KisNodeFacade nodeFacade;
    TestUtil::TestGraphListener listener;

    KisNodeSP rootNode = new TestNode();
    KisNodeSP child1 = new TestNode();
    KisNodeSP child2 = new TestNode();

    rootNode->setGraphListener(&listener);

    QCOMPARE(rootNode->graphListener(), &listener);
    QCOMPARE(child1->graphListener(), (KisNodeGraphListener*)0);
    QCOMPARE(child2->graphListener(), (KisNodeGraphListener*)0);

    nodeFacade.addNode(child1, rootNode);
    nodeFacade.addNode(child2, rootNode);

    QCOMPARE(rootNode->graphListener(), &listener);
    QCOMPARE(child1->graphListener(), &listener);
    QCOMPARE(child2->graphListener(), &listener);
}

void KisNodeGraphListenerTest::testRecursiveUpdateOfListener()
{
    KisNodeFacade nodeFacade;
    TestUtil::TestGraphListener listener;

    KisNodeSP rootNode = new TestNode();
    KisNodeSP child1 = new TestNode();
    KisNodeSP child2 = new TestNode();

    QCOMPARE(rootNode->graphListener(), (KisNodeGraphListener*)0);
    QCOMPARE(child1->graphListener(), (KisNodeGraphListener*)0);
    QCOMPARE(child2->graphListener(), (KisNodeGraphListener*)0);

    nodeFacade.addNode(child1, rootNode);
    nodeFacade.addNode(child2, rootNode);

    QCOMPARE(rootNode->graphListener(), (KisNodeGraphListener*)0);
    QCOMPARE(child1->graphListener(), (KisNodeGraphListener*)0);
    QCOMPARE(child2->graphListener(), (KisNodeGraphListener*)0);

    rootNode->setGraphListener(&listener);

    QCOMPARE(rootNode->graphListener(), &listener);
    QCOMPARE(child1->graphListener(), &listener);
    QCOMPARE(child2->graphListener(), &listener);
}

void KisNodeGraphListenerTest::testSequenceNumber()
{
    KisNodeFacade nodeFacade;
    TestUtil::TestGraphListener listener;

    KisNodeSP rootNode = new TestNode();
    KisNodeSP child1 = new TestNode();
    KisNodeSP child2 = new TestNode();

    nodeFacade.setRoot(rootNode);
    rootNode->setGraphListener(&listener);

    int seqno = 0;

    seqno = listener.graphSequenceNumber();
    nodeFacade.addNode(child1, rootNode);
    QVERIFY(seqno != listener.graphSequenceNumber());

    seqno = listener.graphSequenceNumber();
    nodeFacade.addNode(child2, rootNode);
    QVERIFY(seqno != listener.graphSequenceNumber());

    seqno = listener.graphSequenceNumber();
    nodeFacade.moveNode(child1, rootNode, child2);
    QVERIFY(seqno != listener.graphSequenceNumber());

    seqno = listener.graphSequenceNumber();
    nodeFacade.removeNode(child1);
    QVERIFY(seqno != listener.graphSequenceNumber());

    seqno = listener.graphSequenceNumber();
    nodeFacade.removeNode(child2);
    QVERIFY(seqno != listener.graphSequenceNumber());
}

QTEST_MAIN(KisNodeGraphListenerTest)
