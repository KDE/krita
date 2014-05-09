/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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


#include "KoXmlResourceBundleMetaTest.h"
#include <QCoreApplication>
#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include "KoXmlResourceBundleMeta.h"

#include <QTest>

void KoXmlResourceBundleMetaTest::ctorTest()
{
    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta();
    QCOMPARE(meta->toString().toUtf8().data(), "<!DOCTYPE meta>\n<package/>\n");

    meta = new KoXmlResourceBundleMeta(QString("metaFileName"));
    QCOMPARE(meta->toString().toUtf8().data(), "<!DOCTYPE metaFileName>\n<package/>\n");

    meta = new KoXmlResourceBundleMeta(QString(""));
    QCOMPARE(meta->toString().toUtf8().data(), "<package/>\n");

    //Ca ne devrait pas arriver
    meta = new KoXmlResourceBundleMeta(QString(" "));
    QCOMPARE(meta->toString().toUtf8().data(), "<!DOCTYPE  >\n<package/>\n");

//    meta = new KoXmlResourceBundleMeta(QString("packName"),QString("lgpl"),QString("meta"));
//    QCOMPARE(meta->toString().toUtf8().data(),"<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <license>lgpl</license>\n</package>\n");

////    meta = new KoXmlResourceBundleMeta(QString(""),QString(""),QString("meta"));
////    QCOMPARE(meta->toString().toUtf8().data(),"<!DOCTYPE meta>\n<package/>\n");

//    meta = new KoXmlResourceBundleMeta(QString(""),QString("lgpl"),QString("meta"));
//    QCOMPARE(meta->toString().toUtf8().data(),"<!DOCTYPE meta>\n<package>\n <license>lgpl</license>\n</package>\n");

//    meta = new KoXmlResourceBundleMeta(QString("packName"),QString(""),QString("meta"));
//    QCOMPARE(meta->toString().toUtf8().data(),"<!DOCTYPE meta>\n<package>\n <name>packName</name>\n</package>\n");



}

void KoXmlResourceBundleMetaTest::getTagEnumValueTest()
{

    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("name")) == 0);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("license")) == 3);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("author")) == 1);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("created")) == 2);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("updated")) == 4);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("description")) == 5);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("tag")) == 6);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("autreTag")) == 7);
    QVERIFY(KoXmlResourceBundleMeta::getTagEnumValue(QString("")) == 7);
}

void KoXmlResourceBundleMetaTest::checkSortTest()
{
    QByteArray array("<!DOCTYPE meta>\n<package>\n <created>01/01/01</created>\n <tag>tag content2</tag>\n <Other>other content2</Other>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <Other>other content1</Other>\n <author>Assyl</author>\n <tag>tag content 1</tag>\n <description>description content</description>\n <name>packName</name>\n</package>\n");
    QString chaineRes = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <created>01/01/01</created>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <description>description content</description>\n <tag>tag content2</tag>\n <tag>tag content 1</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n</package>\n";
    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    QCOMPARE(meta->toString(), chaineRes);

    array = QByteArray("<!DOCTYPE meta>\n<package>\n <created>01/01/01</created>\n <tag>tag content2</tag>\n <Other>other content2</Other>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <Other>other content1</Other>\n <tag>tag content 1</tag>\n <description>description content</description>\n <name>packName</name>\n</package>\n");
    meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    chaineRes = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <created>01/01/01</created>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <description>description content</description>\n <tag>tag content2</tag>\n <tag>tag content 1</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n</package>\n";
    QCOMPARE(meta->toString(), chaineRes);

    array = QByteArray("<!DOCTYPE meta>\n<package>\n <tag>tag content2</tag>\n <Other>other content2</Other>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <Other>other content1</Other>\n <tag>tag content 1</tag>\n <description>description content</description>\n <name>packName</name>\n</package>\n");
    meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    chaineRes = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <description>description content</description>\n <tag>tag content2</tag>\n <tag>tag content 1</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n</package>\n";
    QCOMPARE(meta->toString(), chaineRes);

    array = QByteArray("<!DOCTYPE meta>\n<package>\n <tag>tag content2</tag>\n <Other>other content2</Other>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <Other>other content1</Other>\n <tag>tag content 1</tag>\n <description>description content</description>\n</package>\n");
    meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    chaineRes = "<!DOCTYPE meta>\n<package>\n <license>lgpl</license>\n <updated>01/02/01</updated>\n <description>description content</description>\n <tag>tag content2</tag>\n <tag>tag content 1</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n</package>\n";
    QCOMPARE(meta->toString(), chaineRes);

    array = QByteArray("<!DOCTYPE meta>\n<package>\n <tag>tag content2</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n <tag>tag content 1</tag>\n</package>\n");
    meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    chaineRes = "<!DOCTYPE meta>\n<package>\n <tag>tag content2</tag>\n <tag>tag content 1</tag>\n <Other>other content2</Other>\n <Other>other content1</Other>\n</package>\n";
    QCOMPARE(meta->toString(), chaineRes);

    array = QByteArray("<!DOCTYPE meta>\n<package/>\n");
    meta = new KoXmlResourceBundleMeta(array);
    meta->checkSort();
    QCOMPARE(meta->toByteArray().data(), "<!DOCTYPE meta>\n<package/>\n");
}

