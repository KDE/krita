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

KisOpenGLUpdateInfo::KisOpenGLUpdateInfo(ConversionOptions options)
    : m_options(options),
      m_levelOfDetail(0)
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

bool KisOpenGLUpdateInfo::needsConversion() const
{
    return m_options.m_needsConversion;
}
void KisOpenGLUpdateInfo::convertColorSpace()
{
    KIS_ASSERT_RECOVER_RETURN(needsConversion());

    Q_FOREACH (KisTextureTileUpdateInfoSP tileInfo, tileList) {
        tileInfo->convertTo(m_options.m_destinationColorSpace,
                            m_options.m_renderingIntent,
                            m_options.m_conversionFlags);
    }
}

int KisOpenGLUpdateInfo::levelOfDetail() const
{
    return m_levelOfDetail;
}
