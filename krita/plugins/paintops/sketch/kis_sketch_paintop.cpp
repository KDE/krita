/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Ricardo Cabello <hello@mrdoob.com>
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

#include "kis_sketch_paintop.h"
#include "kis_sketch_paintop_settings.h"

#include <cmath>
#include <QRect>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_paint_information.h>

#include <kis_pressure_opacity_option.h>

/*
* Based on Harmony project http://github.com/mrdoob/harmony/
*/
KisSketchPaintOp::KisSketchPaintOp(const KisSketchPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
        : KisPaintOp(painter)
{
    Q_UNUSED(image);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_sketchProperties.readOptionSetting(settings);

    m_opacityOption.sensor()->reset();
    m_sizeOption.sensor()->reset();

    m_painter = 0;
    m_count = 0;
}

KisSketchPaintOp::~KisSketchPaintOp()
{
    delete m_painter;
}

KisDistanceInformation KisSketchPaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist)
{
    Q_UNUSED(savedDist);
    if (!painter()) return KisDistanceInformation();

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
        m_painter = new KisPainter(m_dab);
    } else {
        m_dab->clear();
    }

    QPointF prevMouse = pi1.pos();
    QPointF mouse = pi2.pos();

    m_points.append(mouse);

    // chrome, fur 0.1 * BRUSH_PRESSURE
    // sketchy, long fur 0.05
    qreal opacity = 0.05;

    m_painter->setPaintColor( painter()->paintColor() );
    // shaded: does not draw this line, chrome does, fur does
    if (m_sketchProperties.makeConnection){
        m_painter->drawThickLine(prevMouse,mouse,m_sketchProperties.lineWidth,m_sketchProperties.lineWidth);
    }

    qreal distance;
    QPointF diff;
    int size = m_points.size();

    double scale = KisPaintOp::scaleForPressure(m_sizeOption.apply(pi2));
    qreal radius = m_sketchProperties.radius * scale;
    qreal thresholdDistance =  radius * radius;
    // shaded: probabity : paint always - 0.0 density
    qreal density = thresholdDistance * m_sketchProperties.probability;

    for (int i = 0; i < size; i++){
            diff = m_points.at(i) - m_points.at(m_count);
            // chrome : diff 0.2, sketchy : 0.3, fur: 0.5
            distance = diff.x() * diff.x() + diff.y() * diff.y();
            // fur : distance / thresholdDistance
            if ((distance < thresholdDistance) && drand48() > (distance / density))
            {
                QPointF offsetPt = diff * m_sketchProperties.offset;

                // shaded: opacity per line :/
                // ((1 - (d / 1000)) * 0.1 * BRUSH_PRESSURE), offset == 0
                // chrome: color per line :/
                //this.context.strokeStyle = "rgba(" + Math.floor(Math.random() * COLOR[0]) + ", " + Math.floor(Math.random() * COLOR[1]) + ", " + Math.floor(Math.random() * COLOR[2]) + ", " + 0.1 * BRUSH_PRESSURE + " )";

                // long fur
                // from: count + offset * -random
                // to: i point - (offset * -random)  + random * 2
                // probability distance / thresholdDistnace
                if (m_sketchProperties.magnetify){
                    m_painter->drawThickLine(m_points.at(m_count) + offsetPt,
                                         m_points.at(i) - offsetPt,
                                         m_sketchProperties.lineWidth,m_sketchProperties.lineWidth );
                }else{
                    // fur mode
                    m_painter->drawThickLine(m_points.at(m_count) + offsetPt,
                                         m_points.at(m_count) - offsetPt,
                                         m_sketchProperties.lineWidth,m_sketchProperties.lineWidth );
                }

            }
    }
    m_count++;

    QRect rc = m_dab->extent();
    quint8 origOpacity = m_opacityOption.apply(painter(), pi2);
    if (m_sketchProperties.useLowOpacity){
        painter()->setOpacity(painter()->opacity() * opacity);
    }

    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());
    painter()->setOpacity(origOpacity);

    KisVector2D end = toKisVector2D(pi2.pos());
    KisVector2D start = toKisVector2D(pi1.pos());
    KisVector2D dragVec = end - start;

    return KisDistanceInformation(0, dragVec.norm());
}



double KisSketchPaintOp::paintAt(const KisPaintInformation& info)
{
    return paintLine(info, info).spacing;
}
