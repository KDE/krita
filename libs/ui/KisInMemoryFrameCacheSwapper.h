/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISINMEMORYFRAMECACHESWAPPER_H
#define KISINMEMORYFRAMECACHESWAPPER_H

#include <QScopedPointer>

#include "KisAbstractFrameCacheSwapper.h"
#include "opengl/kis_texture_tile_info_pool.h"

class KisOpenGLUpdateInfoBuilder;


class KRITAUI_EXPORT KisInMemoryFrameCacheSwapper : public KisAbstractFrameCacheSwapper
{
public:
    KisInMemoryFrameCacheSwapper();
    ~KisInMemoryFrameCacheSwapper();

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

#endif // KISINMEMORYFRAMECACHESWAPPER_H
