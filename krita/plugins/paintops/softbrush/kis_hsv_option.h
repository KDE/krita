/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_HSV_OPTION_H
#define KIS_HSV_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

const QString HSV_ENABLED = "HSV/enabled";

const QString HSV_HMODE = "HSV/hueMode";
const QString HSV_SMODE = "HSV/saturationMode";
const QString HSV_VMODE = "HSV/valueMode";

const QString HSV_HUE_CURVE = "HSV/hueCurve";
const QString HSV_HUE_INK_AMOUNT = "HSV/hueInkAmount";

const QString HSV_SATURATION_CURVE = "HSV/saturationCurve";
const QString HSV_SATURATION_INK_AMOUNT = "HSV/saturationInkAmount";

const QString HSV_VALUE_CURVE = "HSV/valueCurve";
const QString HSV_VALUE_INK_AMOUNT = "HSV/valueInkAmount";

class KisHsvOptionsWidget;

class KisHSVOption : public KisPaintOpOption
{
public:
    KisHSVOption();
    ~KisHSVOption();
   
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisHsvOptionsWidget * m_options;
};

class HsvProperties{
public:
    quint16 hAmount;
    quint16 sAmount;
    quint16 vAmount;
        
    KisCubicCurve hcurve;
    KisCubicCurve scurve;
    KisCubicCurve vcurve;
    
    quint16 hmode;
    quint16 smode; 
    quint16 vmode; 

    bool enabled;

    void readOptionSetting(const KisPropertiesConfiguration * setting){
        hAmount = setting->getDouble(HSV_HUE_INK_AMOUNT);
        sAmount = setting->getDouble(HSV_SATURATION_INK_AMOUNT);
        vAmount = setting->getDouble(HSV_VALUE_INK_AMOUNT);

        hcurve = setting->getCubicCurve(HSV_HUE_CURVE);
        scurve = setting->getCubicCurve(HSV_SATURATION_CURVE);
        vcurve = setting->getCubicCurve(HSV_VALUE_CURVE);

        hmode = setting->getInt(HSV_HMODE);
        smode = setting->getInt(HSV_SMODE);
        vmode = setting->getInt(HSV_VMODE);
        
        enabled = setting->getBool(HSV_ENABLED);
    }
};

#endif
