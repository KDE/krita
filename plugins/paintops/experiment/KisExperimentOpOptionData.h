/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_EXPERIMENTOP_OPTION_DATA_H
#define KIS_EXPERIMENTOP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;

const QString EXPERIMENT_DISPLACEMENT_ENABLED = "Experiment/displacementEnabled";
const QString EXPERIMENT_DISPLACEMENT_VALUE = "Experiment/displacement";
const QString EXPERIMENT_SMOOTHING_ENABLED = "Experiment/smoothing";
const QString EXPERIMENT_SMOOTHING_VALUE = "Experiment/smoothingValue";
const QString EXPERIMENT_SPEED_ENABLED = "Experiment/speedEnabled";
const QString EXPERIMENT_SPEED_VALUE = "Experiment/speed";
const QString EXPERIMENT_WINDING_FILL = "Experiment/windingFill";
const QString EXPERIMENT_HARD_EDGE = "Experiment/hardEdge";
const QString EXPERIMENT_FILL_TYPE = "Experiment/fillType";

enum ExperimentFillType {
    SolidColor,
    Pattern
};

struct PAINTOP_EXPORT KisExperimentOpOptionData : boost::equality_comparable<KisExperimentOpOptionData>
{
    inline friend bool operator==(const KisExperimentOpOptionData &lhs, const KisExperimentOpOptionData &rhs) {
        return lhs.isDisplacementEnabled == rhs.isDisplacementEnabled
			&& lhs.displacement == rhs.displacement
			&& lhs.isSpeedEnabled == rhs.isSpeedEnabled
			&& lhs.speed == rhs.speed
			&& lhs.isSmoothingEnabled == rhs.isSmoothingEnabled
			&& lhs.smoothing == rhs.smoothing
			&& lhs.windingFill == rhs.windingFill
			&& lhs.hardEdge == rhs.hardEdge
			&& lhs.fillType == rhs.fillType;
    }


	bool isDisplacementEnabled {false};
    qreal displacement {50.0};
    
    bool isSpeedEnabled {false};
    qreal speed {50.0};
    
    bool isSmoothingEnabled {true};
    qreal smoothing {20.0};
    
    bool windingFill {true};
    bool hardEdge {false};
    ExperimentFillType fillType;

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_EXPERIMENTOP_OPTION_DATA_H
