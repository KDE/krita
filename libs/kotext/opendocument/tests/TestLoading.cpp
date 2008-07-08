/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
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

#include <kdeversion.h>

#include <KoStyleManager.h>
#include <KoDocument.h>
#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoOdfStylesReader.h>
#include <KoTextLoader.h>
#include <KoXmlReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KTemporaryFile>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoTextShapeData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <kcomponentdata.h>
#include <KoTextDebug.h>
#include <KoListStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoText.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoInlineTextObjectManager.h>

typedef KoText::Tab KoTextTab;
// because in a QtScript, I don't seem to be able to use a namespaced type

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
           && actualFormat.property(KoCharacterStyle::FontCharset).toString()
                 == expectedFormat.property(KoCharacterStyle::FontCharset).toString()
           && actualFormat.font().styleHint() == expectedFormat.font().styleHint() // test this explicity since font == ignores it
           && actualFormat.foreground() == expectedFormat.foreground()
           && actualFormat.background() == expectedFormat.background()
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
           && actualFormat.fontCapitalization() == expectedFormat.fontCapitalization()
#endif
           && actualFormat.underlineColor() == expectedFormat.underlineColor()
           && actualFormat.textOutline() == expectedFormat.textOutline()
           && actualFormat.property(KoCharacterStyle::UnderlineStyle).toInt() 
                  == expectedFormat.property(KoCharacterStyle::UnderlineStyle).toInt()
           && actualFormat.property(KoCharacterStyle::UnderlineMode).toInt() 
                  == expectedFormat.property(KoCharacterStyle::UnderlineMode).toInt()
           && actualFormat.property(KoCharacterStyle::UnderlineWeight).toInt() 
                  == expectedFormat.property(KoCharacterStyle::UnderlineWeight).toInt()
           && actualFormat.property(KoCharacterStyle::UnderlineWidth).toDouble() 
                  == expectedFormat.property(KoCharacterStyle::UnderlineWidth).toDouble()
           && qvariant_cast<QColor>(actualFormat.property(KoCharacterStyle::StrikeOutColor))
                  == qvariant_cast<QColor>(expectedFormat.property(KoCharacterStyle::StrikeOutColor))
           && actualFormat.property(KoCharacterStyle::StrikeOutStyle).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutStyle).toInt()  
           && actualFormat.property(KoCharacterStyle::StrikeOutType).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutType).toInt() 
           && actualFormat.property(KoCharacterStyle::StrikeOutMode).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutMode).toInt() 
           && actualFormat.property(KoCharacterStyle::StrikeOutWeight).toInt() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutWeight).toInt()
           && actualFormat.property(KoCharacterStyle::StrikeOutWidth).toDouble() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutWidth).toDouble()
           && actualFormat.property(KoCharacterStyle::StrikeOutText).toString() 
                  == expectedFormat.property(KoCharacterStyle::StrikeOutText).toString()
           && actualFormat.property(KoCharacterStyle::Country).toString()
                  == expectedFormat.property(KoCharacterStyle::Country).toString()
           && actualFormat.property(KoCharacterStyle::Language).toString()
                  == expectedFormat.property(KoCharacterStyle::Language).toString() 
           && actualFormat.verticalAlignment() == expectedFormat.verticalAlignment(); // FIXME: Compare other properties
    
    if (!equal) {
        qDebug() << "Actual property:   " << KoTextDebug::textAttributes(actualFormat.properties());
        qDebug() << "Expected property: " << KoTextDebug::textAttributes(expectedFormat.properties());
        qDebug() << "compareFragment: property mismatch at " << actualFragment.text();
    }

    return equal;
}

static bool compareTabProperties(QVariant actualTabs, QVariant expectedTabs) {
    QList<QVariant> actualTabList = qvariant_cast<QList<QVariant> >(actualTabs);
    QList<QVariant> expectedTabList = qvariant_cast<QList<QVariant> >(expectedTabs);
    if (actualTabList.count() != expectedTabList.count())
        return false;
    for (int i = 0; i<actualTabList.count(); i++) {
        KoText::Tab actualTab = actualTabList[i].value<KoText::Tab>();
        KoText::Tab expectedTab = expectedTabList[i].value<KoText::Tab>();
        if (actualTab.position != expectedTab.position
            || actualTab.type != expectedTab.type
            || actualTab.delimiter != expectedTab.delimiter
            || actualTab.leaderType != expectedTab.leaderType
            || actualTab.leaderStyle != expectedTab.leaderStyle
            || actualTab.leaderColor != expectedTab.leaderColor
            || actualTab.leaderWeight != expectedTab.leaderWeight
            || actualTab.leaderWidth != expectedTab.leaderWidth
            || actualTab.leaderText != expectedTab.leaderText
            ) {
            return false;
        }
    }
    return true;
}

