/* This file is part of the KDE project
 * Copyright (c) 2007 Boudewijn Rempt (boud@valdyas.org)
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
#include "TestKoShapeFactory.h"
#include <qtest_kde.h>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoShapeLoadingContext.h>
#include <KoPathShapeFactory.h>
#include <KoShape.h>
#include <KoShapeFactory.h>
#include <KoXmlNS.h>
#include <kdebug.h>

void TestKoShapeFactory::testCreateFactory()
{
    KoShapeFactory * factory = new KoPathShapeFactory(0, QStringList());
    QVERIFY(factory != 0);
    delete factory;
}

void TestKoShapeFactory::testSupportsKoXmlElement()
{
}

void TestKoShapeFactory::testPriority()
{
    KoShapeFactory * factory = new KoPathShapeFactory(0, QStringList());
    QVERIFY(factory->loadingPriority() == 0);
    delete factory;
}

void TestKoShapeFactory::testCreateDefaultShape()
{
    KoShapeFactory * factory = new KoPathShapeFactory(0, QStringList());
    QMap<QString, KoDataCenter *> dataCenterMap;
    KoShape * shape = factory->createDefaultShapeAndInit(dataCenterMap);
    QVERIFY(shape != 0);
    delete shape;
    delete factory;
}

void TestKoShapeFactory::testCreateShape()
{
    KoShapeFactory * factory = new KoPathShapeFactory(0, QStringList());
    QMap<QString, KoDataCenter *> dataCenterMap;
    KoShape * shape = factory->createShapeAndInit(0, dataCenterMap);
    QVERIFY(shape != 0);
    delete shape;
    delete factory;
}

void TestKoShapeFactory::testOdfElement()
{
    KoShapeFactory * factory = new KoPathShapeFactory(0, QStringList());
    QVERIFY(factory->odfElementNames().contains("path"));
    QVERIFY(factory->odfElementNames().contains("line"));
    QVERIFY(factory->odfElementNames().contains("polyline"));
    QVERIFY(factory->odfElementNames().contains("polygon"));
    QVERIFY(factory->odfNameSpace() == KoXmlNS::draw);

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:koffice=\"http://www.koffice.org/2005/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    xmlstream << "<office:body>";
    xmlstream << "<office:text>";
    xmlstream << "<text:p text:style-name=\"P1\"><?opendocument cursor-position?></text:p>";
    xmlstream << "<draw:path svg:d=\"M10,10L100,100\"></draw:path>";
    xmlstream << "</office:text>";
    xmlstream << "</office:body>";
    xmlstream << "</office:document-content>";
    xmldevice.close();

    KoXmlDocument doc;
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    KoXmlElement contentElement = doc.documentElement();
    KoXmlElement bodyElement = contentElement.firstChild().toElement();

    KoXmlElement textElement = bodyElement.firstChild().firstChild().toElement();
    QVERIFY(textElement.tagName() == "p");
    QCOMPARE(factory->supports(textElement), false);

    KoXmlElement pathElement = bodyElement.firstChild().lastChild().toElement();
    QVERIFY(pathElement.tagName() == "path");
    QCOMPARE(factory->supports(pathElement), true);

    QMap<QString, KoDataCenter *> dataCenterMap;
    KoShape * shape = factory->createDefaultShapeAndInit(dataCenterMap);
    QVERIFY(shape);

    // XXX: When loading is implemented, these no doubt have to be
    // sensibly filled.
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext odfContext(stylesReader, 0);
    KoShapeLoadingContext shapeContext(odfContext, dataCenterMap);

    QVERIFY(shape->loadOdf(pathElement, shapeContext));

    delete shape;
    delete factory;
}

QTEST_KDEMAIN(TestKoShapeFactory, NoGUI)
#include <TestKoShapeFactory.moc>
