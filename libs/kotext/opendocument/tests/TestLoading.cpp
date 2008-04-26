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
#include <KoTextDebug.h>
#include <KoListStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>

static void showDocument(QTextDocument *document)
{
    QTextEdit *textEdit = new QTextEdit;
    textEdit->setDocument(document);
    textEdit->show();
    qApp->exec();
    delete textEdit;
}

// Functions that help compare two QTextDocuments
static bool compareFragments(const QTextFragment &actualFragment, const QTextFragment &expectedFragment)
{
    if (actualFragment.text() != expectedFragment.text()) {
        qDebug() << "compareFragments: text not equal " << actualFragment.text() << expectedFragment.text();
        return false;
    }
    if (actualFragment.position() != expectedFragment.position()) {
        qDebug() << "compareFragments: position not equal " << actualFragment.position() << expectedFragment.position();
        return false;
    }

    QTextCharFormat actualFormat = actualFragment.charFormat();
    QTextImageFormat actualImageFormat = actualFormat.toImageFormat();

    QTextCharFormat expectedFormat = expectedFragment.charFormat();
    QTextImageFormat expectedImageFormat = expectedFormat.toImageFormat();
    
    if (actualImageFormat.isValid()) {
        if (!expectedImageFormat.isValid()) {
            qDebug() << "compareFragments: actualDocument has unexpected image at " << actualFragment.text();
            return false;
         }
        return true; // FIXME: Compare the image formats
    } else {
        if (expectedImageFormat.isValid()) {
            qDebug() << "compareFragment: expecting image in actualDocument at " << actualFragment.text();
            return false;
        }
    }

    // this should really be actualFormat.properties() == expectedFormat.properties()
    bool equal = actualFormat.font() == expectedFormat.font()
           && actualFormat.foreground() == expectedFormat.foreground()
           && actualFormat.background() == expectedFormat.background()
           && actualFormat.fontCapitalization() == expectedFormat.fontCapitalization()
           && actualFormat.underlineColor() == expectedFormat.underlineColor()
           && actualFormat.property(KoCharacterStyle::UnderlineStyle).toInt() 
                  == expectedFormat.property(KoCharacterStyle::UnderlineStyle).toInt()
           /*&& qvariant_cast<QColor>(actualFormat.property(KoCharacterStyle::StrikeOutColor))
                  == qvariant_cast<QColor>(expectedFormat.property(KoCharacterStyle::StrikeOutColor))*/
           && actualFormat.property(KoCharacterStyle::StrikeOutStyle).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutStyle).toInt() 
           && actualFormat.property(KoCharacterStyle::StrikeOutType).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutType).toInt() 
           && actualFormat.verticalAlignment() == expectedFormat.verticalAlignment(); // FIXME: Compare other properties
    
    if (!equal)
        qDebug() << "compareFragment: property mismatch at " << actualFragment.text();

    return equal;
}

