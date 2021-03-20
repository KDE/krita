/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisActionsSnapshotTest.h"

#include <simpletest.h>
#include <KisActionsSnapshot.h>

void KisActionsSnapshotTest::testCreation()
{
    {
        KisActionsSnapshot *snapshot = new KisActionsSnapshot();
        delete snapshot;
    }
    
    {
        KisActionsSnapshot snapshot;
        Q_UNUSED(snapshot);
    }
    
    {
        QScopedPointer<KisActionsSnapshot> snapshot;
        Q_UNUSED(snapshot);
    }
    
}


QTEST_GUILESS_MAIN(KisActionsSnapshotTest)
