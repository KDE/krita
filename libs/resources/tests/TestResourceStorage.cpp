/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestResourceStorage.h"
#include <QTest>

#include <KisResourceStorage.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif


void TestResourceStorage ::testStorage()
{
    {
        KisResourceStorage storage(QString(FILES_DATA_DIR));
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Folder);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage(QString(FILES_DATA_DIR) + "/bundles/test1.bundle");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Bundle);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage(QString(FILES_DATA_DIR) + "/bundles/test2.bundle");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Bundle);
        QVERIFY(storage.valid());
    }

    {
        KisResourceStorage storage("");
        QVERIFY(storage.type() == KisResourceStorage::StorageType::Unknown);
        QVERIFY(!storage.valid());
    }


}

QTEST_MAIN(TestResourceStorage)

