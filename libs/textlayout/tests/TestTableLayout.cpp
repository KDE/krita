/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

void TestTableLayout::setupTest(const QString &mergedText, const QString &topRightText, const QString &midRightText, const QString &bottomLeftText, const QString &bottomMidText, const QString &bottomRightText)
{
    m_doc = new QTextDocument;
    Q_ASSERT(m_doc);

    MockRootAreaProvider *provider = new MockRootAreaProvider();
    Q_ASSERT(provider);
    KoTextDocument(m_doc).setInlineTextObjectManager(new KoInlineTextObjectManager);

    m_doc->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false)); //do it manually since we do not load the appDefaultStyle

    m_styleManager = new KoStyleManager();
    KoTextDocument(m_doc).setStyleManager(m_styleManager);

    m_layout = new KoTextDocumentLayout(m_doc, provider);
    Q_ASSERT(m_layout);
    m_doc->setDocumentLayout(m_layout);

    m_area = provider->provide(0, m_layout);

    m_block = m_doc->begin();
    QTextCursor cursor(m_doc);
    KoParagraphStyle style;
    style.setStyleId(101); // needed to do manually since we don't use the stylemanager
    style.applyStyle(m_block);
    QTextTable *m_table = cursor.insertTable(3,3);
    m_table->mergeCells(0,0,2,2);
    if (mergedText.length() > 0) {
        m_table->cellAt(0,0).firstCursorPosition().insertText(mergedText);
        m_mergedCellBlock = m_table->cellAt(0,0).firstCursorPosition().block();
        QTextBlock b2 = m_mergedCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (topRightText.length() > 0) {
        m_table->cellAt(0,2).firstCursorPosition().insertText(topRightText);
        m_topRightCellBlock = m_table->cellAt(0,2).firstCursorPosition().block();
        QTextBlock b2 = m_topRightCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (midRightText.length() > 0) {
        m_table->cellAt(1,2).firstCursorPosition().insertText(midRightText);
        m_midRightCellBlock = m_table->cellAt(1,2).firstCursorPosition().block();
        QTextBlock b2 = m_midRightCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomLeftText.length() > 0) {
        m_table->cellAt(2,0).firstCursorPosition().insertText(bottomLeftText);
        m_bottomLeftCellBlock = m_table->cellAt(2,0).firstCursorPosition().block();
        QTextBlock b2 = m_bottomLeftCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomMidText.length() > 0) {
        m_table->cellAt(2,1).firstCursorPosition().insertText(bottomMidText);
        m_bottomMidCellBlock = m_table->cellAt(2,1).firstCursorPosition().block();
        QTextBlock b2 = m_bottomMidCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    if (bottomRightText.length() > 0) {
        m_table->cellAt(2,2).firstCursorPosition().insertText(bottomRightText);
        m_bottomRightCellBlock = m_table->cellAt(2,2).firstCursorPosition().block();
        QTextBlock b2 = m_bottomRightCellBlock;
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
}


void TestTableLayout::testSetupTest()
{
    setupTest("m","02","12","20","21","22");
    m_layout->layout();
    QCOMPARE(m_mergedCellBlock.text(), QString("m"));
    QCOMPARE(m_topRightCellBlock.text(), QString("02"));
    QCOMPARE(m_midRightCellBlock.text(), QString("12"));
    QCOMPARE(m_bottomLeftCellBlock.text(), QString("20"));
    QCOMPARE(m_bottomMidCellBlock.text(), QString("21"));
    QCOMPARE(m_bottomRightCellBlock.text(), QString("22"));
}

void TestTableLayout::testColumnLayout()
{
    QTextLayout *lay;
    
    setupTest("","","","","","");
    m_layout->layout();
    lay = m_bottomLeftCellBlock.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
    lay = m_bottomMidCellBlock.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
    lay = m_bottomRightCellBlock.layout();
    QVERIFY(lay);
    QCOMPARE(lay->lineCount(), 1);
    QVERIFY(qAbs(lay->lineAt(0).width() - 200.0/3) < ROUNDING);
}


QTEST_KDEMAIN(TestTableLayout, GUI)

#include <TestTableLayout.moc>
