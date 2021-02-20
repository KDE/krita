/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPRAY_SHAPE_DYNAMICS_OPTION_H
#define KIS_SPRAY_SHAPE_DYNAMICS_OPTION_H

#include <kis_paintop_option.h>

const QString SHAPE_DYNAMICS_VERSION = "ShapeDynamicsVersion";

// Old Krita 2.2.x strings for backward compatibility
const QString SPRAYSHAPE_RANDOM_SIZE = "SprayShape/randomSize";
const QString SPRAYSHAPE_FIXED_ROTATION = "SprayShape/fixedRotation";
const QString SPRAYSHAPE_FIXED_ANGEL = "SprayShape/fixedAngle";
const QString SPRAYSHAPE_RANDOM_ROTATION = "SprayShape/randomRotation";
const QString SPRAYSHAPE_RANDOM_ROTATION_WEIGHT = "SprayShape/randomRotationWeight";
const QString SPRAYSHAPE_FOLLOW_CURSOR = "SprayShape/followCursor";
const QString SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT = "SprayShape/followCursorWeigth";
const QString SPRAYSHAPE_DRAWING_ANGLE = "SprayShape/followDrawingAngle";
const QString SPRAYSHAPE_DRAWING_ANGLE_WEIGHT = "SprayShape/followDrawingAngleWeigth";

// My intention is to have the option dialog more general so that it can be share
// hence the suffix ShapeDynamics
const QString SHAPE_DYNAMICS_ENABLED = "ShapeDynamics/enabled";
const QString SHAPE_DYNAMICS_RANDOM_SIZE = "ShapeDynamics/randomSize";
const QString SHAPE_DYNAMICS_FIXED_ROTATION = "ShapeDynamics/fixedRotation";
const QString SHAPE_DYNAMICS_FIXED_ANGEL = "ShapeDynamics/fixedAngle";
const QString SHAPE_DYNAMICS_RANDOM_ROTATION = "ShapeDynamics/randomRotation";
const QString SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT = "ShapeDynamics/randomRotationWeight";
const QString SHAPE_DYNAMICS_FOLLOW_CURSOR = "ShapeDynamics/followCursor";
const QString SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT = "ShapeDynamics/followCursorWeigth";
const QString SHAPE_DYNAMICS_DRAWING_ANGLE = "ShapeDynamics/followDrawingAngle";
const QString SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT = "ShapeDynamics/followDrawingAngleWeigth";

class KisShapeDynamicsOptionsWidget;

class KisSprayShapeDynamicsOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisSprayShapeDynamicsOption();
    ~KisSprayShapeDynamicsOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    void setupBrushPreviewSignals();

private:
    KisShapeDynamicsOptionsWidget * m_options;
};


class KisShapeDynamicsProperties
{
public:
    bool enabled;
    // particle size dynamics
    bool randomSize;
    // rotation dynamics
    bool fixedRotation;
    bool randomRotation;
    bool followCursor;
    bool followDrawingAngle;
    quint16 fixedAngle;
    qreal randomRotationWeight;
    qreal followCursorWeigth;
    qreal followDrawingAngleWeight;

public:

    void loadSettings(const KisPropertiesConfigurationSP settings) {
        // Krita 2.2
        if (settings->getString(SHAPE_DYNAMICS_VERSION, "2.2") == "2.2") {
            randomSize = settings->getBool(SPRAYSHAPE_RANDOM_SIZE);
            // rotation
            fixedRotation = settings->getBool(SPRAYSHAPE_FIXED_ROTATION);
            randomRotation = settings->getBool(SPRAYSHAPE_RANDOM_ROTATION);
            followCursor = settings->getBool(SPRAYSHAPE_FOLLOW_CURSOR);
            followDrawingAngle = settings->getBool(SPRAYSHAPE_DRAWING_ANGLE);
            fixedAngle = settings->getInt(SPRAYSHAPE_FIXED_ANGEL);
            randomRotationWeight = settings->getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT);
            followCursorWeigth = settings->getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT);
            followDrawingAngleWeight = settings->getDouble(SPRAYSHAPE_DRAWING_ANGLE_WEIGHT);
            enabled = true;
        }
        // Krita latest
        else {
            enabled = settings->getBool(SHAPE_DYNAMICS_ENABLED);
            // particle type size
            randomSize = settings->getBool(SHAPE_DYNAMICS_RANDOM_SIZE);
            // rotation dynamics
            fixedRotation = settings->getBool(SHAPE_DYNAMICS_FIXED_ROTATION);
            randomRotation = settings->getBool(SHAPE_DYNAMICS_RANDOM_ROTATION);
            followCursor = settings->getBool(SHAPE_DYNAMICS_FOLLOW_CURSOR);
            followDrawingAngle = settings->getBool(SHAPE_DYNAMICS_DRAWING_ANGLE);
            fixedAngle = settings->getInt(SHAPE_DYNAMICS_FIXED_ANGEL);
            randomRotationWeight = settings->getDouble(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT);
            followCursorWeigth = settings->getDouble(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT);
            followDrawingAngleWeight = settings->getDouble(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT);
        }
    }
};

#endif // KIS_SPRAY_SHAPE_DYNAMICS_OPTION_H

