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
#include <testutil.h>
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
    TestUtil::MaskParent context;

    // Get/create channel..
    KisScalarKeyframeChannel *opacityChannel = dynamic_cast<KisScalarKeyframeChannel*>(
                context.layer->getKeyframeChannel(KisKeyframeChannel::Opacity.id(), true));
    QVERIFY(opacityChannel);
    QVERIFY(opacityChannel->limits());

    const int timeA = 7;
    const int timeB = 15;
    const int half_offset = (timeB - timeA) / 2;

    const qreal valueA = 128;
    const qreal valueB = 64;
    // Interpolated value lands on frame edge, so value isn't quite in the middle..
    const qreal valueDeltaPerFrame = (valueA - valueB) / qreal(timeB - timeA);
    const qreal interpolatedValueAB = valueB + valueDeltaPerFrame * half_offset;

    // Add frames..
    opacityChannel->addScalarKeyframe(timeA, 128);
    opacityChannel->addScalarKeyframe(timeB, 64);

    // Paint starting color..
    const KoColorSpace *colorSpace = context.layer->paintDevice()->colorSpace();
    const KoColor originalColor = KoColor(Qt::red, colorSpace);
    context.layer->paintDevice()->fill(context.imageRect, originalColor);

    // Regenerate projection..
    context.image->refreshGraph();

    {   // Before A (Opacity should be the same as A!)
        context.image->animationInterface()->switchCurrentTimeAsync(0);
        context.image->waitForDone();

        KoColor sample(colorSpace);
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueA);
    }

    {   // A
        context.image->animationInterface()->switchCurrentTimeAsync(timeA);
        context.image->waitForDone();

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueA);
    }

    {   // Between A-B (Opacity interpolated)
        context.image->animationInterface()->switchCurrentTimeAsync(timeA + half_offset);
        context.image->waitForDone();

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), interpolatedValueAB);

        // Restore opacity and double check color continuity..
        sample.setOpacity(originalColor.opacityU8());
        QCOMPARE(sample, originalColor);
    }

    {   // B
        context.image->animationInterface()->switchCurrentTimeAsync(timeB);
        context.image->waitForDone();

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueB);
    }

    {   // After B (Opacity should be the same as B!)
        context.image->animationInterface()->switchCurrentTimeAsync(timeB + half_offset);
        context.image->waitForDone();

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueB);
    }
}

QTEST_MAIN(KisBaseNodeTest)


