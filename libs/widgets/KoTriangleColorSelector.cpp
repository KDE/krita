/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License,
 *  or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoTriangleColorSelector.h"
#include <math.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversions.h>
#include <KoColorDisplayRendererInterface.h>


enum CurrentHandle {
    NoHandle,
    HueHandle,
    ValueSaturationHandle };

struct Q_DECL_HIDDEN KoTriangleColorSelector::Private {
    Private(KoTriangleColorSelector *_q, const KoColorDisplayRendererInterface *_displayRenderer)
        : q(_q),
          displayRenderer(_displayRenderer),
          hue(0),
          saturation(0),
          value(0),
          updateAllowed(true),
          invalidTriangle(true),
          lastX(-1),
          lastY(-1)
    {
    }

    KoTriangleColorSelector *q;
    const KoColorDisplayRendererInterface *displayRenderer;
    QPixmap wheelPixmap;
    QPixmap trianglePixmap;
    int hue;
    int saturation;
    int value;
    int sizeColorSelector;
    qreal centerColorSelector;
    qreal wheelWidthProportion;
    qreal wheelWidth;
    qreal wheelNormExt;
    qreal wheelNormInt;
    qreal wheelInnerRadius;
    qreal triangleRadius;
    qreal triangleLength;
    qreal triangleHeight;
    qreal triangleBottom;
    qreal triangleTop;
    qreal normExt;
    qreal normInt;
    bool updateAllowed;
    CurrentHandle handle;
    qreal triangleHandleSize;
    bool invalidTriangle;
    int lastX, lastY;
    QTimer updateTimer;

    void init();
};

void KoTriangleColorSelector::Private::init()
{
    q->setMinimumHeight( 100 );
    q->setMinimumWidth( 100 );
    q->setMouseTracking( true );
    q->updateTriangleCircleParameters();
    updateTimer.setInterval(1);
    updateTimer.setSingleShot(true);
    q->connect(&updateTimer, SIGNAL(timeout()), q, SLOT(update()));
}

KoTriangleColorSelector::KoTriangleColorSelector(QWidget* parent)
    : KisColorSelectorInterface(parent),
      d(new Private(this, KoDumbColorDisplayRenderer::instance()))
{
    d->init();
}

KoTriangleColorSelector::KoTriangleColorSelector(const KoColorDisplayRendererInterface *displayRenderer, QWidget *parent)
    : KisColorSelectorInterface(parent),
      d(new Private(this, displayRenderer))
{
    d->init();
    connect(displayRenderer, SIGNAL(displayConfigurationChanged()), this, SLOT(configurationChanged()));
}

KoTriangleColorSelector::~KoTriangleColorSelector()
{
    delete d;
}

