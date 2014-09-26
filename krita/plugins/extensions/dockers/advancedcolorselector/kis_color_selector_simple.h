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

#ifndef KIS_COLOR_SELECTOR_SIMPLE_H
#define KIS_COLOR_SELECTOR_SIMPLE_H

typedef unsigned int QRgb;

#include <QColor>
#include <QImage>

#include "KoColor.h"
#include "kis_color_selector_component.h"

namespace Acs {
    class PixelCacheRenderer;
}


class KisColorSelectorSimple : public KisColorSelectorComponent
{
Q_OBJECT
public:
    explicit KisColorSelectorSimple(KisColorSelector *parent);
    void setColor(const KoColor &color);

protected:
    void paint(QPainter*);
    KoColor selectColor(int x, int y);

private:
    friend class Acs::PixelCacheRenderer;
    KoColor colorAt(int x, int y);

private:
    QPointF m_lastClickPos;
    QImage m_pixelCache;
    qreal R;
    qreal G;
    qreal B;
};

#endif
