/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestResourceLoaderRegistry.h"
#include <simpletest.h>
#include <QtSql>
#include <QStandardPaths>
#include <QDir>

#include <KisResourceLoaderRegistry.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisGlobalResourcesInterface.h>

#include "DummyResource.h"

void TestResourceLoaderRegistry::testRegistry()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();

    KisResourceLoader<DummyResource> *loader = new KisResourceLoader<DummyResource>("dummy", "dummy", i18n("Dummy"), QStringList() << "x-dummy");
    reg->add(loader);
    QVERIFY(reg->count() == 1);

    KisResourceLoaderBase *l2 = reg->get("dummy");
    QBuffer f;
    f.open(QFile::ReadOnly);
    KoResourceSP res = l2->load("test", f, KisGlobalResourcesInterface::instance());
    QVERIFY(res.data());
    QVERIFY(dynamic_cast<DummyResource*>(res.data()));
}

SIMPLE_TEST_MAIN(TestResourceLoaderRegistry)

