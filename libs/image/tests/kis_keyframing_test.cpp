/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#include "kis_keyframing_test.h"
#include <QTest>
#include <qsignalspy.h>

#include "kis_paint_device_frames_interface.h"
#include "kis_keyframe_channel.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_node.h"
#include "kis_time_span.h"
#include "kundo2command.h"


#include <KoColorSpaceRegistry.h>

#include "testing_timed_default_bounds.h"


void KisKeyframingTest::initTestCase()
{
    cs = KoColorSpaceRegistry::instance()->rgb8();

    red = new quint8[cs->pixelSize()];
    green = new quint8[cs->pixelSize()];
    blue = new quint8[cs->pixelSize()];

    cs->fromQColor(Qt::red, red);
    cs->fromQColor(Qt::green, green);
    cs->fromQColor(Qt::blue, blue);
}

void KisKeyframingTest::cleanupTestCase()
{
    delete[] red;
    delete[] green;
    delete[] blue;
}

// General.

void KisKeyframingTest::testChannelSignals()
{
      QVERIFY(false);
//    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(KoID(""), -17, 31, 0);
//    KisKeyframeSP key;
//    KisKeyframeSP resKey;

//    qRegisterMetaType<KisKeyframeSP>("KisKeyframeSP");
//    QSignalSpy spyPreAdd(channel, SIGNAL(sigKeyframeAboutToBeAdded(KisKeyframeSP)));
//    QSignalSpy spyPostAdd(channel, SIGNAL(sigKeyframeAdded(KisKeyframeSP)));

//    QSignalSpy spyPreRemove(channel, SIGNAL(sigKeyframeAboutToBeRemoved(KisKeyframeSP)));
//    QSignalSpy spyPostRemove(channel, SIGNAL(sigKeyframeRemoved(KisKeyframeSP)));

//    QSignalSpy spyPreMove(channel, SIGNAL(sigKeyframeAboutToBeMoved(KisKeyframeSP,int)));
//    QSignalSpy spyPostMove(channel, SIGNAL(sigKeyframeMoved(KisKeyframeSP,int)));

//    QVERIFY(spyPreAdd.isValid());
//    QVERIFY(spyPostAdd.isValid());
//    QVERIFY(spyPreRemove.isValid());
//    QVERIFY(spyPostRemove.isValid());
//    QVERIFY(spyPreMove.isValid());
//    QVERIFY(spyPostMove.isValid());

//    // Adding a keyframe

//    QCOMPARE(spyPreAdd.count(), 0);
//    QCOMPARE(spyPostAdd.count(), 0);

//    key = channel->addKeyframe(10);

//    QCOMPARE(spyPreAdd.count(), 1);
//    QCOMPARE(spyPostAdd.count(), 1);

//    resKey = spyPreAdd.at(0).at(0).value<KisKeyframeSP>();
//    QVERIFY(resKey == key);
//    resKey = spyPostAdd.at(0).at(0).value<KisKeyframeSP>();
//    QVERIFY(resKey == key);

//    // Moving a keyframe

//    QCOMPARE(spyPreMove.count(), 0);
//    QCOMPARE(spyPostMove.count(), 0);
//    channel->moveKeyframe(10, 15);
//    QCOMPARE(spyPreMove.count(), 1);
//    QCOMPARE(spyPostMove.count(), 1);

//    resKey = spyPreMove.at(0).at(0).value<KisKeyframeSP>();
//    QVERIFY(resKey == key);
//    QCOMPARE(spyPreMove.at(0).at(1).toInt(), 15);
//    resKey = spyPostMove.at(0).at(0).value<KisKeyframeSP>();
//    QVERIFY(resKey == key);

//    // No-op move (no signals)

//    channel->moveKeyframe(15, 15);
//    QCOMPARE(spyPreMove.count(), 1);
//    QCOMPARE(spyPostMove.count(), 1);

//    // Deleting a keyframe

//    QCOMPARE(spyPreRemove.count(), 0);
//    QCOMPARE(spyPostRemove.count(), 0);
//    channel->deleteKeyframe(15);
//    QCOMPARE(spyPreRemove.count(), 1);
//    QCOMPARE(spyPostRemove.count(), 1);

//    delete channel;
}

