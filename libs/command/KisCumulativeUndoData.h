/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCUMULATIVEUNDODATA_H
#define KISCUMULATIVEUNDODATA_H

#include <boost/operators.hpp>
#include "kritacommand_export.h"

class KConfigGroup;
class QDebug;

struct KRITACOMMAND_EXPORT KisCumulativeUndoData : boost::equality_comparable<KisCumulativeUndoData>
{
    inline friend bool operator==(const KisCumulativeUndoData &lhs, const KisCumulativeUndoData &rhs) {
        return lhs.excludeFromMerge == rhs.excludeFromMerge &&
            lhs.mergeTimeout == rhs.mergeTimeout &&
            lhs.maxGroupSeparation == rhs.maxGroupSeparation &&
            lhs.maxGroupDuration == rhs.maxGroupDuration;
    }

    int excludeFromMerge {10};
    int mergeTimeout {5000};
    int maxGroupSeparation {1000};
    int maxGroupDuration {5000};

    bool read(const KConfigGroup *config);
    void write(KConfigGroup *config) const;

    static const KisCumulativeUndoData defaultValue;
};

QDebug KRITACOMMAND_EXPORT operator<<(QDebug dbg, const KisCumulativeUndoData &data);

#endif // KISCUMULATIVEUNDODATA_H
