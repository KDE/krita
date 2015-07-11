/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_animation_interface_test.h"

#include <qtest_kde.h>

#include <testutil.h>
#include <KoColor.h>

#include "kis_debug.h"

#include "kis_image_animation_interface.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_time_range.h"


void checkFrame(KisImageAnimationInterface *i, int frameId, bool externalFrameActive, const QRect &rc)
{
    QCOMPARE(i->currentTime(), frameId);
    QCOMPARE(i->externalFrameActive(), externalFrameActive);
    QCOMPARE(i->frameProjection()->exactBounds(), rc);
}

void KisImageAnimationInterfaceTest::testFrameRegeneration()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    const QRect rc1(101,101,100,100);
    const QRect rc2(102,102,100,100);
    const QRect rc3(103,103,100,100);
    const QRect rc4(104,104,100,100);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();
    KisPaintDeviceSP dev2 = layer2->paintDevice();

    // check frame 0
    {
        dev1->fill(rc1, KoColor(Qt::red, dev1->colorSpace()));
        QCOMPARE(dev1->exactBounds(), rc1);

        dev2->fill(rc2, KoColor(Qt::green, dev1->colorSpace()));
        QCOMPARE(dev2->exactBounds(), rc2);

        p.image->refreshGraph();
        checkFrame(i, 0, false, rc1 | rc2);
    }

    // switch/create frame 10
    i->switchCurrentTimeAsync(10);
    p.image->waitForDone();

    KisKeyframeChannel *channel1 = dev1->keyframeChannel();
    channel1->addKeyframe(10);

    KisKeyframeChannel *channel2 = dev2->keyframeChannel();
    channel2->addKeyframe(10);


    // check frame 10
    {
        QVERIFY(dev1->exactBounds().isEmpty());
        QVERIFY(dev2->exactBounds().isEmpty());

        dev1->fill(rc3, KoColor(Qt::red, dev2->colorSpace()));
        QCOMPARE(dev1->exactBounds(), rc3);

        dev2->fill(rc4, KoColor(Qt::green, dev2->colorSpace()));
        QCOMPARE(dev2->exactBounds(), rc4);

        p.image->refreshGraph();
        checkFrame(i, 10, false, rc3 | rc4);
    }


    // check external frame (frame 0)
    {
        SignalToFunctionProxy proxy1(boost::bind(checkFrame, i, 0, true, rc1 | rc2));
        connect(i, SIGNAL(sigFrameReady()), &proxy1, SLOT(start()), Qt::DirectConnection);
        i->requestFrameRegeneration(0, QRegion(refRect));
        QTest::qWait(200);
    }

    // current frame (flame 10) is still unchanged
    checkFrame(i, 10, false, rc3 | rc4);

    // switch back to frame 0
    i->switchCurrentTimeAsync(0);
    p.image->waitForDone();

    // check frame 0
    {
        QCOMPARE(dev1->exactBounds(), rc1);
        QCOMPARE(dev2->exactBounds(), rc2);

        checkFrame(i, 0, false, rc1 | rc2);
    }

    // check external frame (frame 10)
    {
        SignalToFunctionProxy proxy2(boost::bind(checkFrame, i, 10, true, rc3 | rc4));
        connect(i, SIGNAL(sigFrameReady()), &proxy2, SLOT(start()), Qt::DirectConnection);
        i->requestFrameRegeneration(10, QRegion(refRect));
        QTest::qWait(200);
    }

    // current frame is still unchanged
    checkFrame(i, 0, false, rc1 | rc2);
}

void KisImageAnimationInterfaceTest::testFramesChangedSignal()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();
    KisPaintDeviceSP dev2 = layer2->paintDevice();

    KisKeyframeChannel *channel = dev2->keyframeChannel();
    channel->addKeyframe(10);
    channel->addKeyframe(20);

    p.image->animationInterface()->switchCurrentTimeAsync(15);
    p.image->waitForDone();

    QSignalSpy spy(i, SIGNAL(sigFramesChanged(KisTimeRange,QRect)));

    i->notifyNodeChanged(layer1.data(), QRect(), false);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KisTimeRange>(), KisTimeRange::infinite(0));

    i->notifyNodeChanged(layer2.data(), QRect(), false);

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KisTimeRange>(), KisTimeRange(10, 10));

    // Recursive

    channel = dev1->keyframeChannel();
    channel->addKeyframe(13);

    spy.clear();
    i->notifyNodeChanged(p.image->root().data(), QRect(), true);

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KisTimeRange>(), KisTimeRange::infinite(10));

}

QTEST_KDEMAIN(KisImageAnimationInterfaceTest, GUI)
