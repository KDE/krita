/*
 *  This file is part of KOffice tests
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

    /// make sure our private method setupTest() does what we think it does
    void testSetupTest();

    /// If not column-width is defined then the available width should be distributed among the available columns.
    void testColumnWidthUndefined();
    /// If the column-width is explicit defined then we expect that those widths are used.
    void testColumnWidthFixed();
    /// Test fixed column-width of zero.
    void testColumnWidthFixedZero();
    /// Test relative column-width.
    void testColumnWidthRelative();

private:
    void setupTest(const QString &mergedText, const QString &topRightText, const QString &midRightText, const QString &bottomLeftText, const QString &bottomMidText, const QString &bottomRightText);

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
    KoTextLayoutRootArea *m_area;
    QTextTable *m_table;
};

#endif
