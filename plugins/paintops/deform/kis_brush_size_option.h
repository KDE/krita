/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_SIZE_OPTION_H_
#define KIS_SIZE_OPTION_H_

#include <cmath>
#include <QtGlobal>

#include <kis_paintop_option.h>

class KisBrushSizeOptionsWidget;

const QString BRUSH_SHAPE = "Brush/shape";
const QString BRUSH_DIAMETER = "Brush/diameter";
const QString BRUSH_ASPECT = "Brush/aspect";
const QString BRUSH_SCALE = "Brush/scale";
const QString BRUSH_ROTATION = "Brush/rotation";
const QString BRUSH_SPACING = "Brush/spacing";
const QString BRUSH_DENSITY = "Brush/density";
const QString BRUSH_JITTER_MOVEMENT = "Brush/jitterMovement";
const QString BRUSH_JITTER_MOVEMENT_ENABLED = "Brush/jitterMovementEnabled";

class KisBrushSizeOption : public KisPaintOpOption
{
public:
    KisBrushSizeOption();
    ~KisBrushSizeOption() override;

    int diameter() const;
    void setDiameter(int diameter);

    void setSpacing(qreal spacing);
    qreal spacing() const;

    qreal brushAspect() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisBrushSizeOptionsWidget * m_options;
};

class KisBrushSizeOptionProperties : public KisPaintopPropertiesBase
{

public:
    qreal brush_diameter;
    qreal brush_aspect;
    qreal brush_rotation;
    qreal brush_scale;
    qreal brush_spacing;
    qreal brush_density;
    qreal brush_jitter_movement;
    bool brush_jitter_movement_enabled;

public:

    void readOptionSettingImpl(const KisPropertiesConfiguration *setting) override {
        brush_diameter = setting->getDouble(BRUSH_DIAMETER);
        brush_aspect = setting->getDouble(BRUSH_ASPECT);
        brush_rotation = setting->getDouble(BRUSH_ROTATION);
        brush_scale = setting->getDouble(BRUSH_SCALE);
        brush_spacing = setting->getDouble(BRUSH_SPACING);
        brush_density = setting->getDouble(BRUSH_DENSITY);
        brush_jitter_movement = setting->getDouble(BRUSH_JITTER_MOVEMENT);
        brush_jitter_movement_enabled = setting->getBool(BRUSH_JITTER_MOVEMENT_ENABLED);
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override {
        setting->setProperty(BRUSH_DIAMETER, brush_diameter);
        setting->setProperty(BRUSH_ASPECT, brush_aspect);
        setting->setProperty(BRUSH_ROTATION, brush_rotation);
        setting->setProperty(BRUSH_SCALE, brush_scale);
        setting->setProperty(BRUSH_SPACING, brush_spacing);
        setting->setProperty(BRUSH_DENSITY, brush_density);
        setting->setProperty(BRUSH_JITTER_MOVEMENT, brush_jitter_movement);
        setting->setProperty(BRUSH_JITTER_MOVEMENT_ENABLED, brush_jitter_movement_enabled);
    }
};


#endif
