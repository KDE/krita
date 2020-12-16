/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_update_info.h"

/**
 * The connection in KisCanvas2 uses queued signals
 * with an argument of KisNodeSP type, so we should
 * register it beforehand
 */
struct KisUpdateInfoSPStaticRegistrar {
    KisUpdateInfoSPStaticRegistrar() {
        qRegisterMetaType<KisUpdateInfoSP>("KisUpdateInfoSP");
    }
};
static KisUpdateInfoSPStaticRegistrar __registrar;

KisUpdateInfo::KisUpdateInfo()
{
}

KisUpdateInfo::~KisUpdateInfo()
{
}

QRect KisUpdateInfo::dirtyViewportRect()
{
    return QRect();
}

bool KisUpdateInfo::canBeCompressed() const
{
    return true;
}

QRect KisPPUpdateInfo::dirtyViewportRect() {
    return viewportRect.toAlignedRect();
}

QRect KisPPUpdateInfo::dirtyImageRect() const {
    return dirtyImageRectVar;
}

int KisPPUpdateInfo::levelOfDetail() const
{
    return 0;
}

KisOpenGLUpdateInfo::KisOpenGLUpdateInfo()
    : m_levelOfDetail(0)
{
}

QRect KisOpenGLUpdateInfo::dirtyViewportRect() {
    qFatal("Not implemented yet!");
    return QRect();
}

void KisOpenGLUpdateInfo::assignDirtyImageRect(const QRect &rect)
{
    m_dirtyImageRect = rect;
}

void KisOpenGLUpdateInfo::assignLevelOfDetail(int lod)
{
    m_levelOfDetail = lod;
}

QRect KisOpenGLUpdateInfo::dirtyImageRect() const
{
    return m_dirtyImageRect;
}

int KisOpenGLUpdateInfo::levelOfDetail() const
{
    return m_levelOfDetail;
}

bool KisOpenGLUpdateInfo::tryMergeWith(const KisOpenGLUpdateInfo &rhs)
{
    if (m_levelOfDetail != rhs.m_levelOfDetail) return false;

    // TODO: that makes the algorithm of updates compressor incorrect!
    m_dirtyImageRect |= rhs.m_dirtyImageRect;

    tileList.append(rhs.tileList);

    return true;
}

KisMarkerUpdateInfo::KisMarkerUpdateInfo(KisMarkerUpdateInfo::Type type, const QRect &dirtyImageRect)
    : m_type(type),
      m_dirtyImageRect(dirtyImageRect)
{
}

KisMarkerUpdateInfo::Type KisMarkerUpdateInfo::type() const
{
    return m_type;
}

QRect KisMarkerUpdateInfo::dirtyImageRect() const
{
    return m_dirtyImageRect;
}

int KisMarkerUpdateInfo::levelOfDetail() const
{
    // return invalid level of detail to avoid merging the update info
    // with other updates
    return -1 - (int)m_type;
}

bool KisMarkerUpdateInfo::canBeCompressed() const
{
    return false;
}
