/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filterop.h"

#include <kis_debug.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include <kis_processing_information.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_pressure_size_option.h>
#include <kis_filter_option.h>
#include <kis_filterop_settings.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>
#include <kis_transaction.h>
#include <kis_lod_transform.h>


KisFilterOp::KisFilterOp(const KisFilterOpSettings *settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_filterConfiguration(0)
{
    Q_UNUSED(node);
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_tmpDevice = source()->createCompositionSourceDevice();
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_sizeOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_filter = KisFilterRegistry::instance()->get(settings->getString(FILTER_ID));
    m_filterConfiguration = settings->filterConfig();
    m_smudgeMode = settings->getBool(FILTER_SMUDGE_MODE);

    m_rotationOption.applyFanCornersInfo(this);
}

KisFilterOp::~KisFilterOp()
{
}

KisSpacingInformation KisFilterOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) {
        return KisSpacingInformation(1.0);
    }

    if (!m_filter) {
        return KisSpacingInformation(1.0);
    }

    if (!source()) {
        return KisSpacingInformation(1.0);
    }

    KisBrushSP brush = m_brush;
    if (!brush) return KisSpacingInformation(1.0);

    if (! brush->canPaintFor(info))
        return KisSpacingInformation(1.0);

    qreal scale = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());

    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    setCurrentScale(scale);

    qreal rotation = m_rotationOption.apply(info);

    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP dab =
        m_dabCache->fetchDab(cs, color, info.pos(),
                             scale, scale, rotation,
                             info, 1.0,
                             &dstRect);

    if (dstRect.isEmpty()) return KisSpacingInformation(1.0);

    QRect dabRect = dab->bounds();

    // sanity check
    Q_ASSERT(dstRect.size() == dabRect.size());


    // Filter the paint device
    QRect neededRect = m_filter->neededRect(dstRect, m_filterConfiguration, painter()->device()->defaultBounds()->currentLevelOfDetail());

    KisPainter p(m_tmpDevice);
    if (!m_smudgeMode) {
        p.setCompositeOp(COMPOSITE_COPY);
    }
    p.bitBltOldData(neededRect.topLeft() - dstRect.topLeft(), source(), neededRect);

    KisTransaction transaction(m_tmpDevice);
    m_filter->process(m_tmpDevice, dabRect, m_filterConfiguration, 0);
    transaction.end();


    painter()->
    bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                             m_tmpDevice, dab,
                             0, 0,
                             dabRect.x(), dabRect.y(),
                             dabRect.width(), dabRect.height());

    painter()->renderMirrorMaskSafe(dstRect, m_tmpDevice, 0, 0, dab,
                                    !m_dabCache->needSeparateOriginal());

    return effectiveSpacing(scale, rotation);
}
