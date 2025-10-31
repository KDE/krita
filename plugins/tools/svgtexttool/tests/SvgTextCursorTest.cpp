/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "SvgTextCursorTest.h"

#include <SvgTextCursor.h>
#include <SvgTextInsertCommand.h>
#include <SvgTextRemoveCommand.h>

#include <KoSvgTextShape.h>
#include <KoSvgTextShapeMarkupConverter.h>
#include <KoFontRegistry.h>

#include <tests/MockShapes.h>
#include <simpletest.h>
#include <testui.h>
void SvgTextCursorTest::initTestCase()
{
    QString fileName = QString(FILES_DATA_DIR) + '/' + "DejaVuSans.ttf";
    bool res = KoFontRegistry::instance()->addFontFilePathToRegistry(fileName);

    QVERIFY2(res, QString("KoFontRegistry could not add the test font %1").arg(fileName).toLatin1());
}

void SvgTextCursorTest::test_ltr_data()
{
    QTest::addColumn<SvgTextCursor::MoveMode>("mode");
    QTest::addColumn<bool>("visual");
    QTest::addColumn<int>("result");

    QTest::addRow("down")  << SvgTextCursor::MoveDown << false << 16;
    QTest::addRow("up  ")  << SvgTextCursor::MoveUp << false  << 0;
    QTest::addRow("left")  << SvgTextCursor::MoveLeft << false  << 4;
    QTest::addRow("right") << SvgTextCursor::MoveRight << false  << 6;
    QTest::addRow("word left")  << SvgTextCursor::MoveWordLeft << false  << 4;
    QTest::addRow("word right") << SvgTextCursor::MoveWordRight << false  << 9;
    QTest::addRow("line end")   << SvgTextCursor::MoveLineEnd << false  << 10;
    QTest::addRow("line start") << SvgTextCursor::MoveLineStart << false  << 0;
    QTest::addRow("paragraph end")   << SvgTextCursor::ParagraphEnd << false  << 48;
    QTest::addRow("paragraph start") << SvgTextCursor::ParagraphStart << false  << 0;
    QTest::addRow("visual - down")  << SvgTextCursor::MoveDown << true << 16;
    QTest::addRow("visual - up  ")  << SvgTextCursor::MoveUp << true  << 0;
    QTest::addRow("visual - left")  << SvgTextCursor::MoveLeft << true  << 4;
    QTest::addRow("visual - right") << SvgTextCursor::MoveRight << true  << 6;
    QTest::addRow("visual - word left")  << SvgTextCursor::MoveWordLeft << true  << 4;
    QTest::addRow("visual - word right") << SvgTextCursor::MoveWordRight << true  << 9;
    QTest::addRow("visual - line end")   << SvgTextCursor::MoveLineEnd << true  << 10;
    QTest::addRow("visual - line start") << SvgTextCursor::MoveLineStart << true  << 0;
    QTest::addRow("visual - paragraph end")   << SvgTextCursor::ParagraphEnd << true  << 48;
    QTest::addRow("visual - paragraph start") << SvgTextCursor::ParagraphStart << true  << 0;
}

void SvgTextCursorTest::test_ltr()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0;font-family:Deja Vu Sans\">The quick brown fox jumps over the lazy dog.</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    MockCanvas canvas;
    SvgTextCursor cursor(&canvas);
    cursor.setShape(textShape);

    QFETCH(SvgTextCursor::MoveMode, mode);
    QFETCH(bool, visual);
    QFETCH(int, result);
    cursor.setVisualMode(visual);

    //The current textcursor sets the pos to the text end...
    //QCOMPARE(cursor.getPos(), 0);
    cursor.setPos(5, 5);

    cursor.moveCursor(mode);
    QCOMPARE(cursor.getPos(), result);
}