static bool compareBlockFormats(const QTextBlockFormat &actualFormat, const QTextBlockFormat &expectedFormat) {
    if (actualFormat.background() != expectedFormat.background()
        || actualFormat.alignment() != expectedFormat.alignment()
        || actualFormat.indent() != expectedFormat.indent()
        || actualFormat.textIndent() != expectedFormat.textIndent()
        || actualFormat.foreground() != expectedFormat.foreground()) {
        return false;
    }
    // check custom properties
    const QMap<int, QVariant> actualProperty = actualFormat.properties();
    const QMap<int, QVariant> expectedProperty = expectedFormat.properties();
    QList<int> allPropertyIds = actualProperty.keys();
    allPropertyIds << expectedProperty.keys();
    bool match = true;
    foreach(int id, allPropertyIds) {
      QString key, value;
        switch (id) {
        case KoParagraphStyle::AutoTextIndent:
        case KoParagraphStyle::DropCaps:
        case KoParagraphStyle::DropCapsLines:
        case KoParagraphStyle::DropCapsLength:
            if (actualProperty[id].toInt() != expectedProperty[id].toInt())
              match = false;
            break;
        case KoParagraphStyle::LeftBorderWidth:
        case KoParagraphStyle::TopBorderWidth:
        case KoParagraphStyle::RightBorderWidth:
        case KoParagraphStyle::BottomBorderWidth:
        case KoParagraphStyle::TabStopDistance:
        case KoParagraphStyle::DropCapsDistance:
        case QTextFormat::BlockLeftMargin:
        case QTextFormat::BlockRightMargin:
        case QTextFormat::BlockTopMargin:
        case QTextFormat::BlockBottomMargin:
            if (actualProperty[id].toDouble() != expectedProperty[id].toDouble())
              match = false;
            break;
        case KoParagraphStyle::TabPositions: 
            if (!compareTabProperties(actualProperty[id], expectedProperty[id]))
              match = false;
            break;
        }
        if (!match) {
            qDebug() << "Actual property:   " << KoTextDebug::paraAttributes(actualProperty);
            qDebug() << "Expected property: " << KoTextDebug::paraAttributes(expectedProperty);
            return false;
        }
    }
    return match;
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
        if (!compareBlockFormats(actualFormat, expectedFormat)) {
                qDebug() << "compareBlock: block properties mismatch at " << actualBlock.text();
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
        qDebug() << "compareBlock: Iterator are not in the end! at " << actualBlock.text() << expectedBlock.text();

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
Q_DECLARE_METATYPE(QTextFormat)
Q_DECLARE_METATYPE(QTextFormat *)

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
    QList<QVariant> qvlist;
    QList<KoText::Tab> tabList;
    if (arg.isNumber()) {
        // ### hack to detect if the number is of type int
        if ((qsreal)arg.toInt32() == arg.toNumber())
            value = arg.toInt32();
        else
            value = arg.toNumber();
        format->setProperty(id, value);
    } else if (arg.isArray()) {
        switch (id) {
        case KoParagraphStyle::TabPositions:
            qvlist.clear();
            tabList.clear();
            qScriptValueToSequence(arg, tabList);
            foreach(KoText::Tab tab, tabList) {
                QVariant v;
                v.setValue(tab);
                qvlist.append(v);
            }
            format->setProperty(id, qvlist);
            break;
        default:
            value = arg.toVariant();
            format->setProperty(id, value);
            break;
        }
        //FIXME: Ain't able to convert KoText::Tab->QVariant>QScriptValue 
        //       in QtScript and back to QScriptValue->QVariant->KoText::Tab
        //       in C++. If one can, there's no need for a switch-case here.
    } else {
        value = arg.toVariant();
        format->setProperty(id, value);
    }

    return QScriptValue();
}

Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextCharFormat *)

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