// ===================== Raster Channel. ===============================

void KisKeyframingTest::testRasterChannel()
{
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    // Create raster channel.
    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KoID());
    const int initialFrames = 1;

    QVERIFY(channel->keyframeCount() == initialFrames);
    QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());
    QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0) != nullptr);


    // Draw RED on initial frame..
    bounds->testingSetTime(0);
    dev->fill(0, 0, 512, 512, red);
    QImage key0_thumbnail = dev->createThumbnail(50, 50);


    // Create a second raster keyframe..
    channel->addKeyframe(3);

    QVERIFY(channel->keyframeCount() == initialFrames + 1);
    QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());
    QVERIFY(channel->keyframeAt<KisRasterKeyframe>(3) != nullptr);

    {   // Check that physical keyframeIDs are different!
        KisRasterKeyframeSP key0 = channel->keyframeAt<KisRasterKeyframe>(0);
        KisRasterKeyframeSP key3 = channel->keyframeAt<KisRasterKeyframe>(3);
        QVERIFY(key0->frameID() != key3->frameID());
    }

    {   // Check that the NEW frame's thumbnail looks DIFFERENT!
        bounds->testingSetTime(3);
        QImage key3_thumbnail = dev->createThumbnail(50, 50);
        QVERIFY(key0_thumbnail != key3_thumbnail);
    }


    // Move raster keyframe (3->6)..
    {
        const int originalFrameID = channel->keyframeAt<KisRasterKeyframe>(3)->frameID();

        channel->moveKeyframe(3, 6);

        QVERIFY(channel->keyframeAt(3) == nullptr);
        QVERIFY(channel->keyframeAt(6));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(6)->frameID() ==  originalFrameID);
    }


    // Copy raster keyframe (0>>5)..
    channel->copyKeyframe(0, 5);

    QVERIFY(channel->keyframeCount() == initialFrames + 2);
    QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());

    {   // Check that physical keyframeIDs are different!
        KisRasterKeyframeSP key0 = channel->keyframeAt<KisRasterKeyframe>(0);
        KisRasterKeyframeSP key5 = channel->keyframeAt<KisRasterKeyframe>(5);
        QVERIFY(key0->frameID() != key5->frameID());
    }

    {   // Check that the COPIED frame's thumbnail looks THE SAME!
        bounds->testingSetTime(5);
        QImage key5_thumbnail = dev->createThumbnail(50, 50);
        QVERIFY(key0_thumbnail == key5_thumbnail);
    }


    // Swap raster keyframes ()..
    {
        const int original_f5_frameID = channel->keyframeAt<KisRasterKeyframe>(5)->frameID();
        const int original_f6_frameID = channel->keyframeAt<KisRasterKeyframe>(6)->frameID();
        channel->swapKeyframes(5,6);

        QVERIFY(channel->keyframeAt(5));
        QVERIFY(channel->keyframeAt(6));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(5)->frameID() == original_f6_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(6)->frameID() == original_f5_frameID);
    }

    // Delete raster keyrame..
    channel->removeKeyframe(5);

    QVERIFY(channel->keyframeAt(5) == nullptr);
    QVERIFY(channel->keyframeCount() == initialFrames + 1);
    QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());


    {   // Overwrite raster keyframe..
        const int previousKeyframeCount = channel->keyframeCount();

        channel->addKeyframe(0); // Overwrite's initial frame!

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0));
        QVERIFY(channel->keyframeCount() == previousKeyframeCount);
        QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());

        bounds->testingSetTime(0);
        QImage new_key0_thumbnail = dev->createThumbnail(50, 50);
        QVERIFY(new_key0_thumbnail != key0_thumbnail);
    }


    // Clone raster keyframe..
        // clone a frame..
        // verify same keyframeID
        // verify same thumbnail.
        // verify # virtual frames != # physical frames.
        // edit cloneA, and compare thumbnails.
        // edit cloneB, and compare thumbnails.
}

