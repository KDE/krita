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

#include "TestResourceLoaderRegistry.h"
#include <QTest>
#include <QtSql>
#include <QStandardPaths>
#include <QDir>

#include <KisResourceLoaderRegistry.h>
#include <KisResourceLoader.h>
#include <KoResource.h>

#include "DummyResource.h"

void TestResourceLoaderRegistry::testRegistry()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();

    KisResourceLoader<DummyResource> *loader = new KisResourceLoader<DummyResource>("dummy", "dummy", i18n("Dummy"), QStringList() << "x-dummy");
    reg->add(loader);
    QVERIFY(reg->count() == 1);

    KisResourceLoaderBase *l2 = reg->get("dummy");
    QByteArray ba;
    QBuffer f(&ba);
    f.open(QFile::ReadOnly);
    KoResourceSP res = l2->load("test", f);
    QVERIFY(res.data());
    QVERIFY(dynamic_cast<DummyResource*>(res.data()));
}

QTEST_MAIN(TestResourceLoaderRegistry)