void SvgTextCursorTest::test_rtl_data()
{
    QTest::addColumn<SvgTextCursor::MoveMode>("mode");
    QTest::addColumn<bool>("visual");
    QTest::addColumn<int>("result");

    QTest::addRow("down")  << SvgTextCursor::MoveDown << false << 22;
    QTest::addRow("up  ")  << SvgTextCursor::MoveUp << false  << 6;
    QTest::addRow("left")  << SvgTextCursor::MoveLeft << false  << 11;
    QTest::addRow("right") << SvgTextCursor::MoveRight << false  << 9;
    QTest::addRow("word left")  << SvgTextCursor::MoveWordLeft << false  << 11;
    QTest::addRow("word right") << SvgTextCursor::MoveWordRight << false  << 8;
    QTest::addRow("line end")   << SvgTextCursor::MoveLineEnd << false  << 16;
    QTest::addRow("line start") << SvgTextCursor::MoveLineStart << false  << 8;
    QTest::addRow("paragraph end")   << SvgTextCursor::ParagraphEnd << false  << 33;
    QTest::addRow("paragraph start") << SvgTextCursor::ParagraphStart << false  << 0;
    QTest::addRow("visual -down")  << SvgTextCursor::MoveDown << true << 22;
    QTest::addRow("visual - up  ")  << SvgTextCursor::MoveUp << true  << 6;
    QTest::addRow("visual - left")  << SvgTextCursor::MoveLeft << true  << 9;
    QTest::addRow("visual - right") << SvgTextCursor::MoveRight << true  << 11;
    QTest::addRow("visual - word left")  << SvgTextCursor::MoveWordLeft << true  << 11;
    QTest::addRow("visual - word right") << SvgTextCursor::MoveWordRight << true  << 8;
    QTest::addRow("visual - line end")   << SvgTextCursor::MoveLineEnd << true  << 16;
    QTest::addRow("visual - line start") << SvgTextCursor::MoveLineStart << true  << 8;
    QTest::addRow("visual - paragraph end")   << SvgTextCursor::ParagraphEnd << true  << 33;
    QTest::addRow("visual - paragraph start") << SvgTextCursor::ParagraphStart << true  << 0;
}

void SvgTextCursorTest::test_rtl()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0; direction:rtl; font-family:Deja Vu Sans\">داستان SVG 1.1 SE طولا ني است.</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    MockCanvas canvas;
    SvgTextCursor cursor(&canvas);
    cursor.setShape(textShape);

    QFETCH(SvgTextCursor::MoveMode, mode);
    QFETCH(bool, visual);
    QFETCH(int, result);
    cursor.setVisualMode(visual);

    //The current textcursor sets the pos to the text end...
    //QCOMPARE(cursor.getPos(), 0);
    cursor.setPos(10, 10);

    cursor.moveCursor(mode);
    QCOMPARE(cursor.getPos(), result);
}
void SvgTextCursorTest::test_ttb_rl_data()
{
    QTest::addColumn<SvgTextCursor::MoveMode>("mode");
    QTest::addColumn<bool>("visual");
    QTest::addColumn<int>("result");

    QTest::addRow("down")  << SvgTextCursor::MoveDown << false << 6;
    QTest::addRow("up  ")  << SvgTextCursor::MoveUp << false  << 4;
    QTest::addRow("left")  << SvgTextCursor::MoveLeft << false  << 12;
    QTest::addRow("right") << SvgTextCursor::MoveRight << false  << 0;
    QTest::addRow("word left")  << SvgTextCursor::MoveWordLeft << false  << 12;
    QTest::addRow("word right") << SvgTextCursor::MoveWordRight << false  << 0;
    QTest::addRow("line end")   << SvgTextCursor::MoveLineEnd << false  << 6;
    QTest::addRow("line start") << SvgTextCursor::MoveLineStart << false  << 0;
    QTest::addRow("paragraph end")   << SvgTextCursor::ParagraphEnd << false  << 36;
    QTest::addRow("paragraph start") << SvgTextCursor::ParagraphStart << false  << 0;
    QTest::addRow("visual - down")  << SvgTextCursor::MoveDown << true << 6;
    QTest::addRow("visual - up  ")  << SvgTextCursor::MoveUp << true  << 4;
    QTest::addRow("visual - left")  << SvgTextCursor::MoveLeft << true  << 12;
    QTest::addRow("visual - right") << SvgTextCursor::MoveRight << true  << 0;
    QTest::addRow("visual - word left")  << SvgTextCursor::MoveWordLeft << true  << 12;
    QTest::addRow("visual - word right") << SvgTextCursor::MoveWordRight << true  << 0;
    QTest::addRow("visual - line end")   << SvgTextCursor::MoveLineEnd << true  << 6;
    QTest::addRow("visual - line start") << SvgTextCursor::MoveLineStart << true  << 0;
    QTest::addRow("visual - paragraph end")   << SvgTextCursor::ParagraphEnd << true  << 36;
    QTest::addRow("visual - paragraph start") << SvgTextCursor::ParagraphStart << true  << 0;
}

