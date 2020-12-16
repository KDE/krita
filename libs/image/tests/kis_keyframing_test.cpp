/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_keyframing_test.h"
#include <QTest>
#include <qsignalspy.h>
#include <QRandomGenerator>

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

// ===================== General =======================

void KisKeyframingTest::testChannelSignals()
{
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    // Create raster channel..
    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KoID());

    qRegisterMetaType<const KisKeyframeChannel*>("const KisKeyframeChannel*");
    qRegisterMetaType<KisKeyframeSP>("KisKeyframeSP");
    QSignalSpy spyUpdated(channel, SIGNAL(sigChannelUpdated(KisTimeSpan, QRect)));
    QSignalSpy spyAdded(channel, SIGNAL(sigAddedKeyframe(const KisKeyframeChannel*,int)));
    QSignalSpy spyRemoving(channel, SIGNAL(sigRemovingKeyframe(const KisKeyframeChannel*,int)));

    QVERIFY(spyUpdated.isValid());
    QVERIFY(spyAdded.isValid());
    QVERIFY(spyRemoving.isValid());

    int updateSignalCount = spyUpdated.count();

    {   // Adding a keyframe..
        int originalSignalCount = spyAdded.count();
        channel->addKeyframe(7);

        QVERIFY(spyAdded.count() == originalSignalCount + 1);
        QVERIFY(spyUpdated.count() > updateSignalCount);
    }

    updateSignalCount = spyUpdated.count();

    {    // Moving a keyframe (7->11)..
        channel->moveKeyframe(7, 11);

        QVERIFY(spyUpdated.count() > updateSignalCount);

        // No-op move (no signals) ...why?
    }

    updateSignalCount = spyUpdated.count();

    {   // Removing a keyframe..
        int originalSignalCount = spyRemoving.count();
        channel->removeKeyframe(11);

        QVERIFY(spyRemoving.count() == originalSignalCount + 1);
        QVERIFY(spyUpdated.count() > updateSignalCount);
    }


    // Setup scalar channel test environment
    qRegisterMetaType<const KisScalarKeyframeChannel*>("const KisScalarKeyframeChannel*");
    QScopedPointer<KisScalarKeyframeChannel> scalarChannel( new KisScalarKeyframeChannel(KoID(), bounds) );
    scalarChannel->setLimits(0, 64);
    scalarChannel->addScalarKeyframe(0, 32);
    scalarChannel->addScalarKeyframe(10, 64);
    QSignalSpy spyScalarKeyframeChanged(scalarChannel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)));

    {   // Test changing value of scalar keyframe. Should always emit **1** signal per assignment, undo or redo.
        KUndo2Command undoCmd;
        KisScalarKeyframeSP scalarKey = scalarChannel->keyframeAt<KisScalarKeyframe>(0);
        scalarKey->setValue(20, &undoCmd);

        QVERIFY(spyScalarKeyframeChanged.count() == 1);
        spyScalarKeyframeChanged.clear();

        undoCmd.undo();

        QVERIFY(spyScalarKeyframeChanged.count() == 1);
        spyScalarKeyframeChanged.clear();

        undoCmd.redo();

        QVERIFY(spyScalarKeyframeChanged.count() == 1);
    }
}

