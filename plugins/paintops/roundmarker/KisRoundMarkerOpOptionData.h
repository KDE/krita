/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ROUNDMARKEROP_OPTION_DATA_H
#define KIS_ROUNDMARKEROP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>

class KisPropertiesConfiguration;

struct KisRoundMarkerOpOptionData : boost::equality_comparable<KisRoundMarkerOpOptionData>
{
    inline friend bool operator==(const KisRoundMarkerOpOptionData &lhs, const KisRoundMarkerOpOptionData &rhs) {
        return lhs.diameter == rhs.diameter
			&& lhs.spacing == rhs.spacing
            && lhs.use_auto_spacing == rhs.use_auto_spacing
            && lhs.auto_spacing_coeff == rhs.auto_spacing_coeff;
    }
    
    qreal diameter {30.0};
    qreal spacing {0.02};
    bool use_auto_spacing {false};
    qreal auto_spacing_coeff {1.0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KIS_ROUNDMARKEROP_OPTION_DATA_H
