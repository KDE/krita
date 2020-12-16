/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