// ===================== Raster Channel =================

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


    // Swap raster keyframes (5<->6)..
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


    {   // Clone raster keyframe..
        channel->cloneKeyframe(6, 12);

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(6) == channel->keyframeAt<KisRasterKeyframe>(12));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(6)->frameID() == channel->keyframeAt<KisRasterKeyframe>(12)->frameID());
        QVERIFY(channel->clonesOf(6).size() == 1);
        QVERIFY(channel->clonesOf(12).size() == 1);
        QVERIFY(channel->areClones(6, 12));

        // Writing to 6 should change 12..
        bounds->testingSetTime(6);
        dev->fill(0, 0, 512, 512, green);

        {
            QImage key6_thumbnail = dev->createThumbnail(50,50);
            bounds->testingSetTime(12);
            QImage key12_thumbnail = dev->createThumbnail(50,50);

            QVERIFY(key6_thumbnail == key12_thumbnail);
        }

        // Writing to 12 should change 6..
        bounds->testingSetTime(12);
        dev->fill(0,0, 512, 512, red);

        {
            QImage key6_thumbnail = dev->createThumbnail(50,50);
            bounds->testingSetTime(12);
            QImage key12_thumbnail = dev->createThumbnail(50,50);

            QVERIFY(key6_thumbnail == key12_thumbnail);
        }

        QVERIFY(channel->keyframeCount() > dev->framesInterface()->frames().count());

        // Remove one of the clones..
        channel->removeKeyframe(6);

        QVERIFY(!channel->keyframeAt(6));
        QVERIFY(channel->keyframeAt(12));
        QVERIFY(channel->clonesOf(12).size() == 0);
    }
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
    channel->writeToDevice(0, devTarget);
    QImage fetched0 = devTarget->createThumbnail(50, 50);

    channel->writeToDevice(10, devTarget);
    QImage fetched10 = devTarget->createThumbnail(50, 50);

    channel->writeToDevice(50, devTarget);
    QImage fetched50 = devTarget->createThumbnail(50, 50);

    channel->writeToDevice(20, devTarget);
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

        cmd.undo();

        QVERIFY(channel->keyframeAt(25) == nullptr);

        cmd.redo();
        keyframe = channel->keyframeAt<KisRasterKeyframe>(25);

        QVERIFY(channel->keyframeAt(25));
        QVERIFY(keyframe->frameID() == originalFrameID);
    }

    {   // Remove
        KUndo2Command cmd;

        KisRasterKeyframeSP keyframe = channel->keyframeAt<KisRasterKeyframe>(25);
        const int originalFrameID = keyframe->frameID();

        channel->removeKeyframe(25, &cmd);

        QVERIFY(channel->keyframeAt(25) == nullptr);

        cmd.undo();
        keyframe = channel->keyframeAt<KisRasterKeyframe>(25);

        QVERIFY(channel->keyframeAt(25));
        QVERIFY(keyframe->frameID() == originalFrameID);

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

    {   // Clone
        KUndo2Command cmd;
        int original_f33_frameID = channel->keyframeAt<KisRasterKeyframe>(33)->frameID();
        int original_f66_frameID = channel->keyframeAt<KisRasterKeyframe>(66)->frameID();

        //Clone / overwrite frame 33 over 66
        channel->cloneKeyframe(33, 66, &cmd);

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(66));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f33_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() != original_f66_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f33_frameID);
        QVERIFY(channel->clonesOf(33).size() == 1);
        QVERIFY(channel->areClones(33, 66));

        cmd.undo();

        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(66));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f33_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f66_frameID);
        QVERIFY(channel->clonesOf(33).size() == 0);
        QVERIFY(channel->areClones(33, 66) == false);

        cmd.redo();
        QVERIFY(channel->keyframeAt(33));
        QVERIFY(channel->keyframeAt(66));
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(33)->frameID() == original_f33_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() != original_f66_frameID);
        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(66)->frameID() == original_f33_frameID);
        QVERIFY(channel->clonesOf(33).size() == 1);
        QVERIFY(channel->areClones(33, 66));

        //Let's remove the clone frame again
        cmd.undo();
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

    {   // Check writing to first frame and overwriting first frame contents.
        bounds->testingSetTime(0);
        dev->fill(0, 0, 50, 50, blue);
        QImage thumbnail = dev->createThumbnail(50, 50);

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0)->hasContent());

        channel->addKeyframe(0);
        QImage thumbnail2 = dev->createThumbnail(50, 50);

        QVERIFY(channel->keyframeAt<KisRasterKeyframe>(0)->hasContent() == false);
        QVERIFY(thumbnail2 != thumbnail);
    }

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
    QVERIFY(channelA->keyframeCount() == 2);
    QVERIFY(channelB->keyframeCount() == 4);
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

// ===================== Scalar Channel =================

