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

    virtual void aboutToAddANode( KisNode *parent, int index )
        {
            beforeInsertRow = true;
        }

    virtual void nodeHasBeenAdded( KisNode *parent, int index )
        {
            afterInsertRow = true;
        }

    virtual void aboutToRemoveANode( KisNode *parent, int index )
        {
            beforeRemoveRow  = true;
        }

    virtual void nodeHasBeenRemoved( KisNode *parent, int index )
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


void KisNodeTest::testCreation()
{
    TestGraphListener graphListener;

    KisNodeSP node = new KisNode();
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
}

void KisNodeTest::testOrdering()
{
#if 0
    TestGraphListener graphListener( this );

    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace( "RGBA", 0 );
    KisImageSP image = new KisImage( 0, 512, 512, colorSpace, "merge test" );

    KisNodeSP node1 = new KisNode();
    KisNodeSP node2 = new KisNode();
    KisNodeSP node3 = new KisNode();

     /*
      +---------+
      | node 2 |
      | node 3 |
      | node 1 |
      |root     |
      +---------+
     */

    image->addNode( node1, image->rootNode() );
    image->addNode( node2, image->rootNode() );
    image->addNode( node3, image->rootNode(), node1 );

    QVERIFY( image->nnodes() == 3 );

    QVERIFY( node1->parentNode() == image->rootNode() );
    QVERIFY( node2->parentNode() == image->rootNode() );
    QVERIFY( node3->parentNode() == image->rootNode() );

    QVERIFY( image->rootNode()->firstChild() == node2 );
    QVERIFY( image->rootNode()->lastChild() == node1 );

    QVERIFY( image->rootNode()->at( 0 ) == node2 );
    QVERIFY( image->rootNode()->at( 1 ) == node3 );
    QVERIFY( image->rootNode()->at( 2 ) == node1 );

    QVERIFY( image->rootNode()->index( node1 ) == 2 );
    QVERIFY( image->rootNode()->index( node3 ) == 1 );
    QVERIFY( image->rootNode()->index( node2 ) == 0 );

    QVERIFY( node3->prevSibling() == node2 );
    QVERIFY( node2->prevSibling() == 0 );
    QVERIFY( node1->prevSibling() == node3 );

    QVERIFY( node3->nextSibling() == node1 );
    QVERIFY( node2->nextSibling() == node3 );
    QVERIFY( node1->nextSibling() == 0 );


    /*
      +---------+
      | node 3 |
      | node 2 |
      | node 1 |
      |root     |
      +---------+
     */
    image->moveNode( node2, image->rootNode(), node1 );

    QVERIFY( image->rootNode()->firstChild() == node3 );
    QVERIFY( image->rootNode()->lastChild() == node1 );

    QVERIFY( image->rootNode()->at( 0 ) == node3 );
    QVERIFY( image->rootNode()->at( 1 ) == node2 );
    QVERIFY( image->rootNode()->at( 2 ) == node1 );

    QVERIFY( image->rootNode()->index( node1 ) == 2 );
    QVERIFY( image->rootNode()->index( node2 ) == 1 );
    QVERIFY( image->rootNode()->index( node3 ) == 0 );

    QVERIFY( node3->prevSibling() == 0 );
    QVERIFY( node2->prevSibling() == node3 );
    QVERIFY( node1->prevSibling() == node2 );

    QVERIFY( node3->nextSibling() == node2 );
    QVERIFY( node2->nextSibling() == node1 );
    QVERIFY( node1->nextSibling() == 0 );
#endif
}


QTEST_KDEMAIN(KisNodeTest, NoGUI)
#include "kis_node_test.moc"


