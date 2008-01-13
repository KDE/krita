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
};

KisTriangleColorSelector::KisTriangleColorSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    d->hue = 0;
    d->saturation = 0;
    d->value = 0;
}

KisTriangleColorSelector::~KisTriangleColorSelector()
{
    delete d;
}

void KisTriangleColorSelector::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing);
    int sizeColorSelector_ = qMin(width(), height());
    double xpos = sizeColorSelector_ * 0.5;
    double ypos = sizeColorSelector_ * 0.5;
    double wheelWidth_ = 0.5 * sizeColorSelector_ * wheelWidth();
    QPointF pos(xpos, ypos);
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
        double center_ = 0.5 * sizeColorSelector_;
        double radius_ = center_ * (1.0 - wheelWidth());
        double lt_ = 3.0 / sqrt(3.0) * radius_; // Length of triangle
        double ht_ = lt_ * sqrt(3.0) * 0.5;
        double triangle_bottom_ = ht_ + wheelWidth_; // bottom of the triangle
        double ls_ = (1.0 - vs_selector_ypos_) * lt_; // length of the saturation on the triangle
        double vs_selector_xpos_ = ls_ * (saturation() / 255.0 - 0.5);
        // Draw it
        p.save();
        p.setPen( QPen( Qt::white, 1.0) );
        p.rotate( hue() + 90 );
        p.drawEllipse( QRectF( -1.5 + vs_selector_xpos_,
                               -1.5 + (ypos - triangle_bottom_) + vs_selector_ypos_ * ht_,
                                3.0 , 3.0 ));
    }
    p.restore();
    // Draw Hue selector
    p.save();
    p.setPen( QPen( Qt::white, 1.0) );
    p.rotate( hue() - 90 );
    double hueSelectorWidth_ = 0.8;
    double hueSelectorOffset_ = 0.5 *( 1.0 - hueSelectorWidth_) * wheelWidth_;
    double hueSelectorSize_ = 0.8 * wheelWidth_ ;
    p.drawRect( QRectF( -1.5, -ypos + hueSelectorOffset_, 3.0, hueSelectorSize_ ));
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
}

int KisTriangleColorSelector::saturation() const
{
    return d->saturation;
}

void KisTriangleColorSelector::setSaturation(int s)
{
    d->saturation = s;
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
    generateWheel();
    generateTriangle();
}

inline double pow2(double v)
{
    return v*v;
}

double KisTriangleColorSelector::wheelWidth() const
{
    return 0.3;
}

void KisTriangleColorSelector::generateTriangle()
{
    int size_ = qMin(width(), height());
    QImage img(size_, size_, QImage::Format_ARGB32_Premultiplied);
    double center_ = 0.5 * size_;
    double radius_ = center_ * (1.0 - wheelWidth());
    double wheelWidth_ = wheelWidth() * center_;
    // Length of triangle
    double lt_ = 3.0 / sqrt(3.0) * radius_;
    double ht_ = lt_ * sqrt(3.0) * 0.5;
    double bottom_ = ht_ + wheelWidth_;
    double top_ = wheelWidth_;
    int hue_ = hue();
    
    for(int y = 0; y < size_; y++)
    {
        double ynormalize = (top_ - y ) / ( top_ - bottom_ );
        double v = 255 * ynormalize;
        double ls_ = (ynormalize) * lt_;
        double startx_ = center_ - 0.5 * ls_;
        for(int x = 0; x < size_; x++)
        {
            double s = 255 * (x - startx_) / ls_;
            if(v < 0.0 or v > 255.0 or s < 0.0 or s > 255.0 )
            {
                img.setPixel(x,y, qRgba(0,0,0,0));
            } else {
                int r,g,b;
                hsv_to_rgb(hue_, (int)s, (int)v, &r, &g, &b);
                img.setPixel(x,y, qRgb(r,g,b));
            }
        }
    }
    
    d->trianglePixmap = QPixmap::fromImage(img);
}

void KisTriangleColorSelector::generateWheel()
{
    int size = qMin(width(), height());
    QImage img(size,size, QImage::Format_ARGB32_Premultiplied);
    double center = 0.5 * size;
    double normExt = pow2(center);
    double normInt = pow2(center * (1.0 - wheelWidth()));
    for(int y = 0; y < size; y++)
    {
        double yc = y - center;
        double y2 = pow2( yc );
        for(int x = 0; x < size; x++)
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
