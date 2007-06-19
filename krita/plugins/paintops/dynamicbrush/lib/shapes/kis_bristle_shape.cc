/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <kis_autobrush_resource.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_shared.h>


struct KisBristle {
    KisBristle(double x, double y) : m_x(x), m_y(y), m_lastX(0.0), m_lastY(0.0)
    {
        color = KoColor( QColor( (255.0*rand()) / RAND_MAX, (255.0*rand()) / RAND_MAX, (255.0*rand()) / RAND_MAX ), KoColorSpaceRegistry::instance()->rgb8());
    }
    // Position in the paintbrush
    double m_x, m_y;
    // Last drawn position
    double m_lastX, m_lastY;
    KoColor color;
};

struct KisPaintBrush : public KisShared {
    KisPaintBrush(double paintbrushMinRadius, double paintbrushMaxRadius, double bristlesDensity, double bristlesMinRadius, double bristlesMaxRadius) : 
            m_paintbrushMinRadius(paintbrushMinRadius), m_paintbrushMaxRadius(paintbrushMaxRadius), m_bristlesDensity(bristlesDensity), m_bristlesMinRadius(bristlesMinRadius), m_bristlesMaxRadius(bristlesMaxRadius), m_firstStroke(true)
    {
        m_bristles.push_back( KisBristle(-0.5,-0.5 ) );
        m_bristles.push_back( KisBristle(-0.5, 0.5 ) );
        m_bristles.push_back( KisBristle( 0.5,-0.5 ) );
        m_bristles.push_back( KisBristle( 0.5, 0.5 ) );
        m_bristles.push_back( KisBristle( 0.0, 0.7 ) );
        m_bristles.push_back( KisBristle( 0.0,-0.7 ) );
        m_bristles.push_back( KisBristle( 0.7, 0.0 ) );
        m_bristles.push_back( KisBristle(-0.7, 0.0 ) );
        // More bristles
        m_bristles.push_back( KisBristle(-0.25,-0.25 ) );
        m_bristles.push_back( KisBristle(-0.25, 0.25 ) );
        m_bristles.push_back( KisBristle( 0.25,-0.25 ) );
        m_bristles.push_back( KisBristle( 0.25, 0.25 ) );
        m_bristles.push_back( KisBristle( 0.0, 0.35 ) );
        m_bristles.push_back( KisBristle( 0.0,-0.35 ) );
        m_bristles.push_back( KisBristle( 0.35, 0.0 ) );
        m_bristles.push_back( KisBristle(-0.35, 0.0 ) );
        bristlesPainter = 0;
    }
    double m_paintbrushMinRadius, m_paintbrushMaxRadius, m_bristlesDensity, m_bristlesMinRadius, m_bristlesMaxRadius;
    QList< KisBristle > m_bristles;
    bool m_firstStroke;
    KisPainter* bristlesPainter;
};

KisBristleShape::KisBristleShape(double paintbrushMinRadius, double paintbrushMaxRadius, double bristlesDensity, double bristlesMinRadius, double bristlesMaxRadius) :
        m_paintBrush(new KisPaintBrush(paintbrushMinRadius, paintbrushMaxRadius, bristlesDensity, bristlesMinRadius, bristlesMaxRadius) ),
        m_radius( 0.5 * (paintbrushMinRadius + paintbrushMaxRadius) ),
        m_angle( 0.0 )
{
}

QRect KisBristleShape::rect()
{
  return m_rect;
}

KisDynamicShape* KisBristleShape::clone() const
{
  return new KisBristleShape(*this);
}

void KisBristleShape::resize(double xs, double ys)
{
    kDebug() << xs << " " << ys << endl;
    m_radius *= (xs + ys) * 0.5;
}

void KisBristleShape::rotate(double r)
{
    m_angle = r;
}

