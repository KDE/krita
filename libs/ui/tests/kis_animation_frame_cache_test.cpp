/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_frame_cache_test.h"

#include <simpletest.h>
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

    KisOpenGLImageTexturesSP glTex = KisOpenGLImageTextures::createImageTextures(image, 0, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::Empty);
    KisAnimationFrameCacheSP cache = new KisAnimationFrameCache(glTex);
    glTex->testingForceInitialized();

    m_globalAnimationCache = cache.data();
    connect(animation, SIGNAL(sigFrameReady(int)), this, SLOT(slotFrameGenerationFinished(int)));

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

void KisAnimationFrameCacheTest::slotFrameGenerationFinished(int time)
{
    KisImageSP image = m_globalAnimationCache->image();
    KisOpenGLUpdateInfoSP info = m_globalAnimationCache->fetchFrameData(time, image, KisRegion(image->bounds()));
    m_globalAnimationCache->addConvertedFrameData(info, time);

}

#include <kis_animation_frame_cache_p.h>

using MapType = QMap<int, int>;
using DroppedFramesType = std::vector<int>;
using MovedFramesType = std::vector<std::pair<int, int>>;


struct TestingFramesGluer : FramesGluerBase
{
    TestingFramesGluer(QMap<int, int> &_frames) : FramesGluerBase(_frames) {}

    DroppedFramesType droppedSwapFrames;
    MovedFramesType movedSwapFrames;

    void moveFrame(int oldStart, int newStart) override
    {
        movedSwapFrames.emplace_back(oldStart, newStart);
    }
    void forgetFrame(int start) override
    {
        droppedSwapFrames.emplace_back(start);
    }
};


void KisAnimationFrameCacheTest::testFrameGlueing_data()
{
    QTest::addColumn<KisTimeSpan>("glueRange");
    QTest::addColumn<MapType>("referenceFrames");
    QTest::addColumn<DroppedFramesType>("droppedSwapFrames");
    QTest::addColumn<MovedFramesType>("movedSwapFrames");
    QTest::addColumn<bool>("framesChanged");

    QTest::newRow("overlap-first")
        << KisTimeSpan::fromTimeWithDuration(0, 3)
        << QMap<int, int> {
               {0, 3},
               {5, 3},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{}
        << false;

    QTest::newRow("extend-first")
        << KisTimeSpan::fromTimeWithDuration(1, 3)
        << QMap<int, int> {
               {0, 4},
               {5, 3},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-first-up-to-next")
        << KisTimeSpan::fromTimeWithDuration(2, 3)
        << QMap<int, int> {
               {0, 5},
               {5, 3},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-first-consume-next")
        << KisTimeSpan::fromTimeWithDuration(2, 4)
        << QMap<int, int> {
               {0, 6},
               {6, 2},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{{5, 6}}
        << true;

    QTest::newRow("extend-first-consume-next-fully")
        << KisTimeSpan::fromTimeWithDuration(2, 6)
        << QMap<int, int> {
               {0, 8},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{5}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-first-consume-1.5")
        << KisTimeSpan::fromTimeWithDuration(2, 7)
        << QMap<int, int> {
               {0, 9},
               {9, 2},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{5}
        << MovedFramesType{{8, 9}}
        << true;

    QTest::newRow("extend-first-consume-2")
        << KisTimeSpan::fromTimeWithDuration(2, 9)
        << QMap<int, int> {
               {0, 11},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{5, 8}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-first-infinite")
        << KisTimeSpan::infinite(2)
        << QMap<int, int> {
               {0, -1}
           }
        << DroppedFramesType{5, 8, 11, 16}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-middle")
        << KisTimeSpan::fromTimeWithDuration(12, 3)
        << QMap<int, int> {
               {0, 3},
               {5, 3},
               {8, 3},
               {11, 4},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{}
        << true;

    QTest::newRow("extend-middle-half-consume-infinite")
        << KisTimeSpan::fromTimeWithDuration(12, 5)
        << QMap<int, int> {
               {0, 3},
               {5, 3},
               {8, 3},
               {11, 6},
               {17, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{{16, 17}}
        << true;

    QTest::newRow("extend-middle-consume-infinite")
        << KisTimeSpan::infinite(12)
        << QMap<int, int> {
               {0, 3},
               {5, 3},
               {8, 3},
               {11, -1}
           }
        << DroppedFramesType{16}
        << MovedFramesType{}
        << true;

    QTest::newRow("end-consume-infinite")
        << KisTimeSpan::infinite(17)
        << QMap<int, int> {
               {0, 3},
               {5, 3},
               {8, 3},
               {11, 3},
               {16, -1}
           }
        << DroppedFramesType{}
        << MovedFramesType{}
        << false;
}

void KisAnimationFrameCacheTest::testFrameGlueing()
{
    QMap<int, int> frames;

    frames.insert(0, 3);
    frames.insert(5, 3);
    frames.insert(8, 3);
    frames.insert(11, 3);
    frames.insert(16, -1);

    const QMap<int, int> originalFrames = frames;

    QFETCH(KisTimeSpan, glueRange);
    QFETCH(MapType, referenceFrames);
    QFETCH(bool, framesChanged);
    QFETCH(DroppedFramesType, droppedSwapFrames);
    QFETCH(MovedFramesType, movedSwapFrames);

    TestingFramesGluer gluer(frames);

    const bool result = gluer.glueFrames(glueRange);

    if (frames != referenceFrames) {
        qDebug() << "=== FAILURE ===";
        qDebug() << ppVar(originalFrames);
        qDebug() << ppVar(glueRange);
        qDebug() << "===============";
        qDebug() << ppVar(frames);
        qDebug() << ppVar(referenceFrames);
        qDebug() << "===============";

        QFAIL("unexpected frames after gluing");
    }

    if (gluer.droppedSwapFrames != droppedSwapFrames ||
        gluer.movedSwapFrames != movedSwapFrames) {

        qDebug() << "=== FAILURE (swapper callbacks) ===";
        qDebug() << ppVar(originalFrames);
        qDebug() << ppVar(glueRange);
        qDebug() << "===================================";
        qDebug() << ppVar(frames);
        qDebug() << ppVar(referenceFrames);
        qDebug() << ppVar(droppedSwapFrames);
        qDebug() << ppVar(gluer.droppedSwapFrames);
        qDebug() << ppVar(movedSwapFrames);
        qDebug() << ppVar(gluer.movedSwapFrames);
        qDebug() << "===================================";

        QFAIL("unexpected swapper callbacks");
    }

    QCOMPARE(result, framesChanged);
}

KISTEST_MAIN(KisAnimationFrameCacheTest)
