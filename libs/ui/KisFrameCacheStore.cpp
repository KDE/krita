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
enum FrameType {
    FrameFull,
    FrameCopy,
    FrameDiff
};

struct FrameInfo;
typedef QSharedPointer<FrameInfo> FrameInfoSP;

struct FrameInfo {
    // full frame
    FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, const KisFrameDataSerializer::Frame &frame);
    // diff frame
    FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, FrameInfoSP baseFrame, const KisFrameDataSerializer::Frame &frame);
    // copy frame
    FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, FrameInfoSP baseFrame);

    ~FrameInfo();

    FrameType type() const {
        return m_type;
    }

    int levelOfDetail() const {
        return m_levelOfDetail;
    }

    QRect dirtyImageRect() const {
        return m_dirtyImageRect;
    }

    QRect imageBounds() const {
        return m_imageBounds;
    }

    int frameDataId() const {
        return m_savedFrameDataId;
    }

    FrameInfoSP baseFrame() const {
        return m_baseFrame;
    }

    int m_levelOfDetail = 0;
    QRect m_dirtyImageRect;
    QRect m_imageBounds;
    FrameInfoSP m_baseFrame;
    FrameType m_type = FrameFull;
    int m_savedFrameDataId = -1;
    KisFrameDataSerializer &m_serializer;
};

// full frame
FrameInfo::FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, const KisFrameDataSerializer::Frame &frame)
    : m_levelOfDetail(levelOfDetail),
      m_dirtyImageRect(dirtyImageRect),
      m_imageBounds(imageBounds),
      m_baseFrame(0),
      m_type(FrameFull),
      m_serializer(serializer)
{
    m_savedFrameDataId = m_serializer.saveFrame(frame);
}

// diff frame
FrameInfo::FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, FrameInfoSP baseFrame, const KisFrameDataSerializer::Frame &frame)
    : m_levelOfDetail(levelOfDetail),
      m_dirtyImageRect(dirtyImageRect),
      m_imageBounds(imageBounds),
      m_baseFrame(baseFrame),
      m_type(FrameDiff),
      m_serializer(serializer)
{
    m_savedFrameDataId = m_serializer.saveFrame(frame);
}

// copy frame
FrameInfo::FrameInfo(const QRect &dirtyImageRect, const QRect &imageBounds, int levelOfDetail, KisFrameDataSerializer &serializer, FrameInfoSP baseFrame)
    : m_levelOfDetail(levelOfDetail),
      m_dirtyImageRect(dirtyImageRect),
      m_imageBounds(imageBounds),
      m_baseFrame(baseFrame),
      m_type(FrameCopy),
      m_savedFrameDataId(-1),
      m_serializer(serializer)
{
}

FrameInfo::~FrameInfo()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_savedFrameDataId >= 0 || m_type == FrameCopy);

    if (m_savedFrameDataId >= 0) {
        m_serializer.forgetFrame(m_savedFrameDataId);
    }
}

}


struct KRITAUI_NO_EXPORT KisFrameCacheStore::Private
{
    Private(const QString &frameCachePath)
        : serializer(frameCachePath)
    {
    }

    // the serializer should be killed after *all* the frame info objects
    // got destroyed, because they use it in their own destruction
    KisFrameDataSerializer serializer;

    KisFrameDataSerializer::Frame lastSavedFullFrame;
    int lastSavedFullFrameId = -1;

    KisFrameDataSerializer::Frame lastLoadedBaseFrame;
    FrameInfoSP lastLoadedBaseFrameInfo;

    QMap<int, FrameInfoSP> savedFrames;
};

KisFrameCacheStore::KisFrameCacheStore()
    : KisFrameCacheStore(QString())
{
}

KisFrameCacheStore::KisFrameCacheStore(const QString &frameCachePath)
    : m_d(new Private(frameCachePath))
{
}


KisFrameCacheStore::~KisFrameCacheStore()
{
}

void KisFrameCacheStore::saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds)
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

    // TODO: assert that dirty image rect is equal to the full image rect
    // TODO: assert tile color space coicides with the destination color space

    KisFrameDataSerializer::Frame frame;
    frame.pixelSize = pixelSize;

    for (auto it = info->tileList.begin(); it != info->tileList.end(); ++it) {
        KisFrameDataSerializer::FrameTile tile(KisTextureTileInfoPoolSP(0)); // TODO: fix the pool should never be null!
        tile.col = (*it)->tileCol();
        tile.row = (*it)->tileRow();
        tile.rect = (*it)->realPatchRect();
        tile.data = std::move((*it)->takePixelData());

        frame.frameTiles.push_back(std::move(tile));
    }

    FrameInfoSP frameInfo;

    if (m_d->lastSavedFullFrame.isValid()) {
        boost::optional<qreal> uniqueness = KisFrameDataSerializer::estimateFrameUniqueness(m_d->lastSavedFullFrame, frame, 0.01);

        if (uniqueness) {
            if (*uniqueness == 0.0) {
                FrameInfoSP baseFrameInfo = m_d->savedFrames[m_d->lastSavedFullFrameId];
                frameInfo = toQShared(new FrameInfo(info->dirtyImageRect(),
                                                    imageBounds,
                                                    info->levelOfDetail(),
                                                    m_d->serializer,
                                                    baseFrameInfo));

            } else if (*uniqueness < 0.5) {
                FrameInfoSP baseFrameInfo = m_d->savedFrames[m_d->lastSavedFullFrameId];

                KisFrameDataSerializer::subtractFrames(frame, m_d->lastSavedFullFrame);
                frameInfo = toQShared(new FrameInfo(info->dirtyImageRect(),
                                                    imageBounds,
                                                    info->levelOfDetail(),
                                                    m_d->serializer,
                                                    baseFrameInfo,
                                                    frame));
            }
        }
    }

    if (!frameInfo) {
        frameInfo = toQShared(new FrameInfo(info->dirtyImageRect(),
                                            imageBounds,
                                            info->levelOfDetail(),
                                            m_d->serializer,
                                            frame));
    }

    m_d->savedFrames.insert(frameId, frameInfo);

    if (frameInfo->type() == FrameFull) {
        m_d->lastSavedFullFrame = std::move(frame);
        m_d->lastSavedFullFrameId = frameId;
    }
}

