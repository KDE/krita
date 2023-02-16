/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFORM_OPTION_DATA_H
#define KIS_DEFORM_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

enum DeformModes {
    GROW = 1,
    SHRINK,
    SWIRL_CW,
    SWIRL_CCW,
    MOVE,
    LENS_IN,
    LENS_OUT,
    DEFORM_COLOR
};

struct KisDeformOptionData : boost::equality_comparable<KisDeformOptionData>
{
    inline friend bool operator==(const KisDeformOptionData &lhs, const KisDeformOptionData &rhs) {
        return qFuzzyCompare(lhs.deformAmount, rhs.deformAmount)
            && lhs.deformUseBilinear == rhs.deformUseBilinear
            && lhs.deformUseCounter == rhs.deformUseCounter
            && lhs.deformUseOldData == rhs.deformUseOldData
            && lhs.deformAction == rhs.deformAction;
    }

    qreal deformAmount {0.2};
    bool deformUseBilinear {false};
    bool deformUseCounter {false};
    bool deformUseOldData {false};
    DeformModes deformAction {DeformModes::GROW};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KIS_DEFORM_OPTION_DATA_H
