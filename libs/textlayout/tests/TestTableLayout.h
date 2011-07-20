/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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
#ifndef TESTTABLELAYOUT_H
#define TESTTABLELAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include <KoTextDocumentLayout.h>
#include <KoTextLayoutRootArea.h>
#include <KoTextLayoutTableArea.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class KoTableStyle;
class QTextDocument;
class QTextLayout;
class QTextTable;

#define ROUNDING 0.126

class TestTableLayout : public QObject
{
    Q_OBJECT
public:
    TestTableLayout() {}

private slots:
    void initTestCase();
    void cleanupTestCase();

    /**
     * make sure our private method setupTest() does what we think it does
     */
    void testSetupTest();

    /**
     * Test merged cells. In this testcase the table is modelled after a special case where we happened
     * to end in an infinite layout-loop in the past. The loop was caused by us not being able to place
     * content in a cell and therefore we keeped on to ask for new root-areas. This test should verify
     * exactly this case.
     */
    void testMergedCells();

    /**
     * If no column-width is defined then the available width should be distributed among
     * the available columns. Since the provided rootArea's have a width of 200 and we
     * have 3 columns it is expected that every of the columns has a width of 200/3.
     */
    void testColumnWidthUndefined();

    /**
     * If the column-width is explicit defined then we expect that those widths are used.
     */
    void testColumnWidthFixed();

    /**
     * Test fixed column-width of zero.
     */
    void testColumnWidthFixedZero();

    /**
     * If the table-width is not defined then the table get's the width of it's parent
     * what is the rootArea in our case. If now the defined fixed column-width's are
     * in total larger then those table-width then they need to be shrink proportional
     * to match into the available table-width.
     */
    void testColumnWidthFixedShrink();

    /**
     * Test relative column-width.
     */
    void testColumnWidthRelative();

    /**
      * Test fixed row height. The row is smaller then the text that is within the cells. Expected
      * is that the resulting table will still match to the available space and won't be spread
      * over multiple pages. This tests for bug #275409.
      */
    void testRowHeightFixed();

    /**
     * Test minimum row-height.
     */
    void testRowHeightMinimum();

private:
    QTextCursor setupTest();
    void setupTest(const QString &mergedText, const QString &topRightText, const QString &midRightText, const QString &bottomLeftText, const QString &bottomMidText, const QString &bottomRightText, KoTableStyle* tableStyle = 0);

private:
    QTextDocument *m_doc;
    KoTextDocumentLayout *m_layout;
    QTextBlock m_block;
    QTextBlock mergedCellBlock() const;
    QTextBlock topRightCellBlock() const;
    QTextBlock midRightCellBlock() const;
    QTextBlock bottomLeftCellBlock() const;
    QTextBlock bottomMidCellBlock() const;
    QTextBlock bottomRightCellBlock() const;
    QString m_loremIpsum;
    KoStyleManager *m_styleManager;
    QTextTable *m_table;
};

#endif
