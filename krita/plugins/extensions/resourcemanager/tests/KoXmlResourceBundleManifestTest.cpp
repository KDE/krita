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


#include "KoXmlResourceBundleManifestTest.h"
#include <QCoreApplication>
#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include "KoXmlResourceBundleManifest.h"
#include <QTest>


void KoXmlResourceBundleManifestTest::ctorTest()
{
    KoXmlResourceBundleManifest *manifest = new KoXmlResourceBundleManifest(QString("manifest"));
    QCOMPARE(manifest->toString(), QString("<!DOCTYPE manifest>\n<package/>\n"));

    env = QProcessEnvironment::systemEnvironment().value("HOME");
    QFile* device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE manifest>\n<package>\n <paintoppresets>\n  <file name=\"/krita/paintoppresets/Airbrush_eraser.kpp\"/>\n </paintoppresets>\n <patterns>\n  <file name=\"/krita/patterns/02b_WoofTissue.png\"/>\n </patterns>\n</package>\n");
    device->close();
    manifest = new KoXmlResourceBundleManifest(device);
    device->open(QIODevice::ReadOnly);
    QCOMPARE(manifest->toString().toUtf8().data(), device->readAll().data());
    device->close();
    QFile::remove(env + "/fileTest.xml");

    QByteArray array("<!DOCTYPE manifest>\n<package>\n <patterns>\n  <file name=\"/krita/patterns/02b_WoofTissue.png\"/>\n </patterns>\n</package>\n");
    manifest = new KoXmlResourceBundleManifest(array);
    QCOMPARE(manifest->toString().toUtf8().data(), array.data());
}

void KoXmlResourceBundleManifestTest:: getTagEnumValueTest()
{
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("brushes")) == 0);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("palettes")) == 3);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("gradients")) == 1);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("paintoppresets")) == 2);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("patterns")) == 4);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("templates")) == 5);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("workspaces")) == 6);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("ref")) == 7);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("autre")) == 8);
    QVERIFY(KoXmlResourceBundleManifest::getTagEnumValue(QString("")) == 8);
}

void KoXmlResourceBundleManifestTest::checkSortTest()
{

//    QByteArray array("<!DOCTYPE manifest>\n<package>\n <paintoppresets>paintoppreset 1</paintoppresets>\n <brushes>brush 1</bushes>\n</package>\n");
//    QString chaineRes = "<!DOCTYPE manifest>\n<package>\n <brushes>brush 1</bushes>\n <paintoppresets>paintoppreset 1</paintoppresets>\n</package>\n";
//    KoXmlResourceBundleManifest *manifest = new KoXmlResourceBundleManifest(array);
//    manifest->checkSort();
//    QCOMPARE(manifest->toString(),chaineRes);



}

void KoXmlResourceBundleManifestTest::mergeTest()
{
    QByteArray array("<!DOCTYPE manifest>\n<package>\n <paintoppresets>\n  <file name=\"popp1\"/>\n </paintoppresets>\n <brushes>\n  <file name=\"brush1\"/>\n </brushes>\n <paintoppresets>\n  <file name=\"popp2\"/>\n </paintoppresets>\n</package>\n");
    QString chaineRes = "<!DOCTYPE manifest>\n<package>\n <paintoppresets>\n  <file name=\"popp1\"/>\n  <file name=\"popp2\"/>\n </paintoppresets>\n  <brushes>\n  <file name=\"brush1\"/>\n </brushes>\n</package>\n";

    KoXmlResourceBundleManifest *manifest = new KoXmlResourceBundleManifest(array);
    manifest->merge(manifest->getXmlDocument().firstChild().firstChild(), manifest->getXmlDocument().firstChild().firstChild().nextSibling().nextSibling());
    QCOMPARE(manifest->toString(), chaineRes);

}

QTEST_KDEMAIN(KoXmlResourceBundleManifestTest, GUI)
#include "KoXmlResourceBundleManifestTest.moc"
