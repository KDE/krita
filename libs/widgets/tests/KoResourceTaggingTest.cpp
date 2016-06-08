/*
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "KoResourceTaggingTest.h"

#include <QTest>
#include <QCoreApplication>

#include <kactioncollection.h>
#include <WidgetsDebug.h>

#include <resources/KoResource.h>
#include "KoResourceServerProvider.h"

void KoResourceTaggingTest::testInitialization()
{
    KoResourceTagStore tagStore(KoResourceServerProvider::instance()->patternServer());
    QVERIFY(tagStore.tagNamesList().isEmpty());
    QVERIFY(tagStore.assignedTagsList(0).isEmpty());
    QVERIFY(tagStore.searchTag("bla").isEmpty());
}

void KoResourceTaggingTest::testTagging()
{
    KoResourceServer<KoPattern>* patServer = KoResourceServerProvider::instance()->patternServer();
    KoResourceTagStore tagStore(patServer);
    KoResource *res = patServer->resources().first();
    QVERIFY(res);
    QVERIFY(patServer->resourceByFilename(res->shortFilename()) == res);

    tagStore.addTag(res, "test1");
    QVERIFY(tagStore.tagNamesList().size() == 1);
    QStringList resources = tagStore.searchTag("test1");
    QVERIFY(resources.size() == 1);
    KoResource *res2 = patServer->resourceByFilename(resources.first());
    QVERIFY(res == res2);

    tagStore.addTag(res, "test2");
    QVERIFY(tagStore.tagNamesList().size() == 2);
    resources = tagStore.searchTag("test1");
    QVERIFY(resources.size() == 1);
    res2 = patServer->resourceByFilename(resources.first());
    QVERIFY(res == res2);

    tagStore.addTag(res, "test2");
    QVERIFY(tagStore.tagNamesList().size() == 2);

    resources = tagStore.searchTag("test2");
    QVERIFY(resources.size() == 1);
    res2 = patServer->resourceByFilename(resources.first());
    QVERIFY(res == res2);

    resources = tagStore.searchTag("test1,test2");
    QVERIFY(resources.size() == 1);
    res2 = patServer->resourceByFilename(resources.first());
    QVERIFY(res == res2);

    tagStore.delTag(res, "test1");
    QVERIFY(tagStore.tagNamesList().size() == 2);
    resources = tagStore.searchTag("test1");
    QVERIFY(resources.size() == 0);

    resources = tagStore.searchTag("test2");
    QVERIFY(resources.size() == 1);
    res2 = patServer->resourceByFilename(resources.first());
    QVERIFY(res == res2);

    tagStore.delTag("test1");
    QVERIFY(tagStore.tagNamesList().size() == 1);

}

void KoResourceTaggingTest::testReadWriteXML()
{
    KoResourceServer<KoPattern>* patServer = KoResourceServerProvider::instance()->patternServer();
    KoResourceTagStore tagStore(patServer);

    QList<KoPattern*> patterns = patServer->resources();
    Q_ASSERT(patterns.size() > 5);
    tagStore.addTag(patterns[0], "test0");
    tagStore.addTag(patterns[1], "test1");
    tagStore.addTag(patterns[2], "test2");
    tagStore.addTag(patterns[2], "test2");
    tagStore.addTag(patterns[2], "test1");
    tagStore.addTag(patterns[3], "test3");
    tagStore.addTag(patterns[4], "test4");
    tagStore.addTag(patterns[5], "test5");
    tagStore.addTag(patterns[5], "test5.1");
    tagStore.addTag(patterns[5], "test5.2");
    tagStore.addTag(0, "dummytest");

    QVERIFY(tagStore.tagNamesList().size() == 9);

    tagStore.writeXMLFile(QString(FILES_OUTPUT_DIR) + "/" + "kis_pattern_tags.xml");

    KoResourceTagStore tagStore2(patServer);
    tagStore2.readXMLFile(QString(FILES_OUTPUT_DIR) + "/" + "kis_pattern_tags.xml");

    QVERIFY(tagStore2.tagNamesList().size() == 9);
    QStringList resources = tagStore2.searchTag("test0");
    QVERIFY(resources.size() == 1);
    QVERIFY(patServer->resourceByFilename(resources[0]) == patterns[0]);

    resources = tagStore2.searchTag("test1");
    QVERIFY(resources.size() == 2);

    resources = tagStore2.searchTag("test2");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("test3");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("test4");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("test5");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("test5.1");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("test5.2");
    QVERIFY(resources.size() == 1);

    resources = tagStore2.searchTag("dummytest");
    QVERIFY(resources.size() == 0);
}

QTEST_MAIN(KoResourceTaggingTest)
