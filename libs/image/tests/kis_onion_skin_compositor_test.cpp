/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_onion_skin_compositor_test.h"

#include <simpletest.h>

#include "kis_onion_skin_compositor.h"
#include "kis_paint_device.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include <testutil.h>
#include "KoColor.h"
#include "kis_image_config.h"

void KisOnionSkinCompositorTest::testComposite()
{
    TestUtil::ReferenceImageChecker chk("composite", "onion_skins", TestUtil::ReferenceImageChecker::InternalStorage);

    KisImageConfig config(false);
    config.setOnionSkinTintFactor(64);
    config.setOnionSkinTintColorBackward(Qt::blue);
    config.setOnionSkinTintColorForward(Qt::red);
    config.setNumberOfOnionSkins(1);
    config.setOnionSkinOpacity(-1, 128);
    config.setOnionSkinOpacity(1, 128);

    KisOnionSkinCompositor *compositor = KisOnionSkinCompositor::instance();

    TestUtil::MaskParent p;

    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP paintDevice = p.layer->paintDevice();
    paintDevice->createKeyframeChannel(KoID());
    KisKeyframeChannel *keyframes = paintDevice->keyframeChannel();

    keyframes->addKeyframe(0);
    keyframes->addKeyframe(10);
    keyframes->addKeyframe(20);

    paintDevice->fill(QRect(0,0,256,512), KoColor(Qt::red, paintDevice->colorSpace()));

    i->switchCurrentTimeAsync(10);
    p.image->waitForDone();

    paintDevice->fill(QRect(0,0,512,256), KoColor(Qt::green, paintDevice->colorSpace()));

    i->switchCurrentTimeAsync(20);
    p.image->waitForDone();

    paintDevice->fill(QRect(0,256,512,256), KoColor(Qt::blue, paintDevice->colorSpace()));

    KisPaintDeviceSP compositeDevice = new KisPaintDevice(p.image->colorSpace());

    // Frame 0

    i->switchCurrentTimeAsync(0);
    p.image->waitForDone();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    QVERIFY(chk.checkDevice(compositeDevice, p.image, "frame00"));

    // Frame 10

    i->switchCurrentTimeAsync(10);
    p.image->waitForDone();
    compositeDevice->clear();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    QVERIFY(chk.checkDevice(compositeDevice, p.image, "frame10"));

    // Frame 20

    i->switchCurrentTimeAsync(20);
    p.image->waitForDone();
    compositeDevice->clear();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    QVERIFY(chk.checkDevice(compositeDevice, p.image, "frame20"));
}

void KisOnionSkinCompositorTest::testSettings()
{
    TestUtil::ReferenceImageChecker chk("settings", "onion_skins", TestUtil::ReferenceImageChecker::InternalStorage);

    KisOnionSkinCompositor *compositor = KisOnionSkinCompositor::instance();

    TestUtil::MaskParent p;
    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP paintDevice = p.layer->paintDevice();
    paintDevice->createKeyframeChannel(KoID());
    KisKeyframeChannel *keyframes = paintDevice->keyframeChannel();

    keyframes->addKeyframe(0);
    keyframes->addKeyframe(1);
    keyframes->addKeyframe(2);
    keyframes->addKeyframe(3);

    paintDevice->fill(QRect(0,0,512,512), KoColor(Qt::red, paintDevice->colorSpace()));

    i->switchCurrentTimeAsync(2);
    p.image->waitForDone();

    paintDevice->fill(QRect(0,0,512,512), KoColor(Qt::green, paintDevice->colorSpace()));

    i->switchCurrentTimeAsync(3);
    p.image->waitForDone();

    paintDevice->fill(QRect(0,0,512,512), KoColor(Qt::blue, paintDevice->colorSpace()));

    i->switchCurrentTimeAsync(1);
    p.image->waitForDone();

    KisImageConfig config(false);
    config.setOnionSkinOpacity(-1, 32);
    config.setOnionSkinOpacity(1, 192);
    config.setOnionSkinOpacity(2, 64);
    config.setOnionSkinTintFactor(0);

    KisPaintDeviceSP compositeDevice = new KisPaintDevice(p.image->colorSpace());

    config.setNumberOfOnionSkins(1);
    compositor->configChanged();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    QVERIFY(chk.checkDevice(compositeDevice, p.image, "00_single_skin"));

    config.setNumberOfOnionSkins(2);
    compositor->configChanged();

    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));
    QVERIFY(chk.checkDevice(compositeDevice, p.image, "01_double_skin"));

    // Test tint options

    config.setNumberOfOnionSkins(1);
    config.setOnionSkinTintFactor(64);
    config.setOnionSkinTintColorBackward(Qt::blue);
    config.setOnionSkinTintColorForward(Qt::red);
    compositor->configChanged();

    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));
    QVERIFY(chk.checkDevice(compositeDevice, p.image, "02_single_skin_tinted"));
}

SIMPLE_TEST_MAIN(KisOnionSkinCompositorTest)
