/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_onion_skin_compositor_test.h"

#include <QTest>

#include "kis_onion_skin_compositor.h"
#include "kis_paint_device.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_image_animation_interface.h"
#include "testutil.h"
#include "KoColor.h"
#include "kis_image_config.h"
#include "kis_config_notifier.h"

void KisOnionSkinCompositorTest::testComposite()
{
    KisImageConfig config;
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
    KisPaintDeviceSP expectedComposite = new KisPaintDevice(p.image->colorSpace());

    // Frame 0

    i->switchCurrentTimeAsync(0);
    p.image->waitForDone();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    expectedComposite->fill(QRect(256,0,256,256), KoColor(QColor(64, 191, 0, 128), paintDevice->colorSpace()));
    expectedComposite->fill(QRect(0,0,256,512), KoColor(Qt::red, paintDevice->colorSpace()));

    QImage result = compositeDevice->createThumbnail(64, 64);
    QImage expected = expectedComposite->createThumbnail(64, 64);
    QVERIFY(result == expected);

    // Frame 10

    i->switchCurrentTimeAsync(10);
    p.image->waitForDone();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    expectedComposite->clear();
    expectedComposite->fill(QRect(0,256,256,256), KoColor(QColor(106, 0, 149, 192), paintDevice->colorSpace()));
    expectedComposite->fill(QRect(256,256,256,256), KoColor(QColor(64, 0, 191, 128), paintDevice->colorSpace()));
    expectedComposite->fill(QRect(0,0,512,256), KoColor(Qt::green, paintDevice->colorSpace()));

    result = compositeDevice->createThumbnail(64, 64);
    expected = expectedComposite->createThumbnail(64, 64);

    QVERIFY(result == expected);

    // Frame 20

    i->switchCurrentTimeAsync(20);
    p.image->waitForDone();
    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));

    expectedComposite->clear();
    expectedComposite->fill(QRect(0,0,512,256), KoColor(QColor(0, 191, 64, 128), paintDevice->colorSpace()));
    expectedComposite->fill(QRect(0,256,512,256), KoColor(Qt::blue, paintDevice->colorSpace()));

    result = compositeDevice->createThumbnail(64, 64);
    expected = expectedComposite->createThumbnail(64, 64);
    QVERIFY(result == expected);

}

void KisOnionSkinCompositorTest::testSettings()
{
    KisOnionSkinCompositor *compositor = KisOnionSkinCompositor::instance();

    TestUtil::MaskParent p;
    KisImageAnimationInterface *i = p.image->animationInterface();
    KisPaintDeviceSP paintDevice = p.layer->paintDevice();
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

    KisImageConfig config;
    config.setOnionSkinOpacity(-1, 32);
    config.setOnionSkinOpacity(1, 192);
    config.setOnionSkinOpacity(2, 64);
    config.setOnionSkinTintFactor(0);

    KisPaintDeviceSP compositeDevice = new KisPaintDevice(p.image->colorSpace());
    KisPaintDeviceSP expectedComposite = new KisPaintDevice(p.image->colorSpace());

    config.setNumberOfOnionSkins(1);
    compositor->configChanged();

    expectedComposite->clear();
    expectedComposite->fill(QRect(0,0,512,512), KoColor(QColor(10, 245, 0, 200), paintDevice->colorSpace()));
    QImage expected = expectedComposite->createThumbnail(64, 64);

    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));
    QImage result = compositeDevice->createThumbnail(64, 64);

    QVERIFY(result == expected);

    config.setNumberOfOnionSkins(2);
    compositor->configChanged();

    expectedComposite->fill(QRect(0,0,512,512), KoColor(QColor(9, 229, 16, 214), paintDevice->colorSpace()));
    expected = expectedComposite->createThumbnail(64, 64);

    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));
    result = compositeDevice->createThumbnail(64, 64);

    QVERIFY(result == expected);

    // Test tint options

    config.setNumberOfOnionSkins(1);
    config.setOnionSkinTintFactor(64);
    config.setOnionSkinTintColorBackward(Qt::blue);
    config.setOnionSkinTintColorForward(Qt::red);
    compositor->configChanged();

    compositor->composite(paintDevice, compositeDevice, QRect(0,0,512,512));
    result = compositeDevice->createThumbnail(64, 64);

    expectedComposite->fill(QRect(0,0,512,512), KoColor(QColor(69, 183, 3, 200), paintDevice->colorSpace()));
    expected = expectedComposite->createThumbnail(64, 64);

    QVERIFY(result == expected);
}

QTEST_MAIN(KisOnionSkinCompositorTest)
#include "kis_onion_skin_compositor_test.moc"
