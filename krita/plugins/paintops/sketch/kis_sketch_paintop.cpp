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
#include <kis_fixed_paint_device.h>

#include <kis_pressure_opacity_option.h>
#include <kis_dab_cache.h>

/*
* Based on Harmony project http://github.com/mrdoob/harmony/
*/
// chrome : diff 0.2, sketchy : 0.3, fur: 0.5
// fur : distance / thresholdDistance

// shaded: opacity per line :/
// ((1 - (d / 1000)) * 0.1 * BRUSH_PRESSURE), offset == 0
// chrome: color per line :/
//this.context.strokeStyle = "rgba(" + Math.floor(Math.random() * COLOR[0]) + ", " + Math.floor(Math.random() * COLOR[1]) + ", " + Math.floor(Math.random() * COLOR[2]) + ", " + 0.1 * BRUSH_PRESSURE + " )";

// long fur
// from: count + offset * -random
// to: i point - (offset * -random)  + random * 2
// probability distance / thresholdDistnace

// shaded: probabity : paint always - 0.0 density

KisSketchPaintOp::KisSketchPaintOp(const KisSketchPaintOpSettings *settings, KisPainter * painter, KisImageWSP image)
    : KisPaintOp(painter)
{
    Q_UNUSED(image);
    m_opacityOption.readOptionSetting(settings);
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_sketchProperties.readOptionSetting(settings);
    m_brushOption.readOptionSetting(settings);
    m_densityOption.readOptionSetting(settings);
    m_lineWidthOption.readOptionSetting(settings);
    m_offsetScaleOption.readOptionSetting(settings);

    m_brush = m_brushOption.brush();
    m_dabCache = new KisDabCache(m_brush);

    m_opacityOption.sensor()->reset();
    m_sizeOption.sensor()->reset();
    m_rotationOption.sensor()->reset();

    m_painter = 0;
    m_count = 0;
}

KisSketchPaintOp::~KisSketchPaintOp()
{
    delete m_painter;
    delete m_dabCache;
}

void KisSketchPaintOp::drawConnection(const QPointF& start, const QPointF& end, double lineWidth)
{
    if (lineWidth == 1.0){
        m_painter->drawThickLine(start, end, lineWidth,lineWidth);
    }else{
        m_painter->drawLine(start, end, lineWidth, true);
    }
}

void KisSketchPaintOp::updateBrushMask(const KisPaintInformation& info, qreal scale, qreal rotation){
    m_maskDab = m_dabCache->fetchDab(m_dab->colorSpace(), painter()->paintColor(), scale, scale,
                                     rotation, info, 0.0, 0.0, 0.0);

    // update bounding box
    m_brushBoundingBox = m_maskDab->bounds();
    m_hotSpot = m_brush->hotSpot(scale,scale,rotation,info);
    m_brushBoundingBox.translate(info.pos() - m_hotSpot);
}

