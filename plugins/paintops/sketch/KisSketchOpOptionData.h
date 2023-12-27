/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SKETCHOP_OPTION_DATA_H
#define KIS_SKETCHOP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

struct KisSketchOpOptionData : boost::equality_comparable<KisSketchOpOptionData>
{
    inline friend bool operator==(const KisSketchOpOptionData &lhs, const KisSketchOpOptionData &rhs) {
        return qFuzzyCompare(lhs.offset, rhs.offset)
            && qFuzzyCompare(lhs.probability, rhs.probability)
            && lhs.simpleMode == rhs.simpleMode
            && lhs.makeConnection == rhs.makeConnection
            && lhs.magnetify == rhs.magnetify
            && lhs.randomRGB == rhs.randomRGB
            && lhs.randomOpacity == rhs.randomOpacity
            && lhs.distanceOpacity == rhs.distanceOpacity
            && lhs.distanceDensity == rhs.distanceDensity
            && lhs.antiAliasing == rhs.antiAliasing
            && lhs.lineWidth == rhs.lineWidth;
    }

    qreal offset {30.0}; // perc
    qreal probability {50.0}; // perc
    bool simpleMode {false};
    bool makeConnection {true};
    bool magnetify {true};
    bool randomRGB {false};
    bool randomOpacity {false};
    bool distanceOpacity {false};
    bool distanceDensity {true};
    bool antiAliasing {false};
    int lineWidth {1}; // px

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KIS_SKETCHOP_OPTION_DATA_H
