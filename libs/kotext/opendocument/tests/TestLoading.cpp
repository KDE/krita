/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "TestLoading.h"

#include <QtGui>
#include <QDebug>
#include <QtScript>
#include <QtTest>

#include <KoStyleManager.h>
#include <KoDocument.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoOdfStylesReader.h>
#include <KoTextLoader.h>
#include <KoTextLoadingContext.h>
#include <KoXmlReader.h>
#include <KoOdfReadStore.h>
#include <KoTextShapeData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <kcomponentdata.h>

// Functions to print the contents of QTextDocument as XML
static int depth = 0;
static const int INDENT = 2;

static void dumpFragment(const QTextFragment &fragment)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<fragment>");

    QTextCharFormat textFormat = fragment.charFormat();
    QTextImageFormat imageFormat = textFormat.toImageFormat();

    if (imageFormat.isValid()) {
    } else {
    }
  
    qDebug("%*s%s", depth+INDENT, " ", qPrintable(fragment.text()));

    qDebug("%*s%s", depth, " ", "</fragment>");
    depth -= INDENT;
}

static void dumpTable(QTextTable *)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<table>");
    // FIXME
    qDebug("%*s%s", depth, " ", "</table>");
    depth -= INDENT;
}

static void dumpBlock(const QTextBlock &block)
{
    depth += INDENT;

    QTextList *list = block.textList();

    QString startTag;
    if (list) {
        startTag.sprintf("<block listitem=\"%d/%d\">", list->itemNumber(block)+1, list->count());
    } else {
        startTag = "<block>";
    }

    qDebug("%*s%s", depth, " ", qPrintable(startTag));

    QTextBlock::Iterator iterator = block.begin();
    for(; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFragment fragment = iterator.fragment();
        if (fragment.isValid()) {
            dumpFragment(fragment);
        }
    }
    qDebug("%*s%s", depth, " ", "</block>");
    depth -= INDENT;
    if (block.next().isValid())
        qDebug(" ");
}

static void dumpFrame(QTextFrame *frame)
{
    depth += INDENT;
    qDebug("%*s%s", depth, " ", "<frame>");

    QTextFrame::iterator iterator = frame->begin();

    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFrame *childFrame = iterator.currentFrame();
        QTextBlock textBlock = iterator.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable *>(childFrame);
            if (table) {
                dumpTable(table);
            } else {
                dumpFrame(frame);
            }
        } else if (textBlock.isValid()) {
            dumpBlock(textBlock);
        }
    }

    qDebug("%*s%s", depth, " ", "</frame>");
    depth -= INDENT;
}

static void dumpDocument(QTextDocument *document)
{
    qDebug() << "<document>";
    dumpFrame(document->rootFrame());
    qDebug() << "</document>";
}

static void showDocument(QTextDocument *document)
{
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setDocument(document);
    textEdit->show();
    qApp->exec();
    delete textEdit;
}

// Functions that compare two QTextDocuments
static bool compareFragments(const QTextFragment &actualFragment, const QTextFragment &expectedFragment)
{
    if (actualFragment.text() != expectedFragment.text())
        return false;
    if (actualFragment.position() != expectedFragment.position())
        return false;
    QTextCharFormat actualFormat = actualFragment.charFormat();
    QTextImageFormat actualImageFormat = actualFormat.toImageFormat();

    QTextCharFormat expectedFormat = expectedFragment.charFormat();
    QTextImageFormat expectedImageFormat = expectedFormat.toImageFormat();
    
    if (actualImageFormat.isValid()) {
        if (!expectedImageFormat.isValid())
            return false;
        return true; // FIXME: Compare the image formats
    } else {
        if (expectedImageFormat.isValid())
            return false;
    }

    return true; // FIXME: Compare the char formats
}

static bool compareBlocks(const QTextBlock &actualBlock, const QTextBlock &expectedBlock)
{
    QTextList *actualList = actualBlock.textList();
    QTextList *expectedList = expectedBlock.textList();

    if (actualList) {
        if (!expectedList)
            return false;
        if (actualList->format() != expectedList->format()
            || actualList->itemNumber(actualBlock) != expectedList->itemNumber(expectedBlock))
            return false;
    } else {
        if (expectedList)
            return false;
    }

    QTextBlock::Iterator actualIterator = actualBlock.begin();
    QTextBlock::Iterator expectedIterator = expectedBlock.begin();
    for(; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFragment actualFragment = actualIterator.fragment();
        QTextFragment expectedFragment = expectedIterator.fragment();
        if (actualFragment.isValid()) {
            if (!expectedFragment.isValid())
                return false;
            if (!compareFragments(actualFragment, expectedFragment))
                return false;
        } else {
            if (expectedFragment.isValid())
                return false;
        }
    }

   return actualIterator.atEnd() == expectedIterator.atEnd();
}

static bool compareTables(QTextTable * /*actualTable*/, QTextTable * /*expectedTable*/)
{
    // FIXME: Cells of Tables are QTextTableCell's which contain QTextFrames
    // KWord does not implement tables, yet.
    return false;
}