QScriptValue KoTextTabToQScriptValue(QScriptEngine *engine, const KoTextTab &tab)
{
  QScriptValue obj = engine->newObject();
  obj.setProperty("position", QScriptValue(engine, tab.position)); // double
  obj.setProperty("type", QScriptValue(engine, tab.type)); // enum
  obj.setProperty("delimiter", QScriptValue(engine, tab.delimiter)); // QChar
  obj.setProperty("leaderType", QScriptValue(engine, tab.leaderType)); // enum
  obj.setProperty("leaderStyle", QScriptValue(engine, tab.leaderStyle)); // enum
  obj.setProperty("leaderWeight", QScriptValue(engine, tab.leaderWeight)); // enum
  obj.setProperty("leaderWidth", QScriptValue(engine, tab.leaderWidth)); // double
  if (tab.leaderColor.isValid())
      obj.setProperty("leaderColor", QScriptValue(engine, tab.leaderColor.name())); // QColor
  else
      obj.setProperty("leaderColor", QScriptValue(engine, "invalid")); // QColor
  obj.setProperty("leaderText", QScriptValue(engine, tab.leaderText)); // QString
  return obj;
}

void QScriptValueToKoTextTab(const QScriptValue &obj, KoTextTab &tab)
{
  tab.position = obj.property("position").toNumber();
  tab.type = (
#if QT_VERSION >= KDE_MAKE_VERSION(4,4,0)
              QTextOption::
#else
              KoText::
#endif
                      TabType) obj.property("type").toInteger();
  tab.delimiter = obj.property("delimiter").toString()[0];
  tab.leaderType = (KoCharacterStyle::LineType) obj.property("leaderType").toInteger();
  tab.leaderStyle = (KoCharacterStyle::LineStyle) obj.property("leaderStyle").toInteger();
  tab.leaderWeight = (KoCharacterStyle::LineWeight) obj.property("leaderWeight").toInteger();
  tab.leaderWidth = obj.property("leaderWidth").toNumber();
  if (obj.property("leaderColor").toString() != "invalid")
      tab.leaderColor = QColor(obj.property("leaderColor").toString());
  tab.leaderText = obj.property("leaderText").toString();
}

QScriptValue constructKoTextTab(QScriptContext *, QScriptEngine *engine)
{
    return engine->toScriptValue(KoTextTab());
}
Q_DECLARE_METATYPE(QList<KoText::Tab>)

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

    globalObject.setProperty("KoTextTab", engine->newFunction(constructKoTextTab));
    qScriptRegisterMetaType(engine, KoTextTabToQScriptValue, QScriptValueToKoTextTab);
    qScriptRegisterSequenceMetaType< QList<KoText::Tab> > (engine);
}

void TestLoading::cleanupTestCase()
{
    delete engine;
    engine = 0;
}

// init/cleanup are called beginning and end of every test case
void TestLoading::init()
{
    // FIXME: the line below exists because I haven't manage get includeFunction to work
    evaluate(engine, QString(FILES_DATA_DIR) + "common.qs");
}

void TestLoading::cleanup()
{
}