void KisKeyframingTest::testScalarChannel()
{
    float defaultValue = 0.2;
    float lowerLimit = -1.0;
    float upperLimit = 1.0;

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    QScopedPointer<KisScalarKeyframeChannel> channel(new KisScalarKeyframeChannel(KoID(), bounds));
    channel->setLimits(lowerLimit, upperLimit);
    channel->setDefaultValue(defaultValue);
    channel->setDefaultInterpolationMode(KisScalarKeyframe::Constant);


    {   // Add keyframe..
        channel->addKeyframe(0);
        KisScalarKeyframeSP key0 = channel->keyframeAt<KisScalarKeyframe>(0);

        QVERIFY(key0->value() == defaultValue);
        QVERIFY(key0->interpolationMode() == KisScalarKeyframe::Constant);

        // Test limits..
        QRandomGenerator generator;
        for (int i = 0; i < 100; i++) {
            float randomValue = (generator.generateDouble() - 0.5) * (upperLimit - lowerLimit);

            key0->setValue(randomValue);
            QVERIFY(key0->value() >= lowerLimit && key0->value() <= upperLimit);

            key0->setValue(upperLimit + randomValue);
            QVERIFY(key0->value() >= lowerLimit && key0->value() <= upperLimit);

            key0->setValue(lowerLimit - randomValue);
            QVERIFY(key0->value() >= lowerLimit && key0->value() <= upperLimit);
        }
    }


    {   // Copy keyframe (0>>5)..
        channel->copyKeyframe(0, 5);

        KisScalarKeyframeSP key0 = channel->keyframeAt<KisScalarKeyframe>(0);
        KisScalarKeyframeSP key5 = channel->keyframeAt<KisScalarKeyframe>(5);

        QVERIFY(key5);
        QVERIFY(key0);
        QVERIFY(key0 != key5); //Should be different instances..
        QVERIFY(key5->value() == key0->value()); //But should be the same value!
    }


    {   // Move keyframe (5->7)..
        KisScalarKeyframeSP old_key5 = channel->keyframeAt<KisScalarKeyframe>(5);

        channel->moveKeyframe(5, 7);

        KisScalarKeyframeSP key5 = channel->keyframeAt<KisScalarKeyframe>(5);
        KisScalarKeyframeSP key7 = channel->keyframeAt<KisScalarKeyframe>(7);

        QVERIFY(key5 == nullptr);
        QVERIFY(key7);
        QVERIFY(key7 == old_key5); //Verify same instance has simply moved.
    }


    {   // Swap keyframe (0<->7)..
        KisScalarKeyframeSP old_key0 = channel->keyframeAt<KisScalarKeyframe>(0);
        KisScalarKeyframeSP old_key7 = channel->keyframeAt<KisScalarKeyframe>(7);

        channel->swapKeyframes(0, 7);

        KisScalarKeyframeSP new_key0 = channel->keyframeAt<KisScalarKeyframe>(0);
        KisScalarKeyframeSP new_key7 = channel->keyframeAt<KisScalarKeyframe>(7);

        QVERIFY(new_key0 && new_key7);
        QVERIFY(new_key0 == old_key7);
        QVERIFY(new_key7 == old_key0);
    }


    {   // Overwrite keyframe (7)..
        channel->keyframeAt<KisScalarKeyframe>(7)->setValue(777);
        channel->addKeyframe(7);
        KisScalarKeyframeSP new_key7 = channel->keyframeAt<KisScalarKeyframe>(7);

        QVERIFY(new_key7->value() == defaultValue);
    }


    {   // Remove keyframe..
        channel->removeKeyframe(7);

        QVERIFY(channel->keyframeAt(7) == nullptr);
    }
}

