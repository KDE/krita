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

#ifndef _MYPAINT_BRUSH_RESOURCE_H_
#define _MYPAINT_BRUSH_RESOURCE_H_

#include <QImage>
#include <QString>
#include <QColor>

#include <KoResource.h>

#include <stdint.h>
#include <math.h>

#include "surface.hpp"
#include "brush.hpp"

#include "kis_types.h"

class BrushSetting;

/**
 * Subclass of MyPaint's brush that can load and save itself
 * and conforms to the KoResource interface so we can use resource
 * servers.
 *
 * MyPaint implements loading/saving of brushes in Python.
 */
class MyPaintBrushResource : public KoResource, public Brush
{

public:

    MyPaintBrushResource(const QString& filename);
    virtual ~MyPaintBrushResource();

    // XXX: implement copy constructor!!!

    bool load();

    bool save();

    QImage img() const;

public: // From mypaint/lib/brush.py Brush_Lowlevel, which inherits brushlib/Brush

    BrushSetting* setting_by_cname(const QString& cname);

    void get_color_hsv(float* h, float* s, float* v);

    void set_color_hsv(float h, float s, float v);

    void set_color_rgb(QRgb rgb);

    void set_color_rgb(float r, float g, float b);

    QRgb get_color_rgb();



    bool is_eraser();

private:

    QImage m_icon;
    QVector<BrushSetting*> m_settings;
};


#endif
