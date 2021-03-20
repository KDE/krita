/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisKeyframeAnimationInterfaceSignalTest.h"

#include <simpletest.h>
#include <qsignalspy.h>


void KisKeyframeAnimationInterfaceSignalTest::initTestCase()
{
    m_image1 = new KisImage(0, 100, 100, nullptr, "image1");
    m_image2 = new KisImage(0, 100, 100, nullptr, "image2");
    m_layer = new KisPaintLayer(m_image1, "layer1", OPACITY_OPAQUE_U8);
    m_image1->addNode(m_layer);
    m_channel = m_layer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
}

void KisKeyframeAnimationInterfaceSignalTest::init()
{
    m_channel->addKeyframe(0);

    //delete all  keyframes other than 1st
    while (m_channel->keyframeAt(m_channel->firstKeyframeTime()) != m_channel->keyframeAt(m_channel->lastKeyframeTime())) {
        m_channel->removeKeyframe(m_channel->lastKeyframeTime());
    }
    QCOMPARE(m_channel->keyframeCount(), 1);
}

void KisKeyframeAnimationInterfaceSignalTest::testSignalFromKeyframeChannelToInterface()
{

    QCOMPARE(m_channel->keyframeCount(), 1);

    //add keyframe
    qRegisterMetaType<const KisKeyframeChannel*>("const KisKeyframeChannel*");
    QSignalSpy spyFrameAdded(m_image1->animationInterface() , SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)));
    QVERIFY(spyFrameAdded.isValid());

    m_channel->addKeyframe(2);
    QCOMPARE(spyFrameAdded.count(), 1);

    //remove keyframe
    QSignalSpy spyFrameRemoved(m_image1->animationInterface() , SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*, int)));
    QVERIFY(spyFrameRemoved.isValid());

    m_channel->removeKeyframe(5);
    QCOMPARE(spyFrameRemoved.count(), 1);
}

void KisKeyframeAnimationInterfaceSignalTest::testSignalOnImageReset()
{
    m_image1->removeNode(m_layer);
    m_image2->addNode(m_layer);
    m_layer->setImage(m_image2);

    //test the connections between m_channel and new image's animation interface
    QVERIFY(!connect(m_channel, SIGNAL(sigAddedKeyframe(const KisKeyframeChannel*,int)), m_image2->animationInterface(), SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)), Qt::UniqueConnection));

    //test signals from the old image on changing m_channel after image reset
    QSignalSpy spyFrameAdded(m_image1->animationInterface() , SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)));
    QVERIFY(spyFrameAdded.isValid());
    
    QSignalSpy spyFrameRemoved(m_image1->animationInterface() , SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*, int)));
    QVERIFY(spyFrameRemoved.isValid());

    //check if signal are emitted from the new image on changing m_channnel
    QSignalSpy newSpyFrameAdded(m_image2->animationInterface() , SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*, int)));
    QVERIFY(newSpyFrameAdded.isValid());

    QSignalSpy newSpyFrameRemoved(m_image2->animationInterface() , SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*, int)));
    QVERIFY(newSpyFrameRemoved.isValid());

    m_channel->addKeyframe(2);
    m_channel->removeKeyframe(2);

    QCOMPARE(spyFrameAdded.count(), 0);
    QCOMPARE(spyFrameRemoved.count(), 0);

    QCOMPARE(newSpyFrameAdded.count(), 1);
    QCOMPARE(newSpyFrameRemoved.count(), 1);

    QCOMPARE(m_channel->keyframeCount(), 1);
}

SIMPLE_TEST_MAIN(KisKeyframeAnimationInterfaceSignalTest)
