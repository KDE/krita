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

#include "kis_node_test.h"

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

    virtual void aboutToMoveNode( KisNode *, int, int )
        {
            beforeMove = true;
        }

    virtual void nodeHasBeenMoved( KisNode *, int, int )
        {
            afterMove = true;
        }


    bool beforeInsertRow;
    bool afterInsertRow;
    bool beforeRemoveRow;
    bool afterRemoveRow;
    bool beforeMove;
    bool afterMove;

    void resetBools()
        {
            beforeRemoveRow = false;
            afterRemoveRow = false;
            beforeInsertRow = false;
            afterInsertRow = false;
            beforeMove = false;
            afterMove = false;
        }
};


void KisNodeTest::testCreation()
{
    TestGraphListener graphListener;

    KisNode * node = new KisNode();
    QVERIFY( node->graphListener() == 0 );

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

    delete node;
}

void dumpNodeStack( KisNodeSP node, QString prefix = QString( "\t" ) ) {
    for ( uint i = 0; i < node->childCount(); ++i ) {
        if ( node->at( i )->parent() )
        if ( node->at( i )->childCount() > 0 ) {
            dumpNodeStack( node->at( i ), prefix + "\t" );
        }
    }

}

void KisNodeTest::testOrdering()
{
    TestGraphListener graphListener;

    KisNodeSP root = new KisNode();
    root->setGraphListener( &graphListener );
    KisNodeSP node1 = new KisNode();
    KisNodeSP node2 = new KisNode();
    KisNodeSP node3 = new KisNode();
    KisNodeSP node4 = new KisNode();

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
    root->add( node1, root->lastChild() );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node1 );
    graphListener.resetBools();

    root->add( node2, root->lastChild() );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node2 );
    graphListener.resetBools();

    root->add( node3, node1 );
    QVERIFY( graphListener.beforeInsertRow == true );
    QVERIFY( graphListener.afterInsertRow == true );
    QVERIFY( graphListener.beforeRemoveRow == false );
    QVERIFY( graphListener.afterRemoveRow == false );
    QVERIFY( root->firstChild() == node1 );
    QVERIFY( root->lastChild() == node2 );
    graphListener.resetBools();

    root->add( node4, root->lastChild() );
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
    QVERIFY( root->remove( root->at( 3 ) ) == true );
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

    node3->add( node4, node3->lastChild() );
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
    QVERIFY( root->remove( node4 ) == false );
    graphListener.resetBools();

    node3->remove( node4 );
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
    KisNodeSP root = new KisNode();
    root->setName( "root" );

    KisNodeSP node1 = new KisNode();
    node1->setName( "node1" );
    QVERIFY( root->add( node1, 0 ) );

    KisNodeSP node2 = new KisNode();
    node2->setName( "node2" );
    QVERIFY( root->add( node2, node1 ) );

    KisNodeSP node3 = new KisNode();
    node3->setName( "node3" );
    QVERIFY( node1->add( node3, 0 ) );

    KisNodeSP node4 = new KisNode();
    node4->setName( "node4" );
    QVERIFY( node1->add( node4, node3 ) );

    KisNodeSP node5 = new KisNode();
    node5->setName( "node5" );
    QVERIFY( node3->add( node5, 0 ) );

    KisNodeSP node6 = new KisNode();
    node6->setName( "node6" );
    QVERIFY( node5->add( node6, 0 ) );

    KisNodeSP node7 = new KisNode();
    node7->setName( "node7" );
    QVERIFY( node6->add( node7, 0 ) );

    node1->setDirty();
    QVERIFY( node1->isDirty() );
    QVERIFY( !node2->isDirty() );
    QVERIFY( root->isDirty() );
    root->setClean();
    QVERIFY( !root->isDirty() );
    node1->setClean();
    QVERIFY( !node1->isDirty() );

    node7->setDirty( QRect( 10, 10, 100, 100 ) );
    QVERIFY( node7->isDirty() );
    QVERIFY( node7->isDirty( QRect( 5, 5, 15, 15 ) ) );
    QVERIFY( root->isDirty( QRect( 5, 5, 15, 15 ) ) );
    QVERIFY( !root->isDirty( QRect( -10, -10, 20, 20 ) ) );
    QVERIFY( !node2->isDirty() );

    node7->setClean( QRect( 10, 10, 10, 10 ) );
    QVERIFY( !node7->isDirty(QRect( 10, 10, 10, 10 )) );
    QVERIFY( node7->isDirty() );
    QVERIFY( node7->isDirty( QRect( 0, 0, 50, 50 ) ) );


}


void KisNodeTest::testChildNodes()
{
    KisNodeSP root = new KisNode();
    KisNodeSP a = new TestNodeA();
    root->add( a, 0 );
    a->setVisible( true );
    a->setLocked( true );

    KisNodeSP b = new TestNodeB();
    root->add( b, 0 );
    b->setVisible( false );
    b->setLocked( true );

    KisNodeSP c = new TestNodeC();
    root->add( c, 0 );
    c->setVisible( false );
    c->setVisible( false );

    QList<KisNodeSP> allNodes = root->childNodes( QStringList(), KoProperties() );
    QCOMPARE( (int) allNodes.count(), 3 ); // a, b, c

    QStringList nodeTypes;
    nodeTypes << "TestNodeA" << "TestNodeB";
    QList<KisNodeSP> subSetOfNodeTypes = root->childNodes( nodeTypes, KoProperties() );
    QVERIFY( subSetOfNodeTypes.count() == 2 ); // a, b

    nodeTypes.clear();
    nodeTypes << "TestNodeB" << "TestNodeC";
    KoProperties props;
    props.setProperty( "visibile", false );
    props.setProperty( "locked", true );
    QList<KisNodeSP> subsetOfTypesAndProps = root->childNodes( nodeTypes, props );
    QVERIFY( subsetOfTypesAndProps.count() == 1 ); // b

    KoProperties props2;
    props.setProperty( "visibile", false );
    QList<KisNodeSP> subSetOfProps = root->childNodes( QStringList(), props );
    QVERIFY( subSetOfProps.count() == 2 ); // b, c
}

QTEST_KDEMAIN(KisNodeTest, NoGUI)
#include "kis_node_test.moc"


