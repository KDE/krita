/*
 *  Copyright (c) 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_curve_paintop.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"

KisCurvePaintOp::KisCurvePaintOp(const KisCurvePaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter), m_painter(0)
{
    Q_ASSERT(settings);
    Q_UNUSED(image);

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
    Q_UNUSED(info);
    return 1.0;
}


KisDistanceInformation KisCurvePaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist)
{
    Q_UNUSED(savedDist);
    if (!painter()) {
        return KisDistanceInformation();
    }

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
    } else {
        m_dab->clear();
    }

    paintLine(m_dab, pi1, pi2);

    QRect rc = m_dab->extent();

    quint8 origOpacity = m_opacityOption.apply(painter(), pi2);
    painter()->bitBlt(rc.topLeft(), m_dab, rc);
    painter()->renderMirrorMask(rc, m_dab);
    painter()->setOpacity(origOpacity);

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;

    return KisDistanceInformation(0, dragVec.norm());
}

void KisCurvePaintOp::paintLine(KisPaintDeviceSP dab, const KisPaintInformation &pi1, const KisPaintInformation &pi2) {
    if (!m_painter) {
        m_painter = new KisPainter(dab);
        m_painter->setPaintColor(painter()->paintColor());
    }

    int maxPoints = m_curveProperties.historySize;

    m_points.append(pi2.pos());

    while (m_points.length() > maxPoints) {
        m_points.removeFirst();
    }

    qreal lineWidth = m_lineWidthOption.apply(pi2, m_curveProperties.lineWidth);

    QPen pen(QBrush(Qt::white), lineWidth);
    QPainterPath path;

    if ( m_curveProperties.paintConnectionLine ) {
        path.moveTo(pi1.pos());
        path.lineTo(pi2.pos());
        m_painter->drawPainterPath(path, pen);
        path = QPainterPath();
    }

    if (m_points.length() >= maxPoints) {
        // alpha * 0.2;
        path.moveTo(m_points.first());

        if ( m_curveProperties.smoothing ) {
            path.quadTo(m_points.at(maxPoints / 2), m_points.last());
        } else {
            // control point is at 1/3 of the history, 2/3 of the history and endpoint at 3/3
            int step = maxPoints / 3;
            path.cubicTo(m_points.at(step), m_points.at(step+step), m_points.last());
        }

        qreal curveOpacity = m_curvesOpacityOption.apply(pi2, m_curveProperties.curvesOpacity);
        m_painter->setOpacity(qRound(255.0 * curveOpacity));
        m_painter->drawPainterPath(path, pen);
        m_painter->setOpacity(255); // full
    }
}
