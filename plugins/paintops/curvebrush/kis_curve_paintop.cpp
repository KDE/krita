/*
 *  SPDX-FileCopyrightText: 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_curve_paintop.h"

#include <cmath>

#include <QPainterPath>
#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_spacing_information.h"
#include <kis_lod_transform.h>


KisCurvePaintOp::KisCurvePaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp(painter)
    , m_painter(0)
{
    Q_ASSERT(settings);
    Q_UNUSED(image);
    Q_UNUSED(node);

    m_curveProperties.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_lineWidthOption.readOptionSetting(settings);
    m_curvesOpacityOption.readOptionSetting(settings);
}

KisCurvePaintOp::~KisCurvePaintOp()
{
    delete m_painter;
}

KisSpacingInformation KisCurvePaintOp::paintAt(const KisPaintInformation& info)
{
    return updateSpacingImpl(info);
}

KisSpacingInformation KisCurvePaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    Q_UNUSED(info);
    return KisSpacingInformation(1.0);
}

void KisCurvePaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance)
{
    Q_UNUSED(currentDistance);
    if (!painter()) return;

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
    }
    else {
        m_dab->clear();
    }

    paintLine(m_dab, pi1, pi2);

    QRect rc = m_dab->extent();

    quint8 origOpacity = m_opacityOption.apply(painter(), pi2);
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);
    painter()->setOpacity(origOpacity);
}

void KisCurvePaintOp::paintLine(KisPaintDeviceSP dab, const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    if (!m_painter) {
        m_painter = new KisPainter(dab);
        m_painter->setPaintColor(painter()->paintColor());
    }

    int maxPoints = m_curveProperties.curve_stroke_history_size;

    m_points.append(pi2.pos());

    while (m_points.length() > maxPoints) {
        m_points.removeFirst();
    }

    const qreal additionalScale = KisLodTransform::lodToScale(painter()->device());
    const qreal lineWidth = additionalScale * m_lineWidthOption.apply(pi2, m_curveProperties.curve_line_width);

    QPen pen(QBrush(Qt::white), lineWidth);
    QPainterPath path;

    if (m_curveProperties.curve_paint_connection_line) {
        path.moveTo(pi1.pos());
        path.lineTo(pi2.pos());
        m_painter->drawPainterPath(path, pen);
        path = QPainterPath();
    }

    if (m_points.length() >= maxPoints) {
        // alpha * 0.2;
        path.moveTo(m_points.first());

        if (m_curveProperties.curve_smoothing) {
            path.quadTo(m_points.at(maxPoints / 2), m_points.last());
        }
        else {
            // control point is at 1/3 of the history, 2/3 of the history and endpoint at 3/3
            int step = maxPoints / 3;
            path.cubicTo(m_points.at(step), m_points.at(step + step), m_points.last());
        }

        qreal curveOpacity = m_curvesOpacityOption.apply(pi2, m_curveProperties.curve_curves_opacity);
        m_painter->setOpacity(qRound(255.0 * curveOpacity));
        m_painter->drawPainterPath(path, pen);
        m_painter->setOpacity(255); // full
    }
}
