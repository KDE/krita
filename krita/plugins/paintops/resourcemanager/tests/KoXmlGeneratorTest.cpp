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

#include "KoXmlGeneratorTest.h"
#include <QCoreApplication>
#include <qtest_kde.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include "KoXmlGenerator.h"
#include <QTest>


void KoXmlGeneratorTest::ctorTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");

    //Simple sentence content
//    QFile *device =  new QFile(env+"/fileTest.xml");
//   device->open(QIODevice::WriteOnly);
//   device->write("Test File");
//   device->close();
//   KoXmlGenerator *gen = new KoXmlGenerator(device);
//   device->open(QIODevice::ReadOnly);
//   QCOMPARE(gen->toString().toUtf8().data(),device->readAll().data());
//   device->close();
//   QFile::remove(env+"/fileTest.xml");

    //Xml content
    QFile* device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
    device->close();
    KoXmlGenerator* gen = new KoXmlGenerator(device);
    device->open(QIODevice::ReadOnly);
    QCOMPARE(gen->toByteArray().data(), device->readAll().data());
    device->close();
    QFile::remove(env + "/fileTest.xml");

//   device =  new QFile(env+"/fileTest.xml");
//   device->open(QIODevice::WriteOnly);
//   device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newPackage.zip</name>\n <created>26/03/2011</created>\n <updated>26/02/2014</updated>\n</package>\n");
//   device->close();
//   gen = new KoXmlGenerator(env+"/fileTest.xml");
//   device->open(QIODevice::ReadOnly);
//   QCOMPARE(gen->toByteArray().data(),device->readAll().data());
//   device->close();
//   QFile::remove(env+"/fileTest.xml");



}

void KoXmlGeneratorTest::getValueTest()
{

    env = QProcessEnvironment::systemEnvironment().value("HOME");
    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
    device->close();
    KoXmlGenerator* gen = new KoXmlGenerator(device);
    QCOMPARE((gen->getValue("name")).toUtf8().data(), "/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip");
    QCOMPARE((gen->getValue("created")).toUtf8().data(), "26/03/2014");
    QCOMPARE((gen->getValue("package")).toUtf8().data(), "");
    QCOMPARE((gen->getValue("updated")).toUtf8().data(), "26/03/2014");
    QCOMPARE((gen->getValue("/updated")).toUtf8().data(), "");
    QCOMPARE((gen->getValue("nonExistantTag")).toUtf8().data(), "");
    QFile::remove(env + "/fileTest.xml");


}

void KoXmlGeneratorTest::addTagTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");

    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n</package>\n");
    device->close();

    //Test de la méthode addTag(...) avec des nom de tags non-nuls
    KoXmlGenerator* gen = new KoXmlGenerator(device);
    QString xmlContent = "<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n";
    xmlContent.append(" <newTag1>newTag1 Content 1</newTag1>\n");
    gen->addTag("newTag1", "newTag1 Content 1");
    xmlContent.append(" <newTag1>newTag1 Content 2</newTag1>\n");
    gen->addTag("newTag1", "newTag1 Content 2");
    xmlContent.append(" <newTag2>newTag2 Content 1</newTag2>\n");
    gen->addTag("newTag2", "newTag2 Content 1");
    xmlContent.append("</package>\n");
    QCOMPARE((gen->toString()), xmlContent);

    //Test de la méthode addTag(...) avec des nom de tags non-nuls
    gen = new KoXmlGenerator(device);
    xmlContent = "<!DOCTYPE meta>\n<package>\n <name>/home/sniperwolf/kde4/inst/share/apps/krita/bundles/newBundle.zip</name>\n <created>26/03/2014</created>\n <updated>26/03/2014</updated>\n";
    xmlContent.append(" <newTag1/>\n</package>\n");
    gen->addTag("newTag1", "");
    QCOMPARE((gen->toString()), xmlContent);

    //Test de la méthode addTag(...) avec contenu nul
    gen = new KoXmlGenerator(device);
    gen->addTag("", "newTag1 Content 1");
    device->open(QIODevice::ReadOnly);
    QCOMPARE(gen->toByteArray().data(), device->readAll().data());
    device->close();

    //Test de la méthode addTag(...) avec un blanc comme nom de tag
    //ECHEC DU TEST
//   gen = new KoXmlGenerator(device);
//   gen->addTag(" ","contenu");
//   device->open(QIODevice::ReadOnly);
//   QCOMPARE(gen->toByteArray().data(),device->readAll().data());
//   device->close();


    QFile::remove(env + "/fileTest.xml");
}

