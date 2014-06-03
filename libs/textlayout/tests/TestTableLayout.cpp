/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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
#include "TestTableLayout.h"
#include "MockRootAreaProvider.h"

#include <KoParagraphStyle.h>
#include <KoListStyle.h>
#include <KoListLevelProperties.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTableColumnStyle.h>
#include <KoTableRowStyle.h>
#include <KoTableStyle.h>
#include <KoTextDebug.h>

#include <QtGui>
#include <QTextCursor>

#include <kdebug.h>
#include <kcomponentdata.h>

#define FRAME_SPACING 10.0


void TestTableLayout::initTestCase()
{
    m_doc = 0;
    m_layout = 0;

    m_loremIpsum = QString("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");
}

void TestTableLayout::cleanupTestCase()
{
    delete m_doc;
}

QTextCursor TestTableLayout::setupTest()
{
    delete m_doc;
    m_doc = new QTextDocument;
    Q_ASSERT(m_doc);

    MockRootAreaProvider *provider = new MockRootAreaProvider();
    Q_ASSERT(provider);
    KoTextDocument(m_doc).setInlineTextObjectManager(new KoInlineTextObjectManager);

    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false)); //do it manually since we do not load the appDefaultStyle

    m_styleManager = new KoStyleManager(0);
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    m_layout = new KoTextDocumentLayout(m_doc, provider);
    Q_ASSERT(m_layout);
    m_doc->setDocumentLayout(m_layout);

    m_block = m_doc->begin();
    QTextCursor cursor(m_doc);

    return cursor;
}

