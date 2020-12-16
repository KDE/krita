/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_test.h"
#include <QTest>
#include <limits.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_node_graph_listener.h"
#include <KoProperties.h>
#include <testutil.h>


void KisNodeTest::testCreation()
{
    TestUtil::TestGraphListener graphListener;

    KisNode * node = new TestNodeA();
    QVERIFY(node->graphListener() == 0);

    node->setGraphListener(&graphListener);
    QVERIFY(node->graphListener() != 0);

    // Test contract for initial state
    QVERIFY(node->parent() == 0);
    QVERIFY(node->firstChild() == 0);
    QVERIFY(node->lastChild() == 0);
    QVERIFY(node->prevSibling() == 0);
    QVERIFY(node->nextSibling() == 0);
    QVERIFY(node->childCount() == 0);
    QVERIFY(node->at(0) == 0);
    QVERIFY(node->at(UINT_MAX) == 0);
    QVERIFY(node->index(0) == -1);

    delete node;
}


void KisNodeTest::testOrdering()
{
    TestUtil::TestGraphListener graphListener;

    KisNodeSP root = new TestNodeA();
    root->setGraphListener(&graphListener);
    KisNodeSP node1 = new TestNodeA();
    KisNodeSP node2 = new TestNodeA();
    KisNodeSP node3 = new TestNodeA();
    KisNodeSP node4 = new TestNodeA();

    /*
     +---------+
     | node 4  |
     | node 2  |
     | node 3  |
     | node 1  |
     |root     |
     +---------+
    */

    graphListener.resetBools();
    QVERIFY(root->lastChild() == 0);
    root->add(node1, root->lastChild());
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node1);
    graphListener.resetBools();

    QVERIFY(root->lastChild() == node1);
    root->add(node2, root->lastChild());
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node2);
    graphListener.resetBools();

    QVERIFY(root->lastChild() == node2);
    root->add(node3, node1);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node2);
    graphListener.resetBools();

    root->add(node4, root->lastChild());
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node4);
    graphListener.resetBools();

    QVERIFY(root->childCount() == 4);

    QVERIFY(node1->parent() == root);
    QVERIFY(node2->parent() == root);
    QVERIFY(node3->parent() == root);
    QVERIFY(node4->parent() == root);

    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node4);

    QVERIFY(root->at(0) == node1);
    QVERIFY(root->at(1) == node3);
    QVERIFY(root->at(2) == node2);
    QVERIFY(root->at(3) == node4);

    QVERIFY(root->index(node1) == 0);
    QVERIFY(root->index(node3) == 1);
    QVERIFY(root->index(node2) == 2);
    QVERIFY(root->index(node4) == 3);

    QVERIFY(node4->prevSibling() == node2);
    QVERIFY(node3->prevSibling() == node1);
    QVERIFY(node2->prevSibling() == node3);
    QVERIFY(node1->prevSibling() == 0);

    QVERIFY(node4->nextSibling() == 0);
    QVERIFY(node3->nextSibling() == node2);
    QVERIFY(node2->nextSibling() == node4);
    QVERIFY(node1->nextSibling() == node3);

    /*
      +---------+
      | node 3  |
      |  node 4 |
      | node 2  |
      | node 1  |
      |root     |
      +---------+
     */
    graphListener.resetBools();
    QVERIFY(root->remove(root->at(3)) == true);
    QVERIFY(node4->parent() == 0);
    QVERIFY(graphListener.beforeInsertRow == false);
    QVERIFY(graphListener.afterInsertRow == false);
    QVERIFY(graphListener.beforeRemoveRow == true);
    QVERIFY(graphListener.afterRemoveRow == true);
    QVERIFY(root->childCount() == 3);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
    graphListener.resetBools();

    node3->add(node4, node3->lastChild());
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->childCount() == 3);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node3->childCount() == 1);
    QVERIFY(node3->firstChild() == node4);
    QVERIFY(node3->lastChild() == node4);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
    QVERIFY(root->remove(node4) == false);
    graphListener.resetBools();

    node3->remove(node4);
    QVERIFY(graphListener.beforeInsertRow == false);
    QVERIFY(graphListener.afterInsertRow == false);
    QVERIFY(graphListener.beforeRemoveRow == true);
    QVERIFY(graphListener.afterRemoveRow == true);
    QVERIFY(node3->childCount() == 0);
    QVERIFY(node4->parent() == 0);
    QVERIFY(root->childCount() == 3);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
}