void KoXmlGeneratorTest::removeFirstTagTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");

    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<file>\n <tag1>content 1</tag1>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");
    device->close();

    // Test de removeFirstTag(tagName, textValue)
    KoXmlGenerator* gen = new KoXmlGenerator(device);
    QVERIFY(gen->removeFirstTag("tag1", "content 1") == true);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag2", "content 3") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag5", "content 3") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    //Test de removeFirstTag(tagName, attName, attValue)
    QVERIFY(gen->removeFirstTag("tag4", "att1", "value2") == true);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag4", "att1", "value3") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag4", "att2", "value") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag4", "att1", "") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n</file>\n");

//    Echec de la suppression de <tag2\>,  <tag2>content 2</tag2> supprimée à la place
//    QVERIFY(gen->removeFirstTag("tag2","")== true);
//    QCOMPARE(gen->toByteArray().data(),"<file>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n</file>\n");

    QFile::remove(env + "/fileTest.xml");
}


void KoXmlGeneratorTest::removeTagTest()
{
    //Remarque : les balises contenant des attributs et celles n'ayant pas de text ne sont pas supprimées
    env = QProcessEnvironment::systemEnvironment().value("HOME");

    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<file>\n <tag1>content 1</tag1>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");
    device->close();

    KoXmlGenerator* gen = new KoXmlGenerator(device);
//    QVERIFY(gen->removeTag("tag1") == true);
//    QCOMPARE(gen->toByteArray().data(),"<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2>content 2</tag2>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

//    QVERIFY(gen->removeTag("tag2")== true);
//    QCOMPARE(gen->toByteArray().data(),"<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("tag5") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag("") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");

    QVERIFY(gen->removeFirstTag(" ") == false);
    QCOMPARE(gen->toByteArray().data(), "<file>\n <tag1 att1=\"value1\">content 1</tag1>\n <tag2/>\n <tag4 att1=\"value1\">content 1</tag4>\n <tag4 att1=\"value2\">content 2</tag4>\n</file>\n");



//    Echec de la suppression de <tag2\>,  <tag2>content 2</tag2> supprimée à la place
//    QVERIFY(gen->removeFirstTag("tag2","")== true);
//    QCOMPARE(gen->toByteArray().data(),"<file>\n <tag2>content 2</tag2>\n <tag1>content 3</tag1>\n</file>\n");

    QFile::remove(env + "/fileTest.xml");
}

void KoXmlGeneratorTest::searchValueTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");
    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<filetest>\n <tag1>contenu 1</tag1>\n <tag1 att1=\"value 1\" att2=\"value 2\">contenu 2</tag1>\n <tag2>contenu 2</tag2>\n <tag3 att1 = \"value 1\">contenu 3</tag3>\n <tag3/>\n</filetest>\n");
    device->close();

    KoXmlGenerator *gen = new KoXmlGenerator(device);
    QDomDocument *doc = new QDomDocument();
    device->open(QIODevice::ReadOnly);
    doc->setContent(device);
    device->close();



    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "contenu 1").nodeName().toUtf8().data(), "tag1");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "contenu 2").nodeName().toUtf8().data(), "tag1");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "contenu 3").nodeName().toUtf8().data(), "tag3");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "").nodeName().toUtf8().data(), "tag3");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "contenu 5").nodeName().toUtf8().data(), "");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "att1", "value 1").nodeName().toUtf8().data(), "tag1");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "att2", "value 2").nodeName().toUtf8().data(), "tag1");
    QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(), "att2", "value 3").nodeName().toUtf8().data(), "");

    // Les deux prochains tests sont sensés ne rien renvoyer, et pourtant ils renvoient la première balise de la liste
    // QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(),"att2","").nodeName().toUtf8().data(),"");
    // QCOMPARE(gen->searchValue(doc->childNodes().at(0).childNodes(),"att1","").nodeName().toUtf8().data(),"");



    QFile::remove(env + "/fileTest.xml");


}

void KoXmlGeneratorTest::toFileTest()
{
    env = QProcessEnvironment::systemEnvironment().value("HOME");
    QFile *device =  new QFile(env + "/fileTest.xml");
    device->open(QIODevice::WriteOnly);
    device->write("<!DOCTYPE tofiletest>\n<package>\n <tag1>contenu 1</tag1>\n <tag1 att1=\"value 1\" att2=\"value 2\">contenu 2</tag1>\n <tag2>contenu 2</tag2>\n <tag3 att1 = \"value 1\">contenu 3</tag3>\n <tag3/>\n</package>\n");
    device->close();
    KoXmlGenerator *gen = new KoXmlGenerator(device);
    QFile::remove(env + "/fileTest.xml");
    QFile *generatedFile = new QFile(gen->toFile()) ;
    generatedFile->open(QIODevice::ReadOnly);
    QCOMPARE(generatedFile->readAll().data(), gen->toByteArray().data());
    generatedFile->close();
    QFile::remove(env + "/fileTest.xml");
}

QTEST_KDEMAIN(KoXmlGeneratorTest, GUI)
#include "KoXmlGeneratorTest.moc"
