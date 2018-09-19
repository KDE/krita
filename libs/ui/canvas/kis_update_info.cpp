/*
 *  Copyright (c) 2010, Dmitry Kazakov <dimula73@gmail.com>
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

KisBatchControlUpdateInfo::KisBatchControlUpdateInfo(KisBatchControlUpdateInfo::Type type, const QRect &dirtyImageRect)
    : m_type(type),
      m_dirtyImageRect(dirtyImageRect)
{
}

KisBatchControlUpdateInfo::Type KisBatchControlUpdateInfo::type() const
{
    return m_type;
}

QRect KisBatchControlUpdateInfo::dirtyImageRect() const
{
    return m_dirtyImageRect;
}

int KisBatchControlUpdateInfo::levelOfDetail() const
{
    // return invalid level of detail to avoid merging the update info
    // with other updates
    return m_type == StartBatch ? -1 : -2;
}
