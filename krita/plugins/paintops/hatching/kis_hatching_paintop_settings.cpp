/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_hatching_paintop_settings.h"

#include <kis_paint_action_type_option.h>

KisHatchingPaintOpSettings::KisHatchingPaintOpSettings()
{
}

KisHatchingPaintOpSettings::~KisHatchingPaintOpSettings()
{
}

void KisHatchingPaintOpSettings::initializeTwin(KisHatchingPaintOpSettings* convenienttwin) const
{
    convenienttwin->angle = getDouble("Hatching/angle");
    convenienttwin->separation = getDouble("Hatching/separation");
    convenienttwin->thickness = getDouble("Hatching/thickness");
    convenienttwin->origin_x = getDouble("Hatching/origin_x");
    convenienttwin->origin_y = getDouble("Hatching/origin_y");
    
    convenienttwin->nocrosshatching = getBool("Hatching/bool_nocrosshatching");
    convenienttwin->perpendicular = getBool("Hatching/bool_perpendicular");
    convenienttwin->minusthenplus = getBool("Hatching/bool_minusthenplus");
    convenienttwin->plusthenminus = getBool("Hatching/bool_plusthenminus");
    convenienttwin->moirepattern = getBool("Hatching/bool_moirepattern");
    
    convenienttwin->trigonometryalgebra = getBool("Hatching/bool_trigonometryalgebra");
    convenienttwin->scratchoff = getBool("Hatching/bool_scratchoff");
    convenienttwin->antialias = getBool("Hatching/bool_antialias");
    convenienttwin->opaquebackground = getBool("Hatching/bool_opaquebackground");
    convenienttwin->subpixelprecision = getBool("Hatching/bool_subpixelprecision");
                         
    if (getBool("Hatching/bool_nocrosshatching"))
        convenienttwin->crosshatchingstyle = 1;
    else if (getBool("Hatching/bool_perpendicular"))
        convenienttwin->crosshatchingstyle = 2;
    else if (getBool("Hatching/bool_minusthenplus"))
        convenienttwin->crosshatchingstyle = 3;
    else if (getBool("Hatching/bool_plusthenminus"))
        convenienttwin->crosshatchingstyle = 4;
    if (getBool("Hatching/bool_moirepattern"))
        convenienttwin->crosshatchingstyle = 5;
    
}




