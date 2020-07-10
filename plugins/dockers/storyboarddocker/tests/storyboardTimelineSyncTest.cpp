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

#include "storyboardTimelineSyncTest.h"
#include "storyboardModel.h"
#include "storyboardView.h"

#include <QTest>


void StoryboardTimelineSyncTest::initTestCase()
{
    m_image = new KisImage(0, 100, 100, nullptr, "image");

    m_storyboardModel = new StoryboardModel();
    m_storyboardView = new StoryboardView();
    m_storyboardModel->setView(m_storyboardView);
    m_storyboardView->setModel(m_storyboardModel);
    m_storyboardModel->setImage(m_image);

    m_layer1 = new KisPaintLayer(m_image, "layer1", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP paintDevice = m_layer1->paintDevice();
    paintDevice->createKeyframeChannel(KoID("abc"));
    m_channel1 = paintDevice->keyframeChannel();

    m_layer2 = new KisPaintLayer(m_image, "layer2", OPACITY_OPAQUE_U8);
    KisPaintDeviceSP paintDevice2 = m_layer2->paintDevice();
    paintDevice2->createKeyframeChannel(KoID("xyz"));
    m_channel2 = paintDevice2->keyframeChannel();

    m_channel1->addKeyframe(0);
    m_channel2->addKeyframe(0);

    QCOMPARE(m_storyboardModel->rowCount(), 1);
}

void StoryboardTimelineSyncTest::cleanupTestCase()
{
    delete m_storyboardModel;
    delete m_storyboardView;
}

void StoryboardTimelineSyncTest::cleanup()
{
    testStoryboardItemSortedUniquePositive();
}

void StoryboardTimelineSyncTest::testStoryboardItemSortedUniquePositive()
{
    int numRows = m_storyboardModel->rowCount();

    int lastFrame = -1;
    for (int i = 0; i < numRows; i++) {
        QModelIndex parentIndex = m_storyboardModel->index(i, 0);
        QModelIndex frameIndex = m_storyboardModel->index(StoryboardModel::FrameNumber, 0, parentIndex);
        QVERIFY(frameIndex.data().toInt() > lastFrame);
        lastFrame = frameIndex.data().toInt();
    }
}

void StoryboardTimelineSyncTest::testStoryboardItemAddFromTimeline()
{
    m_channel1->addKeyframe(2);
    QCOMPARE(m_storyboardModel->rowCount(), 2);

    m_channel1->addKeyframe(2);
    QCOMPARE(m_storyboardModel->rowCount(), 2);

    m_channel2->addKeyframe(4);
    QCOMPARE(m_storyboardModel->rowCount(), 3);

    m_channel2->addKeyframe(2);
    QCOMPARE(m_storyboardModel->rowCount(), 3);

    QCOMPARE(m_channel1->keyframeCount(), 2);
    QCOMPARE(m_channel2->keyframeCount(), 3);
}

void StoryboardTimelineSyncTest::testStoryboardItemMoveFromTimeline()
{
    /*          0 1 2 3 4 5 6 7 8 9 
      channel1  | . | . . . . . . .
      channel2  | . | . | . . . . .
    */

    QCOMPARE(m_storyboardModel->rowCount(), 3);

    m_channel1->moveKeyframe(m_channel1->keyframeAt(2), 3);
    QCOMPARE(m_storyboardModel->rowCount(), 4);

    m_channel1->moveKeyframe(m_channel1->keyframeAt(3), 4);
    QCOMPARE(m_storyboardModel->rowCount(), 3);
}

void StoryboardTimelineSyncTest::testStoryboardItemRemoveFromTimeline()
{
    /*          0 1 2 3 4 5 6 7 8 9 
      channel1  | . . . | . . . . .
      channel2  | . | . | . . . . .
    */
    m_channel2->deleteKeyframe(m_channel2->keyframeAt(4));
    QCOMPARE(m_storyboardModel->rowCount(), 3);

    m_channel1->deleteKeyframe(m_channel2->keyframeAt(4));
    QCOMPARE(m_storyboardModel->rowCount(), 2);
}

void StoryboardTimelineSyncTest::testStorybaordTimeleineSync()
{
}

void StoryboardTimelineSyncTest::testKeframeChangesAffectedItems()
{

}

void StoryboardTimelineSyncTest::testDurationChange()
{

}

void StoryboardTimelineSyncTest::testFpsChanged()
{

}

QTEST_MAIN(StoryboardTimelineSyncTest)