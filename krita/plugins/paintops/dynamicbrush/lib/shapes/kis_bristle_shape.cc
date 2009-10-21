/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_bristle_shape.h"
#include <KoColorSpaceRegistry.h>

#include <kis_mask_generator.h>
#include <kis_auto_brush.h>
#include <kis_dynamic_coloring.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_shared.h>
#include <kis_image.h>


struct KisBristle {
    KisBristle(double x, double y) : m_x(x), m_y(y), m_lastX(0.0), m_lastY(0.0) {
    }
    // Position in the paintbrush
    double m_x, m_y;
    // Last drawn position
    double m_lastX, m_lastY;
};

struct KisPaintBrush : public KisShared {
    KisPaintBrush(double paintbrushMinRadius, double paintbrushMaxRadius, double bristlesDensity, double bristlesMinRadius, double bristlesMaxRadius) :
            m_paintbrushMinRadius(paintbrushMinRadius), m_paintbrushMaxRadius(paintbrushMaxRadius), m_bristlesDensity(bristlesDensity), m_bristlesMinRadius(bristlesMinRadius), m_bristlesMaxRadius(bristlesMaxRadius), m_firstStroke(true) {
        m_bristles.push_back(KisBristle(-0.5, -0.5));
        m_bristles.push_back(KisBristle(-0.5, 0.5));
        m_bristles.push_back(KisBristle(0.5, -0.5));
        m_bristles.push_back(KisBristle(0.5, 0.5));
        m_bristles.push_back(KisBristle(0.0, 0.7));
        m_bristles.push_back(KisBristle(0.0, -0.7));
        m_bristles.push_back(KisBristle(0.7, 0.0));
        m_bristles.push_back(KisBristle(-0.7, 0.0));
        // More bristles
        m_bristles.push_back(KisBristle(-0.25, -0.25));
        m_bristles.push_back(KisBristle(-0.25, 0.25));
        m_bristles.push_back(KisBristle(0.25, -0.25));
        m_bristles.push_back(KisBristle(0.25, 0.25));
        m_bristles.push_back(KisBristle(0.0, 0.35));
        m_bristles.push_back(KisBristle(0.0, -0.35));
        m_bristles.push_back(KisBristle(0.35, 0.0));
        m_bristles.push_back(KisBristle(-0.35, 0.0));
        bristlesPainter = 0;
    }
    double m_paintbrushMinRadius, m_paintbrushMaxRadius, m_bristlesDensity, m_bristlesMinRadius, m_bristlesMaxRadius;
    QList< KisBristle > m_bristles;
    bool m_firstStroke;
    KisPainter* bristlesPainter;
};

KisBristleShape::KisBristleShape(double paintbrushMinRadius, double paintbrushMaxRadius, double bristlesDensity, double bristlesMinRadius, double bristlesMaxRadius) :
        m_paintBrush(new KisPaintBrush(paintbrushMinRadius, paintbrushMaxRadius, bristlesDensity, bristlesMinRadius, bristlesMaxRadius)),
        m_radius(0.5 *(paintbrushMinRadius + paintbrushMaxRadius)),
        m_angle(0.0)
{
}

QRect KisBristleShape::rect() const
{
    return m_rect;
}

KisDynamicShape* KisBristleShape::clone() const
{
    return new KisBristleShape(*this);
}

void KisBristleShape::resize(double xs, double ys)
{
    dbgPlugins << xs << "" << ys;
    m_radius *= (xs + ys) * 0.5;
}

void KisBristleShape::rotate(double r)
{
    m_angle = r;
}

void KisBristleShape::startPainting(KisPainter* _painter)
{
    dbgPlugins << "startPainting";
    KisDynamicShape::startPainting(_painter);
    m_paintBrush->bristlesPainter = new KisPainter(painter()->device());
    dbgPlugins << (int)((255.0*rand()) / RAND_MAX) << " " << (int)((255.0*rand()) / RAND_MAX) << " " << (int)((255.0*rand()) / RAND_MAX);
    m_paintBrush->bristlesPainter->setPaintColor(KoColor(QColor((int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX), (int)((255.0*rand()) / RAND_MAX)), KoColorSpaceRegistry::instance()->rgb8()));

}

