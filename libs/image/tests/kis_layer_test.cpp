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
#include <QTest>

#include <QRect>
#include <QIcon>
#include <QBitArray>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_filter_mask.h"
#include "kis_transparency_mask.h"

#include "kis_group_layer.h"
#include "kis_paint_layer.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"


void KisLayerTest::testCreation()
{

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP layer = new TestLayer(image, "test", OPACITY_OPAQUE_U8);
    QCOMPARE(layer->name(), QString("test"));
    QCOMPARE(layer->opacity(), OPACITY_OPAQUE_U8);
    QCOMPARE(layer->image().data(), image.data());
    QCOMPARE(layer->colorSpace(), image->colorSpace());
    QCOMPARE(layer->visible(), true);
    QCOMPARE(layer->userLocked(), false);
    QCOMPARE(layer->temporary(), false);

    image->addNode(layer, image->rootLayer());

    QBitArray channels(4);
    channels.fill(true);
    channels.setBit(1, false);
    layer->setChannelFlags(channels);
    QVERIFY(layer->channelFlags().count() == 4);
    QCOMPARE(layer->channelFlags().at(0), true);
    QCOMPARE(layer->channelFlags().at(1), false);
    QCOMPARE(layer->channelFlags().at(2), true);
    QCOMPARE(layer->channelFlags().at(3), true);


    layer->setOpacity(OPACITY_TRANSPARENT_U8);
    QCOMPARE(layer->opacity(), OPACITY_TRANSPARENT_U8);
    layer->setPercentOpacity(100);
    QCOMPARE(layer->opacity(), OPACITY_OPAQUE_U8);
    layer->setPercentOpacity(0);
    QCOMPARE(layer->opacity(), OPACITY_TRANSPARENT_U8);


}

void KisLayerTest::testOrdering()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP layer1 = new TestLayer(image, "layer1", OPACITY_OPAQUE_U8);
    KisLayerSP layer2 = new TestLayer(image, "layer2", OPACITY_OPAQUE_U8);
    KisLayerSP layer3 = new TestLayer(image, "layer3", OPACITY_OPAQUE_U8);

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
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP node1 = new TestLayer(image, "layer1", OPACITY_OPAQUE_U8);
    KisLayerSP node2 = new TestLayer(image, "layer2", OPACITY_OPAQUE_U8);
    KisLayerSP node3 = new TestLayer(image, "layer3", OPACITY_OPAQUE_U8);

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
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "layer test");
    image->lock();

    KisLayerSP node1 = new TestLayer(image, "layer1", OPACITY_OPAQUE_U8);
    KisLayerSP node2 = new TestLayer(image, "layer2", OPACITY_OPAQUE_U8);
    KisLayerSP node3 = new TestLayer(image, "layer3", OPACITY_OPAQUE_U8);
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

    /*
      +----------+
      |root      |
      | paint 1  |
      |  fmask2  |
      |  fmask1  |
      +----------+
     */

void KisLayerTest::testMasksChangeRect()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "walker test");

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer1, image->rootLayer());

    KisFilterMaskSP filterMask1 = new KisFilterMask();
    KisFilterMaskSP filterMask2 = new KisFilterMask();

    KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(filter);
    KisFilterConfigurationSP configuration1 = filter->defaultConfiguration();
    KisFilterConfigurationSP configuration2 = filter->defaultConfiguration();

    filterMask1->setFilter(configuration1);
    filterMask2->setFilter(configuration2);

    image->addNode(filterMask1, paintLayer1);
    image->addNode(filterMask2, paintLayer1);

    QVERIFY(paintLayer1->hasEffectMasks());

    QRect testRect(10, 10, 100, 100);
    QRect resultRect;

    resultRect = paintLayer1->changeRect(testRect, KisNode::N_FILTHY);
    QVERIFY2(resultRect == QRect(0, 0, 120, 120),
              "KisNode::N_FILTHY node should take masks into account");

    resultRect = paintLayer1->changeRect(testRect, KisNode::N_ABOVE_FILTHY);
    QVERIFY2(resultRect == testRect,
              "KisNode::N_ABOVE_FILTHY node should NOT take "
              "masks into account");

    /**
     * KisNode::N_BELOW_FILTHY, KisNode::N_FILTHY_PROJECTION
     * should not be use by the caller, because the walker
     * should not visit these node on a forward way.
     * So the behavior here is undefined.
     *
     * resultRect = paintLayer1->changeRect(testRect, KisNode::N_BELOW_FILTHY);
     * resultRect = paintLayer1->changeRect(testRect, KisNode::N_FILTHY_PROJECTION);
     */

}

void KisLayerTest::testMoveLayerWithMaskThreaded()
{
    /**
     * This test ensures that the layer's original() can be moved
     * while its projection is still being updated
     */

    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 2000, 2000, colorSpace, "walker test");

    KisLayerSP paintLayer = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    image->addNode(paintLayer, image->rootLayer());

    paintLayer->paintDevice()->fill(image->bounds(), KoColor(Qt::black, colorSpace));

    KisTransparencyMaskSP transpMask = new KisTransparencyMask();
    transpMask->initSelection(paintLayer);
    image->addNode(transpMask, paintLayer);

    for(int i = 0; i < 100; i++) {
        paintLayer->setDirty();

        QTest::qSleep(1 + (qrand() & 63));

        paintLayer->setX((i*67) % 1873);
        paintLayer->setY((i*23) % 1873);
    }
}


QTEST_MAIN(KisLayerTest)

