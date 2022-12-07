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

enum ExperimentFillType {
    SolidColor,
    Pattern
};

struct KisExperimentOpOptionData : boost::equality_comparable<KisExperimentOpOptionData>
{
    inline friend bool operator==(const KisExperimentOpOptionData &lhs, const KisExperimentOpOptionData &rhs) {
        return lhs.isDisplacementEnabled == rhs.isDisplacementEnabled
			&& qFuzzyCompare(lhs.displacement, rhs.displacement)
			&& lhs.isSpeedEnabled == rhs.isSpeedEnabled
			&& qFuzzyCompare(lhs.speed, rhs.speed)
			&& lhs.isSmoothingEnabled == rhs.isSmoothingEnabled
			&& qFuzzyCompare(lhs.smoothing, rhs.smoothing)
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
