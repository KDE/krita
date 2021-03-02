/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFrameCacheStoreTest.h"

#include <simpletest.h>
#include <testutil.h>

#include <KoColor.h>
#include "KisAsyncAnimationRendererBase.h"
#include "kis_image_animation_interface.h"
#include "opengl/KisOpenGLUpdateInfoBuilder.h"

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"

// TODO: conversion options into a separate file!
#include "kis_update_info.h"

#include "opengl/kis_texture_tile_update_info.h"


#include "KisFrameCacheStore.h"

static const int maxTileSize = 256;

bool compareTextureTileUpdateInfo(KisTextureTileUpdateInfoSP tile1, KisTextureTileUpdateInfoSP tile2)
{
    KIS_COMPARE_RF(tile1->patchLevelOfDetail(), tile2->patchLevelOfDetail());
    KIS_COMPARE_RF(tile1->realPatchOffset(), tile2->realPatchOffset());
    KIS_COMPARE_RF(tile1->realPatchRect(), tile2->realPatchRect());
    KIS_COMPARE_RF(tile1->realTileSize(), tile2->realTileSize());
    KIS_COMPARE_RF(tile1->isTopmost(), tile2->isTopmost());
    KIS_COMPARE_RF(tile1->isLeftmost(), tile2->isLeftmost());
    KIS_COMPARE_RF(tile1->isRightmost(), tile2->isRightmost());
    KIS_COMPARE_RF(tile1->isBottommost(), tile2->isBottommost());
    KIS_COMPARE_RF(tile1->isEntireTileUpdated(), tile2->isEntireTileUpdated());

    KIS_COMPARE_RF(tile1->tileCol(), tile2->tileCol());
    KIS_COMPARE_RF(tile1->tileRow(), tile2->tileRow());
    KIS_COMPARE_RF(tile1->pixelSize(), tile2->pixelSize());
    KIS_COMPARE_RF(tile1->valid(), tile2->valid());

    KIS_COMPARE_RF(tile1->patchPixelsLength(), tile2->patchPixelsLength());


    const uint numRealPixelBytes = static_cast<uint>(tile1->realPatchRect().width() * tile1->realPatchRect().height() * tile1->pixelSize());

    if (memcmp(tile1->data(), tile2->data(), numRealPixelBytes) != 0) {
        qWarning() << "Tile pixels differ!";
        qWarning() << "    " << ppVar(tile1->tileCol()) << ppVar(tile1->tileRow());
        qWarning() << "    " << ppVar(numRealPixelBytes);

        quint8 *src = tile1->data();
        quint8 *dst = tile2->data();

        for (uint i = 0; i < numRealPixelBytes; i++) {
            if (*src != *dst) {
                qDebug() << "    " << ppVar(i) << ppVar(*src) << ppVar(*dst);
            }

            src++;
            dst++;
        }

        return false;
    }

    return true;
}


bool compareUpdateInfo(KisOpenGLUpdateInfoSP info1, KisOpenGLUpdateInfoSP info2)
{
    KIS_COMPARE_RF(info1->dirtyImageRect(), info2->dirtyImageRect());
    KIS_COMPARE_RF(info1->levelOfDetail(), info2->levelOfDetail());
    KIS_COMPARE_RF(info1->tileList.size(), info2->tileList.size());

    for (int i = 0; i < info1->tileList.size(); i++) {
        if (!compareTextureTileUpdateInfo(info1->tileList[i], info2->tileList[i])) {
            return false;
        }
    }

    return true;
}


class TestFramesRenderer : public KisAsyncAnimationRendererBase
{
    Q_OBJECT

public:
    TestFramesRenderer()
        : m_pool(m_poolRegistry.getPool(maxTileSize, maxTileSize))
    {
        m_updateInfoBuilder.setTextureInfoPool(m_pool);

        const KoColorSpace *dstColorSpace = KoColorSpaceRegistry::instance()->rgb8();
        m_updateInfoBuilder.setConversionOptions(
            ConversionOptions(dstColorSpace,
                              KoColorConversionTransformation::internalRenderingIntent(),
                              KoColorConversionTransformation::internalConversionFlags()));

        // TODO: refactor setting texture size in raw values!
        m_updateInfoBuilder.setTextureBorder(8);
        m_updateInfoBuilder.setEffectiveTextureSize(QSize(256 - 16, 256 - 16));

        connect(this, SIGNAL(sigCompleteRegenerationInternal(int)), SLOT(notifyFrameCompleted(int)));
        connect(this, SIGNAL(sigCancelRegenerationInternal(int)), SLOT(notifyFrameCancelled(int)));
    }

    void frameCompletedCallback(int frame, const KisRegion &requestedRegion) override {
        KisImageSP image = requestedImage();
        KIS_SAFE_ASSERT_RECOVER_NOOP(frame == image->animationInterface()->currentTime());

        // by default we request update for the entire image
        KIS_SAFE_ASSERT_RECOVER_NOOP(requestedRegion == image->bounds());

        KisOpenGLUpdateInfoSP info = m_updateInfoBuilder.buildUpdateInfo(image->bounds(), image, true);

        KIS_ASSERT_RECOVER_NOOP(info);
        qDebug() << ppVar(info->tileList.size());

        KisOpenGLUpdateInfoSP infoForSave = m_updateInfoBuilder.buildUpdateInfo(image->bounds(), image, true);
        m_store.saveFrame(11, infoForSave, image->bounds());

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_store.hasFrame(11));

        KisOpenGLUpdateInfoSP loadedInfo = m_store.loadFrame(11, m_updateInfoBuilder);

        qDebug() << ppVar(loadedInfo->tileList.size());

        KIS_SAFE_ASSERT_RECOVER_NOOP(compareUpdateInfo(info, loadedInfo));


        emit sigCompleteRegenerationInternal(frame);
    }

    void frameCancelledCallback(int frame) override {
        emit sigCancelRegenerationInternal(frame);
    }

Q_SIGNALS:
    void sigCompleteRegenerationInternal(int frame);
    void sigCancelRegenerationInternal(int frame);

private:
    KisOpenGLUpdateInfoBuilder m_updateInfoBuilder;
    KisTextureTileInfoPoolRegistry m_poolRegistry;
    KisTextureTileInfoPoolSP m_pool;
    KisFrameCacheStore m_store;
};




void KisFrameCacheStoreTest::test()
{
    QRect refRect(QRect(0,0,512,512));
    TestUtil::MaskParent p(refRect);
    const KoColor fillColor(Qt::red, p.image->colorSpace());

    KisPaintLayerSP layer1 = p.layer;
    layer1->paintDevice()->fill(QRect(100,100,300,300), fillColor);

    TestFramesRenderer renderer;
    renderer.startFrameRegeneration(p.image, 10);


    p.image->waitForDone();

    while (renderer.isActive()) {
        QTest::qWait(500);
    }

}

SIMPLE_TEST_MAIN(KisFrameCacheStoreTest)

#include "KisFrameCacheStoreTest.moc"