void KisBristleShape::startPainting(KisPainter* _painter)
{
    KisDynamicShape::startPainting(_painter);
    KisAutobrushCircleShape cs(1, 1, 1.0, 1.0);
    QImage img;
    cs.createBrush(&img);
    m_paintBrush->bristlesPainter = new KisPainter(painter()->device() );
    m_paintBrush->bristlesPainter->setBrush( new KisAutobrushResource(img) );
    m_paintBrush->bristlesPainter->setPaintOp( KisPaintOpRegistry::instance()->paintOp( "paintbrush", 0, m_paintBrush->bristlesPainter, 0) );
}

void KisBristleShape::endPainting()
{
    KisDynamicShape::endPainting();
    m_paintBrush->bristlesPainter = 0;
    delete m_paintBrush->bristlesPainter;
}

void KisBristleShape::paintAt(const QPointF &brushPos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc)
{
    double angleCos = cos(m_angle);
    double angleSin = sin(m_angle);
    for( QList< KisBristle >::iterator it = m_paintBrush->m_bristles.begin();
        it != m_paintBrush->m_bristles.end(); ++it)
    {
        m_paintBrush->bristlesPainter->setPaintColor( it->color );
        double x = it->m_x * m_radius;
        double y = it->m_y * m_radius;
        QPointF pos( angleCos*x - angleSin*y , angleSin*x + angleCos*y );
//         kDebug() << pos << endl;
        pos += brushPos;
//         kDebug() << m_radius << " " << pos << " " << brushPos << endl;
        if( m_paintBrush->m_firstStroke)
        {
            m_paintBrush->bristlesPainter->paintLine( pos, pos );
        } else {
            m_paintBrush->bristlesPainter->paintLine( QPointF(it->m_lastX, it->m_lastY), pos );
        }
        it->m_lastX = pos.x();
        it->m_lastY = pos.y();
    }
    m_paintBrush->m_firstStroke = false;
    painter()->device()->setDirty( m_paintBrush->bristlesPainter->dirtyRegion() );
}

#if 0
void KisBristleShape::createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc,const QPointF &brushPos, const KisPaintInformation& info)
{
    
    double angleCos = cos(m_angle);
    double angleSin = sin(m_angle);
    kDebug() << angleCos << " " << angleSin << endl;
    // Clear the stamp
    stamp->clear();
    // Create a painter
    KisPainter p(stamp);
    KisAutobrushCircleShape cs(1, 1, 1.0, 1.0);
    QImage img;
    cs.createBrush(&img);
    p.setBrush( new KisAutobrushResource(img) );
    kDebug() << KisPaintOpRegistry::instance() << endl;
    kDebug() << KisPaintOpRegistry::instance()->paintOp( "paintbrush", 0, &p, 0) << endl;
    p.setPaintOp( KisPaintOpRegistry::instance()->paintOp( "paintbrush", 0, &p, 0) );
    m_rect = QRect(0,0,0,0);
    for( QList< KisBristle >::iterator it = m_paintBrush->m_bristles.begin();
        it != m_paintBrush->m_bristles.end(); ++it)
    {
        p.setPaintColor( it->color );
        double x = it->m_x * m_radius;
        double y = it->m_y * m_radius;
        QPointF pos( angleCos*x - angleSin*y , angleSin*x + angleCos*y );
//         kDebug() << pos << " " << pos2 << endl;
       if( m_paintBrush->m_firstStroke)
        {
            p.paintLine( pos, 0.5, 1.0, 1.0, pos, 0.5, 1.0, 1.0);
        } else {
            p.paintLine( QPointF(it->m_lastX - brushPos.x(), it->m_lastY - brushPos.y()), 0.5, 1.0, 1.0, pos, .5, 1.0, 1.0);
        }
        it->m_lastX = pos.x() + brushPos.x();
        it->m_lastY = pos.y() + brushPos.y();
    }
    m_paintBrush->m_firstStroke = false;
    m_rect = p.dirtyRegion() /*.boundingRect()*/;
//     m_rect = QRect( m_radius, m_radius, 2.0 * m_radius, 2.0 * m_radius );
    kDebug() << "Bristle shape rect: " << m_rect << endl;
}
#endif