void KisBristleShape::endPainting()
{
    dbgPlugins << "endPainting";
    KisDynamicShape::endPainting();
    m_paintBrush->bristlesPainter = 0;
    delete m_paintBrush->bristlesPainter;
}

void KisBristleShape::paintAt(const QPointF &brushPos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc)
{
    Q_UNUSED(info);
    Q_UNUSED(coloringsrc);

    double angleCos = cos(m_angle);
    double angleSin = sin(m_angle);
    KoColor color(m_paintBrush->bristlesPainter->device()->colorSpace());
    dbgPlugins << "paintAt painter : " << m_paintBrush->bristlesPainter;
    for (QList< KisBristle >::iterator it = m_paintBrush->m_bristles.begin();
            it != m_paintBrush->m_bristles.end(); ++it) {
        double x = it->m_x * m_radius;
        double y = it->m_y * m_radius;
        QPointF pos(angleCos*x - angleSin*y , angleSin*x + angleCos*y);
//         dbgPlugins << pos;
        pos += brushPos;
//         dbgPlugins << m_radius <<"" << pos <<"" << brushPos;
        coloringsrc->colorAt((int)pos.x(), (int)pos.y(), &color);
        Q_ASSERT(m_paintBrush->bristlesPainter);
        m_paintBrush->bristlesPainter->setPaintColor(color);
        if (m_paintBrush->m_firstStroke) {
            m_paintBrush->bristlesPainter->drawLine(pos, pos);
        } else {
            m_paintBrush->bristlesPainter->drawLine(QPointF(it->m_lastX, it->m_lastY), pos);
        }
        it->m_lastX = pos.x();
        it->m_lastY = pos.y();
    }
    QRegion region = m_paintBrush->bristlesPainter->dirtyRegion();
    dbgPlugins << region;
    painter()->device()->setDirty(region);
    m_paintBrush->m_firstStroke = false;
}

#if 0
void KisBristleShape::createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc, const QPointF &brushPos, const KisPaintInformation& info)
{

    double angleCos = cos(m_angle);
    double angleSin = sin(m_angle);
    dbgPlugins << angleCos << "" << angleSin;
    // Clear the stamp
    stamp->clear();
    // Create a painter
    KisPainter p(stamp);
    KisAutobrushCircleShape cs(1, 1, 1.0, 1.0);
    QImage img = cs.createBrush();
    p.setBrush(new KisAutoBrush(img));
    dbgPlugins << KisPaintOpRegistry::instance();
    dbgPlugins << KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &p, 0);
    p.setPaintOp(KisPaintOpRegistry::instance()->paintOp("paintbrush", 0, &p, 0));
    m_rect = QRect(0, 0, 0, 0);
    for (QList< KisBristle >::iterator it = m_paintBrush->m_bristles.begin();
            it != m_paintBrush->m_bristles.end(); ++it) {
        p.setPaintColor(it->color);
        double x = it->m_x * m_radius;
        double y = it->m_y * m_radius;
        QPointF pos(angleCos*x - angleSin*y , angleSin*x + angleCos*y);
//         dbgPlugins << pos <<"" << pos2;
        if (m_paintBrush->m_firstStroke) {
            p.paintLine(pos, 0.5, 1.0, 1.0, pos, 0.5, 1.0, 1.0);
        } else {
            p.paintLine(QPointF(it->m_lastX - brushPos.x(), it->m_lastY - brushPos.y()), 0.5, 1.0, 1.0, pos, .5, 1.0, 1.0);
        }
        it->m_lastX = pos.x() + brushPos.x();
        it->m_lastY = pos.y() + brushPos.y();
    }
    m_paintBrush->m_firstStroke = false;
    m_rect = p.dirtyRegion() /*.boundingRect()*/;
//     m_rect = QRect( m_radius, m_radius, 2.0 * m_radius, 2.0 * m_radius );
    dbgPlugins << "Bristle shape rect:" << m_rect;
}
#endif
