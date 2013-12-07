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
#include "mypaint_paintop_settings.h"

#include <math.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>

#include "mypaint_paintop_settings_widget.h"

MyPaintSettings::MyPaintSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION),
      m_options(0)
{
}

MyPaintBrushResource* MyPaintSettings::brush() const
{
    return m_options->brush();
}

QPainterPath MyPaintSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode) const
{
    QPainterPath path;
    if (mode == CursorIsOutline){
        qreal radius = expf(getFloat("radius_logarithmic"));
        path = ellipseOutline(2*radius, 2*radius, 1.0, 0.0 );
        path = outlineFetcher()->fetchOutline(info, this, path);
    }
    return path;
}
