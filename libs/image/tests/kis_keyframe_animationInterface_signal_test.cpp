/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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

#include "kis_keyframe_animationInterface_signal_test.h"

#include <QTest>
#include <qsignalspy.h>


void KisKeyframeAnimationInterfaceSignalTest::initTestCase()
{
    m_image1 = new KisImage(0, 100, 100, nullptr, "image1");
    m_layer1 = new KisPaintLayer(m_image1, "layer1", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP paintDevice = m_layer1->paintDevice();
    paintDevice->createKeyframeChannel(KoID());
    m_channel = paintDevice->keyframeChannel();

    m_image2 = new KisImage(0, 100, 100, nullptr, "image1");
    m_layer2 = new KisPaintLayer(m_image1, "layer1", OPACITY_OPAQUE_U8);
}

void KisKeyframeAnimationInterfaceSignalTest::init()
{
    m_channel->addKeyframe(0);

    //delete all  keyframes other than 1st
    while (m_channel->firstKeyframe() != m_channel->lastKeyframe()) {
        m_channel->deleteKeyframe(m_channel->lastKeyframe());
    }
    QCOMPARE(m_channel->keyframeCount(), 1);
}

void KisKeyframeAnimationInterfaceSignalTest::testSignalFromKeyframeChannelToInterface()
{

    QCOMPARE(m_channel->keyframeCount(), 1);

    //add keyframe
    QSignalSpy spyFrameAdded(m_image1->animationInterface() , SIGNAL(sigKeyframeAdded(KisKeyframeSP)));
    QVERIFY(spyFrameAdded.isValid());

    m_channel->addKeyframe(2);
    QCOMPARE(spyFrameAdded.count(), 1);

    //move keyframe
    QSignalSpy spyFrameMoved(m_image1->animationInterface() , SIGNAL(sigKeyframeMoved(KisKeyframeSP, int)));
    QVERIFY(spyFrameMoved.isValid());

    m_channel->moveKeyframe(m_channel->keyframeAt(2), 5);
    QCOMPARE(spyFrameMoved.count(), 1);

    //remove keyframe
    QSignalSpy spyFrameRemoved(m_image1->animationInterface() , SIGNAL(sigKeyframeRemoved(KisKeyframeSP)));
    QVERIFY(spyFrameRemoved.isValid());

    m_channel->deleteKeyframe(m_channel->keyframeAt(5));
    QCOMPARE(spyFrameRemoved.count(), 1);
}

void KisKeyframeAnimationInterfaceSignalTest::testSignalOnNodeReset()
{
    //set node to null 
    m_channel->setNode(KisNodeWSP(nullptr));

    //change node check connections
    m_channel->setNode((KisNodeWSP)(m_layer2));

    QVERIFY(!connect(m_layer2, SIGNAL(sigBeginImageReset(KisNodeWSP, KisImageWSP)), 
                m_channel, SLOT(slotUnbindSignalsToAnimationInterface(KisNodeWSP, KisImageWSP)), Qt::UniqueConnection));
    QVERIFY(!connect(m_layer2, SIGNAL(sigEndImageReset(KisNodeWSP)), 
                m_channel, SLOT(slotBindSignalsToAnimationInterface(KisNodeWSP)), Qt::UniqueConnection));

    testSignalFromKeyframeChannelToInterface();
}

void KisKeyframeAnimationInterfaceSignalTest::testSignalOnImageReset()
{
    //change the image and check for signals from node
    QSignalSpy newSpyBegImageReset(m_layer2 , SIGNAL(sigBeginImageReset(KisNodeWSP, KisImageWSP)));
    QVERIFY(newSpyBegImageReset.isValid());

    QSignalSpy newSpyEndImageReset(m_layer2 , SIGNAL(sigEndImageReset(KisNodeWSP)));
    QVERIFY(newSpyEndImageReset.isValid());

    m_layer2->setImage(m_image2);
    
    QCOMPARE(newSpyBegImageReset.count(), 1);
    newSpyBegImageReset.clear();

    QCOMPARE(newSpyEndImageReset.count(), 1);
    newSpyEndImageReset.clear();

    //test the connections between m_channel and new image's animation interface
    QVERIFY(!connect(m_channel, SIGNAL(sigKeyframeAdded(KisKeyframeSP)), m_image2->animationInterface(), SIGNAL(sigKeyframeAdded(KisKeyframeSP)), Qt::UniqueConnection));  

    //test signals from the old image on changing m_channel after image reset
    QSignalSpy spyFrameAdded(m_image1->animationInterface() , SIGNAL(sigKeyframeAdded(KisKeyframeSP)));
    QVERIFY(spyFrameAdded.isValid());
    
    QSignalSpy spyFrameMoved(m_image1->animationInterface() , SIGNAL(sigKeyframeMoved(KisKeyframeSP, int)));
    QVERIFY(spyFrameMoved.isValid());
    
    QSignalSpy spyFrameRemoved(m_image1->animationInterface() , SIGNAL(sigKeyframeRemoved(KisKeyframeSP)));
    QVERIFY(spyFrameRemoved.isValid());

    //check if signal are emitted from the new image on changing m_channnel
    QSignalSpy newSpyFrameAdded(m_image2->animationInterface() , SIGNAL(sigKeyframeAdded(KisKeyframeSP)));
    QVERIFY(newSpyFrameAdded.isValid());

    QSignalSpy newSpyFrameMoved(m_image2->animationInterface() , SIGNAL(sigKeyframeMoved(KisKeyframeSP, int)));
    QVERIFY(newSpyFrameMoved.isValid());

    QSignalSpy newSpyFrameRemoved(m_image2->animationInterface() , SIGNAL(sigKeyframeRemoved(KisKeyframeSP)));
    QVERIFY(newSpyFrameRemoved.isValid());

    m_channel->addKeyframe(2);
    m_channel->moveKeyframe(m_channel->keyframeAt(2), 3);
    m_channel->deleteKeyframe(m_channel->keyframeAt(3));

    QCOMPARE(spyFrameAdded.count(), 0);
    QCOMPARE(spyFrameRemoved.count(), 0);
    QCOMPARE(spyFrameMoved.count(), 0);

    QCOMPARE(newSpyFrameAdded.count(), 1);
    QCOMPARE(newSpyFrameRemoved.count(), 1);
    QCOMPARE(newSpyFrameMoved.count(), 1);

    QCOMPARE(m_channel->keyframeCount(), 1);
}

QTEST_MAIN(KisKeyframeAnimationInterfaceSignalTest)
