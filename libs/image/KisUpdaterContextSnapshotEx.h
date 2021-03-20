/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISUPDATERCONTEXTSNAPSHOTEX_H
#define KISUPDATERCONTEXTSNAPSHOTEX_H

enum KisUpdaterContextSnapshotExTag {
    ContextEmpty = 0x00,
    HasSequentialJob = 0x01,
    HasUniquelyConcurrentJob = 0x02,
    HasConcurrentJob = 0x04,
    HasBarrierJob = 0x08,
    HasMergeJob = 0x10
};

Q_DECLARE_FLAGS(KisUpdaterContextSnapshotEx, KisUpdaterContextSnapshotExTag);
Q_DECLARE_OPERATORS_FOR_FLAGS(KisUpdaterContextSnapshotEx);

#endif // KISUPDATERCONTEXTSNAPSHOTEX_H
