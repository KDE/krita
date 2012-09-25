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

#ifndef KIS_HATCHING_PAINTOP_SETTINGS_H_
#define KIS_HATCHING_PAINTOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_brush_based_paintop_settings.h>

#include "kis_hatching_paintop_settings_widget.h"


class KisHatchingPaintOpSettings : public KisBrushBasedPaintOpSettings
{
    
public:
    KisHatchingPaintOpSettings();
    virtual ~KisHatchingPaintOpSettings();
    
    //Dialogs enabled
    bool enabledcurvecrosshatching;
    bool enabledcurveopacity;
    bool enabledcurveseparation;
    bool enabledcurvesize;
    bool enabledcurvethickness;
    
    //Hatching Options
    double angle;
    double separation;
    double thickness;
    double origin_x;
    double origin_y;
    bool nocrosshatching;
    bool perpendicular;
    bool minusthenplus;
    bool plusthenminus;
    bool moirepattern;
    int crosshatchingstyle;
    int separationintervals;
    
    //Hatching Preferences
    //bool trigonometryalgebra;
    //bool scratchoff;
    bool antialias;
    bool subpixelprecision;
    bool opaquebackground;
    
    //Crosshatching, Separation and Thickness curves
    double crosshatchingsensorvalue;
    double separationsensorvalue;
    double thicknesssensorvalue;
    
    void initializeTwin(KisHatchingPaintOpSettings* convenienttwin) const;
    using KisPropertiesConfiguration::fromXML;
    virtual void fromXML(const QDomElement&);
        
};

#endif
