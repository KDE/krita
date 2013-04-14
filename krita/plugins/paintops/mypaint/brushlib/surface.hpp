/* brushlib - The MyPaint Brush Library
 * Copyright (C) 2008 Martin Renold <martinxyz@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY. See the COPYING file for more details.
 */
#ifndef SURFACE_H
#define SURFACE_H

// surface interface required by brush.hpp
class Surface {
public:

    virtual ~Surface() {}

    virtual bool draw_dab (float x, float y,
                           float radius,
                           float color_r, float color_g, float color_b,
                           float opaque, float hardness = 0.5,
                           float alpha_eraser = 1.0,
                           float aspect_ratio = 1.0, float angle = 0.0
                                                                   ) = 0;

    virtual void get_color (float x, float y,
                            float radius,
                            float * color_r, float * color_g, float * color_b, float * color_a
                            ) = 0;
};
#endif
