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

#include "StoryboardTimelineSyncTest.h"
#include "StoryboardModel.h"
#include "StoryboardView.h"

#include <kis_group_layer.h>

#include <QTest>


void StoryboardTimelineSyncTest::initTestCase()
{
    m_image = new KisImage(0, 100, 100, nullptr, "image");

    m_storyboardModel = new StoryboardModel(this);
    m_storyboardView = new StoryboardView();
    m_storyboardModel->setView(m_storyboardView);
    m_storyboardView->setModel(m_storyboardModel);
    m_storyboardModel->setImage(m_image);

    m_layer1 = new KisPaintLayer(m_image, "layer1", OPACITY_OPAQUE_U8);
    m_image->addNode(m_layer1, m_image->rootLayer());

    m_layer1->enableAnimation();
    m_channel1 = m_layer1->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);

    m_layer2 = new KisPaintLayer(m_image, "layer2", OPACITY_OPAQUE_U8);
    m_image->addNode(m_layer2, m_image->rootLayer());

    m_layer2->enableAnimation();
    m_channel2 = m_layer2->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);

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

    //delete all keyframes
    foreach(int time,  m_channel1->allKeyframeTimes()) {
        m_channel1->removeKeyframe(time);
    }
    foreach(int time,  m_channel2->allKeyframeTimes()) {
        m_channel2->removeKeyframe(time);
    }

    QCOMPARE(m_channel1->keyframeCount(), 1);
    QCOMPARE(m_channel2->keyframeCount(), 1);
    QCOMPARE(m_storyboardModel->rowCount(), 1);
}

void StoryboardTimelineSyncTest::testStoryboardItemSortedUniquePositive()
{
    int numRows = m_storyboardModel->rowCount();

    int lastFrame = -1;
    for (int i = 0; i < numRows; i++) {
        QModelIndex parentIndex = m_storyboardModel->index(i, 0);
        QModelIndex frameIndex = m_storyboardModel->index(StoryboardItem::FrameNumber, 0, parentIndex);
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

    testStoryboardItemAddFromTimeline();

    /*          0 1 2 3 4 5 6 7 8 9 
      channel1  | . | . . . . . . .
      channel2  | . | . | . . . . .
    */

    QCOMPARE(m_storyboardModel->rowCount(), 3);

    m_channel1->moveKeyframe(2, 3);
    QCOMPARE(m_storyboardModel->rowCount(), 4);

    m_channel1->moveKeyframe(3, 4);
    QCOMPARE(m_storyboardModel->rowCount(), 3);
}

void StoryboardTimelineSyncTest::testStoryboardItemRemoveFromTimeline()
{
    testStoryboardItemAddFromTimeline();

    /*          0 1 2 3 4 5 6 7 8 9 
      channel1  | . | . . . . . . .
      channel2  | . | . | . . . . .
    */

    m_channel2->removeKeyframe(2);
    QCOMPARE(m_storyboardModel->rowCount(), 3);

    m_channel1->removeKeyframe(2);
    QCOMPARE(m_storyboardModel->rowCount(), 2);
}

void StoryboardTimelineSyncTest::testStorybaordTimeleineSync()
{

    testStoryboardItemAddFromTimeline();

    /*          0 1 2 3 4 5 6 7 8 9
      channel1  | . | . . . . . . .
      channel2  | . | . | . . . . .
    */
    QSignalSpy spyTimeChanged(m_image->animationInterface() , SIGNAL(sigUiTimeChanged(int)));
    QVERIFY(spyTimeChanged.isValid());

    m_image->animationInterface()->switchCurrentTimeAsync(2);
    QCOMPARE(spyTimeChanged.count(), 1);

    QModelIndex parentIndex = m_storyboardView->currentIndex();
    QVERIFY(parentIndex.isValid());
    QCOMPARE(m_storyboardModel->index(0, 0, parentIndex).data().toInt(), 2);

    m_image->animationInterface()->switchCurrentTimeAsync(3);
    parentIndex = m_storyboardView->selectionModel()->currentIndex();
    QCOMPARE(m_storyboardModel->index(0, 0, parentIndex).data().toInt(), 2);

    m_image->animationInterface()->switchCurrentTimeAsync(4);
    parentIndex = m_storyboardView->selectionModel()->currentIndex();
    QCOMPARE(m_storyboardModel->index(0, 0, parentIndex).data().toInt(), 4);
}

void StoryboardTimelineSyncTest::testDurationChange()
{
    int fps = m_image->animationInterface()->framerate();

    QCOMPARE(m_storyboardModel->rowCount(), 1);

    m_channel1->addKeyframe(fps + 1);
    QCOMPARE(m_storyboardModel->rowCount(), 2);

    QModelIndex parentIndex = m_storyboardModel->indexFromFrame(0);
    QCOMPARE(m_storyboardModel->index(StoryboardItem::DurationSecond, 0, parentIndex).data().toInt(), 1);
    QCOMPARE(m_storyboardModel->index(StoryboardItem::DurationFrame, 0, parentIndex).data().toInt(), 0);

    m_storyboardModel->setData(m_storyboardModel->index(StoryboardItem::DurationFrame, 0, parentIndex), 3);

    //keyframes at 0 and fps + 3 + 1 = fps + 4
    QVERIFY(m_channel1->keyframeAt(fps + 1).isNull());
    QVERIFY(!m_channel1->keyframeAt(fps + 4).isNull());
}

void StoryboardTimelineSyncTest::testFpsChanged()
{
    int fpsbefore = m_image->animationInterface()->framerate();
    testDurationChange();
    //keyframes at 0 and fps + 4

    m_image->animationInterface()->setFramerate(fpsbefore / 2);
    int fpsafter = m_image->animationInterface()->framerate();

    QModelIndex parentIndex = m_storyboardModel->indexFromFrame(0);
    QCOMPARE(m_storyboardModel->index(StoryboardItem::DurationSecond, 0, parentIndex).data().toInt(), (fpsbefore + 3) / fpsafter);
    QCOMPARE(m_storyboardModel->index(StoryboardItem::DurationFrame, 0, parentIndex).data().toInt(), (fpsbefore + 3) % fpsafter);
}

QTEST_MAIN(StoryboardTimelineSyncTest)
