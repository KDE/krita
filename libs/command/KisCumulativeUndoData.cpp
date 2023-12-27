/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCumulativeUndoData.h"

#include <kconfiggroup.h>
#include <kis_debug.h>

const KisCumulativeUndoData KisCumulativeUndoData::defaultValue;

bool KisCumulativeUndoData::read(const KConfigGroup *config)
{
    excludeFromMerge = config->readEntry("cumulativeUndoExcludeFromMerge",
                                             defaultValue.excludeFromMerge);
    mergeTimeout = config->readEntry("cumulativeUndoMergeTimeout",
                                         defaultValue.mergeTimeout);
    maxGroupSeparation = config->readEntry("cumulativeUndoMaxGroupSeparation",
                                              defaultValue.maxGroupSeparation);
    maxGroupDuration = config->readEntry("cumulativeUndoMaxGroupDuration",
                                            defaultValue.maxGroupDuration);

    return true;
}

void KisCumulativeUndoData::write(KConfigGroup *config) const
{
    config->writeEntry("cumulativeUndoExcludeFromMerge", excludeFromMerge);
    config->writeEntry("cumulativeUndoMergeTimeout", mergeTimeout);
    config->writeEntry("cumulativeUndoMaxGroupSeparation", maxGroupSeparation);
    config->writeEntry("cumulativeUndoMaxGroupDuration", maxGroupDuration);
}

QDebug KRITACOMMAND_EXPORT operator<<(QDebug dbg, const KisCumulativeUndoData &data)
{
    dbg.nospace() << "KisCumulativeUndoData(";
    dbg.space() << ppVar(data.excludeFromMerge);
    dbg.space() << ppVar(data.mergeTimeout);
    dbg.space() << ppVar(data.maxGroupSeparation);
    dbg.space() << ppVar(data.maxGroupDuration);
    dbg.nospace() << ")";

    return dbg.nospace();
}
