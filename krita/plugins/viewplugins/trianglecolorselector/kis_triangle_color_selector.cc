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

#include "kis_triangle_color_selector.h"

#include <math.h>

#include <kis_debug.h>

#include <QPainter>
#include <QPixmap>
#include <KoColorConversions.h>

struct KisTriangleColorSelector::Private {
    QPixmap wheelPixmap;
    QPixmap trianglePixmap;
    int hue;
    int saturation;
    int value;
    int sizeColorSelector;
    double centerColorSelector;
    double wheelWidthProportion;
    double wheelWidth;
    double wheelInnerRadius;
    double triangleLenght;
    double triangleHeight;
    double triangleBottom;
    double triangleTop;
};

KisTriangleColorSelector::KisTriangleColorSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    d->hue = 0;
    d->saturation = 0;
    d->value = 0;
    updateTriangleCircleParameters();
}

KisTriangleColorSelector::~KisTriangleColorSelector()
{
    delete d;
}

void KisTriangleColorSelector::updateTriangleCircleParameters()
{
    d->sizeColorSelector = qMin(width(), height());
    d->centerColorSelector = 0.5 * d->sizeColorSelector;
    d->wheelWidthProportion = 0.3;
    d->wheelWidth = d->centerColorSelector * d->wheelWidthProportion;
    d->wheelInnerRadius = d->centerColorSelector * (1.0 - d->wheelWidthProportion);
    d->triangleLenght = 3.0 / sqrt(3.0) * d->wheelInnerRadius;
    d->triangleHeight = d->triangleLenght * sqrt(3.0) * 0.5;
    d->triangleBottom = d->triangleHeight + d->wheelWidth;
    d->triangleTop = d->wheelWidth;
}

void KisTriangleColorSelector::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    QPointF pos(d->centerColorSelector, d->centerColorSelector);
    p.translate(QPointF( 0.5*width(), 0.5*height()  ) );
    // Draw the wheel
    p.drawPixmap( -pos, d->wheelPixmap );
    // Draw the triangle
    p.save();
    p.rotate( hue() + 150 );
    p.drawPixmap( -pos , d->trianglePixmap );
    // Draw selectors
    p.restore();
    // Draw value,saturation selector
    //   Compute coordinates
    {
        double vs_selector_ypos_ = value() / 255.0;
        double ls_ = (1.0 - vs_selector_ypos_) * d->triangleLenght; // length of the saturation on the triangle
        double vs_selector_xpos_ = ls_ * (saturation() / 255.0 - 0.5);
        // Draw it
        p.save();
        p.setPen( QPen( Qt::white, 1.0) );
        p.rotate( hue() + 90 );
        p.drawEllipse( QRectF( -1.5 + vs_selector_xpos_,
                               -1.5 + (d->centerColorSelector - d->triangleBottom) + vs_selector_ypos_ * d->triangleHeight,
                                3.0 , 3.0 ));
    }
    p.restore();
    // Draw Hue selector
    p.save();
    p.setPen( QPen( Qt::white, 1.0) );
    p.rotate( hue() - 90 );
    double hueSelectorWidth_ = 0.8;
    double hueSelectorOffset_ = 0.5 *( 1.0 - hueSelectorWidth_) * d->wheelWidth;
    double hueSelectorSize_ = 0.8 * d->wheelWidth;
    p.drawRect( QRectF( -1.5, -d->centerColorSelector + hueSelectorOffset_, 3.0, hueSelectorSize_ ));
    p.restore();
    p.end();
}

int KisTriangleColorSelector::hue() const
{
    return d->hue;
}

void KisTriangleColorSelector::setHue(int h)
{
    d->hue = h;
    generateTriangle();
    update();
}

int KisTriangleColorSelector::value() const
{
    return d->value;
}

void KisTriangleColorSelector::setValue(int v)
{
    d->value = v;
    generateTriangle();
    update();
}

int KisTriangleColorSelector::saturation() const
{
    return d->saturation;
}

void KisTriangleColorSelector::setSaturation(int s)
{
    d->saturation = s;
    generateTriangle();
    update();
}

