/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORSOURCEOPTIONDATA_H
#define KISCOLORSOURCEOPTIONDATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;

struct PAINTOP_EXPORT KisColorSourceOptionData : boost::equality_comparable<KisColorSourceOptionData>
{
    enum Type {
        PLAIN,
        GRADIENT,
        UNIFORM_RANDOM,
        TOTAL_RANDOM,
        PATTERN,
        PATTERN_LOCKED
    };

    inline friend bool operator==(const KisColorSourceOptionData &lhs, const KisColorSourceOptionData &rhs) {
        return lhs.type == rhs.type;
    }

    Type type {PLAIN};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    static QVector<KoID> colorSourceTypeIds();
    static KoID type2Id(Type type);
    static Type id2Type(const KoID &id);
};

#endif // KISCOLORSOURCEOPTIONDATA_H
