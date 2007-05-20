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
    QCOMPARE( layer->channelFlags().at( 0 ), true );

    layer->setOpacity( OPACITY_TRANSPARENT );
    QCOMPARE( layer->opacity(), OPACITY_TRANSPARENT );
    layer->setPercentOpacity( 100 );
    QCOMPARE( layer->opacity(), OPACITY_OPAQUE );
    layer->setPercentOpacity( 0 );
    QCOMPARE( layer->opacity(), OPACITY_TRANSPARENT );


}

void KisLayerTest::testOrdering()
{
}

void KisLayerTest::testEffectMasks()
{
}


QTEST_KDEMAIN(KisLayerTest, NoGUI)
#include "kis_layer_test.moc"


