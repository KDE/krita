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

#include "TestTag.h"
#include <QTest>
#include <QBuffer>

#include <KisTag.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>

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

    QLocale nl(QLocale::Dutch, QLocale::Netherlands);
    QLocale::setDefault(nl);

    f.open(QFile::ReadOnly);
    QVERIFY(f.isOpen());
    r = tagLoader.load(f);
    f.close();

    QVERIFY(r);
    QVERIFY(tagLoader.name() == "* Favorieten");
    QVERIFY(tagLoader.comment() == "Jouw favoriete penseel presets");
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

    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QBuffer::WriteOnly);

    tag1.save(buf);

    buf.close();
    buf.open(QBuffer::ReadOnly);
    KisTag tag2;
    tag2.load(buf);
    QVERIFY(tag2.url() == tag1.url());
    QVERIFY(tag2.name() == tag1.name());
    QVERIFY(tag2.comment() == tag1.comment());
    QVERIFY(tag2.defaultResources() == tag1.defaultResources());

}

QTEST_MAIN(TestTag)

