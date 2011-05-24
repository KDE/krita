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

#ifndef KIS_COLOR_SELECTOR_TRIANGLE_H
#define KIS_COLOR_SELECTOR_TRIANGLE_H

#include "kis_color_selector_component.h"

#include <QImage>

class KisColorSelectorTriangle : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorTriangle(KisColorSelector* parent);
    QColor selectColor(int x, int y);
    void setColor(const QColor &color);

protected:
    void paint(QPainter*);
    bool containsPointInComponentCoords(int x, int y) const;

private:
    int triangleWidth() const;
    int triangleHeight() const;
    void updatePixelCache();
    QColor colorAt(int x, int y) const;
    QPoint widgetToTriangleCoordinates(const QPoint& point) const;
    QPoint triangleToWidgetCoordinates(const QPoint& point) const;
    QImage m_pixelCache;
    
    QPointF m_lastClickPos;
};

#endif // KIS_COLOR_SELECTOR_TRIANGLE_H