void SvgTextCursorTest::test_ttb_rl()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0; writing-mode:vertical-rl; font-family:Deja Vu Sans\">A B C D E F G H I J K L M N O P</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    MockCanvas canvas;
    SvgTextCursor cursor(&canvas);
    cursor.setShape(textShape);

    QFETCH(SvgTextCursor::MoveMode, mode);
    QFETCH(bool, visual);
    QFETCH(int, result);
    cursor.setVisualMode(visual);

    //The current textcursor sets the pos to the text end...
    //QCOMPARE(cursor.getPos(), 0);
    cursor.setPos(5, 5);

    cursor.moveCursor(mode);
    QCOMPARE(cursor.getPos(), result);
}
void SvgTextCursorTest::test_ttb_lr_data()
{
    QTest::addColumn<SvgTextCursor::MoveMode>("mode");
    QTest::addColumn<bool>("visual");
    QTest::addColumn<int>("result");

    QTest::addRow("down")  << SvgTextCursor::MoveDown << false << 6;
    QTest::addRow("up  ")  << SvgTextCursor::MoveUp << false  << 4;
    QTest::addRow("left")  << SvgTextCursor::MoveLeft << false  << 0;
    QTest::addRow("right") << SvgTextCursor::MoveRight << false  << 12;
    QTest::addRow("word left")  << SvgTextCursor::MoveWordLeft << false  << 0;
    QTest::addRow("word right") << SvgTextCursor::MoveWordRight << false  << 12;
    QTest::addRow("line end")   << SvgTextCursor::MoveLineEnd << false  << 6;
    QTest::addRow("line start") << SvgTextCursor::MoveLineStart << false  << 0;
    QTest::addRow("paragraph end")   << SvgTextCursor::ParagraphEnd << false  << 36;
    QTest::addRow("paragraph start") << SvgTextCursor::ParagraphStart << false  << 0;
    QTest::addRow("visual - down")  << SvgTextCursor::MoveDown << true << 6;
    QTest::addRow("visual - up  ")  << SvgTextCursor::MoveUp << true  << 4;
    QTest::addRow("visual - left")  << SvgTextCursor::MoveLeft << true  << 0;
    QTest::addRow("visual - right") << SvgTextCursor::MoveRight << true  << 12;
    QTest::addRow("visual - word left")  << SvgTextCursor::MoveWordLeft << true  << 0;
    QTest::addRow("visual - word right") << SvgTextCursor::MoveWordRight << true  << 12;
    QTest::addRow("visual - line end")   << SvgTextCursor::MoveLineEnd << true  << 6;
    QTest::addRow("visual - line start") << SvgTextCursor::MoveLineStart << true  << 0;
    QTest::addRow("visual - paragraph end")   << SvgTextCursor::ParagraphEnd << true  << 36;
    QTest::addRow("visual - paragraph start") << SvgTextCursor::ParagraphStart << true  << 0;
}

void SvgTextCursorTest::test_ttb_lr()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0; writing-mode:vertical-lr; font-family:Deja Vu Sans\">A B C D E F G H I J K L M N O P</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    MockCanvas canvas;
    SvgTextCursor cursor(&canvas);
    cursor.setShape(textShape);

    QFETCH(SvgTextCursor::MoveMode, mode);
    QFETCH(bool, visual);
    QFETCH(int, result);
    cursor.setVisualMode(visual);

    //The current textcursor sets the pos to the text end...
    //QCOMPARE(cursor.getPos(), 0);
    cursor.setPos(5, 5);

    cursor.moveCursor(mode);
    QCOMPARE(cursor.getPos(), result);
}

