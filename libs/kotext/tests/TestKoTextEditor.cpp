/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
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
#include "TestKoTextEditor.h"

#include <QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCharFormat>
#include <QList>
#include <QMetaType>

#include <KoShapeBasedDocumentBase.h>
#include <KoShape.h>
#include <KoStyleManager.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoShapeController.h>
#include <KoDocumentResourceManager.h>
#include <KoDocumentRdfBase.h>
#include <KoParagraphStyle.h>
#include <KoSection.h>
#include <KoSectionEnd.h>
#include <KoSectionUtils.h>

Q_DECLARE_METATYPE(QVector< QVector<int> >)

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );

void TestKoTextEditor::testInsertInlineObject()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    /* Hmm, what kind of inline object should we test. variables maybe?
    // enter some lorem ipsum
    editor.insertText(lorem);
    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setName("single!");
    editor.insertInlineObject(startmark);
    Q_ASSERT(startmark->id() == 1);
    Q_ASSERT(startmark->name() == "single!");
    Q_ASSERT(startmark->position() == 444);

    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    Q_ASSERT(cursor.position() == 444);

    KoInlineObject *obj = inlineObjectManager.inlineTextObject(cursor.charFormat());

    Q_ASSERT(obj == startmark);
*/
}

void TestKoTextEditor::testRemoveSelectedText()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoTextRangeManager rangeManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setTextRangeManager(&rangeManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);

    QTextCursor cur(&doc);
    cur.setPosition(editor.position());
    KoBookmark *bookmark = new KoBookmark(cur);
    bookmark->setName("start!");
    bookmark->setPositionOnlyMode(false); // we want it to be several chars long
    rangeManager.insert(bookmark);

    editor.insertText(lorem);

    bookmark->setRangeEnd(editor.position());

    QCOMPARE(bookmark->rangeStart(), lorem.length());
    QCOMPARE(bookmark->rangeEnd(), lorem.length() * 2);
    Q_ASSERT(rangeManager.textRanges().length() == 1);

    // select all text
    editor.setPosition(0, QTextCursor::MoveAnchor);
    editor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    Q_ASSERT(editor.hasSelection());

    // remove the text + the bookmark from the document
    editor.deleteChar();

    // check whether the bookmark has gone.
    Q_ASSERT(rangeManager.textRanges().length() == 0);
}

void TestKoTextEditor::pushSectionStart(int num, KoSection *sec, KoTextEditor &editor)
{
    editor.insertText(QString("[ %1").arg(num));

    QTextBlockFormat fmt = editor.blockFormat();
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    KoSectionUtils::setSectionStartings(fmt, QList<KoSection *>() << sec);
    editor.setBlockFormat(fmt);

    editor.insertText("\n");
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    editor.setBlockFormat(fmt);
}

void TestKoTextEditor::pushSectionEnd(int num, KoSectionEnd *secEnd, KoTextEditor &editor)
{
    editor.insertText(QString("%1 ]").arg(num));

    QTextBlockFormat fmt = editor.blockFormat();
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    
    KoSectionUtils::setSectionEndings(fmt, QList<KoSectionEnd *>() << secEnd);
    editor.setBlockFormat(fmt);

    editor.insertText("\n");
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    editor.setBlockFormat(fmt);
}

bool TestKoTextEditor::checkStartings(const QVector<int> &needStartings, KoSection **sec, KoTextEditor &editor)
{
    QList<KoSection *> lst = KoSectionUtils::sectionStartings(editor.blockFormat());

    if (lst.size() != needStartings.size()) {
        qDebug() << QString("Startings list size is wrong. Found %1, Expected %2").arg(lst.size()).arg(needStartings.size());
        return false;
    }

    for (int i = 0; i < needStartings.size(); i++) {
        if (lst[i] != sec[needStartings[i]]) {
            qDebug() << QString("Found unexpected section starting. Expected %1 section.").arg(needStartings[i]);
            return false;
        }
    }

    return true;
}

bool TestKoTextEditor::checkEndings(const QVector<int> &needEndings, KoSectionEnd **secEnd, KoTextEditor &editor)
{
    QList<KoSectionEnd *> lst = KoSectionUtils::sectionEndings(editor.blockFormat());

    if (lst.size() != needEndings.size()) {
        qDebug() << QString("Endings list size is wrong. Found %1, expected %2").arg(lst.size()).arg(needEndings.size());
        return false;
    }

    for (int i = 0; i < needEndings.size(); i++) {
        if (lst[i] != secEnd[needEndings[i]]) {
            qDebug() << QString("Found unexpected section ending. Expected %1 section.").arg(needEndings[i]);
            return false;
        }
    }

    return true;
}