KisDistanceInformation KisSketchPaintOp::paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisDistanceInformation& savedDist)
{
    Q_UNUSED(savedDist);

    if (!m_brush || !painter())
        return KisDistanceInformation();

    if (!m_dab) {
        m_dab = source()->createCompositionSourceDevice();
        m_painter = new KisPainter(m_dab);
    } else {
        m_dab->clear();
    }

    QPointF prevMouse = pi1.pos();
    QPointF mousePosition = pi2.pos();
    m_points.append(mousePosition);

    const double scale = m_sizeOption.apply(pi2);
    const double rotation = m_rotationOption.apply(pi2);
    const double currentProbability = m_densityOption.apply(pi2, m_sketchProperties.probability);
    const double currentLineWidth = m_lineWidthOption.apply(pi2, m_sketchProperties.lineWidth);
    const double currentOffsetScale = m_offsetScaleOption.apply(pi2, m_sketchProperties.offset);

    KisVector2D endVec = toKisVector2D(pi2.pos());
    KisVector2D startVec = toKisVector2D(pi1.pos());
    KisVector2D dragVec = endVec - startVec;
    if ((scale * m_brush->width()) <= 0.01 || (scale * m_brush->height()) <= 0.01) return KisDistanceInformation(0, dragVec.norm());;

    // shaded: does not draw this line, chrome does, fur does
    if (m_sketchProperties.makeConnection){
        drawConnection(prevMouse, mousePosition, currentLineWidth);
    }

    setCurrentScale(scale);
    setCurrentRotation(rotation);

    qreal thresholdDistance = 0.0;

    // update the mask for simple mode only once
    // determine the radius
    if (m_count == 0 && m_sketchProperties.simpleMode){
        updateBrushMask(pi2,1.0,0.0);
        //m_radius = qMax(m_maskDab->bounds().width(),m_maskDab->bounds().height()) * 0.5;
        m_radius = 0.5 * qMax(m_brush->width(), m_brush->height());
    }

    if (!m_sketchProperties.simpleMode){
        updateBrushMask(pi2,scale,rotation);
        m_radius = qMax(m_maskDab->bounds().width(),m_maskDab->bounds().height()) * 0.5;
        thresholdDistance = pow(m_radius,2);
    }

    if (m_sketchProperties.simpleMode){
        // update the radius according scale in simple mode
        thresholdDistance = pow(m_radius * scale,2);
    }

    // determine density
    const qreal density = thresholdDistance * currentProbability;

    // probability behaviour
    qreal probability = 1.0 - currentProbability;

    QColor painterColor = painter()->paintColor().toQColor();
    QColor randomColor;
    KoColor color(m_dab->colorSpace());

    int w = m_maskDab->bounds().width();
    quint8 opacityU8 = 0;
    quint8 * pixel;
    qreal distance;
    QPoint  positionInMask;
    QPointF diff;

    m_painter->setPaintColor( painter()->paintColor() );
    int size = m_points.size();
    // MAIN LOOP
    for (int i = 0; i < size; i++) {
        diff = m_points.at(i) - mousePosition;
        distance = diff.x() * diff.x() + diff.y() * diff.y();

        // circle test
        bool makeConnection = false;
        if (m_sketchProperties.simpleMode){
            if (distance < thresholdDistance){
                makeConnection = true;
            }
        // mask test
        }else{

            if ( m_brushBoundingBox.contains( m_points.at(i) ) ){
                positionInMask = (diff + m_hotSpot).toPoint();
                pixel = m_maskDab->data() + ((positionInMask.y() * w + positionInMask.x()) * m_maskDab->pixelSize());
                opacityU8 = m_maskDab->colorSpace()->opacityU8( pixel );
                if (opacityU8 != 0){
                    makeConnection = true;
                }
            }

        }

        if (!makeConnection){
            // check next point
            continue;
        }

        if (m_sketchProperties.distanceDensity){
            probability =  distance / density;
        }

        // density check
        if (drand48() >= probability) {
            QPointF offsetPt = diff * currentOffsetScale;

            if (m_sketchProperties.randomRGB){
                // some color transformation per line goes here
                randomColor.setRgbF(drand48() * painterColor.redF(),
                                    drand48() * painterColor.greenF(),
                                    drand48() * painterColor.blueF()
                                    );
                color.fromQColor(randomColor);
                m_painter->setPaintColor(color);
            }

            // distance based opacity
            quint8 opacity = OPACITY_OPAQUE_U8;
            if (m_sketchProperties.distanceOpacity){
                opacity *= qRound((1.0 - (distance / thresholdDistance)));
            }

            if (m_sketchProperties.randomOpacity){
                opacity *= drand48();
            }

            m_painter->setOpacity(opacity);

            if (m_sketchProperties.magnetify) {
                drawConnection(mousePosition + offsetPt, m_points.at(i) - offsetPt, currentLineWidth);
            }else{
                drawConnection(mousePosition + offsetPt, mousePosition - offsetPt, currentLineWidth);
            }



        }
    }// end of MAIN LOOP

    m_count++;

    QRect rc = m_dab->extent();
    quint8 origOpacity = m_opacityOption.apply(painter(), pi2);

    painter()->bitBlt(rc.x(), rc.y(), m_dab, rc.x(), rc.y(), rc.width(), rc.height());
    painter()->renderMirrorMask(rc, m_dab);
    painter()->setOpacity(origOpacity);

    return KisDistanceInformation(0, dragVec.norm());
}



qreal KisSketchPaintOp::paintAt(const KisPaintInformation& info)
{
    return paintLine(info, info).spacing;
}