void KoXmlResourceBundleMetaTest::addTagTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");
    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta();
    QString expectedStr;

    expectedStr = "<!DOCTYPE meta>\n<package>\n <tag>tag content 1</tag>\n</package>\n";
    meta->addTag("tag", "tag content 1");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <author>Assyl</author>\n <tag>tag content 1</tag>\n</package>\n";
    meta->addTag("author", "Assyl");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <author>Assyl</author>\n <created>01/01/14</created>\n <tag>tag content 1</tag>\n</package>\n";
    meta->addTag("created", "01/01/14");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <author>Assyl</author>\n <created>01/01/14</created>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("othertag", "othertag content");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("name", "packName");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <updated>01/02/14</updated>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("updated", "01/02/14");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName2</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <updated>01/02/14</updated>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("name", "packName2");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName2</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <updated>01/02/14</updated>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("description", "");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName2</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <updated>01/02/14</updated>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("tag", " ");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName2</name>\n <author>Assyl</author>\n <created>01/01/14</created>\n <updated>01/02/14</updated>\n <tag>tag content 1</tag>\n <othertag>othertag content</othertag>\n <othertag>othertag content</othertag>\n</package>\n";
    meta->addTag("othertag", "othertag content");
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);
}

void KoXmlResourceBundleMetaTest::addTagsTest()
{
    QList<QString> tagList ;
    env = QProcessEnvironment::systemEnvironment().value("HOME");
    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta();
    meta->addTag("name", "packName");
    meta->addTag("author", "Assyl");
    QString expectedStr;

    tagList.push_front("Resource 1");
    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <tag>Resource 1</tag>\n</package>\n";
    meta->addTags(tagList);
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    tagList.push_front("Resource 2");
    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <tag>Resource 1</tag>\n <tag>Resource 2</tag>\n</package>\n";
    meta->addTags(tagList);
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    tagList.push_front("Resource 3");
    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <tag>Resource 1</tag>\n <tag>Resource 2</tag>\n <tag>Resource 3</tag>\n</package>\n";
    meta->addTags(tagList);
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    tagList.push_front("");
    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <tag>Resource 1</tag>\n <tag>Resource 2</tag>\n <tag>Resource 3</tag>\n</package>\n";
    meta->addTags(tagList);
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);

    tagList.push_front(" ");
    expectedStr = "<!DOCTYPE meta>\n<package>\n <name>packName</name>\n <author>Assyl</author>\n <tag>Resource 1</tag>\n <tag>Resource 2</tag>\n <tag>Resource 3</tag>\n</package>\n";
    meta->addTags(tagList);
    meta->checkSort();
    QCOMPARE(meta->toString(), expectedStr);
}

void KoXmlResourceBundleMetaTest::getPackNameTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");
    QFile* device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.bundle</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
    device->close();
    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta(device);
    QCOMPARE(meta->getPackName(), QString("/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.bundle"));

    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE meta>\n<package>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
    device->close();
    meta = new KoXmlResourceBundleMeta(device);
    QCOMPARE(meta->getPackName(), QString(""));

    QFile::remove(env + "/fileTest.xml");

}

void KoXmlResourceBundleMetaTest::getShortPackNameTest()
{
//    env=QProcessEnvironment::systemEnvironment().value("HOME");
//    QFile* device =  new QFile(env+"/fileTest.xml");
//    device->open(QIODevice::WriteOnly);
//    device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.bundle</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
//    device->close();
//    KoXmlResourceBundleMeta *meta = new KoXmlResourceBundleMeta(device);
//    QCOMPARE(meta->getShortPackName(),QString("newBundle"));

//    device->open(QIODevice::WriteOnly);
//    device->write("<!DOCTYPE meta>\n<package>\n <name>newBundle.bundle</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
//    device->close();
//    meta = new KoXmlResourceBundleMeta(device);
//    QCOMPARE(meta->getShortPackName(),QString("newBundle"));

//    device->open(QIODevice::WriteOnly);
//    device->write("<!DOCTYPE meta>\n<package>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
//    device->close();
//    meta = new KoXmlResourceBundleMeta(device);
//    QCOMPARE(meta->getShortPackName(),QString(""));

//    QFile::remove(env+"/fileTest.xml");

}

QTEST_KDEMAIN(KoXmlResourceBundleMetaTest, GUI)
#include "KoXmlResourceBundleMetaTest.moc"
