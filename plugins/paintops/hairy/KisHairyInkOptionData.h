/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYINK_OPTION_DATA_H
#define KIS_HAIRYINK_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

#include "kis_cubic_curve.h"

class KisPropertiesConfiguration;

struct KisHairyInkOptionData : boost::equality_comparable<KisHairyInkOptionData>
{
    inline friend bool operator==(const KisHairyInkOptionData &lhs, const KisHairyInkOptionData &rhs) {
        return lhs.inkDepletionEnabled == rhs.inkDepletionEnabled
            && lhs.inkAmount == rhs.inkAmount
            && lhs.inkDepletionCurve == rhs.inkDepletionCurve
            && lhs.useSaturation == rhs.useSaturation
            && lhs.useOpacity == rhs.useOpacity
            && lhs.useWeights == rhs.useWeights
            && lhs.pressureWeight == rhs.pressureWeight
            && lhs.bristleLengthWeight == rhs.bristleLengthWeight
            && lhs.bristleInkAmountWeight == rhs.bristleInkAmountWeight
            && lhs.inkDepletionWeight == rhs.inkDepletionWeight
            && lhs.useSoakInk == rhs.useSoakInk;
    }

    bool inkDepletionEnabled {false};

    int inkAmount {1024};
    QString inkDepletionCurve {DEFAULT_CURVE_STRING};

    bool useSaturation {false};
    bool useOpacity {true};
    bool useWeights {false};

    int pressureWeight {50};
    int bristleLengthWeight {50};
    int bristleInkAmountWeight {50};
    int inkDepletionWeight {50};

    bool useSoakInk {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_HAIRYINK_OPTION_DATA_H