void KoTriangleColorSelector::updateTriangleCircleParameters()
{
    d->sizeColorSelector = qMin(width(), height());
    d->centerColorSelector = 0.5 * d->sizeColorSelector;
    d->wheelWidthProportion = 0.25;
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

void KoTriangleColorSelector::paintEvent( QPaintEvent * event )
{

    if( d->invalidTriangle )
    {
      generateTriangle();
    }
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
        qreal vs_selector_ypos_ = value() / 255.0;
        qreal ls_ = (vs_selector_ypos_) * d->triangleLength; // length of the saturation on the triangle
        qreal vs_selector_xpos_ = ls_ * (saturation() / 255.0 - 0.5);
        // Draw it
        p.save();
        p.setPen( QPen( Qt::white, 1.0) );

        QColor currentColor = d->displayRenderer->toQColor(getCurrentColor());

        p.setBrush(currentColor);
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
    qreal hueSelectorWidth_ = 0.8;
    qreal hueSelectorOffset_ = 0.5 *( 1.0 - hueSelectorWidth_) * d->wheelWidth;
    qreal hueSelectorSize_ = 0.8 * d->wheelWidth;
    p.drawRect( QRectF( -1.5, -d->centerColorSelector + hueSelectorOffset_, 3.0, hueSelectorSize_ ));
    p.restore();
    p.end();
}


// make sure to always use get/set functions when managing HSV properties( don't call directly like d->hue)
// these  settings get updated A LOT when the color picker is being used. You might get unexpected results
int KoTriangleColorSelector::hue() const
{
    return d->hue;
}

void KoTriangleColorSelector::setHue(int h)
{
    // setRealColor() will give you -1 when saturation is 0
    // ignore setting hue in this instance. otherwise it will mess up the hue ring
    if (h == -1)
        return;


    h = qBound(0, h, 359);
    d->hue = h;
    tellColorChanged();
    d->invalidTriangle = true;
    d->updateTimer.start();
}

int KoTriangleColorSelector::value() const
{
    return d->value;
}

void KoTriangleColorSelector::setValue(int v)
{
    v = qBound(0, v, 255);
    d->value = v;
    tellColorChanged();
    d->invalidTriangle = true;
    d->updateTimer.start();
}

int KoTriangleColorSelector::saturation() const
{
    return d->saturation;
}

void KoTriangleColorSelector::setSaturation(int s)
{
    s = qBound(0, s, 255);
    d->saturation = s;
    tellColorChanged();
    d->invalidTriangle = true;
    d->updateTimer.start();
}

void KoTriangleColorSelector::setHSV(int h, int s, int v)
{
    d->invalidTriangle = (hue() != h);
    setHue(h);
    setValue(v);
    setSaturation(s);
}

KoColor KoTriangleColorSelector::getCurrentColor() const
{
    return d->displayRenderer->fromHsv(hue(), saturation(), value());
}

void KoTriangleColorSelector::slotSetColor(const KoColor & color)
{
    if ( getCurrentColor() == color)
        return;

    //displayrenderer->getHsv is what sets the foreground color in the application
    if(d->updateAllowed) {
        int hueRef = hue();
        int saturationRef = saturation();
        int valueRef = value();

        d->displayRenderer->getHsv(color, &hueRef, &saturationRef, &valueRef);
        setHSV(hueRef, saturationRef, valueRef);

        d->invalidTriangle = true;
        d->updateTimer.start();
    }
}

void KoTriangleColorSelector::resizeEvent( QResizeEvent * event )
{
    QWidget::resizeEvent( event );
    updateTriangleCircleParameters();
    generateWheel();
    d->invalidTriangle = true;
}

inline qreal pow2(qreal v)
{
    return v*v;
}

void KoTriangleColorSelector::tellColorChanged()
{
    d->updateAllowed = false;
    emit(sigNewColor(getCurrentColor()));
    emit(colorChanged(getCurrentColor().toQColor()));
    d->updateAllowed = true;
}

void KoTriangleColorSelector::generateTriangle()
{
    QImage image(d->sizeColorSelector, d->sizeColorSelector, QImage::Format_ARGB32);
    // Length of triangle
    int hue_ = hue();

    for(int y = 0; y < d->sizeColorSelector; ++y)
    {
        qreal ynormalize = ( d->triangleTop - y ) / ( d->triangleTop - d->triangleBottom );
        qreal v = 255 * ynormalize;
        qreal ls_ = (ynormalize) * d->triangleLength;
        qreal startx_ = d->centerColorSelector - 0.5 * ls_;
        uint* data = reinterpret_cast<uint*>(image.scanLine(y));
        for(int x = 0; x < d->sizeColorSelector; ++x, ++data)
        {
            qreal s = 255 * (x - startx_) / ls_;
            if(v < -1.0 || v > 256.0 || s < -1.0 || s > 256.0 )
            {
                *data = qRgba(0,0,0,0);
            } else {
                qreal va = 1.0, sa = 1.0;
                if( v < 0.0) { va = 1.0 + v; v = 0; }
                else if( v > 255.0 ) { va = 256.0 - v; v = 255; }
                if( s < 0.0) { sa = 1.0 + s; s = 0; }
                else if( s > 255.0 ) { sa = 256.0 - s; s = 255; }
                qreal coeff = va * sa;

                KoColor color = d->displayRenderer->fromHsv(hue_, s, v, int(coeff * 255.0));
                QColor qcolor = d->displayRenderer->toQColor(color);

                *data = qcolor.rgba();
            }
        }
    }

    d->trianglePixmap = QPixmap::fromImage(image);
    d->invalidTriangle = false;
}

void KoTriangleColorSelector::generateWheel()
{
    QImage image(d->sizeColorSelector, d->sizeColorSelector, QImage::Format_ARGB32);
    for(int y = 0; y < d->sizeColorSelector; y++)
    {
        qreal yc = y - d->centerColorSelector;
        qreal y2 = pow2( yc );
        for(int x = 0; x < d->sizeColorSelector; x++)
        {
            qreal xc = x - d->centerColorSelector;
            qreal norm = sqrt(pow2( xc ) + y2);
            if( norm <= d->wheelNormExt + 1.0 && norm >= d->wheelNormInt - 1.0 )
            {
                qreal acoef = 1.0;
                if(norm > d->wheelNormExt ) acoef = (1.0 + d->wheelNormExt - norm);
                else if(norm < d->wheelNormInt ) acoef = (1.0 - d->wheelNormInt + norm);
                qreal angle = atan2(yc, xc);
                int h = (int)((180 * angle / M_PI) + 180) % 360;

                KoColor color = d->displayRenderer->fromHsv(h, 255, 255, int(acoef * 255.0));
                QColor qcolor = d->displayRenderer->toQColor(color);

                image.setPixel(x,y, qcolor.rgba());
            } else {
                image.setPixel(x,y, qRgba(0,0,0,0));
            }
        }
    }
    d->wheelPixmap = QPixmap::fromImage(image);
}

void KoTriangleColorSelector::mouseReleaseEvent( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton)
    {
        selectColorAt( event->x(), event->y());
        d->handle = NoHandle;
    } else {
        QWidget::mouseReleaseEvent( event );
    }
}

