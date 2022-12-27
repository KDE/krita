/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBrushSizeOptionData.h"

#include "kis_properties_configuration.h"


const QString BRUSH_SHAPE = "Brush/shape";
const QString BRUSH_DIAMETER = "Brush/diameter";
const QString BRUSH_ASPECT = "Brush/aspect";
const QString BRUSH_SCALE = "Brush/scale";
const QString BRUSH_ROTATION = "Brush/rotation";
const QString BRUSH_SPACING = "Brush/spacing";
const QString BRUSH_DENSITY = "Brush/density";
const QString BRUSH_JITTER_MOVEMENT = "Brush/jitterMovement";
const QString BRUSH_JITTER_MOVEMENT_ENABLED = "Brush/jitterMovementEnabled";


bool KisBrushSizeOptionData::read(const KisPropertiesConfiguration *setting)
{
    brushDiameter = setting->getDouble(BRUSH_DIAMETER, 20.0);
    brushAspect = setting->getDouble(BRUSH_ASPECT, 1.0);
    brushRotation = setting->getDouble(BRUSH_ROTATION, 0.0);
    brushScale = setting->getDouble(BRUSH_SCALE, 1.0);
    brushSpacing = setting->getDouble(BRUSH_SPACING, 0.3);
    brushDensity = setting->getDouble(BRUSH_DENSITY, 1.0);
    brushJitterMovement = setting->getDouble(BRUSH_JITTER_MOVEMENT, 0.0);
    brushJitterMovementEnabled = setting->getBool(BRUSH_JITTER_MOVEMENT_ENABLED, false);

    return true;
}

void KisBrushSizeOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(BRUSH_DIAMETER, brushDiameter);
    setting->setProperty(BRUSH_ASPECT, brushAspect);
    setting->setProperty(BRUSH_ROTATION, brushRotation);
    setting->setProperty(BRUSH_SCALE, brushScale);
    setting->setProperty(BRUSH_SPACING, brushSpacing);
    setting->setProperty(BRUSH_DENSITY, brushDensity);
    setting->setProperty(BRUSH_JITTER_MOVEMENT, brushJitterMovement);
    setting->setProperty(BRUSH_JITTER_MOVEMENT_ENABLED, brushJitterMovementEnabled);
}
