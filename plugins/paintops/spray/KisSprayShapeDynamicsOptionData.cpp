/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSprayShapeDynamicsOptionData.h"

#include "kis_properties_configuration.h"


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



bool KisSprayShapeDynamicsOptionData::read(const KisPropertiesConfiguration *settings)
{
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
		followCursorWeight = settings->getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT);
		followDrawingAngleWeight = settings->getDouble(SPRAYSHAPE_DRAWING_ANGLE_WEIGHT);
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
		followCursorWeight = settings->getDouble(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT);
		followDrawingAngleWeight = settings->getDouble(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT);
	}

    return true;
}

void KisSprayShapeDynamicsOptionData::write(KisPropertiesConfiguration *settings) const
{
	settings->setProperty(SHAPE_DYNAMICS_VERSION, "2.3");
    settings->setProperty(SHAPE_DYNAMICS_ENABLED, enabled);
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_SIZE, randomSize);
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ROTATION, fixedRotation);
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ANGEL, fixedAngle);
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION, randomRotation);
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT, randomRotationWeight);
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR, followCursor);
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT, followCursorWeight);
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE, followDrawingAngle);
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT, followDrawingAngleWeight);
	
}