void KisNodeTest::testSetDirty()
{
    // Create a node graph with two branches

    /*
        node2
          node4
              node6
             node5
            node5
          node3
        node1
      root
     */
    KisNodeSP root = new TestNode();
    root->setName("root");

    KisNodeSP node1 = new TestNode();
    node1->setName("node1");
    QVERIFY(root->add(node1, 0));

    KisNodeSP node2 = new TestNode();
    node2->setName("node2");
    QVERIFY(root->add(node2, node1));

    KisNodeSP node3 = new TestNode();
    node3->setName("node3");
    QVERIFY(node1->add(node3, 0));

    KisNodeSP node4 = new TestNode();
    node4->setName("node4");
    QVERIFY(node1->add(node4, node3));

    KisNodeSP node5 = new TestNode();
    node5->setName("node5");
    QVERIFY(node3->add(node5, 0));

    KisNodeSP node6 = new TestNode();
    node6->setName("node6");
    QVERIFY(node5->add(node6, 0));

    KisNodeSP node7 = new TestNode();
    node7->setName("node7");
    QVERIFY(node6->add(node7, 0));
#if 0 // XXX: rewrite tests after redesign to update strategies
    node1->setDirty();
    QVERIFY(node1->isDirty());
    QVERIFY(!node2->isDirty());
    QVERIFY(root->isDirty());
    root->setClean();
    QVERIFY(!root->isDirty());
    node1->setClean();
    QVERIFY(!node1->isDirty());

    node7->setDirty(QRect(10, 10, 100, 100));
    QVERIFY(node7->isDirty());
    QVERIFY(node7->isDirty(QRect(5, 5, 15, 15)));
    QVERIFY(root->isDirty(QRect(5, 5, 15, 15)));
    QVERIFY(!root->isDirty(QRect(-10, -10, 20, 20)));
    QVERIFY(!node2->isDirty());

    node7->setClean(QRect(10, 10, 10, 10));
    QVERIFY(!node7->isDirty(QRect(10, 10, 10, 10)));
    QVERIFY(node7->isDirty());
    QVERIFY(node7->isDirty(QRect(0, 0, 50, 50)));
#endif

}


void KisNodeTest::testChildNodes()
{
    KisNodeSP root = new TestNodeA();
    KisNodeSP a = new TestNodeA();
    root->add(a, 0);
    a->setVisible(true);
    a->setUserLocked(true);

    KisNodeSP b = new TestNodeB();
    root->add(b, 0);
    b->setVisible(false);
    b->setUserLocked(true);

    KisNodeSP c = new TestNodeC();
    root->add(c, 0);
    c->setVisible(false);
    c->setVisible(false);

    QList<KisNodeSP> allNodes = root->childNodes(QStringList(), KoProperties());
    QCOMPARE((int) allNodes.count(), 3);   // a, b, c

    QStringList nodeTypes;
    nodeTypes << "TestNodeA" << "TestNodeB";
    QList<KisNodeSP> subSetOfNodeTypes = root->childNodes(nodeTypes, KoProperties());
    QCOMPARE(subSetOfNodeTypes.count(), 2);   // a, b

    nodeTypes.clear();
    nodeTypes << "TestNodeB" << "TestNodeC";
    KoProperties props;
    props.setProperty("visible", false);
    props.setProperty("locked", true);
    QList<KisNodeSP> subsetOfTypesAndProps = root->childNodes(nodeTypes, props);
    QCOMPARE(subsetOfTypesAndProps.count(), 1);   // b

    KoProperties props2;
    props2.setProperty("visible", false);
    QList<KisNodeSP> subSetOfProps = root->childNodes(QStringList(), props2);
    QCOMPARE(subSetOfProps.count(), 2);   // b, c
}

