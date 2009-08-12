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

#ifndef TESTLOADING_H
#define TESTLOADING_H

#include <QObject>

class QScriptEngine;
class QTextDocument;
class QTextEdit;
class QTextFragment;
class QTextFrame;
class QTextBlock;
class QTextTable;
class QTextTableCell;
class QTextBlockFormat;
class QTextListFormat;
class KoStore;
class KoTextShapeData;
class KComponentData;

class TestLoading : public QObject
{
    Q_OBJECT
public:
    TestLoading();
    ~TestLoading();

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testLoading();
    void testLoading_data();

    void testSaving();
    void testSaving_data();

private:
    QTextDocument *documentFromScript(const QString &script);
    QTextDocument *documentFromOdt(const QString &odt);
    QString documentToOdt(QTextDocument *);
    void addData();

    // Functions that help compare two QTextDocuments.
    bool compareFragments(const QTextFragment &actualFragment, const QTextFragment &expectedFragment);
    bool compareTabProperties(QVariant actualTabs, QVariant expectedTabs);
    bool compareBlockFormats(const QTextBlockFormat &actualFormat, const QTextBlockFormat &expectedFormat);
    bool compareListFormats(const QTextListFormat &actualFormat, const QTextListFormat &expectedFormat);
    bool compareBlocks(const QTextBlock &actualBlock, const QTextBlock &expectedBlock);
    bool compareTableCells(QTextTableCell actualCell, QTextTableCell expectedCell);
    bool compareTables(QTextTable *actualTable, QTextTable *expectedTable);
    bool compareFrames(QTextFrame *actualFrame, QTextFrame *expectedFrame);
    bool compareDocuments(QTextDocument *actualDocument, QTextDocument *expectedDocument);

    KComponentData *componentData;
    QScriptEngine *engine;
};

#endif // TESTLOADING_H
