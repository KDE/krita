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
#ifndef KISFRAMECACHESTORE_H
#define KISFRAMECACHESTORE_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include "kis_types.h"

#include "opengl/kis_texture_tile_info_pool.h"

class KisOpenGLUpdateInfoBuilder;

class KisOpenGLUpdateInfo;
typedef KisSharedPtr<KisOpenGLUpdateInfo> KisOpenGLUpdateInfoSP;

/**
 * KisFrameCacheStore is a middle-level class for reading/writing
 * animation frames on disk. Its main responsibilities:
 *
 * 1) Convert frames from KisOpenGLUpdateInfo format into a serializable
 *    KisFrameDataSerializer::Frame format.
 *
 * 2) Calculate differences between the frames and decide which
 *    frame will be a keyframe for other frames.
 *
 * 3) The keyframes will be used as a base for difference
 *    calculation and stored in a short in-memory cache to avoid
 *    fetching them from disk too often.
 *
 * 4) The in-memory cache of the keyframes is stored in serializable
 *    KisFrameDataSerializer::Frame format.
 */

class KRITAUI_EXPORT KisFrameCacheStore
{
public:
    KisFrameCacheStore();
    KisFrameCacheStore(const QString &frameCachePath);

    ~KisFrameCacheStore();

    // WARNING: after transferring \p info to saveFrame() the object becomes invalid
    void saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds);
    KisOpenGLUpdateInfoSP loadFrame(int frameId, const KisOpenGLUpdateInfoBuilder &builder);

    void moveFrame(int srcFrameId, int dstFrameId);

    void forgetFrame(int frameId);
    bool hasFrame(int frameId) const;

    int frameLevelOfDetail(int frameId) const;
    QRect frameDirtyRect(int frameId) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISFRAMECACHESTORE_H
