/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisExperimentOpOptionData.h"

#include "kis_properties_configuration.h"


const QString EXPERIMENT_DISPLACEMENT_ENABLED = "Experiment/displacementEnabled";
const QString EXPERIMENT_DISPLACEMENT_VALUE = "Experiment/displacement";
const QString EXPERIMENT_SMOOTHING_ENABLED = "Experiment/smoothing";
const QString EXPERIMENT_SMOOTHING_VALUE = "Experiment/smoothingValue";
const QString EXPERIMENT_SPEED_ENABLED = "Experiment/speedEnabled";
const QString EXPERIMENT_SPEED_VALUE = "Experiment/speed";
const QString EXPERIMENT_WINDING_FILL = "Experiment/windingFill";
const QString EXPERIMENT_HARD_EDGE = "Experiment/hardEdge";
const QString EXPERIMENT_FILL_TYPE = "Experiment/fillType";



bool KisExperimentOpOptionData::read(const KisPropertiesConfiguration *setting)
{
	isDisplacementEnabled = setting->getBool(EXPERIMENT_DISPLACEMENT_ENABLED);
    displacement = setting->getDouble(EXPERIMENT_DISPLACEMENT_VALUE, 50.0);
    isSpeedEnabled = setting->getBool(EXPERIMENT_SPEED_ENABLED);
    speed = setting->getDouble(EXPERIMENT_SPEED_VALUE, 50.0);
    isSmoothingEnabled = setting->getBool(EXPERIMENT_SMOOTHING_ENABLED);
    smoothing = setting->getDouble(EXPERIMENT_SMOOTHING_VALUE, 20.0);
    windingFill = setting->getBool(EXPERIMENT_WINDING_FILL);
    hardEdge = setting->getBool(EXPERIMENT_HARD_EDGE);
    fillType = (ExperimentFillType)setting->getInt(EXPERIMENT_FILL_TYPE, 0); // default to solid color

    return true;
}

void KisExperimentOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(EXPERIMENT_DISPLACEMENT_ENABLED, isDisplacementEnabled);
	setting->setProperty(EXPERIMENT_DISPLACEMENT_VALUE, displacement);
	setting->setProperty(EXPERIMENT_SPEED_ENABLED, isSpeedEnabled);
	setting->setProperty(EXPERIMENT_SPEED_VALUE, speed);
	setting->setProperty(EXPERIMENT_SMOOTHING_ENABLED, isSmoothingEnabled);
	setting->setProperty(EXPERIMENT_SMOOTHING_VALUE, smoothing);
	setting->setProperty(EXPERIMENT_WINDING_FILL, windingFill);
	setting->setProperty(EXPERIMENT_HARD_EDGE, hardEdge);
	setting->setProperty(EXPERIMENT_FILL_TYPE, fillType);
}
