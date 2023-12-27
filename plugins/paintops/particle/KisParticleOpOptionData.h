/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PARTICLEOP_OPTION_DATA_H
#define KIS_PARTICLEOP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

struct KisParticleOpOptionData : boost::equality_comparable<KisParticleOpOptionData>
{
    inline friend bool operator==(const KisParticleOpOptionData &lhs, const KisParticleOpOptionData &rhs) {
        return lhs.particleCount == rhs.particleCount
            && lhs.particleIterations == rhs.particleIterations
            && qFuzzyCompare(lhs.particleGravity, rhs.particleGravity)
            && qFuzzyCompare(lhs.particleWeight, rhs.particleWeight)
            && qFuzzyCompare(lhs.particleScaleX, rhs.particleScaleX)
            && qFuzzyCompare(lhs.particleScaleY, rhs.particleScaleY);
    }

    int particleCount {50};
    int particleIterations {10};
    qreal particleGravity {0.989};
    qreal particleWeight {0.2};
    qreal particleScaleX {0.3};
    qreal particleScaleY {0.3};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KIS_PARTICLEOP_OPTION_DATA_H
