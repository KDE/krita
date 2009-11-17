/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _BRUSH_H_
#define _BRUSH_H_

#include <QVector>

#include <KoColor.h>

#include "bristle.h"
#include "brush_shape.h"
#include "kis_chalk_paintop_settings.h"

#include "kis_paint_device.h"

class ChalkBrush
{

public:
    ChalkBrush(const KisChalkPaintOpSettings *settings);
    ~ChalkBrush();
    ChalkBrush(KoColor inkColor, BrushShape shape);
    void paint(KisPaintDeviceSP dev, qreal x, qreal y, const KoColor &color);

private:
    void init();
    QVector<Bristle> m_bristles;
    BrushShape m_initialShape;
    KoColor m_inkColor;
    int m_counter;
    int m_radius;
    const KisChalkPaintOpSettings * m_settings;

};

#endif
