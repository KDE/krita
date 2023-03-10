/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISAIRBRUSHOPTIONDATA_H
#define KISAIRBRUSHOPTIONDATA_H

#include <QtGlobal>
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;


struct PAINTOP_EXPORT KisAirbrushOptionData : boost::equality_comparable<KisAirbrushOptionData>
{
    inline friend bool operator==(const KisAirbrushOptionData &lhs, const KisAirbrushOptionData &rhs) {
        return lhs.isChecked == rhs.isChecked &&
                qFuzzyCompare(lhs.airbrushRate, rhs.airbrushRate) &&
                lhs.ignoreSpacing == rhs.ignoreSpacing;
    }

    bool isChecked {false};
    qreal airbrushRate {50.0};
    bool ignoreSpacing {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
};

#endif // KISAIRBRUSHOPTIONDATA_H
