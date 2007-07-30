/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <qtest_kde.h>
#include <limits.h>

#include "kis_node_facade_test.h"
#include "kis_node_facade.h"
#include "kis_types.h"
#include "kis_global.h"
#include "kis_node.h"
#include "kis_node_graph_listener.h"

class TestGraphListener : public KisNodeGraphListener
{
public:

    virtual void aboutToAddANode( KisNode *, int )
        {
            beforeInsertRow = true;
        }

    virtual void nodeHasBeenAdded( KisNode *, int )
        {
            afterInsertRow = true;
        }

    virtual void aboutToRemoveANode( KisNode *, int )
        {
            beforeRemoveRow  = true;
        }

    virtual void nodeHasBeenRemoved( KisNode *, int )
        {
            afterRemoveRow = true;
        }

    bool beforeInsertRow;
    bool afterInsertRow;
    bool beforeRemoveRow;
    bool afterRemoveRow;

    void resetBools()
        {
            beforeRemoveRow = false;
            afterRemoveRow = false;
            beforeInsertRow = false;
            afterInsertRow = false;
        }
};


void KisNodeFacadeTest::testCreation()
{
    TestGraphListener graphListener;

    KisNodeSP node = new KisNode();
    QVERIFY( node->graphListener() == 0 );

    KisNodeFacade facade(node);

    node->setGraphListener( &graphListener );
    QVERIFY( node->graphListener() != 0 );

    // Test contract for initial state
    QVERIFY( node->parent() == 0 );
    QVERIFY( node->firstChild() == 0 );
    QVERIFY( node->lastChild() == 0 );
    QVERIFY( node->prevSibling() == 0 );
    QVERIFY( node->nextSibling() == 0 );
    QVERIFY( node->childCount() == 0 );
    QVERIFY( node->at( 0 ) == 0 );
    QVERIFY( node->at( UINT_MAX ) == 0 );
    QVERIFY( node->index( 0 ) == -1 );

}

void dumpNodeStack( KisNodeSP node, QString prefix = QString( "\t" ) ) {
    for ( uint i = 0; i < node->childCount(); ++i ) {
        if ( node->at( i )->parent() )
            kDebug() << prefix << "\t" << node->at( i ) << "node at " << i << " has index from parent: " << node->index( node->at( i ) ) << endl;

        if ( node->at( i )->childCount() > 0 ) {
            dumpNodeStack( node->at( i ), prefix + "\t" );
        }
    }

}

void KisNodeFacadeTest::testOrdering()
{
    TestGraphListener graphListener;

    KisNodeSP root = new KisNode();
    root->setGraphListener( &graphListener );
    kDebug() << "Root: " << root << endl;

    KisNodeFacade facade( root );

    KisNodeSP node1 = new KisNode();
    kDebug() << "Node 1: " << node1 << endl;

    KisNodeSP node2 = new KisNode();
    kDebug() << "Node 2: " << node2 << endl;

    KisNodeSP node3 = new KisNode();
    kDebug() << "Node 3: " << node3 << endl;

    KisNodeSP node4 = new KisNode();
    kDebug() << "Node 4: " << node4 << endl;

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
    facade.addNode( node1, root, root->childCount() );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node1 );
    graphListener.resetBools();

    facade.addNode( node2, root, root->childCount() );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node2 );
    graphListener.resetBools();

    facade.addNode( node3, node1 );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node2 );
    graphListener.resetBools();

    facade.addNode( node4, root, root->childCount() );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node4 );
    graphListener.resetBools();

    QVERIFY( root->childCount() == 4 );

    QVERIFY( node1->parent() == root );
    QVERIFY( node2->parent() == root );
    QVERIFY( node3->parent() == root );
    QVERIFY( node4->parent() == root );

    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node4 );

    QVERIFY( root->at( 0 ) == node1 );
    QVERIFY( root->at( 1 ) == node3 );
    QVERIFY( root->at( 2 ) == node2 );
    QVERIFY( root->at( 3 ) == node4 );

    QVERIFY( root->index( node1 ) == 0 );
    QVERIFY( root->index( node3 ) == 1 );
    QVERIFY( root->index( node2 ) == 2 );
    QVERIFY( root->index( node4 ) == 3 );

    QVERIFY( node4->prevSibling() == node2 );
    QVERIFY( node3->prevSibling() == node1 );
    QVERIFY( node2->prevSibling() == node3 );
    QVERIFY( node1->prevSibling() == 0 );

    QVERIFY( node4->nextSibling() == 0 );
    QVERIFY( node3->nextSibling() == node2 );
    QVERIFY( node2->nextSibling() == node4 );
    QVERIFY( node1->nextSibling() == node3 );

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
    QVERIFY( facade.removeNode( node4 ) == true );
    QVERIFY( node4->parent() == 0 );
    QVERIFY( graphListener.beforeInsertRow == false );
    QVERIFY( graphListener.afterInsertRow == false );
    QVERIFY( graphListener.beforeRemoveRow == true );
    QVERIFY( graphListener.afterRemoveRow == true );
    QVERIFY( root->childCount() == 3 );
    QVERIFY( root->lastChild() == node2 );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( node4->prevSibling() == 0 );
    QVERIFY( node4->nextSibling() == 0 );
    graphListener.resetBools();

    facade.addNode( node4, node3 );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->childCount() == 3 );
    QVERIFY( root->lastChild() == node2 );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( node3->childCount() == 1 );
    QVERIFY( node3->firstChild() == node4 );
    QVERIFY( node3->lastChild() == node4 );
    QVERIFY( node4->prevSibling() == 0 );
    QVERIFY( node4->nextSibling() == 0 );
    QVERIFY( facade.removeNode( node4 ) == false );
    graphListener.resetBools();

    facade.removeNode( node4 );
    QVERIFY( graphListener.beforeInsertRow == false );
    QVERIFY( graphListener.afterInsertRow == false );
    QVERIFY( graphListener.beforeRemoveRow == true );
    QVERIFY( graphListener.afterRemoveRow == true );
    QVERIFY( node3->childCount() == 0 );
    QVERIFY( node4->parent() == 0 );
    QVERIFY( root->childCount() == 3 );
    QVERIFY( root->lastChild() == node2 );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( node4->prevSibling() == 0 );
    QVERIFY( node4->nextSibling() == 0 );
}


QTEST_KDEMAIN(KisNodeFacadeTest, NoGUI)
#include "kis_node_facade_test.moc"


