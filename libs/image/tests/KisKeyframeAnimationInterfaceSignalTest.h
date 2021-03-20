/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_KEYFRAME_ANIMATIONINTERFACE_SIGNAL_TEST_H
#define KIS_KEYFRAME_ANIMATIONINTERFACE_SIGNAL_TEST_H

#include <simpletest.h>

#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_undo_store.h"

class KisKeyframeAnimationInterfaceSignalTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase();
    void init();

    void testSignalFromKeyframeChannelToInterface();
    void testSignalOnImageReset();

private:
    KisImageSP m_image1;
    KisImageSP m_image2;
    KisPaintLayerSP m_layer;
    KisKeyframeChannel *m_channel;
};

#endif