void TestTableLayout::setupTest(const QString &mergedText, const QString &topRightText, const QString &midRightText, const QString &bottomLeftText, const QString &bottomMidText, const QString &bottomRightText, KoTableStyle* tableStyle)
{
    QTextCursor cursor = setupTest();

    KoParagraphStyle style;
    style.setStyleId(101); // needed to do manually since we don't use the stylemanager
    style.applyStyle(m_block);
    QTextTableFormat tableFormat;
    if (tableStyle)
        tableStyle->applyStyle(tableFormat);

    m_table = cursor.insertTable(3,3,tableFormat);
    m_table->mergeCells(0,0,2,2);
    if (mergedText.length() > 0) {
        m_table->cellAt(0,0).firstCursorPosition().insertText(mergedText);
        QTextBlock b2 = m_table->cellAt(0,0).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (topRightText.length() > 0) {
        m_table->cellAt(0,2).firstCursorPosition().insertText(topRightText);
        QTextBlock b2 = m_table->cellAt(0,2).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (midRightText.length() > 0) {
        m_table->cellAt(1,2).firstCursorPosition().insertText(midRightText);
        QTextBlock b2 = m_table->cellAt(1,2).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomLeftText.length() > 0) {
        m_table->cellAt(2,0).firstCursorPosition().insertText(bottomLeftText);
        QTextBlock b2 = m_table->cellAt(2,0).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomMidText.length() > 0) {
        m_table->cellAt(2,1).firstCursorPosition().insertText(bottomMidText);
        QTextBlock b2 = m_table->cellAt(2,1).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomRightText.length() > 0) {
        m_table->cellAt(2,2).firstCursorPosition().insertText(bottomRightText);
        QTextBlock b2 = m_table->cellAt(2,2).firstCursorPosition().block();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
}

QTextBlock TestTableLayout::mergedCellBlock() const
{
    return m_table->cellAt(0,0).firstCursorPosition().block();
}

QTextBlock TestTableLayout::topRightCellBlock() const
{
    return m_table->cellAt(0,2).firstCursorPosition().block();
}

QTextBlock TestTableLayout::midRightCellBlock() const
{
    return m_table->cellAt(1,2).firstCursorPosition().block();
}

QTextBlock TestTableLayout::bottomLeftCellBlock() const
{
    return m_table->cellAt(2,0).firstCursorPosition().block();
}

QTextBlock TestTableLayout::bottomMidCellBlock() const
{
    return m_table->cellAt(2,1).firstCursorPosition().block();
}

QTextBlock TestTableLayout::bottomRightCellBlock() const
{
    return m_table->cellAt(2,2).firstCursorPosition().block();
}
    
void TestTableLayout::testSetupTest()
{
    setupTest("m","02","12","20","21","22");
    m_layout->layout();
    QCOMPARE(mergedCellBlock().text(), QString("m"));
    QCOMPARE(topRightCellBlock().text(), QString("02"));
    QCOMPARE(midRightCellBlock().text(), QString("12"));
    QCOMPARE(bottomLeftCellBlock().text(), QString("20"));
    QCOMPARE(bottomMidCellBlock().text(), QString("21"));
    QCOMPARE(bottomRightCellBlock().text(), QString("22"));
}

void TestTableLayout::testMergedCells()
{
    QTextCursor cursor = setupTest();

    m_table = cursor.insertTable(3, 5);
    m_table->mergeCells(0,0,2,2);
    m_table->mergeCells(0,2,3,1);
    m_table->mergeCells(0,3,3,1);
    m_table->mergeCells(0,4,3,1);
    m_table->mergeCells(2,0,1,1);
    m_table->mergeCells(2,1,1,1);

    m_layout->layout();

    QVERIFY(!dynamic_cast<MockRootAreaProvider*>(m_layout->provider())->m_askedForMoreThenOneArea);
}

void TestTableLayout::testColumnWidthUndefined()
{
    setupTest("","","","","","");
    m_layout->layout();

    QCOMPARE(m_table->columns(), 3);
    QCOMPARE(m_table->rows(), 3);
    QVERIFY(qAbs(m_block.layout()->lineAt(0).width() - 200.0) < ROUNDING);
    QTextLayout *lay = bottomLeftCellBlock().layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
    lay = bottomMidCellBlock().layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
    lay = bottomRightCellBlock().layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
}

void TestTableLayout::testColumnWidthFixed()
{
    KoTableStyle *tableStyle = new KoTableStyle;
    tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 150.0));

    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text", tableStyle);
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    KoTableColumnStyle column1style;
    column1style.setColumnWidth(2.3);
    styleManager.setColumnStyle(0, column1style);

    KoTableColumnStyle column2style;
    column2style.setColumnWidth(122.5);
    styleManager.setColumnStyle(1, column2style);

    KoTableColumnStyle column3style;
    column3style.setColumnWidth(362.9);
    styleManager.setColumnStyle(2, column3style);

    m_layout->layout();

    QVERIFY(qAbs(QTextCursor(m_table->parentFrame()).block().layout()->lineAt(0).width() - 200.0) < ROUNDING); // table should grow to 200
    QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).width() - 124.8) < ROUNDING);
    QVERIFY(qAbs(topRightCellBlock().layout()->lineAt(0).width() - 362.9) < ROUNDING);
    QVERIFY(qAbs(bottomLeftCellBlock().layout()->lineAt(0).width() - 2.3) < ROUNDING);
    QVERIFY(qAbs(bottomMidCellBlock().layout()->lineAt(0).width() - 122.5) < ROUNDING);
    QVERIFY(qAbs(bottomRightCellBlock().layout()->lineAt(0).width() - 362.9) < ROUNDING);
}

void TestTableLayout::testColumnWidthFixedZero()
{
    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text");
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    KoTableColumnStyle column1style;
    column1style.setColumnWidth(0.0);
    styleManager.setColumnStyle(0, column1style);

    KoTableColumnStyle column2style;
    column2style.setColumnWidth(120.5);
    styleManager.setColumnStyle(1, column2style);

    KoTableColumnStyle column3style;
    column3style.setColumnWidth(0.1);
    styleManager.setColumnStyle(2, column3style);

    m_layout->layout();

    QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).width() - 120.5) < ROUNDING);
    QVERIFY(qAbs(topRightCellBlock().layout()->lineAt(0).width() - 0.1) < ROUNDING);
    //FIXME QVERIFY(qAbs(bottomLeftCellBlock().layout()->lineAt(0).width() - 0.0) < ROUNDING);
    QVERIFY(qAbs(bottomMidCellBlock().layout()->lineAt(0).width() - 120.5) < ROUNDING);
    //FIXME QVERIFY(qAbs(bottomRightCellBlock().layout()->lineAt(0).width() - 0.0) < ROUNDING);
}

