/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_triangle.h"

#include <QPainter>
#include <QMouseEvent>

#include <Eigen/Core>
USING_PART_OF_NAMESPACE_EIGEN
#include <cmath>
        
#include <KDebug>
        
#include "KoColor.h"

KisColorSelectorTriangle::KisColorSelectorTriangle(KisColorSelectorBase* parent) :
    KisColorSelectorComponent(parent),
    m_hue(0),
    m_lastClickPos(-1,-1)
{
}

void KisColorSelectorTriangle::setRadius(int radius)
{
    m_radius = radius;
}

bool KisColorSelectorTriangle::isComponent(int x, int y) const
{
    QPoint triangleCoords = widgetToTriangleCoordinates(QPoint(x, y));
    if(m_pixelCache.valid(triangleCoords) && m_pixelCache.pixel(triangleCoords)!=qRgba(0,0,0,0))
        return true;
    else
        return false;
}

void KisColorSelectorTriangle::setHue(int hue)
{
    m_hue=hue;
    update();
}

void KisColorSelectorTriangle::paintEvent(QPaintEvent *)
{
    updatePixelCache();
    QPainter painter(this);
    
    painter.drawImage(width()/2-triangleWidth()/2,
                      height()/2-triangleHeight()*(2/3.),
                      m_pixelCache);
    if(m_lastClickPos!=QPoint(-1,-1)) {
        painter.setPen(QColor(0,0,0));
        painter.drawEllipse(m_lastClickPos, 5, 5);
        painter.setPen(QColor(255,255,255));
        painter.drawEllipse(m_lastClickPos, 4, 4);
    }
}

void KisColorSelectorTriangle::updatePixelCache()
{
    QImage cache(triangleWidth(), triangleHeight(), QImage::Format_ARGB32_Premultiplied);
    cache.fill(qRgba(0,0,0,0));
    
    for(int x=0; x<cache.width(); x++) {
        for(int y=0; y<cache.height(); y++) {
            cache.setPixel(x, y, colorAt(x, y));
        }
    }
    m_pixelCache = cache;
}

void KisColorSelectorTriangle::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    
    updatePixelCache();
}

void KisColorSelectorTriangle::mousePressEvent(QMouseEvent * event)
{
    kDebug()<<"##########triangle mouse press";
    if(isComponent(event->x(), event->y())) {
        m_lastClickPos = event->pos();
        update();
        event->accept();
    }
    else {
        event->ignore();
    }
}

int KisColorSelectorTriangle::triangleWidth() const
{
    return triangleHeight()*2/sqrt(3);
}

int KisColorSelectorTriangle::triangleHeight() const
{
    return m_radius*3/2;
}

QRgb KisColorSelectorTriangle::colorAt(int x, int y) const
{
    Q_ASSERT(x>=0 && x<triangleWidth());
    Q_ASSERT(y>=0 && y<triangleHeight());
    
    qreal triangleHeight = this->triangleHeight();
    qreal horizontalLineLength = qreal(y)*(2./sqrt(3.));
    qreal horizontalLineStart = qreal(triangleWidth())/2.-horizontalLineLength/2.;
    qreal horizontalLineEnd = horizontalLineStart+horizontalLineLength;
    
    if(x<horizontalLineStart || x>horizontalLineEnd || y>triangleHeight)
        return qRgba(0,0,0,0);
    qreal relativeX = x-horizontalLineStart;
    
    qreal value = (y)/triangleHeight;
    qreal saturation = relativeX/horizontalLineLength;

    return KoColor(QColor::fromHsvF(m_hue/359., saturation, value).rgb(), colorSpace()).toQColor().rgb();;
}

QPoint KisColorSelectorTriangle::widgetToTriangleCoordinates(const QPoint &point) const
{
    QPoint triangleTopLeft(width()/2-triangleWidth()/2,
                           height()/2-triangleHeight()*(2/3.));
    QPoint ret=point-triangleTopLeft;
    return ret;
}

