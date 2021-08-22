/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_spacing_information.h>


KisFilterOp::KisFilterOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image)
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
    m_filterConfiguration = static_cast<const KisFilterOpSettings *>(settings.data())->filterConfig();
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
    qreal rotation = m_rotationOption.apply(info);
    KisDabShape shape(scale, 1.0, rotation);

    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    QRect dstRect;
    KisFixedPaintDeviceSP dab = m_dabCache->fetchDab(cs, color, info.pos(),
                                                     shape,
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
    p.bitBltOldData(neededRect.topLeft(), source(), neededRect);

    KisTransaction transaction(m_tmpDevice);
    m_filter->process(m_tmpDevice, dstRect, m_filterConfiguration, 0);
    transaction.end();

    painter()->bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                                        m_tmpDevice, dab,
                                        0, 0,
                                        dstRect.x(), dstRect.y(),
                                        dabRect.width(), dabRect.height());

    painter()->renderMirrorMaskSafe(dstRect, m_tmpDevice, 0, 0, dab,
                                    !m_dabCache->needSeparateOriginal());

    return effectiveSpacing(scale, rotation, info);
}

KisSpacingInformation KisFilterOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal scale = m_sizeOption.apply(info) * KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = m_rotationOption.apply(info);
    return effectiveSpacing(scale, rotation, info);
}

QList<KoResourceSP> KisFilterOp::prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    QList<KoResourceSP> resources = KisBrushBasedPaintOp::prepareLinkedResources(settings, resourcesInterface);

    KisFilterConfigurationSP config = static_cast<const KisFilterOpSettings *>(settings.data())->filterConfig();
    resources << config->linkedResources(resourcesInterface);

    return resources;
}

QList<KoResourceSP> KisFilterOp::prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface)
{
    QList<KoResourceSP> resources = KisBrushBasedPaintOp::prepareEmbeddedResources(settings, resourcesInterface);

    KisFilterConfigurationSP config = static_cast<const KisFilterOpSettings *>(settings.data())->filterConfig();
    resources << config->embeddedResources(resourcesInterface);

    return resources;
}
