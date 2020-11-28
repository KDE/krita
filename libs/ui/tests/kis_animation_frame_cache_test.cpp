/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_frame_cache_test.h"

#include <QTest>
#include <testutil.h>

#include "kis_animation_frame_cache.h"
#include "kis_image_animation_interface.h"
#include "opengl/kis_opengl_image_textures.h"
#include "kis_time_span.h"
#include "kis_keyframe_channel.h"
#include "kistest.h"

#include "kundo2command.h"

void verifyRangeIsCachedStatus(KisAnimationFrameCacheSP cache, int start, int end, KisAnimationFrameCache::CacheStatus status)
{
    for (int t = start; t <= end; t++) {
        QVERIFY2(
            cache->frameStatus(t) == status,
            qPrintable(QString("Expected status %1 for frame %2 in range %3 to %4").arg(status == KisAnimationFrameCache::Cached ? "Cached" : "Uncached").arg(t).arg(start).arg(end))
        );
    }
}

void KisAnimationFrameCacheTest::testCache()
{
    TestUtil::MaskParent p;
    KisImageSP image = p.image;
    KisImageAnimationInterface *animation = image->animationInterface();
    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer3 = new KisPaintLayer(p.image, "", OPACITY_OPAQUE_U8);
    image->addNode(layer2);
    image->addNode(layer3);

    KUndo2Command parentCommand;

    KisKeyframeChannel *rasterChannel2 = layer2->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
    rasterChannel2->addKeyframe(10, &parentCommand);
    rasterChannel2->addKeyframe(20, &parentCommand);
    rasterChannel2->addKeyframe(30, &parentCommand);

    KisKeyframeChannel *rasterChannel3 = layer2->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
    rasterChannel3->addKeyframe(17, &parentCommand);

    KisOpenGLImageTexturesSP glTex = KisOpenGLImageTextures::getImageTextures(image, 0, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::Empty);
    KisAnimationFrameCacheSP cache = new KisAnimationFrameCache(glTex);
    glTex->testingForceInitialized();

    m_globalAnimationCache = cache.data();
    connect(animation, SIGNAL(sigFrameReady(int)), this, SLOT(slotFrameGerenationFinished(int)));

    int t;
    animation->saveAndResetCurrentTime(11, &t);
    animation->notifyFrameReady();

    QCOMPARE(cache->frameStatus(9), KisAnimationFrameCache::Uncached);
    verifyRangeIsCachedStatus(cache, 10, 16, KisAnimationFrameCache::Cached);
    QCOMPARE(cache->frameStatus(17), KisAnimationFrameCache::Uncached);

    animation->saveAndResetCurrentTime(30, &t);
    animation->notifyFrameReady();

    QCOMPARE(cache->frameStatus(29), KisAnimationFrameCache::Uncached);
    verifyRangeIsCachedStatus(cache, 30, 40, KisAnimationFrameCache::Cached);
    QCOMPARE(cache->frameStatus(9999), KisAnimationFrameCache::Cached);

    image->invalidateFrames(KisTimeSpan::fromTimeToTime(10, 12), QRect());
    verifyRangeIsCachedStatus(cache, 10, 12, KisAnimationFrameCache::Uncached);
    verifyRangeIsCachedStatus(cache, 13, 16, KisAnimationFrameCache::Cached);

    image->invalidateFrames(KisTimeSpan::fromTimeToTime(15, 20), QRect());
    verifyRangeIsCachedStatus(cache, 13, 14, KisAnimationFrameCache::Cached);
    verifyRangeIsCachedStatus(cache, 15, 20, KisAnimationFrameCache::Uncached);

    image->invalidateFrames(KisTimeSpan::infinite(100), QRect());
    verifyRangeIsCachedStatus(cache, 90, 99, KisAnimationFrameCache::Cached);
    verifyRangeIsCachedStatus(cache, 100, 110, KisAnimationFrameCache::Uncached);

    image->invalidateFrames(KisTimeSpan::fromTimeToTime(90, 100), QRect());
    verifyRangeIsCachedStatus(cache, 80, 89, KisAnimationFrameCache::Cached);
    verifyRangeIsCachedStatus(cache, 90, 100, KisAnimationFrameCache::Uncached);

    image->invalidateFrames(KisTimeSpan::infinite(14), QRect());
    QCOMPARE(cache->frameStatus(13), KisAnimationFrameCache::Cached);
    verifyRangeIsCachedStatus(cache, 15, 100, KisAnimationFrameCache::Uncached);

}

void KisAnimationFrameCacheTest::slotFrameGerenationFinished(int time)
{
    KisImageSP image = m_globalAnimationCache->image();
    KisOpenGLUpdateInfoSP info = m_globalAnimationCache->fetchFrameData(time, image, KisRegion(image->bounds()));
    m_globalAnimationCache->addConvertedFrameData(info, time);

}

KISTEST_MAIN(KisAnimationFrameCacheTest)
