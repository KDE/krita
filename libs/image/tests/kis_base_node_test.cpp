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

#include "kis_base_node_test.h"
#include <QTest>
#include <limits.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kis_base_node.h"
#include "kis_paint_device.h"
#include "testutil.h"
#include "kis_scalar_keyframe_channel.h"
#include "KoColor.h"
#include "kis_image_animation_interface.h"
#include <sdk/tests/testing_nodes.h>

#include <KoProperties.h>

class TestNode : public TestUtil::DefaultNode
{
    using KisBaseNode::accept;

    KisNodeSP clone() const override {
        return new TestNode(*this);
    }
};

void KisBaseNodeTest::testCreation()
{
    KisBaseNodeSP node = new TestNode();
    QVERIFY(node->name().isEmpty());
    QVERIFY(node->name() == node->objectName());
    QVERIFY(node->icon().isNull());
    QVERIFY(node->visible() == true);
    QVERIFY(node->userLocked() == false);
    QVERIFY(node->x() == 0);
    QVERIFY(node->y() == 0);
}

void KisBaseNodeTest::testContract()
{
    KisBaseNodeSP node = new TestNode();

    node->setName("bla");
    QVERIFY(node->name()  == "bla");
    QVERIFY(node->objectName() == "bla");

    node->setObjectName("zxc");
    QVERIFY(node->name()  == "zxc");
    QVERIFY(node->objectName() == "zxc");

    node->setVisible(!node->visible());
    QVERIFY(node->visible() == false);

    node->setUserLocked(!node->userLocked());
    QVERIFY(node->userLocked() == true);

    KisBaseNode::PropertyList list = node->sectionModelProperties();
    QVERIFY(list.count() == 2);
    QVERIFY(list.at(0).state == node->visible());
    QVERIFY(list.at(1).state == node->userLocked());

    QImage image = node->createThumbnail(10, 10);
    QCOMPARE(image.size(), QSize(10, 10));
    QVERIFY(image.pixel(5, 5) == QColor(0, 0, 0, 0).rgba());

}

void KisBaseNodeTest::testProperties()
{
    KisBaseNodeSP node = new TestNode();

    {
        KoProperties props;

        props.setProperty("bladiebla", false);
        QVERIFY(node->check(props));

        props.setProperty("visible", true);
        props.setProperty("locked", false);
        QVERIFY(node->check(props));

        props.setProperty("locked", true);
        QVERIFY(!node->check(props));

        node->setNodeProperty("locked", false);
        QVERIFY(node->userLocked() == false);
    }
    {
        KoProperties props;
        props.setProperty("blablabla", 10);
        node->mergeNodeProperties(props);

        QVERIFY(node->nodeProperties().intProperty("blablabla") == 10);
        QVERIFY(node->check(props));
        props.setProperty("blablabla", 12);
        QVERIFY(!node->check(props));
    }
}

void KisBaseNodeTest::testOpacityKeyframing()
{
    TestUtil::MaskParent p;

    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    KisPaintDeviceSP dev1 = p.layer->paintDevice();
    dev1->fill(QRect(0,0,32,32), KoColor(Qt::red, dev1->colorSpace()));

    KisPaintDeviceSP dev2 = layer2->paintDevice();
    dev2->fill(QRect(0,0,32,32), KoColor(Qt::green, dev2->colorSpace()));

    layer2->setOpacity(192);

    KisKeyframeChannel *channel = layer2->getKeyframeChannel(KisKeyframeChannel::Opacity.id(), true);
    KisScalarKeyframeChannel *opacityChannel = dynamic_cast<KisScalarKeyframeChannel*>(channel);
    QVERIFY(opacityChannel);

    KisKeyframeSP key1 = opacityChannel->addKeyframe(7);
    opacityChannel->setScalarValue(key1, 128);

    KisKeyframeSP key2 = opacityChannel->addKeyframe(20);
    opacityChannel->setScalarValue(key2, 64);

    p.image->refreshGraph();

    // No interpolation

    key1->setInterpolationMode(KisKeyframe::Constant);

    QColor sample;
    p.image->projection()->pixel(16, 16, &sample);
    QCOMPARE(sample, QColor(63, 192, 0, 255));

    p.image->animationInterface()->switchCurrentTimeAsync(10);
    p.image->waitForDone();

    p.image->projection()->pixel(16, 16, &sample);
    QCOMPARE(sample, QColor(127, 128, 0, 255));

    p.image->animationInterface()->switchCurrentTimeAsync(30);
    p.image->waitForDone();

    layer2->setOpacity(32);
    QCOMPARE(opacityChannel->scalarValue(key2), 32.0);

    p.image->waitForDone();
    p.image->projection()->pixel(16, 16, &sample);
    QCOMPARE(sample, QColor(223, 32, 0, 255));

    // With interpolation

    key1->setInterpolationMode(KisKeyframe::Linear);
    key1->setInterpolationTangents(QPointF(), QPointF(0,0));
    key2->setInterpolationTangents(QPointF(0,0), QPointF());

    p.image->animationInterface()->switchCurrentTimeAsync(10);
    p.image->waitForDone();
    p.image->projection()->pixel(16, 16, &sample);

    QCOMPARE(sample, QColor(150, 105, 0, 255));

}

QTEST_MAIN(KisBaseNodeTest)


