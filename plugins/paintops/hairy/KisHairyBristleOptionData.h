/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYBRISTLE_OPTION_DATA_H
#define KIS_HAIRYBRISTLE_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

struct KisHairyBristleOptionData : boost::equality_comparable<KisHairyBristleOptionData>
{
    inline friend bool operator==(const KisHairyBristleOptionData &lhs, const KisHairyBristleOptionData &rhs) {
        return lhs.useMousePressure == rhs.useMousePressure
            && qFuzzyCompare(lhs.scaleFactor, rhs.scaleFactor)
            && qFuzzyCompare(lhs.randomFactor, rhs.randomFactor)
            && qFuzzyCompare(lhs.shearFactor, rhs.shearFactor)
            && qFuzzyCompare(lhs.densityFactor, rhs.densityFactor)
            && lhs.threshold == rhs.threshold
            && lhs.antialias == rhs.antialias
            && lhs.useCompositing == rhs.useCompositing
            && lhs.connectedPath == rhs.connectedPath;
    }

    bool useMousePressure {false};
    double scaleFactor {2.0};
    double randomFactor {2.0};
    double shearFactor {0.0};
    double densityFactor {100.0};
    bool threshold {false};
    bool antialias {false};
    bool useCompositing {false};
    bool connectedPath {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KIS_HAIRYBRISTLE_OPTION_DATA_H
