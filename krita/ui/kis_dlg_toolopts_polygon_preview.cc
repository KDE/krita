/*
 *  kis_dlg_toolopts_polygon_preview.cc - part of Krayon
 *
 *  Base code from Kontour.
 *  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "kis_dlg_toolopts_polygon_preview.h"

#include <qpainter.h>
#include <qpointarray.h>

#include <math.h>

PolygonPreview::PolygonPreview( QWidget* parent, const char* name, int _nCorners, int _sharpness, bool _isConcave,
                                int _lineThickness ) :
  QFrame( parent, name )
{
    nCorners = _nCorners;
    sharpness = _sharpness;
    isConcave = _isConcave;
    lineThickness = _lineThickness;
    setBackgroundMode( PaletteBase );
    setFocusPolicy( QWidget::NoFocus );
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
}

void PolygonPreview::paintEvent( QPaintEvent * )
{
    QPainter p;
    double angle = 2 * M_PI / nCorners;
    double diameter = static_cast<double>( QMAX( width(), height() ) - 10 );
    double radius = diameter * 0.5;

    p.begin(this);
    p.setWindow( qRound( -radius ), qRound( -radius ), qRound( diameter ), qRound( diameter ) );
    p.setViewport( 5, 5, width() - 10, height() - 10 );
    p.setPen( QPen( Qt::black, lineThickness ) );//colorGroup().text() );

    QPointArray points( isConcave ? nCorners * 2 : nCorners );
    points.setPoint( 0, 0, qRound( -radius ) );

    if ( isConcave ) {
        angle = angle / 2.0;
        double a = angle;
        double r = radius - ( sharpness / 100.0 * radius );
        for ( int i = 1; i < nCorners * 2; ++i ) {
            double xp, yp;
            if ( i % 2 ) {
                xp =  r * sin( a );
                yp = -r * cos( a );
            }
            else {
                xp = radius * sin( a );
                yp = -radius * cos( a );
            }
            a += angle;
            points.setPoint( i, (int) xp, (int) yp );
        }
    }
    else {
        double a = angle;
        for ( int i = 1; i < nCorners; i++ ) {
            double xp = radius * sin( a );
            double yp = -radius * cos( a );
            a += angle;
            points.setPoint( i, (int) xp, (int) yp );
        }
    }
    p.drawPolygon( points );
    p.end();
}

void PolygonPreview::slotConvexPolygon()
{
    isConcave = false;
    repaint();
}

void PolygonPreview::slotConcavePolygon()
{
    isConcave = true;
    repaint();
}

void PolygonPreview::slotConersValue( int value )
{
    nCorners = value;
    repaint();
}

void PolygonPreview::slotSharpnessValue( int value )
{
    sharpness = value;
    repaint();
}

void PolygonPreview::slotThicknessValue( int value )
{
    lineThickness = value;
    repaint();
}

#include "kis_dlg_toolopts_polygon_preview.moc"
