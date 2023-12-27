/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_OPTIONS_DATA_H
#define KIS_HATCHING_OPTIONS_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

enum CrosshatchingType {
    NoCrosshatching,
    Perpendicular,
    MinusThenPlus,
    PlusThenMinus,
    MoirePattern
};

struct KisHatchingOptionsData : boost::equality_comparable<KisHatchingOptionsData>
{
    inline friend bool operator==(const KisHatchingOptionsData &lhs, const KisHatchingOptionsData &rhs) {
        return qFuzzyCompare(lhs.angle, rhs.angle)
            && qFuzzyCompare(lhs.separation, rhs.separation)
            && qFuzzyCompare(lhs.thickness, rhs.thickness)
            && qFuzzyCompare(lhs.originX, rhs.originX)
            && qFuzzyCompare(lhs.originY, rhs.originY)
            && lhs.crosshatchingStyle == rhs.crosshatchingStyle
            && lhs.separationIntervals == rhs.separationIntervals;
    }

    qreal angle {-60.0};
    qreal separation {6.0};
    qreal thickness {1.0};
    qreal originX {50.0};
    qreal originY {50.0};
    CrosshatchingType crosshatchingStyle {CrosshatchingType::NoCrosshatching};
    int separationIntervals {2};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KIS_HATCHING_OPTIONS_DATA_H
