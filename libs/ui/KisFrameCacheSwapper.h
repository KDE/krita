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
#ifndef KISFRAMECACHESWAPPER_H
#define KISFRAMECACHESWAPPER_H

#include <QScopedPointer>

#include "KisAbstractFrameCacheSwapper.h"

class KisOpenGLUpdateInfoBuilder;


/**
 * KisFrameCacheSwapper is the most highlevel facade of the frame
 * swapping infrastructure. The main responsibilities of the class:
 *
 * 1) Asynchronously predict and prefetch the pending frames from disk
 *    and maintain a short in-memory cache of these frames (already
 *    converted into KisOpenGLUpdateInfo)
 *
 * 2) Pass all the other requests to the lower-level API,
 *    like KisFrameCacheStore
 */

class KRITAUI_EXPORT KisFrameCacheSwapper : public KisAbstractFrameCacheSwapper
{
public:
    KisFrameCacheSwapper(const KisOpenGLUpdateInfoBuilder &builder);
    KisFrameCacheSwapper(const KisOpenGLUpdateInfoBuilder &builder, const QString &frameCachePath);
    ~KisFrameCacheSwapper();

    // WARNING: after transferring \p info to saveFrame() the object becomes invalid
    void saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds) override;
    KisOpenGLUpdateInfoSP loadFrame(int frameId) override;

    void moveFrame(int srcFrameId, int dstFrameId) override;

    void forgetFrame(int frameId) override;
    bool hasFrame(int frameId) const override;

    int frameLevelOfDetail(int frameId) const override;

    QRect frameDirtyRect(int frameId) const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISFRAMECACHESWAPPER_H
