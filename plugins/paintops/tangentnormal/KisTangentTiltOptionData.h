/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_TANGENTTILT_OPTION_DATA_H
#define KIS_TANGENTTILT_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;

enum TangentTiltDirectionType {
    Tilt,
    Direction,
    Rotation,
    Mix
};

struct KisTangentTiltOptionData : boost::equality_comparable<KisTangentTiltOptionData>
{
    inline friend bool operator==(const KisTangentTiltOptionData &lhs, const KisTangentTiltOptionData &rhs) {
        return lhs.redChannel == rhs.redChannel
            && lhs.greenChannel == rhs.greenChannel
            && lhs.blueChannel == rhs.blueChannel
            && lhs.directionType == rhs.directionType
            && qFuzzyCompare(lhs.elevationSensitivity, rhs.elevationSensitivity)
            && qFuzzyCompare(lhs.mixValue, rhs.mixValue);
    }

    int redChannel {0};
    int greenChannel {2};
    int blueChannel {4};
    TangentTiltDirectionType directionType {TangentTiltDirectionType::Tilt};
    double elevationSensitivity {100.0};
    double mixValue {50.0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_TANGENTTILT_OPTION_DATA_H
