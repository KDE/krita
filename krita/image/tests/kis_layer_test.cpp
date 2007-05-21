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

#include "kis_layer_test.h"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_effect_mask.h"

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_group_layer.h"

class TestLayer : public KisLayer {

public:

    TestLayer( KisImageWSP image, const QString & name, quint8 opacity )
        : KisLayer( image, name, opacity )
        {
        }

    void updateProjection(const QRect&)
        {
        }

    KisPaintDeviceSP projection()
        {
            return 0;
        }

    QIcon icon() const
        {
            return QIcon();
        }

    KisLayerSP clone() const
        {
            return new TestLayer(image(), name(), opacity());
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

    bool accept(KisLayerVisitor&)
        {
            return false;
        }


};

void KisLayerTest::testCreation()
{

    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "merge test");

    KisLayerSP layer = new TestLayer( image, "test", OPACITY_OPAQUE );
    QCOMPARE( layer->name(), QString( "test" ) );
    QCOMPARE( layer->opacity(), OPACITY_OPAQUE );
    QCOMPARE( layer->image(), image );
    QCOMPARE( layer->colorSpace(), image->colorSpace() );
    QCOMPARE( layer->isActive(), false );
    QCOMPARE( layer->visible(), true );
    QCOMPARE( layer->locked(), false );
    QCOMPARE( layer->temporary(), false );

    image->addLayer( layer, image->rootLayer() );
    QCOMPARE( layer->isActive(), true );
    image->activateLayer( layer );
    QCOMPARE( layer->isActive(), true );

    QBitArray channels( 4 );
    channels.fill( true );
    layer->setChannelFlags( channels );
    QVERIFY(layer->channelFlags().count() == 4);
    QCOMPARE( layer->channelFlags().at( 0 ), true );
    QCOMPARE( layer->channelFlags().at( 1 ), true );
    QCOMPARE( layer->channelFlags().at( 2 ), true );
    QCOMPARE( layer->channelFlags().at( 3 ), true );


    layer->setOpacity( OPACITY_TRANSPARENT );
    QCOMPARE( layer->opacity(), OPACITY_TRANSPARENT );
    layer->setPercentOpacity( 100 );
    QCOMPARE( layer->opacity(), OPACITY_OPAQUE );
    layer->setPercentOpacity( 0 );
    QCOMPARE( layer->opacity(), OPACITY_TRANSPARENT );


}

void KisLayerTest::testOrdering()
{
    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace( "RGBA", 0 );
    KisImageSP image = new KisImage( 0, 512, 512, colorSpace, "merge test" );

    KisLayerSP layer1 = new TestLayer( image, "test", OPACITY_OPAQUE );
    KisLayerSP layer2 = new TestLayer( image, "test", OPACITY_OPAQUE );
    KisLayerSP layer3 = new TestLayer( image, "test", OPACITY_OPAQUE );

    /*
      +---------+
      | layer 2 |
      | layer 3 |
      | layer 1 |
      |root     |
      +---------+
     */

    image->addLayer( layer1, image->rootLayer() );
    image->addLayer( layer2, image->rootLayer() );
    image->addLayer( layer3, image->rootLayer(), layer1 );

    QVERIFY( image->nlayers() == 3 );

    QVERIFY( layer1->parentLayer() == image->rootLayer() );
    QVERIFY( layer2->parentLayer() == image->rootLayer() );
    QVERIFY( layer3->parentLayer() == image->rootLayer() );

    QVERIFY( image->rootLayer()->firstChild() == layer2 );
    QVERIFY( image->rootLayer()->lastChild() == layer1 );

    QVERIFY( image->rootLayer()->at( 0 ) == layer2 );
    QVERIFY( image->rootLayer()->at( 1 ) == layer3 );
    QVERIFY( image->rootLayer()->at( 2 ) == layer1 );

    QVERIFY( image->rootLayer()->index( layer1 ) == 2 );
    QVERIFY( image->rootLayer()->index( layer3 ) == 1 );
    QVERIFY( image->rootLayer()->index( layer2 ) == 0 );

    QVERIFY( layer3->prevSibling() == layer2 );
    QVERIFY( layer2->prevSibling() == 0 );
    QVERIFY( layer1->prevSibling() == layer3 );

    QVERIFY( layer3->nextSibling() == layer1 );
    QVERIFY( layer2->nextSibling() == layer3 );
    QVERIFY( layer1->nextSibling() == 0 );


    /*
      +---------+
      | layer 3 |
      | layer 2 |
      | layer 1 |
      |root     |
      +---------+
     */
    image->moveLayer( layer2, image->rootLayer(), layer1 );

    QVERIFY( image->rootLayer()->firstChild() == layer3 );
    QVERIFY( image->rootLayer()->lastChild() == layer1 );

    QVERIFY( image->rootLayer()->at( 0 ) == layer3 );
    QVERIFY( image->rootLayer()->at( 1 ) == layer2 );
    QVERIFY( image->rootLayer()->at( 2 ) == layer1 );

    QVERIFY( image->rootLayer()->index( layer1 ) == 2 );
    QVERIFY( image->rootLayer()->index( layer2 ) == 1 );
    QVERIFY( image->rootLayer()->index( layer3 ) == 0 );

    QVERIFY( layer3->prevSibling() == 0 );
    QVERIFY( layer2->prevSibling() == layer3 );
    QVERIFY( layer1->prevSibling() == layer2 );

    QVERIFY( layer3->nextSibling() == layer2 );
    QVERIFY( layer2->nextSibling() == layer1 );
    QVERIFY( layer1->nextSibling() == 0 );

}

void KisLayerTest::testEffectMasks()
{
    KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->colorSpace( "RGBA", 0 );
    KisImageSP image = new KisImage( 0, 512, 512, colorSpace, "merge test" );

    KisPaintLayerSP layer1 = new KisPaintLayer( image, "test", OPACITY_OPAQUE );
//     KisEffectMask * mask1 = new KisEffectMask();
//     layer1->addEffectMask( mask1 );
//     QVERIFY( layer1->effectMasks().size() == 1 );

}


QTEST_KDEMAIN(KisLayerTest, NoGUI)
#include "kis_layer_test.moc"