void KisKeyframingTest::testRasterFrameFetching()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisPaintDeviceSP devTarget = new KisPaintDevice(cs);
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KoID());

    // Setup initial frames..
    channel->addKeyframe(0);
    channel->addKeyframe(10);
    channel->addKeyframe(50);

    bounds->testingSetTime(0);
    dev->fill(0, 0, 512, 512, red);
    QImage frame0 = dev->createThumbnail(50, 50);

    bounds->testingSetTime(10);
    dev->fill(0, 0, 512, 512, green);
    QImage frame10 = dev->createThumbnail(50, 50);

    bounds->testingSetTime(50);
    dev->fill(0, 0, 512, 512, blue);
    QImage frame50 = dev->createThumbnail(50, 50);

    // Fetch frames..
    channel->fetchFrame(0, devTarget);
    QImage fetched0 = devTarget->createThumbnail(50, 50);

    channel->fetchFrame(10, devTarget);
    QImage fetched10 = devTarget->createThumbnail(50, 50);

    channel->fetchFrame(50, devTarget);
    QImage fetched50 = devTarget->createThumbnail(50, 50);

    channel->fetchFrame(20, devTarget);
    QImage fetched20 = devTarget->createThumbnail(50, 50);

    QVERIFY(fetched0 == frame0);
    QVERIFY(fetched10 == frame10);
    QVERIFY(fetched50 == frame50);
    QVERIFY(fetched20 == frame10);
    QVERIFY(fetched20 == fetched10);
}

void KisKeyframingTest::testRasterUndoRedo()
{
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    // Create raster channel.
    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KoID());

    {   // Add/Insert
        KUndo2Command cmd;
        channel->addKeyframe(25, &cmd);

        QVERIFY(channel->keyframeAt(25));

        KisRasterKeyframeSP keyframe = channel->keyframeAt<KisRasterKeyframe>(25);
        const int originalFrameID = keyframe->frameID();

        //undo
        cmd.undo();

        QVERIFY(channel->keyframeAt(25) == nullptr);

        //redo
        cmd.redo();
        keyframe = channel->keyframeAt<KisRasterKeyframe>(25);

        QVERIFY(channel->keyframeAt(25));
        QVERIFY(keyframe->frameID() == originalFrameID);
        //thumbnail?
    }


    {   // Remove
        KUndo2Command cmd;

        KisRasterKeyframeSP keyframe = channel->keyframeAt<KisRasterKeyframe>(25);
        const int originalFrameID = keyframe->frameID();

        channel->removeKeyframe(25, &cmd);

        QVERIFY(channel->keyframeAt(25) == nullptr);

        //undo
        cmd.undo();
        keyframe = channel->keyframeAt<KisRasterKeyframe>(25);

        QVERIFY(channel->keyframeAt(25));
        QVERIFY(keyframe->frameID() == originalFrameID);
        //thumbnail?

        //redo
        cmd.redo();

        QVERIFY(channel->keyframeAt(25) == nullptr);
    }

    channel->addKeyframe(33);

    {   // Move
        KUndo2Command cmd;

        channel->moveKeyframe(33, 34, &cmd);

        QVERIFY(channel->keyframeAt(33) == nullptr);
        QVERIFY(channel->keyframeAt(34));

        cmd.undo();

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(34) == nullptr);

        cmd.redo();

        QVERIFY(channel->keyframeAt(33) == nullptr);
        QVERIFY(channel->keyframeAt(34));

        cmd.undo(); // (Sets up the next test.)
    }


    {   // Copy
        KUndo2Command cmd;

        channel->copyKeyframe(33, 35, &cmd);

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(35));

        cmd.undo();

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(35) == nullptr);

        cmd.redo();

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(35));
        //thumnail?
    }

    channel->addKeyframe(66);

    {   // Swap
        KUndo2Command cmd;

        int original_f33_frameID = channel->keyframeAt<KisRasterKeyframe>(33)->frameID();
        int original_f66_frameID = channel->keyframeAt<KisRasterKeyframe>(66)->frameID();

        channel->swapKeyframes(33, 66, &cmd);

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(66));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f66_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f33_frameID);

        cmd.undo();

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f33_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f66_frameID);

        cmd.redo();

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f66_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f33_frameID);
    }

    QVERIFY(channel->keyframeCount() == dev->framesInterface()->frames().count());
}