static bool compareFrames(QTextFrame *actualFrame, QTextFrame *expectedFrame)
{
    QTextFrame::iterator actualIterator = actualFrame->begin();
    QTextFrame::iterator expectedIterator = expectedFrame->begin();

    for (; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFrame *actualChildFrame = actualIterator.currentFrame();
        QTextBlock actualTextBlock = actualIterator.currentBlock();

        if (actualChildFrame) {
            QTextFrame *expectedChildFrame = expectedIterator.currentFrame();
            if (!expectedChildFrame)
                return false;
            QTextTable *actualTable = qobject_cast<QTextTable *>(actualChildFrame);
            QTextTable *expectedTable = qobject_cast<QTextTable *>(expectedChildFrame);
            if (actualTable) {
                if (!expectedTable)
                    return false;
                if (!compareTables(actualTable, expectedTable))
                    return false;
            } else {
                if (expectedTable)
                    return false;
            }
            if (!compareFrames(actualChildFrame, expectedChildFrame))
                return false;
        } else if (actualTextBlock.isValid()) {
            QTextBlock expectedTextBlock = expectedIterator.currentBlock();
            if (!expectedTextBlock.isValid())
                return false;
            if (!compareBlocks(actualTextBlock, expectedTextBlock))
                return false;
        } else {
            return false;
        }
    }
    return actualIterator.atEnd() == expectedIterator.atEnd();
}

static bool compareDocuments(QTextDocument *actualDocument, QTextDocument *expectedDocument)
{
    QTextFrame *actualFrame = actualDocument->rootFrame();
    QTextFrame *expectedFrame = expectedDocument->rootFrame();
    return compareFrames(actualFrame, expectedFrame);
}

// May the testing begin
TestLoading::TestLoading() 
{
    componentData = new KComponentData("TestLoading");
}

TestLoading::~TestLoading()
{
    delete componentData;
}

static QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine)
{
    return engine->importExtension(context->argument(0).toString());
}

// initTestCase/cleanupTestCase are called beginning and end of test suite
void TestLoading::initTestCase()
{
    QString pluginPath = qgetenv("QSCRIPT_PLUGIN_PATH");
    if (pluginPath.isEmpty())
        qWarning() << "QSCRIPT_PLUGIN_PATH not set. Set it to the path of QScript plugins";

    QStringList paths = qApp->libraryPaths();
    paths <<  pluginPath;
    qApp->setLibraryPaths(paths);

    engine = new QScriptEngine();

    engine->importExtension("qt.core");
    engine->importExtension("qt.gui");

    QScriptValue globalObject = engine->globalObject();
    globalObject.setProperty("qApp", engine->newQObject(qApp));
    {
        QScriptValue qscript = engine->newObject();
        qscript.setProperty("importExtension", engine->newFunction(importExtension));
        globalObject.property("qt").setProperty("script", qscript);
    }
}

void TestLoading::cleanupTestCase()
{
    delete engine;
    engine = 0;
}

// init/cleanup are called beginning and end of every test case
void TestLoading::init()
{
    textShapeData = 0;
    store = 0;
    textEdit = 0;
}

void TestLoading::cleanup()
{
    delete textShapeData;
    textShapeData = 0;
    delete store;
    store = 0;
    delete textEdit;
    textEdit = 0;
}

QTextDocument *TestLoading::documentFromScript(const QString &script)
{
    QString contents;
    QFile file(script);

    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        contents = stream.readAll();
        file.close();
    }

    // "var cursor = new QTextCursor(document)" doesn't seem to work. So, create a QTextEdit
    // that will expose the document and cursor to the scripts
    textEdit = new QTextEdit;
    QScriptValue textEditValue = engine->newQObject(textEdit);
    QScriptValue globalObject = engine->globalObject();
    globalObject.setProperty("textEdit", textEditValue);

    QScriptValue r = engine->evaluate(contents);
    if (engine->hasUncaughtException()) {
        QStringList backtrace = engine->uncaughtExceptionBacktrace();
        qDebug("    %s\n%s\n", qPrintable(r.toString()),
                qPrintable(backtrace.join("\n")));
        return 0;
    }

    return textEdit->document();
}

QTextDocument *TestLoading::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qDebug() << odt << " does not exist";
        return 0;
    }

    store = KoStore::createStore(odt, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(store);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store());
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0 /* KoShapeControllerBase (KWDocument) */);
    textShapeData = new KoTextShapeData;
    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    return textShapeData->document();
}

void TestLoading::testLoading_data()
{
    QTest::addColumn<QString>("testcase");

    QTest::newRow("Bulleted list") << "data/TextContents/Lists/bulletedList";
}

void TestLoading::testLoading() 
{
    QFETCH(QString, testcase);

    QTextDocument *actualDocument = documentFromOdt(testcase + ".odt");
    QVERIFY(actualDocument != 0);

    QTextDocument *expectedDocument = documentFromScript(testcase + ".qs");
    QVERIFY(expectedDocument != 0);

    QVERIFY(compareDocuments(actualDocument, expectedDocument));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestLoading, argc, argv);
}

#include "TestLoading.moc"
