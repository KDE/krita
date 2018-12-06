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

#include "TestFolderStorage.h"
#include <QTest>

#include <KisFolderStorage.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>

#include "DummyResource.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

void TestFolderStorage ::testStorage()
{
    KisFolderStorage folderStorage(QString(FILES_DATA_DIR));

    KisResourceLoaderRegistry::instance()->add("brushes", new KisResourceLoader<DummyResource>("dummy", "brushes", i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    QSharedPointer<KisResourceStorage::ResourceIterator> iter = folderStorage.resources("brushes");
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->url() << iter->type() << iter->lastModified();
        count++;
    }
    QVERIFY(count == 1);
}

void TestFolderStorage::testTagIterator()
{
    KisFolderStorage folderStorage(QString(FILES_DATA_DIR));
    QSharedPointer<KisResourceStorage::TagIterator> iter = folderStorage.tags("paintoppresets");
    QVERIFY(iter->hasNext());
    int count = 0;
    while (iter->hasNext()) {
        iter->next();
        qDebug() << iter->url() << iter->name() << iter->tag();
        count++;
    }
    QVERIFY(count == 1);
}

QTEST_MAIN(TestFolderStorage)

