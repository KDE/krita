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

#include <QTest>

#include <testutil.h>
#include <KoColor.h>

#include "kundo2command.h"

#include "kis_debug.h"

#include "kis_image_animation_interface.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_time_range.h"


void checkFrame(KisImageAnimationInterface *i, KisImageSP image, int frameId, bool externalFrameActive, const QRect &rc)
{
    QCOMPARE(i->currentTime(), frameId);
    QCOMPARE(i->externalFrameActive(), externalFrameActive);
    QCOMPARE(image->projection()->exactBounds(), rc);
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

    p.layer->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
    layer2->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    // check frame 0
    {
        dev1->fill(rc1, KoColor(Qt::red, dev1->colorSpace()));
        QCOMPARE(dev1->exactBounds(), rc1);

        dev2->fill(rc2, KoColor(Qt::green, dev1->colorSpace()));
        QCOMPARE(dev2->exactBounds(), rc2);

        p.image->refreshGraph();
        checkFrame(i, p.image, 0, false, rc1 | rc2);
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
        checkFrame(i, p.image, 10, false, rc3 | rc4);
    }


    // check external frame (frame 0)
    {
        SignalToFunctionProxy proxy1(std::bind(checkFrame, i, p.image, 0, true, rc1 | rc2));
        connect(i, SIGNAL(sigFrameReady(int)), &proxy1, SLOT(start()), Qt::DirectConnection);
        i->requestFrameRegeneration(0, QRegion(refRect));
        QTest::qWait(200);
    }

    // current frame (flame 10) is still unchanged
    checkFrame(i, p.image, 10, false, rc3 | rc4);

    // switch back to frame 0
    i->switchCurrentTimeAsync(0);
    p.image->waitForDone();

    // check frame 0
    {
        QCOMPARE(dev1->exactBounds(), rc1);
        QCOMPARE(dev2->exactBounds(), rc2);

        checkFrame(i, p.image, 0, false, rc1 | rc2);
    }

    // check external frame (frame 10)
    {
        SignalToFunctionProxy proxy2(std::bind(checkFrame, i, p.image, 10, true, rc3 | rc4));
        connect(i, SIGNAL(sigFrameReady(int)), &proxy2, SLOT(start()), Qt::DirectConnection);
        i->requestFrameRegeneration(10, QRegion(refRect));
        QTest::qWait(200);
    }

    // current frame is still unchanged
    checkFrame(i, p.image, 0, false, rc1 | rc2);
}

void KisImageAnimationInterfaceTest::testFramesChangedSignal()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    layer1->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
    layer2->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();
    KisPaintDeviceSP dev2 = layer2->paintDevice();

    KisKeyframeChannel *channel = dev2->keyframeChannel();
    channel->addKeyframe(10);
    channel->addKeyframe(20);

    // check switching a frame doesn't invalidate cache
    QSignalSpy spy(i, SIGNAL(sigFramesChanged(KisFrameSet,QRect)));

    p.image->animationInterface()->switchCurrentTimeAsync(15);
    p.image->waitForDone();

    QCOMPARE(spy.count(), 0);

    i->notifyNodeChanged(layer1.data(), QRect(), false);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KisFrameSet>(), KisFrameSet::infiniteFrom(0));

    i->notifyNodeChanged(layer2.data(), QRect(), false);

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<KisFrameSet>(), KisFrameSet::between(10, 19));

    // Recursive

    channel = dev1->keyframeChannel();
    channel->addKeyframe(13);

    spy.clear();
    i->notifyNodeChanged(p.image->root().data(), QRect(), true);

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    const KisFrameSet &value = arguments.at(0).value<KisFrameSet>();
    QCOMPARE(value, KisFrameSet::infiniteFrom(0));

}

void KisImageAnimationInterfaceTest::testAnimationCompositionBug()
{
    QRect rect(QRect(0,0,512,512));
    TestUtil::MaskParent p(rect);

    KUndo2Command parentCommand;

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    p.image->addNode(layer2);

    layer1->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
    layer2->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    layer1->paintDevice()->fill(rect, KoColor(Qt::red, layer1->paintDevice()->colorSpace()));
    layer2->paintDevice()->fill(QRect(128,128,128,128), KoColor(Qt::black, layer2->paintDevice()->colorSpace()));

    KisKeyframeChannel *rasterChannel = layer2->getKeyframeChannel(KisKeyframeChannel::Content.id());
    rasterChannel->addKeyframe(10, &parentCommand);
    p.image->refreshGraph();

    m_image = p.image;
    connect(p.image->animationInterface(), SIGNAL(sigFrameReady(int)), this, SLOT(slotFrameDone()), Qt::DirectConnection);
    p.image->animationInterface()->requestFrameRegeneration(5, rect);
    QTest::qWait(200);

    KisPaintDeviceSP tmpDevice = new KisPaintDevice(p.image->colorSpace());
    tmpDevice->fill(rect, KoColor(Qt::red, tmpDevice->colorSpace()));
    tmpDevice->fill(QRect(128,128,128,128), KoColor(Qt::black, tmpDevice->colorSpace()));
    QImage expected = tmpDevice->createThumbnail(512, 512);

    QVERIFY(m_compositedFrame == expected);
}

void KisImageAnimationInterfaceTest::slotFrameDone()
{
    m_compositedFrame = m_image->projection()->createThumbnail(512, 512);
}

void KisImageAnimationInterfaceTest::testSwitchFrameWithUndo()
{
        QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer1 = p.layer;

    layer1->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();

    KisKeyframeChannel *channel = dev1->keyframeChannel();
    channel->addKeyframe(10);
    channel->addKeyframe(20);


    QCOMPARE(i->currentTime(), 0);

    i->requestTimeSwitchWithUndo(15);
    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 15);

    i->requestTimeSwitchWithUndo(16);
    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 16);

    // the two commands have been merged!
    p.undoStore->undo();
    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 0);

    p.undoStore->redo();
    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 16);
}
#include "kis_processing_applicator.h"
void KisImageAnimationInterfaceTest::testSwitchFrameHangup()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);

    KisPaintLayerSP layer1 = p.layer;

    layer1->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP dev1 = p.layer->paintDevice();

    KisKeyframeChannel *channel = dev1->keyframeChannel();
    channel->addKeyframe(10);
    channel->addKeyframe(20);


    QCOMPARE(i->currentTime(), 0);

    i->requestTimeSwitchWithUndo(15);
    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 15);

    KisProcessingApplicator applicator(p.image, 0);

    i->requestTimeSwitchWithUndo(16);

    applicator.end();

    QTest::qWait(100);
    p.image->waitForDone();
    QCOMPARE(i->currentTime(), 16);


}

QTEST_MAIN(KisImageAnimationInterfaceTest)
