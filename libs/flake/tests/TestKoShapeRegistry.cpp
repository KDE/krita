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
#include "TestKoShapeRegistry.h"
#include <QTest>
#include <QBuffer>
#include <QFile>
#include <QDateTime>
#include <QProcess>
#include <QString>
#include <QTextStream>
#include <QtXml>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

#include "KoShapeRegistry.h"
#include "KoShape.h"
#include "KoPathShape.h"
#include "KoShapeLoadingContext.h"

#include <KoXmlReader.h>

#include <FlakeDebug.h>
#include "kis_debug.h"
#include <sdk/tests/kistest.h>

void TestKoShapeRegistry::testGetKoShapeRegistryInstance()
{
    KoShapeRegistry * registry = KoShapeRegistry::instance();
    QVERIFY(registry != 0);
}

void TestKoShapeRegistry::testCreateShapes()
{
    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream.setCodec("UTF-8");

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:calligra=\"http://www.calligra.org/2005/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    xmlstream << "<office:body>";
    xmlstream << "<office:text>";
    xmlstream << "<draw:path svg:d=\"M0,0L100,100\"></draw:path>";
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

    KoShapeRegistry * registry = KoShapeRegistry::instance();

    // XXX: When loading is implemented, these no doubt have to be
    // sensibly filled.
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext odfContext(stylesReader, 0);
    KoShapeLoadingContext shapeContext(odfContext, 0);

    KoShape * shape = registry->createShapeFromOdf(bodyElement, shapeContext);
    QVERIFY(shape == 0);

    KoXmlElement pathElement = bodyElement.firstChild().firstChild().toElement();
    shape = registry->createShapeFromOdf(pathElement, shapeContext);
    QVERIFY(shape != 0);
    QVERIFY(shape->shapeId() == KoPathShapeId);
}


void TestKoShapeRegistry::testCreateFramedShapes()
{
    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream.setCodec("UTF-8");

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:calligra=\"http://www.calligra.org/2005/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    xmlstream << "<office:body>";
    xmlstream << "<office:text>";
    xmlstream << "<draw:path svg:d=\"M0,0L100,100\"></draw:path>";
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

    KoShapeRegistry * registry = KoShapeRegistry::instance();

    // XXX: When loading is implemented, these no doubt have to be
    // sensibly filled.
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext odfContext(stylesReader, 0);
    KoShapeLoadingContext shapeContext(odfContext, 0);

    KoShape * shape = registry->createShapeFromOdf(bodyElement, shapeContext);
    QVERIFY(shape == 0);

    KoXmlElement pathElement = bodyElement.firstChild().firstChild().toElement();
    shape = registry->createShapeFromOdf(pathElement, shapeContext);
    QVERIFY(shape != 0);
    QVERIFY(shape->shapeId() == KoPathShapeId);
}

#include <KoStore.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>
#include <MockShapes.h>
#include "../../sdk/tests/qimage_test_util.h"

void TestKoShapeRegistry::testFramedSvgShapes()
{
    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream.setCodec("UTF-8");

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\" xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\" xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" xmlns:calligra=\"http://www.calligra.org/2005/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">";
    xmlstream << "<office:body>";
    xmlstream << "<office:text>";
    xmlstream << "<draw:frame xml:id=\"shape-1\" draw:z-index=\"2\" draw:id=\"shape-1\" draw:layer=\"\" svg:width=\"226pt\" svg:height=\"141pt\" svg:x=\"83pt\" svg:y=\"41pt\">";
    xmlstream << "  <draw:image xlink:type=\"simple\" draw:z-index=\"3\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" xlink:href=\"VectorImages/Image1\"/>";
    xmlstream << "</draw:frame>";
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

    KoShapeRegistry * registry = KoShapeRegistry::instance();

    // XXX: When loading is implemented, these no doubt have to be
    // sensibly filled.
    KoOdfStylesReader stylesReader;

    const QString resourcesBlob = TestUtil::fetchDataFileLazy("odf_frame_resource_store.zip");
    QScopedPointer<KoStore> store(KoStore::createStore(resourcesBlob, KoStore::Read, "krita", KoStore::Zip));
    QScopedPointer<KoDocumentResourceManager> resourceManager(new KoDocumentResourceManager());


    QScopedPointer<MockShapeController> document(new MockShapeController());
    QScopedPointer<MockCanvas> canvas(new MockCanvas(document.data()));

    QScopedPointer<KoShapeController> shapeController(new KoShapeController(canvas.data(), document.data()));
    resourceManager->setGlobalShapeController(shapeController.data());


    KoOdfLoadingContext odfContext(stylesReader, store.data());
    KoShapeLoadingContext shapeContext(odfContext, resourceManager.data());

    KoXmlElement frameElement = bodyElement.firstChild().firstChild().toElement();

    QCOMPARE(frameElement.tagName(), QString("frame"));

    KoShape *shape = registry->createShapeFromOdf(frameElement, shapeContext);

    QVERIFY(shape);
    QCOMPARE(shape->absoluteOutlineRect(), QRectF(83, 41, 226,141));
}

KISTEST_MAIN(TestKoShapeRegistry)
