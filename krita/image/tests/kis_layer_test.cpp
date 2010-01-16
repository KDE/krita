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

#include "kis_layer_test.h"
#include <qtest_kde.h>

#include <QRect>
#include <QIcon>
#include <QBitArray>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_filter_mask.h"

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_group_layer.h"


void KisLayerTest::testCreation()
{

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP layer = new TestLayer(image, "test", OPACITY_OPAQUE);
    QCOMPARE(layer->name(), QString("test"));
    QCOMPARE(layer->opacity(), OPACITY_OPAQUE);
    QCOMPARE(layer->image(), image);
    QCOMPARE(layer->colorSpace(), image->colorSpace());
    QCOMPARE(layer->visible(), true);
    QCOMPARE(layer->userLocked(), false);
    QCOMPARE(layer->temporary(), false);

    image->addNode(layer, image->rootLayer());

    QBitArray channels(4);
    channels.fill(true);
    layer->setChannelFlags(channels);
    QVERIFY(layer->channelFlags().count() == 4);
    QCOMPARE(layer->channelFlags().at(0), true);
    QCOMPARE(layer->channelFlags().at(1), true);
    QCOMPARE(layer->channelFlags().at(2), true);
    QCOMPARE(layer->channelFlags().at(3), true);


    layer->setOpacity(OPACITY_TRANSPARENT);
    QCOMPARE(layer->opacity(), OPACITY_TRANSPARENT);
    layer->setPercentOpacity(100);
    QCOMPARE(layer->opacity(), OPACITY_OPAQUE);
    layer->setPercentOpacity(0);
    QCOMPARE(layer->opacity(), OPACITY_TRANSPARENT);


}

void KisLayerTest::testOrdering()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP layer1 = new TestLayer(image, "layer1", OPACITY_OPAQUE);
    KisLayerSP layer2 = new TestLayer(image, "layer2", OPACITY_OPAQUE);
    KisLayerSP layer3 = new TestLayer(image, "layer3", OPACITY_OPAQUE);

    QVERIFY(layer1->name() == "layer1");
    QVERIFY(layer2->name() == "layer2");
    QVERIFY(layer3->name() == "layer3");

    /*
      +---------+
      | layer 2 |
      | layer 3 |
      | layer 1 |
      |root     |
      +---------+
     */

    QVERIFY(image->addNode(layer1, image->rootLayer()));
    QVERIFY(image->addNode(layer2, image->rootLayer()));
    QVERIFY(image->addNode(layer3, image->rootLayer(), layer1));

    QCOMPARE((int) image->nlayers(), 4);

    QVERIFY(layer1->parent() == image->root());
    QVERIFY(layer2->parent() == image->root());
    QVERIFY(layer3->parent() == image->root());

    QVERIFY(image->rootLayer()->firstChild() == layer1.data());
    QVERIFY(image->rootLayer()->lastChild() == layer2.data());

    QVERIFY(image->rootLayer()->at(0) == layer1.data());
    QVERIFY(image->rootLayer()->at(1) == layer3.data());
    QVERIFY(image->rootLayer()->at(2) == layer2.data());

    QVERIFY(image->rootLayer()->index(layer1) == 0);
    QVERIFY(image->rootLayer()->index(layer3) == 1);
    QVERIFY(image->rootLayer()->index(layer2) == 2);

    QVERIFY(layer3->prevSibling() == layer1.data());
    QVERIFY(layer2->prevSibling() == layer3.data());
    QVERIFY(layer1->prevSibling() == 0);

    QVERIFY(layer3->nextSibling() == layer2.data());
    QVERIFY(layer2->nextSibling() == 0);
    QVERIFY(layer1->nextSibling() == layer3.data());


    /*
      +---------+
      | layer 3 |
      | layer 2 |
      | layer 1 |
      |root     |
      +---------+
     */
    QVERIFY(image->moveNode(layer2, image->rootLayer(), layer1));

    QVERIFY(image->rootLayer()->at(0) == layer1.data());
    QVERIFY(image->rootLayer()->at(1) == layer2.data());
    QVERIFY(image->rootLayer()->at(2) == layer3.data());

    QVERIFY(image->rootLayer()->firstChild() == layer1.data());
    QVERIFY(image->rootLayer()->lastChild() == layer3.data());

    QVERIFY(image->rootLayer()->index(layer1) == 0);
    QVERIFY(image->rootLayer()->index(layer2) == 1);
    QVERIFY(image->rootLayer()->index(layer3) == 2);

    QVERIFY(layer3->prevSibling() == layer2.data());
    QVERIFY(layer2->prevSibling() == layer1.data());
    QVERIFY(layer1->prevSibling() == 0);

    QVERIFY(layer3->nextSibling() == 0);
    QVERIFY(layer2->nextSibling() == layer3.data());
    QVERIFY(layer1->nextSibling() == layer2.data());

}


void KisLayerTest::testMoveNode()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP node1 = new TestLayer(image, "layer1", OPACITY_OPAQUE);
    KisLayerSP node2 = new TestLayer(image, "layer2", OPACITY_OPAQUE);
    KisLayerSP node3 = new TestLayer(image, "layer3", OPACITY_OPAQUE);

    node1->setName("node1");
    node2->setName("node2");
    node3->setName("node3");

    QVERIFY(image->addNode(node1));
    QVERIFY(image->addNode(node2));
    QVERIFY(image->addNode(node3));

    QVERIFY(image->root()->at(0) == node1.data());
    QVERIFY(image->root()->at(1) == node2.data());
    QVERIFY(image->root()->at(2) == node3.data());

    QVERIFY(image->moveNode(node3, image->root(), node1));

    QVERIFY(image->root()->at(0) == node1.data());
    QVERIFY(image->root()->at(1) == node3.data());
    QVERIFY(image->root()->at(2) == node2.data());

}


void KisLayerTest::testMoveLayer()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageWSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP node1 = new TestLayer(image, "layer1", OPACITY_OPAQUE);
    KisLayerSP node2 = new TestLayer(image, "layer2", OPACITY_OPAQUE);
    KisLayerSP node3 = new TestLayer(image, "layer3", OPACITY_OPAQUE);
    node1->setName("node1");
    node2->setName("node2");
    node3->setName("node3");

    QVERIFY(image->addNode(node1));
    QVERIFY(image->addNode(node2));
    QVERIFY(image->addNode(node3));

    QVERIFY(image->root()->at(0) == node1.data());
    QVERIFY(image->root()->at(1) == node2.data());
    QVERIFY(image->root()->at(2) == node3.data());

    QVERIFY(image->moveNode(node3, image->rootLayer(), node1));

    QVERIFY(image->root()->at(0) == node1.data());
    QVERIFY(image->root()->at(1) == node3.data());
    QVERIFY(image->root()->at(2) == node2.data());

}

void KisLayerTest::testHasEffectMasks()
{
    KisLayerSP layer = new TestLayer(0, "layer1", OPACITY_OPAQUE) ;
    QVERIFY(layer->hasEffectMasks() == false);
    KisFilterMaskSP mask = new KisFilterMask();
    layer->setPreviewMask(mask);
    QVERIFY(layer->hasEffectMasks());
    layer->removePreviewMask();
    QVERIFY(layer->hasEffectMasks() == false);
}


QTEST_KDEMAIN(KisLayerTest, NoGUI)
#include "kis_layer_test.moc"

