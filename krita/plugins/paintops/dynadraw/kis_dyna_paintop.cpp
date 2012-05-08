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
#include <kis_random_accessor_ng.h>

#include "kis_dynaop_option.h"

#include "filter.h"

KisDynaPaintOp::KisDynaPaintOp(const KisDynaPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
        , m_settings(settings)
{
    if (image){
        m_dynaBrush.setCanvasSize(image->width(), image->height());
    }else{
        // some dummy values for scratchpad
        m_dynaBrush.setCanvasSize(1000, 1000);
    }

    m_properties.initWidth = settings->getDouble(DYNA_WIDTH);
    m_properties.action = settings->getDouble(DYNA_ACTION);
    m_properties.mass = settings->getDouble(DYNA_MASS);
    m_properties.drag = settings->getDouble(DYNA_DRAG);

    double angle = settings->getDouble(DYNA_ANGLE);
    m_properties.xAngle = cos(angle * M_PI/180.0);
    m_properties.yAngle = sin(angle * M_PI/180.0);

    m_properties.widthRange = settings->getDouble(DYNA_WIDTH_RANGE);
    m_properties.diameter = settings->getInt(DYNA_DIAMETER);
    m_properties.lineCount = settings->getInt(DYNA_LINE_COUNT);
    m_properties.lineSpacing = settings->getDouble(DYNA_LINE_SPACING);
    m_properties.enableLine = settings->getBool(DYNA_ENABLE_LINE);
    m_properties.useTwoCircles = settings->getBool(DYNA_USE_TWO_CIRCLES);
    m_properties.useFixedAngle = settings->getBool(DYNA_USE_FIXED_ANGLE);

    m_dynaBrush.setProperties( &m_properties );
}

KisDynaPaintOp::~KisDynaPaintOp()
{
}

KisDistanceInformation KisDynaPaintOp::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, const KisDistanceInformation& savedDist)
{
    Q_UNUSED(savedDist);
    if (!painter()) return KisDistanceInformation();

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
    painter()->renderMirrorMask(rc,m_dab);

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return KisDistanceInformation(0, dragVec.norm());
}

qreal KisDynaPaintOp::paintAt(const KisPaintInformation& info)
{
    KisDistanceInformation di(0.0,1.0);
    return paintLine(info, info, di).spacing;
}
