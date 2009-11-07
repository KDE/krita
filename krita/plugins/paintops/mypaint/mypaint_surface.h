/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _MYPAINT_SURFACE_H_
#define _MYPAINT_SURFACE_H_

#include <KoColor.h>
#include <QColor>

#include "surface.hpp"

#include "kis_types.h"

/**
 * Implementation of MyPaint's bidirection surface interface.
 */
class MyPaintSurface : public Surface
{

public:

    /**
     * src: the projection of the node we are painting on. Used in get_color, together with
     *      color present on dst, blended using ALPHA_DARKEN
     * dst; the surface we are drawing on
     */
    MyPaintSurface(KisPaintDeviceSP src, KisPaintDeviceSP dst);

    bool draw_dab (float x, float y,
                   float radius,
                   float color_r, float color_g, float color_b,
                   float opaque, float hardness = 0.5,
                   float alpha_eraser = 1.0,
                   float aspect_ratio = 1.0, float angle = 0.0);

    void get_color (float x, float y,
                    float radius,
                    float * color_r, float * color_g, float * color_b, float * color_a );

private:

    KisPaintDeviceSP m_src;
    KisPaintDeviceSP m_dst;

    KoColor m_color;
    QColor m_qcolor;
};


#endif