static bool compareBlocks(const QTextBlock &actualBlock, const QTextBlock &expectedBlock)
{
    QTextList *actualList = actualBlock.textList();
    QTextList *expectedList = expectedBlock.textList();

    if (actualList) {
        if (!expectedList) {
            qDebug() << "compareBlocks: Unexpected list in actualDocument at " << actualBlock.text();
            return false;
        }
        if ((actualList->format().properties() != expectedList->format().properties())
            || (actualList->itemNumber(actualBlock) != expectedList->itemNumber(expectedBlock))) {
            qDebug() << "compareBlocks: list properties mismatch at " << actualBlock.text()
                     << actualList->format().properties() << expectedList->format().properties()
                     << actualList->itemNumber(actualBlock) << expectedList->itemNumber(expectedBlock);
            return false;
        }
    } else {
        if (expectedList) {
            qDebug() << "compareBlock: Expecting  list in actualDocument at " << actualBlock.text();
            return false;
        }
        // this should really be actualBlock.blockFormat().properties() == expectedBlock.blockFormat().properties()
        QTextBlockFormat actualFormat = actualBlock.blockFormat();
        QTextBlockFormat expectedFormat = expectedBlock.blockFormat();
        if (actualFormat.background() != expectedFormat.background()
            || actualFormat.alignment() != expectedFormat.alignment()
            || actualFormat.indent() != expectedFormat.indent()
            || actualFormat.textIndent() != expectedFormat.textIndent()
            || actualFormat.foreground() != expectedFormat.foreground()) {
            qDebug() << "compareBlock: block properties mismatch at " << actualBlock.text()
                     << actualFormat.properties() << expectedFormat.properties();
            return false;
        }
    }

    QTextBlock::Iterator actualIterator = actualBlock.begin();
    QTextBlock::Iterator expectedIterator = expectedBlock.begin();
    for(; !actualIterator.atEnd() && !expectedIterator.atEnd(); ++actualIterator, ++expectedIterator) {
        QTextFragment actualFragment = actualIterator.fragment();
        QTextFragment expectedFragment = expectedIterator.fragment();
        if (actualFragment.isValid()) {
            if (!expectedFragment.isValid()) {
                qDebug() << "compareBlock: Unexpected fragment in actualDocument at " << actualFragment.text();
                return false;
            }

            if (!compareFragments(actualFragment, expectedFragment))
                return false;
        } else {
            if (expectedFragment.isValid()) {
                qDebug() << "compareBlock: Expecting fragment in actualDocument at " << actualFragment.text();
                return false;
            }
        }
    }

    bool equal = actualIterator.atEnd() == expectedIterator.atEnd();
    if (!equal)
        qDebug() << "compareBlock: Iterator are not in the end! at " << actualBlock.text();

    return equal;
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
            if (!expectedChildFrame) {
                qDebug() << "compareFrames: Unexpected frame at " << actualTextBlock.text();
                return false;
            }
            QTextTable *actualTable = qobject_cast<QTextTable *>(actualChildFrame);
            QTextTable *expectedTable = qobject_cast<QTextTable *>(expectedChildFrame);
            if (actualTable) {
                if (!expectedTable) {
                    qDebug() << "compareFrames: Unexpected table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareTables(actualTable, expectedTable))
                    return false;
            } else {
                if (expectedTable) {
                    qDebug() << "compareFrames: Expecting table at " << actualTextBlock.text();
                    return false;
                }
                if (!compareFrames(actualChildFrame, expectedChildFrame))
                    return false;
            }
        } else if (actualTextBlock.isValid()) {
            QTextBlock expectedTextBlock = expectedIterator.currentBlock();
            if (!expectedTextBlock.isValid()) {
                qDebug() << "compareFrames: Unexpected text block at " << actualTextBlock.text();
                return false;
            }
            if (!compareBlocks(actualTextBlock, expectedTextBlock))
                return false;
        } else {
            qDebug() << "compareFrames: neither frame nor block! - internal error!";
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

// helper to evaluate script on an engine
static QScriptValue evaluate(QScriptEngine *engine, const QString &script)
{
    QString contents;
    QFile file(script);

    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        contents = stream.readAll();
        file.close();
    }

    QScriptValue r = engine->evaluate(contents);
    if (engine->hasUncaughtException()) {
        QStringList backtrace = engine->uncaughtExceptionBacktrace();
        qDebug("    %s\n%s\n", qPrintable(r.toString()),
                qPrintable(backtrace.join("\n")));
    }
    return r;
}


// add a include function for the scripts
static QScriptValue includeFunction(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0)
        return engine->nullValue();

    return evaluate(engine, QString(FILES_DATA_DIR) + context->argument(0).toString());
}

// FIXME: Remove this once the generator is fixed
Q_DECLARE_METATYPE(QTextFormat);
Q_DECLARE_METATYPE(QTextFormat *);

static QScriptValue setFormatProperty(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 3) {
        qWarning() << "too few arguments to setFormatProperty(format, id, value)";
        return engine->nullValue();
    }
    
    QTextFormat *format = qscriptvalue_cast<QTextFormat *>(context->argument(0));
    int id = context->argument(1).toInt32();
    QScriptValue arg = context->argument(2);
    QVariant value;
    if (arg.isNumber()) {
        // ### hack to detect if the number is of type int
        if ((qsreal)arg.toInt32() == arg.toNumber())
            value = arg.toInt32();
        else
            value = arg.toNumber();
    } else {
        value = arg.toVariant();
    }
    format->setProperty(id, value);

    return QScriptValue();
}

Q_DECLARE_METATYPE(QTextCharFormat);
Q_DECLARE_METATYPE(QTextCharFormat *);

