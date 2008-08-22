/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_brush_based_paintop.h"
#include "kis_brush.h"
#include <QImage>
#include <QPainter>

KisBrushBasedPaintOp::KisBrushBasedPaintOp( KisPainter * painter )
    : KisPaintOp( painter )
{
    QImage img( 3, 3, QImage::Format_ARGB32 );
    QPainter p( &img );
    p.setRenderHint( QPainter::Antialiasing );
    p.fillRect( 0, 0, 3, 3, QBrush(QColor( 255, 255, 255, 0) ) );
    p.setBrush( QBrush( QColor( 0, 0, 0, 255 ) ) );
    p.drawEllipse( 0, 0, 3, 3 );
    p.end();

    m_brush = new KisBrush( img );
}

KisBrushBasedPaintOp::~KisBrushBasedPaintOp()
{
    delete m_brush;
}

double KisBrushBasedPaintOp::spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const
{
    // XXX: The spacing should vary as the pressure changes along the line.
    // This is a quick simplification.
    xSpacing = m_brush->xSpacing( scaleForPressure( (pressure1 + pressure2) / 2) );
    ySpacing = m_brush->ySpacing( scaleForPressure( (pressure1 + pressure2) / 2) );

    if (xSpacing < 0.5) {
        xSpacing = 0.5;
    }
    if (ySpacing < 0.5) {
        ySpacing = 0.5;
    }

    if (xSpacing > ySpacing) {
        return xSpacing;
    }
    else {
        return ySpacing;
    }
}
