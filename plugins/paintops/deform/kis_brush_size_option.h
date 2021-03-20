/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
