/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestTag.h"
#include <simpletest.h>
#include <QBuffer>

#include <KisTag.h>
#include <KoResource.h>

#include <KLocalizedString>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

void TestTag::testLoadTag()
{
    KisTag tagLoader;
    QFile f(QString(FILES_DATA_DIR) + "paintoppresets/test.tag");

    QVERIFY(f.exists());

    f.open(QFile::ReadOnly);
    QVERIFY(f.isOpen());

    bool r = tagLoader.load(f);

    f.close();

    QVERIFY(r);
    QVERIFY(tagLoader.name() == "* Favorites");
    QVERIFY(tagLoader.comment() == "Your favorite brush presets");
    QVERIFY(tagLoader.url() == "* Favorites");

    KLocalizedString::setLanguages(QStringList() << QStringLiteral("nl"));

    f.open(QFile::ReadOnly);
    QVERIFY(f.isOpen());
    r = tagLoader.load(f);
    f.close();

    QVERIFY(r);
    QVERIFY(tagLoader.name() == "* Favorieten");
    QVERIFY(tagLoader.comment() == "Uw favorite voorinstellingen van penselen");
    QVERIFY(tagLoader.url() == "* Favorites");

}

void TestTag::testSaveTag()
{
    KisTag tag1;
    QFile f(QString(FILES_DATA_DIR) + "paintoppresets/test.tag");

    QVERIFY(f.exists());

    f.open(QFile::ReadOnly);
    QVERIFY(f.isOpen());

    bool r = tag1.load(f);
    QVERIFY(r);

    tag1.setName(QString("Test"));

    QBuffer buf;
    buf.open(QBuffer::WriteOnly);

    tag1.save(buf);

    buf.close();
    buf.open(QBuffer::ReadOnly);
    KisTag tag2;
    tag2.load(buf);
    QVERIFY(tag2.url() == tag1.url());
    QVERIFY(tag2.name() == tag1.name());
    QVERIFY(tag2.resourceType() == tag1.resourceType());
    QVERIFY(tag2.comment() == tag1.comment());
    QVERIFY(tag2.defaultResources() == tag1.defaultResources());

}

SIMPLE_TEST_MAIN(TestTag)