void KisKeyframingTest::testScalarValueInterpolation()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    QScopedPointer<KisScalarKeyframeChannel> channel(new KisScalarKeyframeChannel(KoID(), bounds));
    channel->setLimits(0, 30);
    channel->setDefaultValue(0);
    channel->setDefaultInterpolationMode(KisScalarKeyframe::Constant);

    const int timeA = 0;
    const int timeB = 10;

    {
        const float valueA = 15;
        const float valueB = 30;
        channel->addKeyframe(timeA);
        channel->keyframeAt<KisScalarKeyframe>(timeA)->setValue(valueA);

        channel->addKeyframe(timeB);
        channel->keyframeAt<KisScalarKeyframe>(timeB)->setValue(valueB);

        // Constant
        KisScalarKeyframeSP keyA = channel->keyframeAt<KisScalarKeyframe>(timeA);
        keyA->setInterpolationMode(KisScalarKeyframe::Constant);

        QVERIFY(keyA->interpolationMode() == KisScalarKeyframe::Constant);
        QVERIFY(channel->valueAt(timeA + 4) == valueA);
        QVERIFY(channel->valueAt(timeA) == valueA);

        // Bezier
        keyA->setInterpolationMode(KisScalarKeyframe::Bezier);
        keyA->setInterpolationTangents(QPointF(), QPointF(1,4));
        KisScalarKeyframeSP keyB = channel->keyframeAt<KisScalarKeyframe>(timeB);
        keyB->setInterpolationTangents(QPointF(-4,2), QPointF());

        QVERIFY(keyA->interpolationMode() == KisScalarKeyframe::Bezier);
        QVERIFY(qAbs(channel->valueAt(timeA + 4) - 24.9812f) < 0.1f);
        QVERIFY(channel->valueAt(timeA) == valueA);
    }


    {   // Bezier, self-intersecting curve (auto-correct)
        KisScalarKeyframeSP keyA = channel->keyframeAt<KisScalarKeyframe>(timeA);
        KisScalarKeyframeSP keyB = channel->keyframeAt<KisScalarKeyframe>(timeB);

        keyB->setValue(15);

        keyA->setInterpolationTangents(QPointF(), QPointF(13,10));
        keyB->setInterpolationTangents(QPointF(-13,10), QPointF());

        QVERIFY(qAbs(channel->valueAt(timeA + 5) - 20.769f) < 0.1f);
    }

    {   // Bezier, result outside allowed range (clamp)
        KisScalarKeyframeSP keyA = channel->keyframeAt<KisScalarKeyframe>(timeA);
        KisScalarKeyframeSP keyB = channel->keyframeAt<KisScalarKeyframe>(timeB);

        keyB->setValue(15);

        keyA->setInterpolationTangents(QPointF(), QPointF(0, 50));
        keyB->setInterpolationTangents(QPointF(0, 50), QPointF());
    }

    QCOMPARE(channel->valueAt(timeA + 5), 30.0f);
}

void KisKeyframingTest::testScalarChannelUndoRedo()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    QScopedPointer<KisScalarKeyframeChannel> channel(new KisScalarKeyframeChannel(KoID(), bounds));

    int defaultValue = 7;
    channel->setDefaultValue(defaultValue);
    channel->setDefaultInterpolationMode(KisScalarKeyframe::Constant);

    {   // Add
        KUndo2Command cmd;

        channel->addKeyframe(1, &cmd);
        KisScalarKeyframeSP key = channel->keyframeAt<KisScalarKeyframe>(1);

        QVERIFY(key);
        QVERIFY(key->value() == defaultValue);

        cmd.undo();

        QVERIFY(channel->keyframeAt(1) == nullptr);

        cmd.redo();

        QVERIFY(channel->keyframeAt(1) == key);
        QVERIFY(key->value() == defaultValue);
    }


    {   // Remove
        KUndo2Command cmd;
        KisScalarKeyframeSP key = channel->keyframeAt<KisScalarKeyframe>(1);
        const int value = 8;
        key->setValue(value);

        channel->removeKeyframe(1, &cmd);

        QVERIFY(channel->keyframeAt(1) == nullptr);

        cmd.undo();

        QVERIFY(channel->keyframeAt(1) == key);
        QVERIFY(key->value() == value);
        QVERIFY(channel->valueAt(1) == value);

        cmd.redo();

        QVERIFY(channel->keyframeAt(1) == nullptr);
    }


    {   // Modifying Keyframe Value..
        const int time = 88;
        const float key88_value = 156;

        channel->addKeyframe(time);
        KisScalarKeyframeSP key88 = channel->keyframeAt<KisScalarKeyframe>(88);

        KUndo2Command cmd;

        QVERIFY(key88);

        key88->setValue(key88_value, &cmd);

        QVERIFY(key88->value() == key88_value);

        cmd.undo();

        QVERIFY(key88->value() == defaultValue);

        cmd.redo();

        QVERIFY(key88->value() == key88_value);

        channel->removeKeyframe(time);
    }


    {   // Modify all values at once, they should all restore as expected.
        const int time = 45;
        const qreal value = 128.0f;
        const KisScalarKeyframe::InterpolationMode interpMode = KisScalarKeyframe::Linear;
        const KisScalarKeyframe::TangentsMode tangentMode = KisScalarKeyframe::Sharp;
        const QPointF leftTangent = QPoint(0, 5);
        const QPointF rightTangent = QPoint(0, -5);

        channel->addKeyframe(time);
        KisScalarKeyframeSP key = channel->keyframeAt<KisScalarKeyframe>(time);

        KUndo2Command cmd;

        QVERIFY(key);

        key->setValue(value, &cmd);
        key->setInterpolationMode(interpMode, &cmd);
        key->setTangentsMode(tangentMode, &cmd);
        key->setInterpolationTangents(leftTangent, rightTangent, &cmd);

        QVERIFY(key->value() == value);
        QVERIFY(key->interpolationMode() == interpMode);
        QVERIFY(key->tangentsMode() == tangentMode);
        QVERIFY(key->leftTangent() == leftTangent);
        QVERIFY(key->rightTangent() == rightTangent);

        cmd.undo();

        QVERIFY(key->value() != value);
        QVERIFY(key->interpolationMode() != interpMode);
        QVERIFY(key->tangentsMode() != tangentMode);
        QVERIFY(key->leftTangent() != leftTangent);
        QVERIFY(key->rightTangent() != rightTangent);

        cmd.redo();

        QVERIFY(key->value() == value);
        QVERIFY(key->interpolationMode() == interpMode);
        QVERIFY(key->tangentsMode() == tangentMode);
        QVERIFY(key->leftTangent() == leftTangent);
        QVERIFY(key->rightTangent() == rightTangent);

        channel->removeKeyframe(time);
    }
}