void TestTableLayout::testColumnWidthFixedShrink()
{
    KoTableStyle *tableStyle = new KoTableStyle;
    //tableStyle->setWidth(QTextLength(QTextLength::FixedLength, 200.0)); // no table-width defined

    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text", tableStyle);
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    KoTableColumnStyle column1style;
    column1style.setColumnWidth(2.3);
    styleManager.setColumnStyle(0, column1style);

    KoTableColumnStyle column2style;
    column2style.setColumnWidth(122.5);
    styleManager.setColumnStyle(1, column2style);

    KoTableColumnStyle column3style;
    column3style.setColumnWidth(362.9);
    styleManager.setColumnStyle(2, column3style);

    m_layout->layout();

    QVERIFY(qAbs(QTextCursor(m_table->parentFrame()).block().layout()->lineAt(0).width() - 200.0) < ROUNDING); // table should grow to 200
    QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).width() - 26.5938) < ROUNDING);
    QVERIFY(qAbs(topRightCellBlock().layout()->lineAt(0).width() - 267) < ROUNDING);
    QVERIFY(qAbs(bottomMidCellBlock().layout()->lineAt(0).width() - 26.5938) < ROUNDING);
    QVERIFY(qAbs(bottomRightCellBlock().layout()->lineAt(0).width() - 267) < ROUNDING);
}

void TestTableLayout::testColumnWidthRelative()
{
    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text");
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    KoTableColumnStyle column1style;
    column1style.setRelativeColumnWidth(0.2);
    styleManager.setColumnStyle(0, column1style);

    KoTableColumnStyle column2style;
    column2style.setRelativeColumnWidth(0.5);
    styleManager.setColumnStyle(1, column2style);

    KoTableColumnStyle column3style;
    column3style.setRelativeColumnWidth(0.1);
    styleManager.setColumnStyle(2, column3style);

    m_layout->layout();

    QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).width() - 200.0*0.2 - 200.0*0.5) < ROUNDING);
    QVERIFY(qAbs(topRightCellBlock().layout()->lineAt(0).width() - 200.0*0.1) < ROUNDING);
    QVERIFY(qAbs(bottomLeftCellBlock().layout()->lineAt(0).width() - 200.0*0.2) < ROUNDING);
    QVERIFY(qAbs(bottomMidCellBlock().layout()->lineAt(0).width() - 200.0*0.5) < ROUNDING);
    QVERIFY(qAbs(bottomRightCellBlock().layout()->lineAt(0).width() - 200.0*0.1) < ROUNDING);
}

void TestTableLayout::testRowHeightFixed()
{
    KoTableStyle *tableStyle = new KoTableStyle;

    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text", tableStyle);
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    KoTableRowStyle row1style;
    row1style.setRowHeight(3.2);
    styleManager.setRowStyle(1, row1style);

    m_layout->layout();

    QVERIFY(!dynamic_cast<MockRootAreaProvider*>(m_layout->provider())->m_askedForMoreThenOneArea);
    //QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).height() - 14) < ROUNDING);
}

void TestTableLayout::testRowHeightMinimum()
{
    KoTableStyle *tableStyle = new KoTableStyle;

    setupTest("merged text", "top right text", "mid right text", "bottom left text", "bottom mid text", "bottom right text", tableStyle);
    KoTableColumnAndRowStyleManager styleManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    KoTableRowStyle row1style;
    row1style.setMinimumRowHeight(3.2);
    styleManager.setRowStyle(1, row1style);

    m_layout->layout();

    QVERIFY(!dynamic_cast<MockRootAreaProvider*>(m_layout->provider())->m_askedForMoreThenOneArea);
    //QVERIFY(qAbs(mergedCellBlock().layout()->lineAt(0).height() - 14) < ROUNDING);
}

QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
