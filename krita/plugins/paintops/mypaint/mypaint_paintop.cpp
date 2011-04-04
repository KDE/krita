/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "mypaint_paintop.h"
#include "mypaint_paintop_settings.h"
#include <kis_debug.h>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_paintop_registry.h>

#include "mypaint_paintop_factory.h"
#include "mypaint_brush_resource.h"
#include "mypaint_paintop_settings.h"
#include "mypaint_surface.h"
#include <kis_paint_information.h>

MyPaint::MyPaint(const MyPaintSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp(painter)
    , m_settings(settings)
    , m_firstPoint(true)
{
    Q_ASSERT(settings);
    Q_UNUSED(image);

    m_surface = new MyPaintSurface(settings->node()->projection(), painter);
    MyPaintFactory *factory = static_cast<MyPaintFactory*>(KisPaintOpRegistry::instance()->get("mypaintbrush"));
    m_brush = factory->brush(settings->getString("filename"));
    m_brush->set_base_value(BRUSH_RADIUS_LOGARITHMIC, settings->getFloat("radius_logarithmic"));
    QColor c = painter->paintColor().toQColor();
    qreal h, s, v, a;
    c.getHsvF(&h, &s, &v, &a);
    m_brush->set_color_hsv((float)h, (float)s, (float)v);
}

MyPaint::~MyPaint()
{
    delete m_surface;
}

qreal MyPaint::paintAt(const KisPaintInformation& info)
{
    Q_UNUSED(info);
    return 1.0;
}

KisDistanceInformation MyPaint::paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, const KisDistanceInformation& savedDist)
{
    Q_UNUSED(savedDist);

    if (!painter()) return KisDistanceInformation();

    if(m_firstPoint){
        m_brush->stroke_to(m_surface,
                           pi1.pos().x(), pi1.pos().y(),
                           0,
                           0.0, 0.0,
                           10.0);
        m_firstPoint = false;
    }
    m_brush->stroke_to(m_surface,
                       pi2.pos().x(), pi2.pos().y(),
                       pi2.pressure(),
                       pi2.xTilt() / 60.0, pi2.yTilt() / 60.0,
                       qreal(pi2.currentTime() - pi1.currentTime()) / 1000);

    // not sure what to do with these...
    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;
    return KisDistanceInformation(0, dragVec.norm());
}