void TestLoading::addData()
{
    QTest::newRow("bulletedList") << "TextContents/Lists/bulletedList";
    QTest::newRow("numberedList") << "TextContents/Lists/numberedList";
    QTest::newRow("embeddedBulletedList") << "TextContents/Lists/embeddedBulletedList";

    QTest::newRow("boldAndItalic") << "TextContents/TextFormatting/boldAndItalic";
    QTest::newRow("attributedText") << "TextContents/Paragraph/attributedText";
    QTest::newRow("basicContents") << "TextContents/Paragraph/basicContents";

    QTest::newRow("fontSize") << "TextContents/TextFormatting/fontSize";

    QTest::newRow("fontColors") << "TextContents/TextFormatting/fontColors";

    QTest::newRow("colors") << "FormattingProperties/TextFormattingProperties/color";
    QTest::newRow("country") << "FormattingProperties/TextFormattingProperties/country";
    QTest::newRow("fontCharset") << "FormattingProperties/TextFormattingProperties/fontCharacterSet";
    QTest::newRow("fontFamily") << "FormattingProperties/TextFormattingProperties/fontFamily";
#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
    QTest::newRow("fontFamilyGeneric") << "FormattingProperties/TextFormattingProperties/fontFamilyGeneric";
#endif
    QTest::newRow("fontName") << "FormattingProperties/TextFormattingProperties/fontName";
    QTest::newRow("fontPitch") << "FormattingProperties/TextFormattingProperties/fontPitch";
    QTest::newRow("fontSize") << "FormattingProperties/TextFormattingProperties/fontSize";
    QTest::newRow("fontStyle") << "FormattingProperties/TextFormattingProperties/fontStyle";
    QTest::newRow("fontWeight") << "FormattingProperties/TextFormattingProperties/fontWeight";
    QTest::newRow("fontVariant") << "FormattingProperties/TextFormattingProperties/fontVariant";
    QTest::newRow("language") << "FormattingProperties/TextFormattingProperties/language";
#if QT_VERSION >= KDE_MAKE_VERSION(4,5,0)
    QTest::newRow("letterKerning") << "FormattingProperties/TextFormattingProperties/letterKerning";
#endif
    QTest::newRow("lineThroughColor") << "FormattingProperties/TextFormattingProperties/lineThroughColor";
    QTest::newRow("lineThroughMode") << "FormattingProperties/TextFormattingProperties/lineThroughMode";
    QTest::newRow("lineThroughStyle") << "FormattingProperties/TextFormattingProperties/lineThroughStyle";
    QTest::newRow("lineThroughType") << "FormattingProperties/TextFormattingProperties/lineThroughType";
    QTest::newRow("lineThroughWidth") << "FormattingProperties/TextFormattingProperties/lineThroughWidth";
    QTest::newRow("lineThroughText") << "FormattingProperties/TextFormattingProperties/lineThroughText";
    QTest::newRow("relativeFontSize") << "FormattingProperties/TextFormattingProperties/relativeFontSize";
    QTest::newRow("textBackgroundColor") << "FormattingProperties/TextFormattingProperties/textBackgroundColor";
    QTest::newRow("textOutline") << "FormattingProperties/TextFormattingProperties/textOutline";
    QTest::newRow("textTranformations") << "FormattingProperties/TextFormattingProperties/textTransformations";
    QTest::newRow("underlineColor") << "FormattingProperties/TextFormattingProperties/underlineColor";
    QTest::newRow("underlineMode") << "FormattingProperties/TextFormattingProperties/underlineMode";
    QTest::newRow("underlineType") << "FormattingProperties/TextFormattingProperties/underlineType";
    QTest::newRow("underlineStyle") << "FormattingProperties/TextFormattingProperties/underlineStyle";
    QTest::newRow("underlineWidth") << "FormattingProperties/TextFormattingProperties/underlineWidth";

    QTest::newRow("autoTextIndent") << "FormattingProperties/ParagraphFormattingProperties/automaticTextIndent";
    QTest::newRow("border") << "FormattingProperties/ParagraphFormattingProperties/border";
    QTest::newRow("borderLineWidth") << "FormattingProperties/ParagraphFormattingProperties/borderLineWidth";
    QTest::newRow("dropCapsDistance") << "FormattingProperties/ParagraphFormattingProperties/dropCapsDistance";
    QTest::newRow("dropCapsLength") << "FormattingProperties/ParagraphFormattingProperties/dropCapsLength";
    QTest::newRow("dropCapsLines") << "FormattingProperties/ParagraphFormattingProperties/dropCapsLines";
    QTest::newRow("margin") << "FormattingProperties/ParagraphFormattingProperties/margin";
    QTest::newRow("marginLeftRight") << "FormattingProperties/ParagraphFormattingProperties/marginLeftRight";
    QTest::newRow("marginTopBottom") << "FormattingProperties/ParagraphFormattingProperties/marginTopBottom";
    QTest::newRow("paragraphBackground") << "FormattingProperties/ParagraphFormattingProperties/paragraphBackgroundColor";
    QTest::newRow("textAlign") << "FormattingProperties/ParagraphFormattingProperties/textAlign";
    QTest::newRow("textIndent") << "FormattingProperties/ParagraphFormattingProperties/textIndent";
    QTest::newRow("tabDelimiterChar") << "FormattingProperties/ParagraphFormattingProperties/tabDelimiterChar";
    QTest::newRow("tabLeaderColor") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderColor";
    QTest::newRow("tabLeaderStyle") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderStyle";
    QTest::newRow("tabLeaderText") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderText";
    QTest::newRow("tabLeaderType") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderType";
    QTest::newRow("tabLeaderWidth") << "FormattingProperties/ParagraphFormattingProperties/tabLeaderWidth";
    QTest::newRow("tabPosition") << "FormattingProperties/ParagraphFormattingProperties/tabPosition";
    QTest::newRow("tabStopDistance") << "FormattingProperties/ParagraphFormattingProperties/tabStopDistance";
    QTest::newRow("tabType") << "FormattingProperties/ParagraphFormattingProperties/tabType";
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

    KoStore *readStore = KoStore::createStore(odt, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store());
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0 /* KoShapeControllerBase (KWDocument) */);
    KoTextShapeData *textShapeData = new KoTextShapeData;
    QTextDocument *document = new QTextDocument;
    textShapeData->setDocument(document, false /* ownership */);
    KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
    textShapeData->document()->setDocumentLayout(layout);
    layout->setInlineObjectTextManager(new KoInlineTextObjectManager(layout)); // required while saving
    KoStyleManager *styleManager = new KoStyleManager;
    layout->setStyleManager(styleManager);
    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    delete readStore;
    delete textShapeData;
    return document;
}

