/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisActionsSnapshotTest.h"

#include <QTest>
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
