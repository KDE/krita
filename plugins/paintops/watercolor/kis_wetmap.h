/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_WETMAP_H
#define KIS_WETMAP_H

#include "kritawatercolorpaintop_export.h"

#include "kis_paint_device.h"

class WATERCOLORPAINT_EXPORT KisWetMap
{
public:
    KisWetMap();

    void addWater(QPoint pos, qreal radius);
    void update();
    int getWater(int x, int y);

    KisPaintDeviceSP getPaintDevice();
private:
    KisPaintDeviceSP m_wetMap;
    float* m_waterVelocitiesX;
    float* m_waterVelocitiesY;
    int m_width;
    int m_height;
};

#endif