KisOpenGLUpdateInfoSP KisFrameCacheStore::loadFrame(int frameId, const KisOpenGLUpdateInfoBuilder &builder)
{
    KisOpenGLUpdateInfoSP info = new KisOpenGLUpdateInfo();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->savedFrames.contains(frameId), info);

    FrameInfoSP frameInfo = m_d->savedFrames[frameId];

    info->assignDirtyImageRect(frameInfo->dirtyImageRect());
    info->assignLevelOfDetail(frameInfo->levelOfDetail());

    KisFrameDataSerializer::Frame frame;

    switch (frameInfo->type()) {
    case FrameFull:
        frame = m_d->serializer.loadFrame(frameInfo->frameDataId(), builder.textureInfoPool());
        m_d->lastLoadedBaseFrame = frame.clone();
        m_d->lastLoadedBaseFrameInfo = frameInfo;
        break;
    case FrameCopy: {
        FrameInfoSP baseFrameInfo = frameInfo->baseFrame();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(baseFrameInfo, KisOpenGLUpdateInfoSP());

        if (baseFrameInfo == m_d->lastLoadedBaseFrameInfo) {
            frame = m_d->lastLoadedBaseFrame.clone();
        } else {
            frame = m_d->serializer.loadFrame(baseFrameInfo->frameDataId(), builder.textureInfoPool());
            m_d->lastLoadedBaseFrame = frame.clone();
            m_d->lastLoadedBaseFrameInfo = baseFrameInfo;
        }
        break;
    }
    case FrameDiff: {
        FrameInfoSP baseFrameInfo = frameInfo->baseFrame();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(baseFrameInfo, KisOpenGLUpdateInfoSP());

        if (baseFrameInfo == m_d->lastLoadedBaseFrameInfo) {
            // noop
        } else {
            m_d->lastLoadedBaseFrame = m_d->serializer.loadFrame(baseFrameInfo->frameDataId(), builder.textureInfoPool());
            m_d->lastLoadedBaseFrameInfo = baseFrameInfo;

            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->lastLoadedBaseFrame.isValid(), KisOpenGLUpdateInfoSP());
        }

        const KisFrameDataSerializer::Frame &baseFrame = m_d->lastLoadedBaseFrame;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(baseFrame.isValid(), KisOpenGLUpdateInfoSP());

        frame = m_d->serializer.loadFrame(frameInfo->frameDataId(), builder.textureInfoPool());
        KisFrameDataSerializer::addFrames(frame, baseFrame);
        break;
    }
    }

    for (auto it = frame.frameTiles.begin(); it != frame.frameTiles.end(); ++it) {
        KisFrameDataSerializer::FrameTile &tile = *it;

        QRect patchRect = tile.rect;

        if (frameInfo->levelOfDetail()) {
            patchRect = KisLodTransform::upscaledRect(patchRect, frameInfo->levelOfDetail());
        }

        const QRect fullSizeTileRect =
            builder.calculatePhysicalTileRect(tile.col, tile.row,
                                              frameInfo->imageBounds(),
                                              frameInfo->levelOfDetail());

        KisTextureTileUpdateInfoSP tileInfo(
            new KisTextureTileUpdateInfo(tile.col, tile.row,
                                         fullSizeTileRect, patchRect,
                                         frameInfo->imageBounds(),
                                         frameInfo->levelOfDetail(),
                                         builder.textureInfoPool()));

        tileInfo->putPixelData(std::move(tile.data), builder.destinationColorSpace());

        info->tileList << tileInfo;
    }

    return info;
}

void KisFrameCacheStore::moveFrame(int srcFrameId, int dstFrameId)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(srcFrameId != dstFrameId);

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->savedFrames.contains(srcFrameId));

    KIS_SAFE_ASSERT_RECOVER(!m_d->savedFrames.contains(dstFrameId)) {
        m_d->savedFrames.remove(dstFrameId);
    }

    m_d->savedFrames.insert(dstFrameId, m_d->savedFrames[srcFrameId]);
    m_d->savedFrames.remove(srcFrameId);

    if (m_d->lastSavedFullFrameId == srcFrameId) {
        m_d->lastSavedFullFrameId = dstFrameId;
    }
}

void KisFrameCacheStore::forgetFrame(int frameId)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->savedFrames.contains(frameId));

    if (m_d->lastSavedFullFrameId == frameId) {
        m_d->lastSavedFullFrame = KisFrameDataSerializer::Frame();
        m_d->lastSavedFullFrameId = -1;
    }

    m_d->savedFrames.remove(frameId);
}

bool KisFrameCacheStore::hasFrame(int frameId) const
{
    return m_d->savedFrames.contains(frameId);
}

int KisFrameCacheStore::frameLevelOfDetail(int frameId) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->savedFrames.contains(frameId), 0);
    return m_d->savedFrames[frameId]->levelOfDetail();
}

QRect KisFrameCacheStore::frameDirtyRect(int frameId) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->savedFrames.contains(frameId), QRect());
    return m_d->savedFrames[frameId]->dirtyImageRect();
}