void KisKeyframingTest::testFirstFrameOperations()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KoID());

    QVERIFY(dev->framesInterface()->frames().count() == channel->keyframeCount());
    QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0) != 0);

    const int initialKeyframes = channel->keyframeCount();
    const int initialPhysicalFrames = dev->framesInterface()->frames().count();

    {   // Delete first and only frame, and a new frame on frame 0 should exist.
        KUndo2Command cmd;
        const int original_frame0_frameID = channel->keyframeAt<KisRasterKeyframe>(0)->frameID();
        channel->removeKeyframe(0, &cmd);

        QVERIFY(channel->keyframeAt(0));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0)->frameID() != original_frame0_frameID);

        cmd.undo();

        QVERIFY(channel->keyframeAt(0));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0)->frameID() == original_frame0_frameID);

        cmd.redo();

        QVERIFY(channel->keyframeAt(0));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0)->frameID() != original_frame0_frameID);
    }

    {   // When the first frame is moved, a new *empty* frame should take it's place
        KUndo2Command cmd;

        bounds->testingSetTime(0);
        int movedFrameId = channel->keyframeAt<KisRasterKeyframe>(0)->frameID();

        channel->moveKeyframe(0, 3, &cmd);

        {
            KisRasterKeyframeSP frame0 = channel->keyframeAt<KisRasterKeyframe>(0);
            KisRasterKeyframeSP frame3 = channel->keyframeAt<KisRasterKeyframe>(3);
            QVERIFY(frame0 != nullptr);
            QVERIFY(frame3 != nullptr);
            QVERIFY(frame0->frameID() != frame3->frameID());
            QVERIFY(frame3->frameID() == movedFrameId);
        }

        cmd.undo();

        {
            KisRasterKeyframeSP frame0 = channel->keyframeAt<KisRasterKeyframe>(0);
            KisRasterKeyframeSP frame3 = channel->keyframeAt<KisRasterKeyframe>(3);
            QVERIFY(frame0 != nullptr);
            QVERIFY(frame3 == nullptr);
            QVERIFY(frame0->frameID() == movedFrameId);
        }
    }

    QVERIFY(channel->keyframeCount() == initialKeyframes);
    QVERIFY(dev->framesInterface()->frames().count() == initialPhysicalFrames);
}

void KisKeyframingTest::testInterChannelMovement()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    KisPaintDeviceSP devA = new KisPaintDevice(cs);
    devA->setDefaultBounds(bounds);

    // Initialize channelA and channelB with the same data.
    KisRasterKeyframeChannel *channelA = devA->createKeyframeChannel(KoID());

    channelA->addKeyframe(0); // A0
    bounds->testingSetTime(0);
    devA->fill(0, 0, 512, 512, red);
    QImage original_thumbnail_ab0 = devA->createThumbnail(50, 50);

    channelA->addKeyframe(10); // A10
    bounds->testingSetTime(10);
    devA->fill(0, 0, 512, 512, green);
    QImage original_thumbnail_ab10 = devA->createThumbnail(50, 50);

    channelA->addKeyframe(50); // A50
    bounds->testingSetTime(50);
    devA->fill(0, 0, 512, 512, blue);
    QImage original_thumbnail_ab50 = devA->createThumbnail(50, 50);

    KisPaintDeviceSP devB = new KisPaintDevice(*devA, KritaUtils::CopyAllFrames);
    devB->setDefaultBounds(bounds);
    KisRasterKeyframeChannel *channelB = devB->keyframeChannel();

    // Move between channels (A10->B15)
    KisKeyframeChannel::moveKeyframe(channelA, 10, channelB, 15);
    QVERIFY(channelA->keyframeAt(10) == nullptr);
    QVERIFY(channelB->keyframeAt(15));
    QVERIFY(channelA->keyframeCount() == devA->framesInterface()->frames().count());
    QVERIFY(channelB->keyframeCount() == devB->framesInterface()->frames().count());

    {   // Test the thumbnails between the two paint devices.
        bounds->testingSetTime(15);
        QImage thumbnail_b15 = devB->createThumbnail(50,50);
        QVERIFY(thumbnail_b15 == original_thumbnail_ab10);
    }

    // Copy between channels (A50>>B25)
    KisKeyframeChannel::copyKeyframe(channelA, 50, channelB, 25);
    QVERIFY(channelA->keyframeAt(50));
    QVERIFY(channelB->keyframeAt(25));

    {   // Test the thumbnails between the two paint devices.
        bounds->testingSetTime(25);
        QImage thumbnail_b25 = devB->createThumbnail(50,50);
        QVERIFY(thumbnail_b25 == original_thumbnail_ab50);
    }

    // Swap between channels (A50<->B0)
    KisKeyframeChannel::swapKeyframes(channelA, 50, channelB, 0);
    QVERIFY(channelA->keyframeAt(50));
    QVERIFY(channelB->keyframeAt(0));

    {   // Test the thumbnails between the two paint devices.
        bounds->testingSetTime(0);
        QImage thumbnail_b0 = devB->createThumbnail(50, 50);

        bounds->testingSetTime(50);
        QImage thumbnail_a50 = devA->createThumbnail(50, 50);

        QVERIFY(thumbnail_b0 == original_thumbnail_ab50);
        QVERIFY(thumbnail_a50 == original_thumbnail_ab0);
    }
}