void KoTriangleColorSelector::mousePressEvent( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton)
    {
        d->handle = NoHandle;
        selectColorAt( event->x(), event->y());
    } else {
        QWidget::mousePressEvent( event );
    }
}

void KoTriangleColorSelector::mouseMoveEvent( QMouseEvent * event )
{
    if(event->buttons() & Qt::LeftButton)
    {
        selectColorAt( event->x(), event->y(), false );
    } else {
        QWidget::mouseMoveEvent( event);
    }
}

void KoTriangleColorSelector::selectColorAt(int _x, int _y, bool checkInWheel)
{
    Q_UNUSED( checkInWheel );

    if (d->lastX == _x && d->lastY == _y)
    {
        return;
    }
    d->lastX = _x;
    d->lastY = _y;

    qreal x = _x - 0.5*width();
    qreal y = _y - 0.5*height();
    // Check if the click is inside the wheel
    qreal norm = sqrt( x * x + y * y);
    if ( ( (norm < d->wheelNormExt) && (norm > d->wheelNormInt) && d->handle == NoHandle )
         || d->handle == HueHandle ) {
        d->handle = HueHandle;
        setHue( (int)(atan2(y, x) * 180 / M_PI ) + 180);
        d->updateTimer.start();
    }
    else {
    // Compute the s and v value, if they are in range, use them
        qreal rotation = -(hue() + 150) * M_PI / 180;
        qreal cr = cos(rotation);
        qreal sr = sin(rotation);
        qreal x1 = x * cr - y * sr; // <- now x1 gives the saturation
        qreal y1 = x * sr + y * cr; // <- now y1 gives the value
        y1 += d->wheelNormExt;
        qreal ynormalize = (d->triangleTop - y1 ) / ( d->triangleTop - d->triangleBottom );
        if( (ynormalize >= 0.0 && ynormalize <= 1.0 ) || d->handle == ValueSaturationHandle)
        {
            d->handle = ValueSaturationHandle;
            qreal ls_ = (ynormalize) * d->triangleLength; // length of the saturation on the triangle
            qreal sat = ( x1 / ls_ + 0.5) ;
            if((sat >= 0.0 && sat <= 1.0) || d->handle == ValueSaturationHandle)
            {
                setHSV( hue(), sat * 255, ynormalize * 255);
            }
        }
        d->updateTimer.start();
    }
}

void KoTriangleColorSelector::configurationChanged()
{
    generateWheel();
    d->invalidTriangle = true;
    update();
}
