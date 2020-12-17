/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    void testAddKeyframeExtendsDuration();
    void testStoryboardTimelineTimeSyncronization();
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
