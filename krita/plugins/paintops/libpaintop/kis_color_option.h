/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#ifndef KIS_COLOR_OPTION_H
#define KIS_COLOR_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

const QString COLOROP_HUE = "ColorOption/hue";
const QString COLOROP_SATURATION = "ColorOption/saturation";
const QString COLOROP_VALUE = "ColorOption/value";
    
const QString COLOROP_USE_RANDOM_HSV = "ColorOption/useRandomHSV";
const QString COLOROP_USE_RANDOM_OPACITY = "ColorOption/useRandomOpacity";
const QString COLOROP_SAMPLE_COLOR = "ColorOption/sampleInputColor";
    
const QString COLOROP_FILL_BG = "ColorOption/fillBackground";
const QString COLOROP_COLOR_PER_PARTICLE = "ColorOption/colorPerParticle";
const QString COLOROP_MIX_BG_COLOR= "ColorOption/mixBgColor";

class KisColorOptionsWidget;

class PAINTOP_EXPORT KisColorOption : public KisPaintOpOption
{
public:
    KisColorOption();
    ~KisColorOption();

    bool useRandomHSV() const;
    bool useRandomOpacity() const;
    bool sampleInputColor() const;
    
    bool fillBackground() const;
    bool colorPerParticle() const;
    bool mixBgColor() const;
    
    // TODO: these should be intervals like 20..180 
    int hue() const;
    int saturation() const;
    int value() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);
private:
   KisColorOptionsWidget * m_options;
};

class PAINTOP_EXPORT KisColorProperties
{
public:
    bool useRandomHSV;
    bool useRandomOpacity;
    bool sampleInputColor;

    bool fillBackground;
    bool colorPerParticle;
    bool mixBgColor;

    qint8 hue;
    qint8 saturation;
    qint8 value;
public:
    /// fill the class members with related properties
    void fillProperties(const KisPropertiesConfiguration* setting);
};

#endif // KIS_COLOR_OPTION_H