QString TestLoading::documentToOdt(QTextDocument *document)
{
    QString odt("test.odt");
    if (QFile::exists(odt))
        QFile::remove(odt);
    QFile f(odt);
    f.open(QFile::WriteOnly);
    f.close();

    KoStore *store = KoStore::createStore(odt, KoStore::Write, "application/vnd.oasis.opendocument.text", KoStore::Zip);
    KoOdfWriteStore odfWriteStore(store);
    KoXmlWriter *manifestWriter = odfWriteStore.manifestWriter("application/vnd.oasis.opendocument.text");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    if (!store->open("content.xml"))
        return QString();

    KoStoreDevice contentDev(store);
    KoXmlWriter* contentWriter = KoOdfWriteStore::createOasisXmlWriter(&contentDev, "office:document-content");

    // for office:body
    KTemporaryFile contentTmpFile;
    if (!contentTmpFile.open())
        qFatal("Error opening temporary file!");
    KoXmlWriter xmlWriter(&contentTmpFile, 1);

    KoGenStyles mainStyles;
    KoEmbeddedDocumentSaver embeddedSaver;
    KoShapeSavingContext context(xmlWriter, mainStyles, embeddedSaver);

    xmlWriter.startElement("office:body");
    xmlWriter.startElement("office:text");

    KoTextShapeData *textShapeData = new KoTextShapeData;
    textShapeData->setDocument(document, false /* ownership */);
    KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
    textShapeData->document()->setDocumentLayout(layout);
    layout->setInlineObjectTextManager(new KoInlineTextObjectManager(layout)); // required while saving
    KoStyleManager *styleManager = new KoStyleManager;
    layout->setStyleManager(styleManager);
    textShapeData->saveOdf(context);

    xmlWriter.endElement(); // office:text
    xmlWriter.endElement(); // office:body

    contentTmpFile.close();

    mainStyles.saveOdfAutomaticStyles(contentWriter, false);
    contentWriter->addCompleteElement(&contentTmpFile);

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;

    if (!store->close())
        qWarning() << "Failed to close the store";

    odfWriteStore.closeManifestWriter();

    delete store;
    delete textShapeData;

    return odt;
}

void TestLoading::testLoading_data()
{
    QTest::addColumn<QString>("testcase");

    addData();
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

//    showDocument(actualDocument);
//    showDocument(expectedDocument);
    if (!documentsEqual) {
        KoTextDebug::dumpDocument(actualDocument);
        KoTextDebug::dumpDocument(expectedDocument);
    }
    delete actualDocument;
    delete expectedDocument;
    QVERIFY(documentsEqual);
}

void TestLoading::testSaving_data()
{
    QTest::addColumn<QString>("testcase");

    addData();
}

void TestLoading::testSaving()
{
    QFETCH(QString, testcase);
    testcase.prepend(FILES_DATA_DIR);

    QTextDocument *actualDocument = documentFromOdt(testcase + ".odt");
    QVERIFY(actualDocument != 0);
    QString fileName = documentToOdt(actualDocument);
    QVERIFY(!fileName.isEmpty());
    QTextDocument *savedDocument = documentFromOdt(fileName);

    QTextDocument *expectedDocument = documentFromScript(testcase + ".qs");
    QVERIFY(expectedDocument != 0);

    bool documentsEqual = compareDocuments(savedDocument, expectedDocument);

    if (!documentsEqual) {
        KoTextDebug::dumpDocument(actualDocument);
        KoTextDebug::dumpDocument(expectedDocument);
    }
    delete actualDocument;
    delete expectedDocument;
    QVERIFY(documentsEqual);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestLoading, argc, argv);
}

#include "TestLoading.moc"
