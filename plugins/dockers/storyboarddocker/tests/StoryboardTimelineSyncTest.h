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

#ifndef __STORYBOARD_TIMELINE_SYNC_TEST_H
#define __STORYBOARD_TIMELINE_SYNC_TEST_H

#include <QtTest>

#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_undo_store.h"

class StoryboardModel;
class StoryboardView;

class StoryboardTimelineSyncTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testStoryboardItemSortedUniquePositive();
    void testStoryboardItemAddFromTimeline();
    void testStoryboardItemMoveFromTimeline();
    void testStoryboardItemRemoveFromTimeline();
    void testStorybaordTimeleineSync();
    void testDurationChange();
    void testFpsChanged();

private:
    KisImageSP m_image;
    KisPaintLayerSP m_layer1;
    KisPaintLayerSP m_layer2;
    KisKeyframeChannel* m_channel1;
    KisKeyframeChannel* m_channel2;
    StoryboardModel *m_storyboardModel;
    StoryboardView *m_storyboardView;
};

#endif
