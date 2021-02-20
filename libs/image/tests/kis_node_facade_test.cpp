/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_facade_test.h"
#include <QTest>
#include <limits.h>


#include "kis_node_facade.h"
#include "kis_types.h"
#include "kis_global.h"
#include <testutil.h>

void KisNodeFacadeTest::testCreation()
{
    TestUtil::TestGraphListener graphListener;

    KisNodeSP node = new TestNodeA();
    QVERIFY(node->graphListener() == 0);

    KisNodeFacade facade(node);
    QVERIFY(facade.root() == node);

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

}

void dumpNodeStack(KisNodeSP node, QString prefix = QString("\t"))
{
    for (uint i = 0; i < node->childCount(); ++i) {
        if (node->at(i)->parent())
            dbgImage << prefix << "\t" << node->at(i) << "node at" << i << " has index from parent:" << node->index(node->at(i));

        if (node->at(i)->childCount() > 0) {
            dumpNodeStack(node->at(i), prefix + '\t');
        }
    }

}

void KisNodeFacadeTest::testOrdering()
{
    TestUtil::TestGraphListener graphListener;

    KisNodeSP root = new TestNodeA();
    root->setGraphListener(&graphListener);

    KisNodeFacade facade(root);

    KisNodeSP node1 = new TestNodeA();
    KisNodeSP node2 = new TestNodeA();
    KisNodeSP node3 = new TestNodeA();
    KisNodeSP node4 = new TestNodeA();
    KisNodeSP node5 = new TestNodeA();

    /*
     +---------+
     | node 4  |
     | node 2  |
     |   node 3|
     | node 1  |
     |root     |
     +---------+
    */

    graphListener.resetBools();
    QVERIFY(facade.addNode(node1, root, root->childCount()) == true);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node1);
    QVERIFY(root->childCount() == 1);
    graphListener.resetBools();

    QVERIFY(facade.addNode(node2, root, root->childCount()) == true);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->childCount() == 2);
    graphListener.resetBools();

    QVERIFY(facade.addNode(node3, node1) == true);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->childCount() == 2);
    QVERIFY(node1->childCount() == 1);
    graphListener.resetBools();

    QVERIFY(facade.addNode(node4, root, root->lastChild()) == true);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node4);
    QVERIFY(root->childCount() == 3);
    graphListener.resetBools();

    QVERIFY(node1->parent() == root);
    QVERIFY(node2->parent() == root);
    QVERIFY(node3->parent() == node1);
    QVERIFY(node4->parent() == root);

    QVERIFY(root->firstChild() == node1);
    QVERIFY(root->lastChild() == node4);

    QVERIFY(root->at(0) == node1);
    QVERIFY(root->at(1) == node2);
    QVERIFY(root->at(2) == node4);
    QVERIFY(node1->at(0) == node3);

    QVERIFY(root->index(node1) == 0);
    QVERIFY(root->index(node2) == 1);
    QVERIFY(root->index(node4) == 2);
    QVERIFY(node1->index(node3) == 0);

    QVERIFY(node4->prevSibling() == node2);
    QVERIFY(node2->prevSibling() == node1);
    QVERIFY(node1->prevSibling() == 0);
    QVERIFY(node3->prevSibling() == 0);

    QVERIFY(node4->nextSibling() == 0);
    QVERIFY(node2->nextSibling() == node4);
    QVERIFY(node1->nextSibling() == node2);
    QVERIFY(node3->nextSibling() == 0);

    /*
          node 4
        node 2
          node 3
        node 1
       root
     */
    graphListener.resetBools();
    QVERIFY(facade.removeNode(node4) == true);
    QVERIFY(node4->parent() == 0);
    QVERIFY(graphListener.beforeInsertRow == false);
    QVERIFY(graphListener.afterInsertRow == false);
    QVERIFY(graphListener.beforeRemoveRow == true);
    QVERIFY(graphListener.afterRemoveRow == true);
    QVERIFY(root->childCount() == 2);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
    graphListener.resetBools();

    QVERIFY(facade.addNode(node4, node3) == true);
    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    QVERIFY(root->childCount() == 2);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node3->childCount() == 1);
    QVERIFY(node3->firstChild() == node4);
    QVERIFY(node3->lastChild() == node4);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
    graphListener.resetBools();

    QVERIFY(facade.removeNode(node4) == true);
    QVERIFY(graphListener.beforeInsertRow == false);
    QVERIFY(graphListener.afterInsertRow == false);
    QVERIFY(graphListener.beforeRemoveRow == true);
    QVERIFY(graphListener.afterRemoveRow == true);
    QVERIFY(node3->childCount() == 0);
    QVERIFY(node4->parent() == 0);
    QVERIFY(root->childCount() == 2);
    QVERIFY(root->lastChild() == node2);
    QVERIFY(root->firstChild() == node1);
    QVERIFY(node4->prevSibling() == 0);
    QVERIFY(node4->nextSibling() == 0);
    graphListener.resetBools();

    QVERIFY(facade.addNode(node4, node2) == true);
    graphListener.resetBools();

    QVERIFY(graphListener.beforeMove == false);
    QVERIFY(graphListener.afterMove == false);
    QVERIFY(facade.moveNode(node3, root, 0) == true);
    QVERIFY(graphListener.beforeMove == true);
    QVERIFY(graphListener.afterMove == true);
    graphListener.resetBools();

    /*
            node4
          node2
          node1
        node 3
      root

     */

    QVERIFY(facade.moveNode(node1, node3, 0) == true);
    QVERIFY(facade.moveNode(node2, node3, node1) == true);

    QVERIFY(graphListener.beforeInsertRow == true);
    QVERIFY(graphListener.afterInsertRow == true);
    QVERIFY(graphListener.beforeRemoveRow == true);
    QVERIFY(graphListener.afterRemoveRow == true);
    graphListener.resetBools();

    QVERIFY(facade.moveNode(node4, node4, node4) == false);

    QVERIFY(graphListener.beforeInsertRow == false);
    QVERIFY(graphListener.afterInsertRow == false);
    QVERIFY(graphListener.beforeRemoveRow == false);
    QVERIFY(graphListener.afterRemoveRow == false);
    graphListener.resetBools();

    QCOMPARE(root->childCount(), 1u);
    QVERIFY(root->firstChild() == node3);
    QVERIFY(root->lastChild() == node3);
    QVERIFY(node3->childCount() == 2);
    QVERIFY(node3->firstChild() == node1);
    QVERIFY(node3->lastChild() == node2);

    /*
        node4
      node2
      node5
      node1
     node3
    root
    */
    QVERIFY(facade.addNode(node5, node3, node1) == true);
    QVERIFY(node5->parent() == node3);
    QVERIFY(node5->prevSibling() == node1);
    QVERIFY(node5->nextSibling() == node2);

    /*
       node5
         node4
       node2
       node1
      node3
     root
    */

    QVERIFY(facade.raiseNode(node5) == true);
    QVERIFY(node5->parent() == node3);
    QVERIFY(node5->nextSibling() == 0);
    QVERIFY(node5->prevSibling() == node2);

    // Try raising topnode to top
    QVERIFY(facade.raiseNode(node5) == true);
    QVERIFY(node5->parent() == node3);
    QVERIFY(node5->nextSibling() == 0);
    QVERIFY(node5->prevSibling() == node2);

    /*
       node5
       node1
         node4
       node2
      node3
     root
    */
    QVERIFY(facade.lowerNode(node2) == true);
    QVERIFY(node2->nextSibling() == node1);
    QVERIFY(node2->prevSibling() == 0);

    // Try lowering bottomnode to bottomg
    QVERIFY(facade.lowerNode(node2) == true);
    QVERIFY(node2->nextSibling() == node1);
    QVERIFY(node2->prevSibling() == 0);

    /**
       node4
     node2
     node5
     node1
    node3
    root
    */
    QVERIFY(facade.toTop(node2) == true);
    QVERIFY(node2->nextSibling() == 0);
    QVERIFY(node2->prevSibling() == node5);

    /**
     node5
     node1
       node4
     node2
    node3
    root
    */
    QVERIFY(facade.toBottom(node2) == true);
    QVERIFY(node2->nextSibling() == node1);
    QVERIFY(node2->prevSibling() == 0);

}


void KisNodeFacadeTest::testMove()
{
    TestUtil::TestGraphListener graphListener;

    KisNodeSP root = new TestNodeA();
    root->setGraphListener(&graphListener);

    KisNodeFacade facade(root);

    KisNodeSP node1 = new TestNodeA();
    node1->setName("node1");
    KisNodeSP node2 = new TestNodeA();
    node2->setName("node2");
    KisNodeSP node3 = new TestNodeA();
    node3->setName("node3");

    facade.addNode(node1);
    facade.addNode(node2);
    facade.addNode(node3);

    QVERIFY(root->at(0) == node1);
    QVERIFY(root->at(1) == node2);
    QVERIFY(root->at(2) == node3);

    facade.moveNode(node3, root, node1);

    QVERIFY(root->at(0) == node1);
    QVERIFY(root->at(1) == node3);
    QVERIFY(root->at(2) == node2);

}

QTEST_MAIN(KisNodeFacadeTest)
