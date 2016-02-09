/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_DUALBRUSHOP_OPTION_H
#define KIS_DUALBRUSHOP_OPTION_H

#include <kis_paintop_option.h>

const QString DUALBRUSH_RADIUS = "DualBrush/radius";
const QString DUALBRUSH_INK_DEPLETION = "DualBrush/inkDepletion";
const QString DUALBRUSH_USE_OPACITY = "DualBrush/opacity";
const QString DUALBRUSH_USE_SATURATION = "DualBrush/saturation";

class KisDualBrushOpOptionsWidget;

class KisDualBrushOpOption : public KisPaintOpOption
{
public:
    KisDualBrushOpOption();
    ~KisDualBrushOpOption();

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:

    KisDualBrushOpOptionsWidget * m_options;

};

class DualBrushProperties
{
public:
    int radius;
    bool inkDepletion;
    bool useOpacity;
    bool useSaturation;

    void readOptionSetting(const KisPropertiesConfiguration* settings) {
        radius = settings->getInt(DUALBRUSH_RADIUS);
        inkDepletion = settings->getBool(DUALBRUSH_INK_DEPLETION);
        useOpacity = settings->getBool(DUALBRUSH_USE_OPACITY);
        useSaturation = settings->getBool(DUALBRUSH_USE_SATURATION);
    }
};

#endif