// Test basic text insertion in a horizontal ltr wrapped text;
void SvgTextCursorTest::test_text_insert_command()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0;font-family:Deja Vu Sans\">The quick brown fox jumps over the lazy dog.</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    int pos = textShape->posForIndex(25, false, true);
    SvgTextInsertCommand *cmd = new SvgTextInsertCommand(textShape, pos, pos, " badly");
    QString test = "The quick brown fox jumps over the lazy dog.";
    QString test2 = test;
    test.insert(25, " badly");
    cmd->redo();
    QCOMPARE(test, textShape->plainText());

    cmd->undo();
    QCOMPARE(test2, textShape->plainText());
}

// Test basic text removal in a horizontal ltr wrapped text;
void SvgTextCursorTest::test_text_remove_command()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0;font-family:Deja Vu Sans\">The quick brown fox jumps over the lazy dog.</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);
    QString test = "The quick brown fox jumps over the lazy dog.";
    QString test2 = test;

    SvgTextRemoveCommand *cmd = new SvgTextRemoveCommand(textShape, 15, 10, 10, 5);
    test.remove(10, 5);

    cmd->redo();
    QCOMPARE(test, textShape->plainText());

    cmd->undo();
    QCOMPARE(test2, textShape->plainText());
}

void SvgTextCursorTest::test_text_remove_dedicated_data()
{
    QTest::addColumn<int>("pos");
    QTest::addColumn<SvgTextCursor::MoveMode>("mode1");
    QTest::addColumn<SvgTextCursor::MoveMode>("mode2");
    QTest::addColumn<int>("length");

    QTest::addRow("backspace") << 11 << SvgTextCursor::MovePreviousChar << SvgTextCursor::MoveNone << 1;
    QTest::addRow("delete") << 10 << SvgTextCursor::MoveNone << SvgTextCursor::MoveNextChar << 1;
    QTest::addRow("start-of-word") << 5 << SvgTextCursor::MoveWordStart << SvgTextCursor::MoveNone << 1;
    QTest::addRow("end-of-word") << 5 << SvgTextCursor::MoveNone << SvgTextCursor::MoveWordEnd << 4;
    QTest::addRow("end-of-line") << 5 << SvgTextCursor::MoveNone << SvgTextCursor::MoveLineEnd << 5;
    QTest::addRow("delete-line") << 5 << SvgTextCursor::MoveLineStart << SvgTextCursor::MoveLineEnd << 10;
}

void SvgTextCursorTest::test_text_remove_dedicated()
{
    KoSvgTextShape *textShape = new KoSvgTextShape();
    QString ref ("<text style=\"inline-size:50.0; font-size:10.0;font-family:Deja Vu Sans\">The quick brown fox jumps over the lazy dog.</text>");
    KoSvgTextShapeMarkupConverter converter(textShape);
    converter.convertFromSvg(ref, QString(), QRectF(0, 0, 300, 300), 72.0);

    QFETCH(int, pos);
    QFETCH(SvgTextCursor::MoveMode, mode1);
    QFETCH(SvgTextCursor::MoveMode, mode2);
    QFETCH(int, length);

    MockCanvas canvas;
    SvgTextCursor cursor(&canvas);
    cursor.setShape(textShape);
    cursor.setPos(pos, pos);
    cursor.moveCursor(mode1);
    int posA = textShape->indexForPos(cursor.getPos());
    cursor.setPos(pos, pos);
    cursor.moveCursor(mode2);
    int posB = textShape->indexForPos(cursor.getPos());
    int posStart = qMin(posA, posB);
    int posEnd = qMax(posA, posB);

    QCOMPARE(posEnd - posStart, length);

}

SIMPLE_TEST_MAIN(SvgTextCursorTest)
