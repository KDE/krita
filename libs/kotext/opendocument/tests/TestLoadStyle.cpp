/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
#include "TestLoadStyle.h"

#include <KoStore.h>
#include <KoTextLoader.h>
#include <KoXmlReader.h>
#include <KoOdfReadStore.h>
#include <KTemporaryFile>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoText.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextDocument.h>
#include <KoChangeTracker.h>

#include <kstandarddirs.h>
#include <KDebug>
#include <kcomponentdata.h>

#include <QTextCursor>

TestLoadStyle::TestLoadStyle()
{
    componentData = new KComponentData("TestLoadStyle");
    componentData->dirs()->addResourceType("styles", "data", "words/styles/");
}

TestLoadStyle::~TestLoadStyle()
{
    delete componentData;
}

// initTestCase/cleanupTestCase are called beginning and end of test suite
void TestLoadStyle::initTestCase()
{
}

void TestLoadStyle::cleanupTestCase()
{
}

// init/cleanup are called beginning and end of every test case
void TestLoadStyle::init()
{
}

void TestLoadStyle::cleanup()
{
}

QTextDocument *TestLoadStyle::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
        return 0;
    }

    KoStore *readStore = KoStore::createStore(odt, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoStyleManager *styleManager = new KoStyleManager;
    KoChangeTracker *changeTracker = new KoChangeTracker;

    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store(), *componentData);
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KoTextSharedLoadingData *textSharedLoadingData = new KoTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KOTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    QTextDocument *document = new QTextDocument;
    KoTextDocument(document).setStyleManager(styleManager);
    KoTextDocument(document).setChangeTracker(changeTracker);

    KoTextLoader loader(shapeLoadingContext);
    QTextCursor cursor(document);
    loader.loadBody(body, cursor);   // now let's load the body from the ODF KoXmlElement.

    delete readStore;
    return document;
}

void TestLoadStyle::testLoadStyle()
{
    QTextDocument *document = documentFromOdt(QString(FILES_DATA_DIR)
            + "/TextContents/TextFormatting/charStyle.odt");
    QVERIFY(document != 0);

    QTextBlock block = document->begin();
    QVERIFY(block.isValid());
    QCOMPARE(block.text(), QString("The following is a word which uses the named character style MyStyle."));

    QTextCursor cursor(block);
    QCOMPARE(cursor.blockFormat().property(KoParagraphStyle::StyleId).toInt(), 101);
    QCOMPARE(cursor.blockCharFormat().property(KoCharacterStyle::StyleId).toInt(), 101);
    QCOMPARE(cursor.charFormat().property(KoCharacterStyle::StyleId).toInt(), 101);

    cursor.setPosition(62);
    //qDebug() << cursor.charFormat().property(KoCharacterStyle::StyleId).toInt();

    KoTextDocument textDoc(document);
    KoStyleManager *sm = textDoc.styleManager();
    KoCharacterStyle *myStyle = sm->characterStyle("MyStyle");
    QVERIFY(myStyle);
    //qDebug() << myStyle->styleId();
    QCOMPARE(cursor.charFormat().property(KoCharacterStyle::StyleId).toInt(), myStyle->styleId());
}

QTEST_KDEMAIN(TestLoadStyle, GUI)
#include <TestLoadStyle.moc>
