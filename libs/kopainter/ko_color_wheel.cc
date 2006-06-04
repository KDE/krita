/*
 * Copyright (c) 2004 Sven Langkamp <longamp@reallygood.de>
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

#include <math.h>

#include <QPoint>
#include <QPainter>
#include <QImage>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QResizeEvent>

#include <kdebug.h>
#include "ko_color_wheel.h"

#define pi 3.14159265

KoOldColorWheel::KoOldColorWheel( QWidget *parent, const char *name ): KXYSelector( parent )
{
    setObjectName(name);
}

void KoOldColorWheel::resizeEvent( QResizeEvent * )
{
    drawWheel(&m_pixmap);
    setRange( 0, 0, contentsRect().width(), contentsRect().height() );
}

void KoOldColorWheel::drawContents( QPainter *painter )
{
    painter->drawPixmap( contentsRect().x(), contentsRect().y(), m_pixmap );
}

void KoOldColorWheel::drawWheel( QPixmap *pixmap )
{
    int size = qMin(contentsRect().width(), contentsRect().height());
    QPoint center(size/2, size/2);

    QImage image( size, size, 32 );
    image.fill(colorGroup ().background().pixel());

    QColor col;
    int a, b, h, s;
    uint *p;

    for ( a = size-1; a >= 0; a-- )
    {
        p = (uint*) image.scanLine( size - a - 1 );
        for( b = 0; b < size; b++ )
        {
            s = (int)(sqrt(pow(a-center.y(), 2) + pow(b-center.x(), 2))/(size/2)*255);
            if(s<=255)
            {
                h = (int)(atan2( b-center.x(), a-center.y())* 180.0 / pi);
                if(h<0) h += 360;
                if(h>360) h -= 360;

                col.setHsv( h, s, 210 );
                *p = col.rgb();
            }
            p++;
        }
    }
    pixmap->convertFromImage( image );
}

void KoOldColorWheel::mousePressEvent( QMouseEvent *e )
{
    int size = qMin(contentsRect().width(), contentsRect().height());
    QPoint center(size/2, size/2);

    int xVal, yVal;
    valuesFromPosition( e->pos().x() - 2, e->pos().y() - 2, xVal, yVal );
    setValues( xVal, yVal );

    int h, s;

    s = (int)(sqrt(pow(yVal-center.y(), 2) + pow(xVal-center.x(), 2))/(size/2)*255);
    if(s>255) s = 255;

    h = (int)(atan2( xVal-center.x(), yVal-center.y())* 180.0 / pi);
    if(h<0) h += 360;
    if(h>360) h -= 360;

    m_color.setHSV( h, s, 255);
    emit valueChanged(m_color);
}

void KoOldColorWheel::mouseMoveEvent( QMouseEvent *e )
{
    mousePressEvent( e );
}

void KoOldColorWheel::slotSetValue(const KoOldColor& c)
{
    int size = qMin(contentsRect().width(), contentsRect().height());
    QPoint center(size/2, size/2);

    int xVal, yVal;
    xVal = (int)(sin(c.H() * pi /180) * c.S() / 255 * (size/2) + center.x());
    yVal = (int)(cos(c.H() * pi /180) * c.S() / 255 * (size/2) + center.y());
    setValues( xVal, yVal );
}

#include "ko_color_wheel.moc"
