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

#include <cmath>

#include "KoColorSpace.h"

#include "kis_display_color_converter.h"
#include "kis_acs_pixel_cache_renderer.h"


KisColorSelectorTriangle::KisColorSelectorTriangle(KisColorSelector* parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

bool KisColorSelectorTriangle::containsPointInComponentCoords(int x, int y) const
{
    QPoint triangleCoords = widgetToTriangleCoordinates(QPoint(x, y));

    if (!m_realPixelCache) return false;
    KoColor pixel = Acs::pickColor(m_realPixelCache, triangleCoords);
    return pixel.opacityU8() == OPACITY_OPAQUE_U8;
}

void KisColorSelectorTriangle::paint(QPainter* painter)
{
    if(isDirty()) {
        updatePixelCache();
    }


    painter->drawImage(width()/2-triangleWidth()/2,
                      height()/2-triangleHeight()*(2/3.),
                      m_renderedPixelCache);


    if(m_lastClickPos.x()>-0.1 && m_parent->displayBlip()) {
        painter->setPen(QColor(0,0,0));
        painter->drawEllipse(m_lastClickPos.x()*width()-5, m_lastClickPos.y()*height()-5, 10, 10);
        painter->setPen(QColor(255,255,255));
        painter->drawEllipse(m_lastClickPos.x()*width()-4, m_lastClickPos.y()*height()-4, 8, 8);
    }
}

void KisColorSelectorTriangle::updatePixelCache()
{
    int width = triangleWidth() + 1;
    int height = triangleHeight();

    QPoint pixelCacheOffset;

    if (m_cachedSize != QSize(width, height) && m_realPixelCache) {
        m_realPixelCache = 0;
    }

    Acs::PixelCacheRenderer::render(this,
                                    m_parent->converter(),
                                    QRect(0, 0, width, height),
                                    m_realPixelCache,
                                    m_renderedPixelCache,
                                    pixelCacheOffset);

//    if (!pixelCacheOffset.isNull()) {
//        warnKrita << "WARNING: offset of the triangle selector is not null!";
//    }

    // antialiased border
    QPainter gc(&m_renderedPixelCache);
    gc.setRenderHint(QPainter::Antialiasing);
    gc.setPen(QPen(QColor(0,0,0,128), 2.5));
    gc.setCompositionMode(QPainter::CompositionMode_Clear);
    gc.drawLine(QPointF(0, triangleHeight()), QPointF((triangleWidth()) / 2.0, 0));
    gc.drawLine(QPointF(triangleWidth() / 2.0 + 1.0, 0), QPointF(triangleWidth() + 1, triangleHeight()));
}

KoColor KisColorSelectorTriangle::selectColor(int x, int y)
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

void KisColorSelectorTriangle::setColor(const KoColor &color)
{
    qreal h, s, v;
    m_parent->converter()->getHsvF(color, &h, &s, &v);

    qreal y = v * triangleHeight();
    qreal horizontalLineLength = y * (2. / sqrt(3.));
    qreal horizontalLineStart = 0.5 * (triangleWidth() - horizontalLineLength);
    qreal x=s * horizontalLineLength + horizontalLineStart;

    QPoint tmp = triangleToWidgetCoordinates(QPoint(x, y));

    m_lastClickPos.setX(tmp.x()/qreal(width()));
    m_lastClickPos.setY(tmp.y()/qreal(height()));

    // Workaround for Bug 287001
    setLastMousePosition(tmp.x(), tmp.y());

    emit paramChanged(-1, s, v, -1, -1, -1, -1, -1, -1);
    emit update();
    KisColorSelectorComponent::setColor(color);
}

int KisColorSelectorTriangle::triangleWidth() const
{
    return triangleHeight()*2/sqrt(3.0);
}

int KisColorSelectorTriangle::triangleHeight() const
{
    return height()*3./4.;
}

KoColor KisColorSelectorTriangle::colorAt(int x, int y) const
{
    Q_ASSERT(x>=0 && x<=triangleWidth());
    Q_ASSERT(y>=0 && y<=triangleHeight());

    int triangleHeight = this->triangleHeight();
    int horizontalLineLength = y*(2./sqrt(3.));
    int horizontalLineStart = triangleWidth()/2.-horizontalLineLength/2.;
    int horizontalLineEnd = horizontalLineStart+horizontalLineLength;

    if(x<horizontalLineStart || x>horizontalLineEnd || y>triangleHeight)
        return KoColor(Qt::transparent, colorSpace());

    qreal relativeX = x-horizontalLineStart;

    qreal value = (y)/qreal(triangleHeight);
    qreal saturation = relativeX/qreal(horizontalLineLength);

    return m_parent->converter()->fromHsvF(m_hue, saturation, value);
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

