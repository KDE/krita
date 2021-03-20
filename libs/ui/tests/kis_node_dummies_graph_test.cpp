/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_dummies_graph_test.h"

#include <simpletest.h>

#include "kis_node_dummies_graph.h"
#include "node_shapes_utils.h"

inline KisNodeDummy* nodeDummyFromId(int id) {
    KisNodeShape *nodeShape = nodeShapeFromId(id);
    return new KisNodeDummy(nodeShape, nodeShape->node());
}


/**
 * node0
 * +--node1
 *    +--node7
 *    +--node8
 * +--node2
 * +--node3
 * +--node4
 *    +--node9
 *    +--node10
 *    +--node11
 * +--node5
 * +--node6
 */

void KisNodeDummiesGraphTest::init()
{
    m_dummiesGraph = new KisNodeDummiesGraph();
    m_rootDummy = nodeDummyFromId(0);

    m_dummiesGraph->addNode(m_rootDummy, 0, 0);
    KisNodeDummy *parent;

    parent = m_rootDummy;
    for(int i = 6; i >= 1; i--) {
        KisNodeDummy *dummy = nodeDummyFromId(i);
        m_dummiesGraph->addNode(dummy, parent, 0);
    }

    parent = findDummyById(m_rootDummy, 1);
    Q_ASSERT(parent);
    for(int i = 8; i >= 7; i--) {
        KisNodeDummy *dummy = nodeDummyFromId(i);
        m_dummiesGraph->addNode(dummy, parent, 0);
    }

    parent = findDummyById(m_rootDummy, 4);
    Q_ASSERT(parent);
    for(int i = 11; i >= 9; i--) {
        KisNodeDummy *dummy = nodeDummyFromId(i);
        m_dummiesGraph->addNode(dummy, parent, 0);
    }

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::cleanup()
{
    delete m_rootDummy;
    delete m_dummiesGraph;
}

void KisNodeDummiesGraphTest::testIndexing()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *dummy10 = findDummyById(m_rootDummy, 10);

    QCOMPARE(parent->childCount(), 3);
    QCOMPARE(parent->indexOf(dummy10), 1);
    QCOMPARE(parent->at(1), dummy10);
}

void KisNodeDummiesGraphTest::testPrepend()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *dummy = nodeDummyFromId(13);
    m_dummiesGraph->addNode(dummy, parent, 0);

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 13 9 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testAppend()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *dummy = nodeDummyFromId(13);
    m_dummiesGraph->addNode(dummy, parent, parent->lastChild());

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 11 13 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testInsert()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *aboveThis = findDummyById(m_rootDummy, 10);

    KisNodeDummy *dummy = nodeDummyFromId(13);
    m_dummiesGraph->addNode(dummy, parent, aboveThis);

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 13 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testNewSubgraph()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 3);

    KisNodeDummy *dummy = nodeDummyFromId(13);
    m_dummiesGraph->addNode(dummy, parent, 0);

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 13 4 9 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testRemoveFirst()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *child = parent->firstChild();
    m_dummiesGraph->removeNode(child);
    delete child;

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testRemoveLast()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 4);
    KisNodeDummy *child = parent->lastChild();
    m_dummiesGraph->removeNode(child);
    delete child;

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 7 8 2 3 4 9 10 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testRemoveBranch()
{
    KisNodeDummy *parent = findDummyById(m_rootDummy, 1);
    KisNodeDummy *child;

    child = parent->firstChild();
    m_dummiesGraph->removeNode(child);
    delete child;

    child = parent->lastChild();
    m_dummiesGraph->removeNode(child);
    delete child;

    QString realGraph = collectGraphPattern(m_rootDummy);
    QString expectedGraph = "0 1 2 3 4 9 10 11 5 6";

    QCOMPARE(realGraph, expectedGraph);
}

void KisNodeDummiesGraphTest::testReverseTraversing()
{
    QString forwardGraph = collectGraphPattern(m_rootDummy);
    QString reverseGraph = collectGraphPatternReverse(m_rootDummy);

    QCOMPARE(reverseGraph, forwardGraph);
}

SIMPLE_TEST_MAIN(KisNodeDummiesGraphTest)