static QScriptValue copyFormatProperties(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1)
        return engine->nullValue();
    
    QTextFormat *dest = qscriptvalue_cast<QTextFormat *>(context->argument(0));
    QTextFormat *src = qscriptvalue_cast<QTextFormat *>(context->argument(1));
    if (dest && src) {
        QMap<int, QVariant> properties = src->properties();
        foreach(int id, properties.keys()) {
            dest->setProperty(id, properties[id]);
        }
    }

    return QScriptValue();
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
    QScriptValue qscript = engine->newObject();
    qscript.setProperty("importExtension", engine->newFunction(importExtension));
    globalObject.property("qt").setProperty("script", qscript);
    
    globalObject.setProperty("include", engine->newFunction(includeFunction));
    globalObject.setProperty("setFormatProperty", engine->newFunction(setFormatProperty));
    globalObject.setProperty("copyFormatProperties", engine->newFunction(copyFormatProperties));
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

    // FIXME: the line below exists because I haven't manage get includeFunction to work
    evaluate(engine, QString(FILES_DATA_DIR) + "common.qs");
}

void TestLoading::cleanup()
{
    delete textShapeData;
    textShapeData = 0;
    delete store;
    store = 0;
}

QTextDocument *TestLoading::documentFromScript(const QString &script)
{
    return qobject_cast<QTextDocument *>(evaluate(engine, script).toQObject());
}

QTextDocument *TestLoading::documentFromOdt(const QString &odt)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
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
    KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
    textShapeData->document()->setDocumentLayout(layout);
    KoStyleManager *styleManager = new KoStyleManager;
    layout->setStyleManager(styleManager);
    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    return textShapeData->document();
}

void TestLoading::testLoading_data()
{
    QTest::addColumn<QString>("testcase");

    QTest::newRow("Bulleted list") << "TextContents/Lists/bulletedList";
    QTest::newRow("Numbered list") << "TextContents/Lists/numberedList";
    QTest::newRow("Embedded bulleted list") << "TextContents/Lists/embeddedBulletedList";

    QTest::newRow("Bold and Italic") << "TextContents/TextFormatting/boldAndItalic";
    QTest::newRow("Attributed Text") << "TextContents/Paragraph/attributedText";

    QTest::newRow("Font Size") << "TextContents/TextFormatting/fontSize";

    QTest::newRow("Font Colors") << "TextContents/TextFormatting/fontColors";

    QTest::newRow("Colors") << "FormattingProperties/TextFormattingProperties/color";
    QTest::newRow("Font family") << "FormattingProperties/TextFormattingProperties/fontFamily";
    QTest::newRow("Font name") << "FormattingProperties/TextFormattingProperties/fontName";
    QTest::newRow("Font pitch") << "FormattingProperties/TextFormattingProperties/fontPitch";
    QTest::newRow("Font size") << "FormattingProperties/TextFormattingProperties/fontSize";
    QTest::newRow("Font style") << "FormattingProperties/TextFormattingProperties/fontStyle";
    QTest::newRow("Font weight") << "FormattingProperties/TextFormattingProperties/fontWeight";
    QTest::newRow("Font variant") << "FormattingProperties/TextFormattingProperties/fontVariant";
    QTest::newRow("Line through style") << "FormattingProperties/TextFormattingProperties/lineThroughStyle";
    QTest::newRow("Line through type") << "FormattingProperties/TextFormattingProperties/lineThroughType";
    QTest::newRow("Text Background Color") << "FormattingProperties/TextFormattingProperties/textBackgroundColor";
    QTest::newRow("Text tranformations") << "FormattingProperties/TextFormattingProperties/textTransformations";
    QTest::newRow("Underline color") << "FormattingProperties/TextFormattingProperties/underlineColor";
    QTest::newRow("Underline type") << "FormattingProperties/TextFormattingProperties/underlineType";
    QTest::newRow("Underline style") << "FormattingProperties/TextFormattingProperties/underlineStyle";

    QTest::newRow("Paragraph background") << "FormattingProperties/ParagraphFormattingProperties/paragraphBackgroundColor";
    QTest::newRow("Text align") << "FormattingProperties/ParagraphFormattingProperties/textAlign";
    QTest::newRow("Auto text indent") << "FormattingProperties/ParagraphFormattingProperties/automaticTextIndent";
}

void TestLoading::testLoading() 
{
    QFETCH(QString, testcase);
    testcase.prepend(FILES_DATA_DIR);

    QTextDocument *actualDocument = documentFromOdt(testcase + ".odt");
    QVERIFY(actualDocument != 0);

    QTextDocument *expectedDocument = documentFromScript(testcase + ".qs");
    QVERIFY(expectedDocument != 0);

    bool documentsEqual = compareDocuments(actualDocument, expectedDocument);

    if (!documentsEqual) {
        KoTextDebug::dumpDocument(actualDocument);
        KoTextDebug::dumpDocument(expectedDocument);
    }

    QVERIFY(documentsEqual);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestLoading, argc, argv);
}

#include "TestLoading.moc"
