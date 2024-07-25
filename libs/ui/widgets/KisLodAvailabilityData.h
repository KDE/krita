/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLODAVAILABILITYDATA_H
#define KISLODAVAILABILITYDATA_H

#include <QtGlobal>
#include <boost/operators.hpp>
#include <kritaui_export.h>

class KisPropertiesConfiguration;

class KRITAUI_EXPORT KisLodAvailabilityData : boost::equality_comparable<KisLodAvailabilityData>
{
public:
    inline friend bool operator==(const KisLodAvailabilityData &lhs, const KisLodAvailabilityData &rhs) {
        return lhs.isLodUserAllowed == rhs.isLodUserAllowed &&
            lhs.isLodSizeThresholdSupported == rhs.isLodSizeThresholdSupported &&
            qFuzzyCompare(lhs.lodSizeThreshold, rhs.lodSizeThreshold);
    }

    bool isLodUserAllowed {true};
    bool isLodSizeThresholdSupported {true};
    qreal lodSizeThreshold {100.0};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KISLODAVAILABILITYDATA_H
