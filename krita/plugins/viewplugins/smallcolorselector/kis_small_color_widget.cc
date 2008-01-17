/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_small_color_widget.h"

#include <QPixmap>
#include <QPainter>

#include <KoColorConversions.h>

struct KisSmallColorWidget::Private
{
    QPixmap rubberPixmap;
    QPixmap squarePixmap;
    double rectangleWidthProportion;
    int rectangleHeight;
    int rectangleWidth;
    int rubberWidth;
    int rubberHeight;
    int margin;
    int hue;
    int value;
    int saturation;
};

KisSmallColorWidget::KisSmallColorWidget(QWidget* parent) : QWidget(parent), d(new Private)
{
    setMinimumHeight(50);
    d->hue = 200;
    d->value = 40;
    d->saturation = 70;
}

KisSmallColorWidget::~KisSmallColorWidget()
{
    delete d;
}

int KisSmallColorWidget::hue() const
{
    return d->hue;
}

int KisSmallColorWidget::value() const
{
    return d->value;
}

int KisSmallColorWidget::saturation() const
{
    return d->saturation;
}

void KisSmallColorWidget::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event);
    QPainter p(this);
    p.drawPixmap( 0, 0, d->rubberPixmap );
    p.drawPixmap( width() - d->rectangleWidth, 0 , d->squarePixmap );
    // Draw Hue handle
    p.save();
    p.setPen( QPen( Qt::white, 1.0) );
    p.translate( (d->hue * d->rubberWidth) / 360.0 , 0.0 );
    p.drawRect( QRectF( -1.5, 0 , 3.0, height()));
    p.restore();
    // Draw Saturation / Value handle
    p.save();
    p.setPen( QPen( Qt::white, 1.0) );
    p.translate( d->saturation * d->rectangleWidth / 255.0 + width() - d->rectangleWidth,
                 d->value * d->rectangleHeight / 255.0 );
    p.drawRect( QRectF( -1.5, -1.5, 3.0, 3.0 ) );
    p.end();
}

void KisSmallColorWidget::resizeEvent( QResizeEvent * event )
{
    QWidget::resizeEvent( event );
    setMaximumHeight( width() / 3 );
    updateParameters();
    generateRubber();
    generateSquare();
}

void KisSmallColorWidget::updateParameters()
{
    d->margin = 5;
    d->rectangleWidthProportion = 0.3;
    d->rectangleWidth = qMax( (int)( width() * d->rectangleWidthProportion) , height());
    d->rectangleHeight = height();
    d->rubberWidth = width() - d->rectangleWidth - d->margin;
    d->rubberHeight = height();
}

void KisSmallColorWidget::generateRubber()
{
    QImage img( d->rubberWidth, d->rubberHeight, QImage::Format_RGB32);
    for(int y = 0; y < d->rubberHeight; y++)
    {
        for(int x = 0; x < d->rubberWidth; x++)
        {
            int h = ( x * 360 ) / d->rubberWidth ;
            int r,g,b;
            hsv_to_rgb(h, 255, 255, &r, &g, &b);
            img.setPixel(x,y, qRgb(r, g, b ));
        }
    }
    d->rubberPixmap = QPixmap::fromImage(img);
}

void KisSmallColorWidget::generateSquare()
{
    QImage img( d->rectangleWidth, d->rectangleHeight, QImage::Format_RGB32);
    for(int y = 0; y < d->rectangleHeight; y++)
    {
        int v = (y * 255 ) / d->rectangleHeight;
        for(int x = 0; x < d->rectangleWidth; x++)
        {
            int s = (x * 255 ) / d->rectangleWidth;
            int r,g,b;
            hsv_to_rgb(hue(), s, v, &r, &g, &b);
            img.setPixel( x, y, qRgb(r, g, b) );
        }
    }
    d->squarePixmap = QPixmap::fromImage(img);
}
