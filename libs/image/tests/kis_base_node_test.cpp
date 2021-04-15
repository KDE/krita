/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_base_node_test.h"
#include <simpletest.h>
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
    const int halfTimeOffset = (timeB - timeA) / 2;

    // Keyframe values are stored in percent,
    // but opacity values are stored as bytes
    // Therefore, we should convert all percentage
    // values to bytes for comparison testing.
    const qreal valueA = 50;
    const quint8 valueAU8 = (valueA / 100) * 255;
    const qreal valueB = 25;
    const quint8 valueBU8 = (valueB / 100) * 255;

    // Interpolated value lands on frame edge, so value isn't quite in the middle..
    const qreal valueDeltaPerFrame = (valueA - valueB) / qreal(timeB - timeA);
    const qreal interpolatedValueAB = valueB + valueDeltaPerFrame * halfTimeOffset;
    const quint8 interpValueABU8 = (interpolatedValueAB / 100) * 255;

    // Add frames..
    opacityChannel->addScalarKeyframe(timeA, valueA);
    opacityChannel->addScalarKeyframe(timeB, valueB);

    QVERIFY(opacityChannel->keyframeCount() == 2);

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
        QCOMPARE(opacityChannel->valueAt(0), valueA);
        QCOMPARE(sample.opacityU8(), valueAU8);
    }

    {   // A
        context.image->animationInterface()->switchCurrentTimeAsync(timeA);
        context.image->waitForDone();

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(opacityChannel->valueAt(timeA), valueA);
        QCOMPARE(sample.opacityU8(), valueAU8);
    }

    {   // Between A-B (Opacity interpolated)
        context.image->animationInterface()->switchCurrentTimeAsync(timeA + halfTimeOffset);
        context.image->waitForDone();

        QCOMPARE(opacityChannel->valueAt(timeA + halfTimeOffset), interpolatedValueAB);

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), interpValueABU8);

        // Restore opacity and double check color continuity..
        sample.setOpacity(originalColor.opacityU8());
        QCOMPARE(sample, originalColor);
    }

    {   // B
        context.image->animationInterface()->switchCurrentTimeAsync(timeB);
        context.image->waitForDone();

        QCOMPARE(opacityChannel->valueAt(timeB), valueB);

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueBU8);
    }

    {   // After B (Opacity should be the same as B!)
        context.image->animationInterface()->switchCurrentTimeAsync(timeB + halfTimeOffset);
        context.image->waitForDone();

        QCOMPARE(opacityChannel->valueAt(timeB + halfTimeOffset), valueB);

        KoColor sample;
        context.image->projection()->pixel(16, 16, &sample);
        QCOMPARE(sample.opacityU8(), valueBU8);
    }
}

SIMPLE_TEST_MAIN(KisBaseNodeTest)


