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

#include <QRect>
#include <QIcon>
#include <QBitArray>

#include "kis_node_test.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_effect_mask.h"

#include "kis_types.h"
#include "kis_node.h"
#include "kis_image.h"

class TestNode : public KisNode {

public:

    TestNode( KisImageWSP image, const QString & name, quint8 opacity )
        : KisNode( image, name, opacity )
        {
        }


    virtual QString nodeType()
        {
            return "TEST";
        }

    virtual bool canHaveChildren()
        {
            return false;
        }

    void updateProjection(const QRect&)
        {
        }

    KisPaintDeviceSP projection() const
        {
            return 0;
        }

    KisPaintDeviceSP paintDevice() const
        {
            return 0;
        }

    QIcon icon() const
        {
            return QIcon();
        }

    KisNodeSP clone() const
        {
            return new TestNode(image(), name(), opacity());
        }

    qint32 x() const
        {
            return 0;
        }

    void setX(qint32)
        {
        }

    qint32 y() const
        {
            return 0;
        }

    void setY(qint32)
        {
        }

    QRect extent() const
        {
            return QRect();
        }

    QRect exactBounds() const
        {
            return QRect();
        }

    bool accept(KisNodeVisitor&)
        {
            return false;
        }


};

void KisNodeTest::testCreation()
{

    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "merge test");

    KisNodeSP node = new TestNode( image, "test", OPACITY_OPAQUE );
    QCOMPARE( node->name(), QString( "test" ) );
    QCOMPARE( node->opacity(), OPACITY_OPAQUE );
    QCOMPARE( node->image(), image );
    QCOMPARE( node->colorSpace(), image->colorSpace() );
    QCOMPARE( node->visible(), true );
    QCOMPARE( node->locked(), false );
    QCOMPARE( node->temporary(), false );

    image->addNode( node, image->rootNode() );

    QBitArray channels( 4 );
    channels.fill( true );
    node->setChannelFlags( channels );
    QVERIFY(node->channelFlags().count() == 4);
    QCOMPARE( node->channelFlags().at( 0 ), true );
    QCOMPARE( node->channelFlags().at( 1 ), true );
    QCOMPARE( node->channelFlags().at( 2 ), true );
    QCOMPARE( node->channelFlags().at( 3 ), true );


    node->setOpacity( OPACITY_TRANSPARENT );
    QCOMPARE( node->opacity(), OPACITY_TRANSPARENT );
    node->setPercentOpacity( 100 );
    QCOMPARE( node->opacity(), OPACITY_OPAQUE );
    node->setPercentOpacity( 0 );
    QCOMPARE( node->opacity(), OPACITY_TRANSPARENT );


}

void KisNodeTest::testOrdering()
{
    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace( "RGBA", 0 );
    KisImageSP image = new KisImage( 0, 512, 512, colorSpace, "merge test" );

    KisNodeSP node1 = new TestNode( image, "test", OPACITY_OPAQUE );
    KisNodeSP node2 = new TestNode( image, "test", OPACITY_OPAQUE );
    KisNodeSP node3 = new TestNode( image, "test", OPACITY_OPAQUE );

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

}


QTEST_KDEMAIN(KisNodeTest, NoGUI)
#include "kis_node_test.moc"


