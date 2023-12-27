/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_SHAPE_DYNAMICS_OPTION_DATA_H
#define KIS_SPRAY_SHAPE_DYNAMICS_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;

struct KisSprayShapeDynamicsOptionData : boost::equality_comparable<KisSprayShapeDynamicsOptionData>
{
    inline friend bool operator==(const KisSprayShapeDynamicsOptionData &lhs, const KisSprayShapeDynamicsOptionData &rhs) {
        return lhs.enabled == rhs.enabled
			&& lhs.randomSize == rhs.randomSize
			&& lhs.fixedRotation == rhs.fixedRotation
			&& lhs.randomRotation == rhs.randomRotation
			&& lhs.followCursor == rhs.followCursor
			&& lhs.followDrawingAngle == rhs.followDrawingAngle
			&& lhs.fixedAngle == rhs.fixedAngle
			&& lhs.randomRotationWeight == rhs.randomRotationWeight
			&& lhs.followCursorWeight == rhs.followCursorWeight
			&& lhs.followDrawingAngleWeight == rhs.followDrawingAngleWeight;
    }
    
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
    qreal followCursorWeight;
    qreal followDrawingAngleWeight;

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_SPRAY_SHAPE_DYNAMICS_OPTION_DATA_H