// ================= Scalar Channel. =============================

void KisKeyframingTest::testScalarChannel()
{
    QVERIFY(false);
    /*
    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(KoID(""), -17, 31, 0);
    bool ok;

    QCOMPARE(channel->minScalarValue(), -17.0);
    QCOMPARE(channel->maxScalarValue(),  31.0);

    QVERIFY(channel->keyframeAt(0) == 0);

    // Adding new keyframe
    channel->addKeyframe(42);
    channel->setScalarValue(42, 7.0);

    QCOMPARE(channel->scalarValue(42), 7.0);

    // Copying a keyframe

    channel->copyKeyframe(channel->keyframeAt(42), 13);
    QVERIFY(channel->keyframeAt(13));
    QCOMPARE(channel->scalarValue(42), channel->scalarValue(13));
    QCOMPARE(channel->scalarValue(42), 7.0);

    // Adding a keyframe where one exists
    {
        KisKeyframeSP original = channel->keyframeAt(13);
        KisKeyframeSP overwrite = channel->addKeyframe(13);
        QVERIFY(original != overwrite);
        QCOMPARE(channel->keyframeCount(), 2);
    }

    // Moving keyframes
    {
        const int oldKeyTime = 13;
        const int newKeyTime = 10;
        ok = channel->moveKeyframe(oldKeyTime, newKeyTime);
        QCOMPARE(ok, true);
        QVERIFY(channel->keyframeAt(oldKeyTime) == 0);

        KisKeyframeSP moved = channel->keyframeAt(newKeyTime);
        QVERIFY(moved);
        QCOMPARE(channel->scalarValue(newKeyTime), 7.0);
    }

    {
        // Moving a keyframe where another one exists
        const int oldKeyTime = channel->activeKeyframeAt(10).time();
        const int newKeyTime = channel->nextKeyframe(10).time();

        KisKeyframeSP removedKey = channel->keyframeAt(newKeyTime);
        ok = channel->moveKeyframe(oldKeyTime, newKeyTime);
        QCOMPARE(ok, true);
        QVERIFY(!channel->keyframeAt(oldKeyTime));
        QVERIFY(channel->keyframeAt(newKeyTime));

        KisKeyframeSP movedKey = channel->keyframeAt(newKeyTime);
        QVERIFY(movedKey != removedKey);

    }

    // Deleting a keyframe
    channel->deleteKeyframe(10);
    QVERIFY(channel->keyframeAt(10) == 0);
    QCOMPARE(channel->keyframeCount(), 0);

    delete channel;
    */
}

void KisKeyframingTest::testScalarChannelUndoRedo()
{
    QVERIFY(false);
    /*
    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(KoID(""), -17, 31, 0);

    QCOMPARE(channel->minScalarValue(), -17.0);
    QCOMPARE(channel->maxScalarValue(),  31.0);

    QVERIFY(channel->keyframeAt(0) == 0);

    // Adding new keyframe

    KUndo2Command addCmd;

    channel->addKeyframe(42, &addCmd);
    channel->setScalarValue(42, 7.0, &addCmd);
    QCOMPARE(channel->scalarValue(42), 7.0);
    const float beforeUndoValue = channel->scalarValue(42);

    addCmd.undo();

    KisKeyframeSP key = channel->keyframeAt(42);
    QVERIFY(!key);

    addCmd.redo();

    key = channel->keyframeAt(42);
    QVERIFY(key);

    QCOMPARE(channel->scalarValue(42), beforeUndoValue);
    delete channel;
    */
}

