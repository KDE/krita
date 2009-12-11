/*
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_curve_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"


KisCurvePaintOp::KisCurvePaintOp(const KisCurvePaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
{
    Q_ASSERT(settings);
    m_curveBrush.setImage(image);
    m_curveBrush.setPainter(painter);

    m_curveBrush.setInkColor(painter->paintColor());
    m_curveBrush.setMode(settings->curveAction());
    m_curveBrush.setMinimalDistance(settings->minimalDistance());
    m_curveBrush.setInterval(settings->interval());
}

KisCurvePaintOp::~KisCurvePaintOp()
{
}

void KisCurvePaintOp::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
}


double KisCurvePaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist)
{
    Q_UNUSED(savedDist);
    if (!painter()) return 0;
    m_dev = painter()->device();
    if (!m_dev) return 0;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }
    //write device, read device, position
    m_curveBrush.paintLine(m_dab, m_dev, pi1, pi2);

    QRect rc = m_dab->extent();

    painter()->bitBlt(rc.topLeft(), m_dab, rc);

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return  dragVec.norm();

}
