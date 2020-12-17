/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisInMemoryFrameCacheSwapper.h"

#include <QMap>
#include <kis_update_info.h>


struct KRITAUI_NO_EXPORT KisInMemoryFrameCacheSwapper::Private
{
    QMap<int, KisOpenGLUpdateInfoSP> framesMap;
};

KisInMemoryFrameCacheSwapper::KisInMemoryFrameCacheSwapper()
    : m_d(new Private)
{
}

KisInMemoryFrameCacheSwapper::~KisInMemoryFrameCacheSwapper()
{
}

void KisInMemoryFrameCacheSwapper::saveFrame(int frameId, KisOpenGLUpdateInfoSP info, const QRect &imageBounds)
{
    Q_UNUSED(imageBounds);
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->framesMap.contains(frameId));

    m_d->framesMap.insert(frameId, info);
}

KisOpenGLUpdateInfoSP KisInMemoryFrameCacheSwapper::loadFrame(int frameId)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->framesMap.contains(frameId));
    return m_d->framesMap.value(frameId, KisOpenGLUpdateInfoSP());
}

void KisInMemoryFrameCacheSwapper::moveFrame(int srcFrameId, int dstFrameId)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->framesMap.contains(srcFrameId));
    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->framesMap.contains(dstFrameId));

    m_d->framesMap[dstFrameId] = m_d->framesMap[srcFrameId];
    m_d->framesMap.remove(srcFrameId);
}

void KisInMemoryFrameCacheSwapper::forgetFrame(int frameId)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->framesMap.contains(frameId));
    m_d->framesMap.remove(frameId);
}

bool KisInMemoryFrameCacheSwapper::hasFrame(int frameId) const
{
    return m_d->framesMap.contains(frameId);
}

int KisInMemoryFrameCacheSwapper::frameLevelOfDetail(int frameId) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->framesMap.contains(frameId), 0);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->framesMap[frameId].isNull(), 0);
    return m_d->framesMap[frameId]->levelOfDetail();
}

QRect KisInMemoryFrameCacheSwapper::frameDirtyRect(int frameId) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->framesMap.contains(frameId), QRect());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->framesMap[frameId].isNull(), QRect());
    return m_d->framesMap[frameId]->dirtyImageRect();
}