void TestKoTextEditor::testDeleteSectionHandling_data()
{
    QTest::addColumn<int>("selectionStart");
    QTest::addColumn<int>("selectionEnd");
    QTest::addColumn<int>("neededBlockCount");
    QTest::addColumn< QVector< QVector<int> > >("needStartings");
    QTest::addColumn< QVector< QVector<int> > >("needEndings");

    QTest::newRow("Simple deletion, no effect to sections.") << 1 << 2 << 11
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting entire 1st section begin.") << 4 << 8 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1 << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting entire 1st section begin and part of 2nd.") << 4 << 9 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1 << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting part of 1st section begin.") << 5 << 8 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting part of 1st section begin and part of 2nd.") << 5 << 9 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting all sections except 0th one.") << 4 << 36 << 3
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting 3rd and part of 4th.") << 20 << 32 << 8
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting all the sections.") << 0 << 40 << 1
    << (QVector< QVector<int> >()
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>()));
    QTest::newRow("Deleting part of 3rd and part of 4th.") << 25 << 29 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting 2nd end.") << 12 << 16 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2 << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting 2nd end and part of 1st.") << 12 << 17 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2 << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting part of 2nd end.") << 13 << 16 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2 << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Deleting part of 2nd end and part of 1st.") << 13 << 17 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2 << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #0") << 5 << 36 << 3
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1 << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #1") << 0 << 23 << 6
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0 << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #2") << 7 << 19 << 8
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #3") << 6 << 32 << 4
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #4") << 17 << 23 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2)
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #5") << 6 << 27 << 6
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #6") << 6 << 17 << 8
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #7") << 8 << 22 << 8
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #8") << 14 << 19 << 10
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 2)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>() << 2 << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
    QTest::newRow("Random test #9") << 3 << 13 << 8
    << (QVector< QVector<int> >()
    << (QVector<int>() << 0)
    << (QVector<int>() << 1)
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>())
    << (QVector<int>())
    << (QVector<int>()))
    << (QVector< QVector<int> >()
    << (QVector<int>())
    << (QVector<int>() << 1)
    << (QVector<int>())
    << (QVector<int>() << 3)
    << (QVector<int>())
    << (QVector<int>() << 4)
    << (QVector<int>() << 0)
    << (QVector<int>()));
}

void TestKoTextEditor::testDeleteSectionHandling()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoTextRangeManager rangeManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setTextRangeManager(&rangeManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    const int TOTAL_SECTIONS = 5;
    KoSection *sec[TOTAL_SECTIONS];
    KoSectionEnd *secEnd[TOTAL_SECTIONS];
    for (int i = 0; i < TOTAL_SECTIONS; i++) {
        sec[i] = new KoSection(editor.constCursor());
        secEnd[i] = new KoSectionEnd(sec[i]);
    }

    pushSectionStart(0, sec[0], editor);
    pushSectionStart(1, sec[1], editor);
    pushSectionStart(2, sec[2], editor);
    pushSectionEnd(2, secEnd[2], editor);
    pushSectionEnd(1, secEnd[1], editor);
    pushSectionStart(3, sec[3], editor);
    pushSectionEnd(3, secEnd[3], editor);
    pushSectionStart(4, sec[4], editor);
    pushSectionEnd(4, secEnd[4], editor);
    pushSectionEnd(0, secEnd[0], editor);

    /**         ** offset  ** block num
     *  [ 0$     0           0
     *  [ 1$     4           1
     *  [ 2$     8           2
     *  2 ]$     12          3
     *  1 ]$     16          4
     *  [ 3$     20          5
     *  3 ]$     24          6
     *  [ 4$     28          7
     *  4 ]$     32          8
     *  0 ]$     36          9
     * (**empty_block**)     10
     */

    QFETCH(int, selectionStart);
    QFETCH(int, selectionEnd);
    QFETCH(int, neededBlockCount);
    QFETCH(QVector< QVector<int> >, needStartings);
    QFETCH(QVector< QVector<int> >, needEndings);

    // placing selection
    editor.setPosition(selectionStart);
    editor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selectionEnd - selectionStart);

    // doing deletion
    editor.deleteChar();

    // checking
    editor.movePosition(QTextCursor::Start);

    QCOMPARE(doc.blockCount(), neededBlockCount);

    for (int i = 0; i < doc.blockCount(); i++) {
        if (!checkStartings(needStartings[i], sec, editor)
            || !checkEndings(needEndings[i], secEnd, editor)) {
            QFAIL("Wrong section information.");
        }
        editor.movePosition(QTextCursor::NextBlock);
    }
}

class TestDocument : public KoShapeBasedDocumentBase
{
public:

    TestDocument()
    {
        m_document = new QTextDocument();

        KoTextDocument textDoc(m_document);
        KoTextEditor *editor = new KoTextEditor(m_document);

        textDoc.setInlineTextObjectManager(&m_inlineObjectManager);
        textDoc.setTextRangeManager(&m_rangeManager);
        textDoc.setStyleManager(new KoStyleManager(0));
        textDoc.setTextEditor(editor);

    }

    virtual ~TestDocument()
    {
        delete m_document;
    }

    virtual void addShape(KoShape *shape)
    {
        m_shapes << shape;
    }

    virtual void removeShape(KoShape *shape)
    {
        m_shapes.removeAll(shape);
    }

    KoTextEditor *textEditor()
    {
        return KoTextDocument(m_document).textEditor();
    }

    QList<KoShape *> m_shapes;

    QTextDocument *m_document;
    KoInlineTextObjectManager m_inlineObjectManager;
    KoTextRangeManager m_rangeManager;
    KoDocumentRdfBase m_rdfBase;
};

QTEST_MAIN(TestKoTextEditor)

#include "TestKoTextEditor.moc"