void KisKeyframingTest::testAffectedFrames()
{
    QVERIFY(false);
    /*
    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(KoID(""), -17, 31, 0);
    KisTimeRange range;

    channel->addKeyframe(10);
    channel->addKeyframe(20);
    channel->addKeyframe(30);

    // At a keyframe
    range = channel->affectedFrames(20);
    QCOMPARE(range.start(), 20);
    QCOMPARE(range.end(), 29);
    QCOMPARE(range.isInfinite(), false);

    // Between frames
    range = channel->affectedFrames(25);
    QCOMPARE(range.start(), 20);
    QCOMPARE(range.end(), 29);
    QCOMPARE(range.isInfinite(), false);

    // Before first frame
    range = channel->affectedFrames(5);
    QCOMPARE(range.start(), 0);
    QCOMPARE(range.end(), 9);
    QCOMPARE(range.isInfinite(), false);

    // After last frame
    range = channel->affectedFrames(35);
    QCOMPARE(range.start(), 30);
    QCOMPARE(range.isInfinite(), true);
    */
}

void KisKeyframingTest::testScalarInterpolation()
{
    QVERIFY(false);
    /*
    KisScalarKeyframeChannel *channel = new KisScalarKeyframeChannel(KoID(""), 0, 30, 0);
    const int keyIndexA = 0;
    const int keyIndexB = 10;

    {
        const float valueA = 15;
        const float valueB = 30;
        channel->addKeyframe(keyIndexA);
        channel->setScalarValue(keyIndexA, valueA);

        channel->addKeyframe(keyIndexB);
        channel->setScalarValue(keyIndexB, valueB);

        // Constant

        KisScalarKeyframeSP keyA = channel->keyframeAt(keyIndexA).dynamicCast<KisScalarKeyframe>();
        keyA->setInterpolationMode(KisScalarKeyframe::Constant);

        QCOMPARE(channel->interpolatedValue(keyIndexA + 4), valueA);

        // Bezier

        keyA->setInterpolationMode(KisScalarKeyframe::Bezier);


        keyA->setInterpolationTangents(QPointF(), QPointF(1,4));
        KisScalarKeyframeSP keyB = channel->keyframeAt(keyIndexB).dynamicCast<KisScalarKeyframe>();
        keyB->setInterpolationTangents(QPointF(-4,2), QPointF());

        QVERIFY(qAbs(channel->interpolatedValue(keyIndexA + 4) - 24.9812f) < 0.1f);
    }

    // Bezier, self-intersecting curve (auto-correct)

    {
        KisScalarKeyframeSP keyA = channel->keyframeAt(keyIndexA).dynamicCast<KisScalarKeyframe>();
        KisScalarKeyframeSP keyB = channel->keyframeAt(keyIndexB).dynamicCast<KisScalarKeyframe>();
        keyB->setValue(15);

        keyA->setInterpolationTangents(QPointF(), QPointF(13,10));
        keyB->setInterpolationTangents(QPointF(-13,10), QPointF());

        QVERIFY(qAbs(channel->interpolatedValue(keyIndexA + 5) - 20.769f) < 0.1f);
    }
    // Bezier, result outside allowed range (clamp)

    {
        KisScalarKeyframeSP keyA = channel->keyframeAt(keyIndexA).dynamicCast<KisScalarKeyframe>();
        KisScalarKeyframeSP keyB = channel->keyframeAt(keyIndexB).dynamicCast<KisScalarKeyframe>();

        keyB->setValue(15);

        keyA->setInterpolationTangents(QPointF(), QPointF(0, 50));
        keyB->setInterpolationTangents(QPointF(0, 50), QPointF());
    }

    QCOMPARE(channel->interpolatedValue(keyIndexA + 5), 30.0f);

    delete channel;
    */
}


QTEST_MAIN(KisKeyframingTest)