#define NUM_CYCLES 100000
#define NUM_THREADS 30

class KisNodeTest::VisibilityKiller : public QRunnable {
public:
    VisibilityKiller(KisNodeSP victimNode, KisNodeSP nastyChild, bool /*isWriter*/)
        : m_victimNode(victimNode),
          m_nastyChild(nastyChild)
    {}

    void run() override {

        int visibility = 0;

        for(int i = 0; i < NUM_CYCLES; i++) {
            if(i % 3 == 0) {
                m_nastyChild->setVisible(visibility++ & 0x1);
                // dbgKrita << "visibility" << i << m_nastyChild->visible();
            }
            else if (i%3 == 1){
                KoProperties props;
                props.setProperty("visible", true);

                QList<KisNodeSP> visibleNodes =
                    m_victimNode->childNodes(QStringList("TestNodeB"), props);

                Q_FOREACH (KisNodeSP node, visibleNodes) {
                    m_nastyChild->setVisible(visibility++ & 0x1);
                }
                // dbgKrita << visibleNodes;
            }
            else {
                Q_ASSERT(m_victimNode->firstChild());
                Q_ASSERT(m_victimNode->lastChild());

                m_victimNode->firstChild()->setVisible(visibility++ & 0x1);
                m_victimNode->lastChild()->setVisible(visibility++ & 0x1);
            }
        }
    }

private:
    KisNodeSP m_victimNode;
    KisNodeSP m_nastyChild;
};


template <class KillerClass>
void KisNodeTest::propertiesStressTestImpl() {
    KisNodeSP root = new TestNodeA();

    KisNodeSP a = new TestNodeA();
    KisNodeSP b = new TestNodeB();
    KisNodeSP c = new TestNodeC();
    root->add(a, 0);
    root->add(b, 0);
    root->add(c, 0);
    a->setVisible(true);
    b->setVisible(true);
    c->setVisible(true);
    a->setUserLocked(true);
    b->setUserLocked(true);
    c->setUserLocked(true);

    QThreadPool threadPool;
    threadPool.setMaxThreadCount(NUM_THREADS);

    for(int i = 0; i< NUM_THREADS; i++) {
        KillerClass *killer = new KillerClass(root, b, i == 0);

        threadPool.start(killer);
    }

    threadPool.waitForDone();
}

void KisNodeTest::propertiesStressTest() {
    propertiesStressTestImpl<VisibilityKiller>();
}

class KisNodeTest::GraphKiller : public QRunnable {
public:
    GraphKiller(KisNodeSP parentNode, KisNodeSP childNode, bool isWriter)
        : m_parentNode(parentNode),
          m_childNode(childNode),
          m_isWriter(isWriter)
    {}

    void run() override {
        int numCycles = qMax(10000, NUM_CYCLES / 100);

        for(int i = 0; i < numCycles; i++) {
            if (m_isWriter) {
                m_parentNode->remove(m_childNode);
                m_parentNode->add(m_childNode, 0);
            } else {
                KisNodeSP a = m_parentNode->firstChild();
                KisNodeSP b = m_parentNode->lastChild();

                if (a) {
                    a->parent();
                    a->nextSibling();
                    a->prevSibling();
                }

                if (b) {
                    b->parent();
                    b->nextSibling();
                    b->prevSibling();
                }

                m_parentNode->at(0);
                m_parentNode->index(m_childNode);
            }

            if (i % 1000 == 0) {
                //dbgKrita << "Alive";
            }
        }
    }

private:
    KisNodeSP m_parentNode;
    KisNodeSP m_childNode;
    bool m_isWriter;
};

void KisNodeTest::graphStressTest() {
    propertiesStressTestImpl<GraphKiller>();
}

QTEST_MAIN(KisNodeTest)


