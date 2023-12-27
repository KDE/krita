/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFILTEROPTIONDATA_H
#define KISFILTEROPTIONDATA_H

#include <QtGlobal>
#include <boost/operators.hpp>
#include <kritapaintop_export.h>
#include <kis_filter_registry.h>

class KisPropertiesConfiguration;


struct PAINTOP_EXPORT KisFilterOptionData : boost::equality_comparable<KisFilterOptionData>
{
    inline friend bool operator==(const KisFilterOptionData &lhs, const KisFilterOptionData &rhs) {
        return lhs.filterId == rhs.filterId &&
                lhs.filterConfig == rhs.filterConfig &&
                lhs.smudgeMode == rhs.smudgeMode;
    }

    QString filterId;
    QString filterConfig;
    bool smudgeMode {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    static QString filterIdTag();
    static QString filterConfigTag();
};

#endif // KISFILTEROPTIONDATA_H
