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
        
#include "KoColor.h"

KisColorSelectorTriangle::KisColorSelectorTriangle(KisColorSelector* parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

bool KisColorSelectorTriangle::containsPointInComponentCoords(int x, int y) const
{
    QPoint triangleCoords = widgetToTriangleCoordinates(QPoint(x, y));
    if(m_pixelCache.valid(triangleCoords) && m_pixelCache.pixel(triangleCoords)!=qRgba(0,0,0,0))
        return true;
    else
        return false;
}

void KisColorSelectorTriangle::paint(QPainter* painter)
{
    if(isDirty())
        updatePixelCache();
    
    painter->drawImage(width()/2-triangleWidth()/2,
                      height()/2-triangleHeight()*(2/3.),
                      m_pixelCache);


    if(m_lastClickPos.x()>-0.1 && m_parent->displayBlip()) {
        painter->setPen(QColor(0,0,0));
        painter->drawEllipse(m_lastClickPos.x()*width()-5, m_lastClickPos.y()*height()-5, 10, 10);
        painter->setPen(QColor(255,255,255));
        painter->drawEllipse(m_lastClickPos.x()*width()-4, m_lastClickPos.y()*height()-4, 8, 8);
    }
}

void KisColorSelectorTriangle::updatePixelCache()
{
    QImage cache(triangleWidth()+1, triangleHeight(), QImage::Format_ARGB32_Premultiplied);

    KoColor koColor(colorSpace());
    QColor qColor;

    for(int x=0; x<cache.width(); x++) {
        for(int y=0; y<cache.height(); y++) {
            qColor = colorAt(x, y);
            if(qColor.isValid()) {
                koColor.fromQColor(qColor);
                koColor.toQColor(&qColor);
                cache.setPixel(x, y, qColor.rgb());
            }
            else {
                cache.setPixel(x, y, qRgba(0,0,0,0));
            }
        }
    }

    // antialiased border
    QPainter painter(&cache);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(0,0,0,128), 2.5));
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.drawLine(QPointF(0, triangleHeight()), QPointF((triangleWidth())/2., 0));
    painter.drawLine(QPointF(triangleWidth()/2+1., 0), QPointF(triangleWidth()+1, triangleHeight()));


    m_pixelCache = cache;
}

QColor KisColorSelectorTriangle::selectColor(int x, int y)
{
    emit update();

    QPoint triangleCoords = widgetToTriangleCoordinates(QPoint(x,y));

    triangleCoords.setY(qBound(0, triangleCoords.y(), int(triangleHeight())));

    int horizontalLineLength = triangleCoords.y()*(2./sqrt(3.));
    int horizontalLineStart = triangleWidth()/2.-horizontalLineLength/2.;
    int horizontalLineEnd = horizontalLineStart+horizontalLineLength;

    triangleCoords.setX(qBound(horizontalLineStart, triangleCoords.x(), horizontalLineEnd));

    QPoint widgetCoords = triangleToWidgetCoordinates(triangleCoords);

    m_lastClickPos.setX(widgetCoords.x()/qreal(width()));
    m_lastClickPos.setY(widgetCoords.y()/qreal(height()));

    return colorAt(triangleCoords.x(), triangleCoords.y());
}

void KisColorSelectorTriangle::setColor(const QColor &color)
{
    qreal y=color.valueF()*triangleHeight();
    qreal horizontalLineLength = y*(2./sqrt(3.));
    qreal horizontalLineStart = 0.5*(triangleWidth()-horizontalLineLength);
    qreal x=color.saturationF()*horizontalLineLength+horizontalLineStart;

    QPoint tmp = triangleToWidgetCoordinates(QPoint(x, y));

    m_lastClickPos.setX(tmp.x()/qreal(width()));
    m_lastClickPos.setY(tmp.y()/qreal(height()));

    // Workaround for Bug 287001
    setLastMousePosition(tmp.x(), tmp.y());

    emit paramChanged(-1, color.saturationF(), color.valueF(), -1, -1);
    emit update();
}

int KisColorSelectorTriangle::triangleWidth() const
{
    return triangleHeight()*2/sqrt(3.0);
}

int KisColorSelectorTriangle::triangleHeight() const
{
    return height()*3./4.;
}

QColor KisColorSelectorTriangle::colorAt(int x, int y) const
{
    Q_ASSERT(x>=0 && x<=triangleWidth());
    Q_ASSERT(y>=0 && y<=triangleHeight());
    
    int triangleHeight = this->triangleHeight();
    int horizontalLineLength = y*(2./sqrt(3.));
    int horizontalLineStart = triangleWidth()/2.-horizontalLineLength/2.;
    int horizontalLineEnd = horizontalLineStart+horizontalLineLength;
    
    if(x<horizontalLineStart || x>horizontalLineEnd || y>triangleHeight)
        return QColor();
    qreal relativeX = x-horizontalLineStart;
    
    qreal value = (y)/qreal(triangleHeight);
    qreal saturation = relativeX/qreal(horizontalLineLength);

    return QColor::fromHsvF(m_hue, saturation, value);
}

QPoint KisColorSelectorTriangle::widgetToTriangleCoordinates(const QPoint &point) const
{
    QPoint triangleTopLeft(width()/2-triangleWidth()/2,
                           height()/2-triangleHeight()*(2/3.));
    QPoint ret=point-triangleTopLeft;
    return ret;
}

QPoint KisColorSelectorTriangle::triangleToWidgetCoordinates(const QPoint &point) const
{
    QPoint triangleTopLeft(width()/2.-triangleWidth()/2.,
                           height()/2.-triangleHeight()*(2./3.));
    QPoint ret=triangleTopLeft+point;
    return ret;
}

