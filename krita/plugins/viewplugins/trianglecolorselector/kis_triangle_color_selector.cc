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

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <KoColorConversions.h>

enum CurrentHandle {
    NoHandle,
    HueHandle,
    ValueSaturationHandle };

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
    double wheelNormExt;
    double wheelNormInt;
    double wheelInnerRadius;
    double triangleRadius;
    double triangleLength;
    double triangleHeight;
    double triangleBottom;
    double triangleTop;
    double normExt;
    double normInt;
    bool updateAllowed;
    CurrentHandle handle;
    double triangleHandleSize;
};

KisTriangleColorSelector::KisTriangleColorSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    setMinimumHeight( 100 );
    setMinimumWidth( 100 );
    d->hue = 0;
    d->saturation = 0;
    d->value = 0;
    d->updateAllowed = true;
    setMouseTracking( true );
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
    d->wheelNormExt = qAbs( d->centerColorSelector );
    d->wheelNormInt = qAbs( d->centerColorSelector * (1.0 - d->wheelWidthProportion));
    d->wheelInnerRadius = d->centerColorSelector * (1.0 - d->wheelWidthProportion);
    d->triangleRadius = d->wheelInnerRadius * 0.9;
    d->triangleLength = 3.0 / sqrt(3.0) * d->triangleRadius;
    d->triangleHeight = d->triangleLength * sqrt(3.0) * 0.5;
    d->triangleTop = 0.5 * d->sizeColorSelector - d->triangleRadius;
    d->triangleBottom = d->triangleHeight + d->triangleTop;
    d->triangleHandleSize = 10.0;
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
        double ls_ = (vs_selector_ypos_) * d->triangleLength; // length of the saturation on the triangle
        double vs_selector_xpos_ = ls_ * (saturation() / 255.0 - 0.5);
        // Draw it
        p.save();
        p.setPen( QPen( Qt::white, 1.0) );
        p.setBrush( color() );
        p.rotate( hue() + 150 );
        p.drawEllipse( QRectF( -d->triangleHandleSize*0.5 + vs_selector_xpos_,
                               -d->triangleHandleSize*0.5 - (d->centerColorSelector - d->triangleTop) + vs_selector_ypos_ * d->triangleHeight,
                                d->triangleHandleSize , d->triangleHandleSize ));
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
    h = qBound(0, h, 360);
    d->hue = h;
    tellColorChanged();
    generateTriangle();
    update();
}

int KisTriangleColorSelector::value() const
{
    return d->value;
}

void KisTriangleColorSelector::setValue(int v)
{
    v = qBound(0, v, 255);
    d->value = v;
    tellColorChanged();
    generateTriangle();
    update();
}

int KisTriangleColorSelector::saturation() const
{
    return d->saturation;
}

void KisTriangleColorSelector::setSaturation(int s)
{
    s = qBound(0, s, 255);
    d->saturation = s;
    tellColorChanged();
    generateTriangle();
    update();
}

void KisTriangleColorSelector::setHSV(int h, int s, int v)
{
    h = qBound(0, h, 360);
    s = qBound(0, s, 255);
    v = qBound(0, v, 255);
    d->hue = h;
    d->value = v;
    d->saturation = s;
    tellColorChanged();
    generateTriangle();
    update();
}

QColor KisTriangleColorSelector::color() const
{
    int r,g,b;
    hsv_to_rgb( d->hue, d->saturation, d->value, &r, &g, &b);
    return QColor(r,g,b);
}

void KisTriangleColorSelector::setQColor(const QColor& c)
{
    if(d->updateAllowed)
    {
        rgb_to_hsv( c.red(), c.green(), c.blue(), &d->hue, &d->saturation, &d->value);
        generateTriangle();
        update();
    }
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

void KisTriangleColorSelector::tellColorChanged()
{
    d->updateAllowed = false;
    emit(colorChanged(color()));
    d->updateAllowed = true;
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
        double ls_ = (ynormalize) * d->triangleLength;
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
    for(int y = 0; y < d->sizeColorSelector; y++)
    {
        double yc = y - d->centerColorSelector;
        double y2 = pow2( yc );
        for(int x = 0; x < d->sizeColorSelector; x++)
        {
            double xc = x - d->centerColorSelector;
            double norm = sqrt(pow2( xc ) + y2);
            if( norm <= d->wheelNormExt + 1.0 and norm >= d->wheelNormInt - 1.0 )
            {
                double acoef = 1.0;
                if(norm > d->wheelNormExt ) acoef = (1.0 + d->wheelNormExt - norm);
                else if(norm < d->wheelNormInt ) acoef = (1.0 - d->wheelNormInt + norm);
                double angle = atan2(yc, xc);
                int h = (int)((180 * angle / M_PI) + 180);
                int r,g,b;
                hsv_to_rgb(h, 255, 255, &r, &g, &b);
                if( acoef < 0.999)
                {
                    img.setPixel(x,y, qRgba( (int)(r * acoef), (int)(g * acoef), (int)(b * acoef), (int)(255 * acoef)));
                } else {
                    img.setPixel(x,y, qRgba(r, g, b, 255 ));
                }
            } else {
                img.setPixel(x,y, qRgba(0,0,0,0));
            }
        }
    }
    d->wheelPixmap = QPixmap::fromImage(img);
}

void KisTriangleColorSelector::mouseReleaseEvent( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton)
    {
        selectColorAt( event->x(), event->y());
        d->handle = NoHandle;
    }
    QWidget::mouseReleaseEvent( event );
}

void KisTriangleColorSelector::mousePressEvent( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton)
    {
        d->handle = NoHandle;
        selectColorAt( event->x(), event->y());
    }
    QWidget::mousePressEvent( event );
}

void KisTriangleColorSelector::mouseMoveEvent( QMouseEvent * event )
{
    if(event->buttons() & Qt::LeftButton)
    {
        selectColorAt( event->x(), event->y(), false );
    }
    QWidget::mouseMoveEvent( event);
}

void KisTriangleColorSelector::selectColorAt(int _x, int _y, bool checkInWheel)
{
    double x = _x - 0.5*width();
    double y = _y - 0.5*height();
    // Check if the click is inside the wheel
    double norm = sqrt( x * x + y * y);
    if ( ( (norm < d->wheelNormExt) and (norm > d->wheelNormInt) and d->handle == NoHandle ) 
         or d->handle == HueHandle ) {
        d->handle = HueHandle;
        setHue( (int)(atan2(y, x) * 180 / M_PI ) + 180);
        update();
    }
    else {
    // Compute the s and v value, if they are in range, use them
        double rotation = -(hue() + 150) * M_PI / 180;
        double cr = cos(rotation);
        double sr = sin(rotation);
        double x1 = x * cr - y * sr; // <- now x1 gives the saturation
        double y1 = x * sr + y * cr; // <- now y1 gives the value
        y1 += d->wheelNormExt;
        double ynormalize = (d->triangleTop - y1 ) / ( d->triangleTop - d->triangleBottom );
        if( (ynormalize >= 0.0 and ynormalize <= 1.0 ) or d->handle == ValueSaturationHandle)
        {
            d->handle = ValueSaturationHandle;
            double ls_ = (ynormalize) * d->triangleLength; // length of the saturation on the triangle
            double sat = ( x1 / ls_ + 0.5) ;
            if((sat >= 0.0 and sat <= 1.0) or d->handle == ValueSaturationHandle)
            {
                setHSV( d->hue, sat * 255, ynormalize * 255);
            }
        }
        update();
    }
}

#include "kis_triangle_color_selector.moc"
