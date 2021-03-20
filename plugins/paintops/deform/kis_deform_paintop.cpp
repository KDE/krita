/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_deform_paintop.h"
#include "kis_deform_paintop_settings.h"

#include <cmath>

#include <QtGlobal>
#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_random_accessor_ng.h"
#include "kis_lod_transform.h"

#include <kis_fixed_paint_device.h>

#include "kis_deform_option.h"
#include "kis_brush_size_option.h"
#include "kis_paintop_plugin_utils.h"
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#ifdef Q_OS_WIN
// quoting DRAND48(3) man-page:
// These functions are declared obsolete by  SVID  3,
// which  states  that rand(3) should be used instead.
#define drand48() (static_cast<double>(qrand()) / static_cast<double>(RAND_MAX))
#endif

KisDeformPaintOp::KisDeformPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
{
    Q_UNUSED(image);
    Q_UNUSED(node);
    Q_ASSERT(settings);

    m_sizeProperties.readOptionSetting(settings);
    m_properties.readOptionSetting(settings);
    m_airbrushOption.readOptionSetting(settings);

    // sensors
    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_rateOption.readOptionSetting(settings);
    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_rotationOption.resetAllSensors();
    m_rateOption.resetAllSensors();

    m_deformBrush.setProperties(&m_properties);
    m_deformBrush.setSizeProperties(&m_sizeProperties);

    m_deformBrush.initDeformAction();

    m_dev = source();

    if ((m_sizeProperties.brush_diameter * 0.5) > 1) {
        m_ySpacing = m_xSpacing = m_sizeProperties.brush_diameter * 0.5 * m_sizeProperties.brush_spacing;
    }
    else {
        m_ySpacing = m_xSpacing = 1.0;
    }
    m_spacing = m_xSpacing;



}

KisDeformPaintOp::~KisDeformPaintOp()
{
}

KisSpacingInformation KisDeformPaintOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) return KisSpacingInformation(m_spacing);
    if (!m_dev) return KisSpacingInformation(m_spacing);

    KisFixedPaintDeviceSP dab = cachedDab(source()->compositionSourceColorSpace());

    qint32 x;
    qreal subPixelX;
    qint32 y;
    qreal subPixelY;

    QPointF pt = info.pos();
    if (m_sizeProperties.brush_jitter_movement_enabled) {
        pt.setX(pt.x() + ((m_sizeProperties.brush_diameter * info.randomSource()->generateNormalized()) - m_sizeProperties.brush_diameter * 0.5) * m_sizeProperties.brush_jitter_movement);
        pt.setY(pt.y() + ((m_sizeProperties.brush_diameter * info.randomSource()->generateNormalized()) - m_sizeProperties.brush_diameter * 0.5) * m_sizeProperties.brush_jitter_movement);
    }

    qreal rotation = m_rotationOption.apply(info);

    // Deform Brush is capable of working with zero scale,
    // so no additional checks for 'zero'ness are needed
    qreal scale = m_sizeOption.apply(info);


    rotation += m_sizeProperties.brush_rotation;
    scale *= m_sizeProperties.brush_scale;

    QPointF pos = pt - m_deformBrush.hotSpot(scale, rotation);

    splitCoordinate(pos.x(), &x, &subPixelX);
    splitCoordinate(pos.y(), &y, &subPixelY);

    KisFixedPaintDeviceSP mask = m_deformBrush.paintMask(dab, m_dev, info.randomSource(),
                                 scale, rotation,
                                 info.pos(),
                                 subPixelX, subPixelY,
                                 x, y
                                                        );

    // this happens for the first dab of the move mode, we need more information for being able to move
    if (!mask) {
        return updateSpacingImpl(info);
    }

    quint8 origOpacity = m_opacityOption.apply(painter(), info);
    painter()->bltFixedWithFixedSelection(x, y, dab, mask, mask->bounds().width() , mask->bounds().height());
    painter()->renderMirrorMask(QRect(QPoint(x, y), QSize(mask->bounds().width() , mask->bounds().height())), dab, mask);
    painter()->setOpacity(origOpacity);

    return updateSpacingImpl(info);
}

KisSpacingInformation KisDeformPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveSpacing(1.0, 1.0, true, 0.0, false, m_spacing, false,
                                                   1.0,
                                                   KisLodTransform::lodToScale(painter()->device()),
                                                   &m_airbrushOption, nullptr, info);
}

KisTimingInformation KisDeformPaintOp::updateTimingImpl(const KisPaintInformation &info) const
{
    return KisPaintOpPluginUtils::effectiveTiming(&m_airbrushOption, &m_rateOption, info);
}
