/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
#include "KisFrameCacheStore.h"

#include <KoColorSpace.h>
#include "kis_update_info.h"
#include "KisFrameDataSerializer.h"
#include "opengl/KisOpenGLUpdateInfoBuilder.h"

#define SANITY_CHECK

namespace {
struct SerializedFrameInfo {
    int levelOfDetail = 0;
    QRect dirtyImageRect;
};
}


struct KRITAUI_NO_EXPORT KisFrameCacheStore::Private
{
    Private(KisTextureTileInfoPoolSP pool, const QString &frameCachePath)
        : serializer(pool, frameCachePath)
    {
    }

    KisFrameDataSerializer serializer;
    QMap<int, SerializedFrameInfo> savedFrames;
};

KisFrameCacheStore::KisFrameCacheStore(KisTextureTileInfoPoolSP pool)
    : KisFrameCacheStore(pool, QString())
{
}

KisFrameCacheStore::KisFrameCacheStore(KisTextureTileInfoPoolSP pool, const QString &frameCachePath)
    : m_d(new Private(pool, frameCachePath))
{
}


KisFrameCacheStore::~KisFrameCacheStore()
{
}

void KisFrameCacheStore::saveFrame(int frameId, KisOpenGLUpdateInfoSP info)
{
    int pixelSize = 0;

    Q_FOREACH (auto tile, info->tileList) {
#ifdef SANITY_CHECK
        if (!pixelSize) {
            pixelSize = tile->pixelSize();
        } else {
            KIS_SAFE_ASSERT_RECOVER_RETURN(pixelSize == tile->pixelSize());
        }
#else
        pixelSize = tile->pixelSize();
        break;
#endif
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(pixelSize);

    SerializedFrameInfo frameInfo;
    frameInfo.levelOfDetail = info->levelOfDetail();
    frameInfo.dirtyImageRect = info->dirtyImageRect();

    // TODO: assert that dirty image rect is equal to the full image rect
    // TODO: assert tile color space coicides with the destination color space

    m_d->savedFrames.insert(frameId, frameInfo);

    KisFrameDataSerializer::Frame frame;
    frame.frameId = frameId;
    frame.pixelSize = pixelSize;

    for (auto it = info->tileList.begin(); it != info->tileList.end(); ++it) {
        KisFrameDataSerializer::FrameTile tile(m_d->serializer.tileInfoPool());
        tile.col = (*it)->tileCol();
        tile.row = (*it)->tileRow();
        tile.rect = (*it)->realPatchRect();
        tile.data = std::move((*it)->takePixelData());

        frame.frameTiles.push_back(std::move(tile));
    }

    m_d->serializer.saveFrame(frame);

}

KisOpenGLUpdateInfoSP KisFrameCacheStore::loadFrame(int frameId, const KisOpenGLUpdateInfoBuilder &builder)
{
    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->savedFrames.contains(frameId), info);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->serializer.hasFrame(frameId), info);

    const SerializedFrameInfo &savedFrameInfo = m_d->savedFrames[frameId];

    info->assignDirtyImageRect(savedFrameInfo.dirtyImageRect);
    info->assignLevelOfDetail(savedFrameInfo.levelOfDetail);

    KisFrameDataSerializer::Frame frame = m_d->serializer.loadFrame(frameId);
    for (auto it = frame.frameTiles.begin(); it != frame.frameTiles.end(); ++it) {
        KisFrameDataSerializer::FrameTile &tile = *it;

        QRect patchRect = tile.rect;

        if (savedFrameInfo.levelOfDetail) {
            patchRect = KisLodTransform::upscaledRect(patchRect, savedFrameInfo.levelOfDetail);
        }

        const QRect fullSizeTileRect =
            builder.calculatePhysicalTileRect(tile.col, tile.row,
                                              savedFrameInfo.dirtyImageRect,
                                              savedFrameInfo.levelOfDetail);

        KisTextureTileUpdateInfoSP tileInfo(
            new KisTextureTileUpdateInfo(tile.col, tile.row,
                                         fullSizeTileRect, patchRect,
                                         savedFrameInfo.dirtyImageRect,
                                         savedFrameInfo.levelOfDetail,
                                         m_d->serializer.tileInfoPool()));

        tileInfo->putPixelData(std::move(tile.data), builder.destinationColorSpace());

        info->tileList << tileInfo;
    }

    return info;
}

void KisFrameCacheStore::forgetFrame(int frameId)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->savedFrames.contains(frameId));
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->serializer.hasFrame(frameId));

    m_d->savedFrames.remove(frameId);
    m_d->serializer.forgetFrame(frameId);
}

bool KisFrameCacheStore::hasFrame(int frameId) const
{
    return m_d->savedFrames.contains(frameId);
}