void KisTriangleColorSelector::setHSV(int h, int v, int s)
{
    d->hue = h;
    d->value = v;
    d->saturation = s;
    generateTriangle();
    update();
}

void KisTriangleColorSelector::setQColor(const QColor& c)
{
    rgb_to_hsv( c.red(), c.green(), c.blue(), &d->hue, &d->value, &d->saturation );
    generateTriangle();
    update();
}

void KisTriangleColorSelector::incHue()
{
    int nh = d->hue + 1;
    if(nh > 360)
    {
        nh = 0;
    }
    setHue(nh);
    int nv = d->value + 1;
    if(nv > 255)
    {
        nv = 0;
    }
    setValue(nv);
    int ns = d->saturation - 1;
    if(ns < 0)
    {
        ns = 255;
    }
    setSaturation(ns);
}


void KisTriangleColorSelector::resizeEvent( QResizeEvent * event )
{
    QWidget::resizeEvent( event );
    updateTriangleCircleParameters();
    generateWheel();
    generateTriangle();
}

inline double pow2(double v)
{
    return v*v;
}

void KisTriangleColorSelector::generateTriangle()
{
    QImage img(d->sizeColorSelector, d->sizeColorSelector, QImage::Format_ARGB32_Premultiplied);
    // Length of triangle
    int hue_ = hue();
    
    for(int y = 0; y < d->sizeColorSelector; y++)
    {
        double ynormalize = ( d->triangleTop - y ) / ( d->triangleTop - d->triangleBottom );
        double v = 255 * ynormalize;
        double ls_ = (ynormalize) * d->triangleLenght;
        double startx_ = d->centerColorSelector - 0.5 * ls_;
        for(int x = 0; x < d->sizeColorSelector; x++)
        {
            double s = 255 * (x - startx_) / ls_;
            if(v < -1.0 or v > 256.0 or s < -1.0 or s > 256.0 )
            {
                img.setPixel(x,y, qRgba(0,0,0,0));
            } else {
                double va = 1.0, sa = 1.0;
                if( v < 0.0) { va = 1.0 + v; v = 0; }
                else if( v > 255.0 ) { va = 256.0 - v; v = 255; }
                if( s < 0.0) { sa = 1.0 + s; s = 0; }
                else if( s > 255.0 ) { sa = 256.0 - s; s = 255; }
                int r,g,b;
                hsv_to_rgb(hue_, (int)s, (int)v, &r, &g, &b);
                double coef = va * sa;
                if( coef < 0.999)
                {
                    img.setPixel(x,y, qRgba( (int)(r * coef), (int)(g * coef), (int)(b * coef), (int)(255 * coef)));
                } else {
                    img.setPixel(x,y, qRgba(r, g, b, 255 ));
                }
            }
        }
    }
    
    d->trianglePixmap = QPixmap::fromImage(img);
}

void KisTriangleColorSelector::generateWheel()
{
    QImage img(d->sizeColorSelector, d->sizeColorSelector, QImage::Format_ARGB32_Premultiplied);
    double center = 0.5 * d->sizeColorSelector;
    double normExt = pow2(center);
    double normInt = pow2(center * (1.0 - d->wheelWidthProportion));
    for(int y = 0; y < d->sizeColorSelector; y++)
    {
        double yc = y - center;
        double y2 = pow2( yc );
        for(int x = 0; x < d->sizeColorSelector; x++)
        {
            double xc = x - center;
            double norm = pow2( xc ) + y2;
            if( norm <= normExt and norm >= normInt )
            {
                double angle = atan2(yc, xc);
                int h = (int)((180 * angle / M_PI) + 180);
                int r,g,b;
                hsv_to_rgb(h, 255, 255, &r, &g, &b);
                img.setPixel(x,y, qRgb(r, g, b));
            } else {
                img.setPixel(x,y, qRgba(0,0,0,0));
            }
        }
    }
    d->wheelPixmap = QPixmap::fromImage(img);
}

#include "kis_triangle_color_selector.moc"
