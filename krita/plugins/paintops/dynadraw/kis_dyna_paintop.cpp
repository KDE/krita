/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_dyna_paintop.h"
#include "kis_dyna_paintop_settings.h"

#include <cmath>

#include <QRect>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_vec.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_random_accessor.h>

#include "kis_dynaop_option.h"

#include "filter.h"

KisDynaPaintOp::KisDynaPaintOp(const KisDynaPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
        , m_image(image)
{
    m_dynaBrush.setImage(image);

    m_properties.initWidth = settings->getDouble(DYNA_WIDTH);
    m_properties.action = settings->getDouble(DYNA_ACTION);
    m_properties.mass = settings->getDouble(DYNA_MASS);
    m_properties.drag = settings->getDouble(DYNA_DRAG);
    m_properties.xAngle = settings->getDouble(DYNA_X_ANGLE);
    m_properties.yAngle = settings->getDouble(DYNA_Y_ANGLE);
    m_properties.widthRange = settings->getDouble(DYNA_WIDTH_RANGE);
    m_properties.circleRadius = settings->getInt(DYNA_CIRCLE_RADIUS);
    m_properties.lineCount = settings->getInt(DYNA_LINE_COUNT);
    m_properties.lineSpacing = settings->getDouble(DYNA_LINE_SPACING);
    m_properties.enableLine = settings->getBool(DYNA_ENABLE_LINE);
    m_properties.useTwoCircles = settings->getBool(DYNA_USE_TWO_CIRCLES);
    m_properties.useFixedAngle = settings->getBool(DYNA_USE_FIXED_ANGLE);
    
    settings->dump();
    
    m_dynaBrush.setProperties( &m_properties );
}

KisDynaPaintOp::~KisDynaPaintOp()
{
}

double KisDynaPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist)
{
    Q_UNUSED(savedDist);
    if (!m_image) return 0;
    if (!painter()) return 0;

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    } else {
        m_dab->clear();
    }

    qreal x1, y1;

    x1 = pi1.pos().x();
    y1 = pi1.pos().y();

    m_dynaBrush.updateCursorPosition(pi1.pos());
    m_dynaBrush.paint(m_dab, x1, y1, painter()->paintColor());

    QRect rc = m_dab->extent();

    painter()->bitBlt(rc.topLeft(), m_dab, rc);

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return  dragVec.norm();
}

void KisDynaPaintOp::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
}