void KisKeyframingTest::testScalarAffectedFrames()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();

    QScopedPointer<KisScalarKeyframeChannel> channel( new KisScalarKeyframeChannel(KoID(""), bounds) );
    channel->setLimits(-17, 31);
    channel->setDefaultValue(0);
    KisTimeSpan range;

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
}

void KisKeyframingTest::testChangeOfScalarLimits()
{
    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    QScopedPointer<KisScalarKeyframeChannel> channel(new KisScalarKeyframeChannel(KoID(), bounds));
    channel->setDefaultValue(0);
    channel->setDefaultInterpolationMode(KisScalarKeyframe::Constant);

    // Set channel scalar limtis..
    const int original_low = 0;
    const int original_high = 64;
    channel->setLimits(original_low, original_high);

    // Add a few keyframes..
    channel->addScalarKeyframe(0, -12);
    channel->addScalarKeyframe(15, 32);
    channel->addScalarKeyframe(30, 100);

    QVERIFY(channel->valueAt(0) >= original_low && channel->valueAt(0) <= original_high);
    QVERIFY(channel->valueAt(15) >= original_low && channel->valueAt(15) <= original_high);
    QVERIFY(channel->valueAt(30) >= original_low && channel->valueAt(30) <= original_high);

    // Change channel scalar limits..
    const int new_low = -10;
    const int new_high = 10;
    channel->setLimits(new_low, new_high);

    QVERIFY(channel->valueAt(0) >= new_low && channel->valueAt(0) <= new_high);
    QVERIFY(channel->valueAt(15) >= new_low && channel->valueAt(15) <= new_high);
    QVERIFY(channel->valueAt(30) >= new_low && channel->valueAt(30) <= new_high);

    // Double check that value directly from the keyframe is also within limits..
    KisScalarKeyframeSP key0 = channel->keyframeAt<KisScalarKeyframe>(0);
    KisScalarKeyframeSP key15 = channel->keyframeAt<KisScalarKeyframe>(15);
    KisScalarKeyframeSP key30 = channel->keyframeAt<KisScalarKeyframe>(30);
    //QVERIFY(key0 && key0->value() == channel->valueAt(0));
    //QVERIFY(key15 && key15->value() == channel->valueAt(15));
    //QVERIFY(key30 && key30->value() == channel->valueAt(30));
    QCOMPARE(key0->value(), channel->valueAt(0));
    QCOMPARE(key15->value(), channel->valueAt(15));
    QCOMPARE(key30->value(), channel->valueAt(30));
}

QTEST_MAIN(KisKeyframingTest)
