/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (c) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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
#include <KoSectionModel.h>
#include <kundo2stack.h>

#include "TextDebug.h"
/**
 * Convenient class to create a document and assign
 * stuff like KoTextRangeManager, etc. automatically.
 */
class TestDocument : public KoShapeBasedDocumentBase
{
public:

    TestDocument()
    {
        m_document = new QTextDocument();

        KoTextDocument textDoc(m_document);
        KoTextEditor *editor = new KoTextEditor(m_document);
        KUndo2Stack *undoStack = new KUndo2Stack();
        textDoc.setUndoStack(undoStack);

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

    KoSectionModel *sectionModel()
    {
        return KoTextDocument(m_document).sectionModel();
    }

    QList<KoShape *> m_shapes;

    QTextDocument *m_document;
    KoInlineTextObjectManager m_inlineObjectManager;
    KoTextRangeManager m_rangeManager;
    KoDocumentRdfBase m_rdfBase;
};

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
);

/* FIXME: all the meaning part of this test was commented by boud in 2011
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

    // Hmm, what kind of inline object should we test. variables maybe?
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
} */

void TestKoTextEditor::testRemoveSelectedText()
{
    TestDocument doc;

    KoTextEditor *editor = doc.textEditor();
    KoTextRangeManager *rangeManager = &doc.m_rangeManager;

    // enter some lorem ipsum
    editor->insertText(lorem);

    QTextCursor cur(doc.m_document);
    cur.setPosition(editor->position());
    KoBookmark *bookmark = new KoBookmark(cur);
    bookmark->setName("start!");
    bookmark->setPositionOnlyMode(false); // we want it to be several chars long
    rangeManager->insert(bookmark);

    editor->insertText(lorem);

    bookmark->setRangeEnd(editor->position());

    QCOMPARE(bookmark->rangeStart(), lorem.length());
    QCOMPARE(bookmark->rangeEnd(), lorem.length() * 2);
    Q_ASSERT(rangeManager->textRanges().length() == 1);

    // select all text
    editor->setPosition(0, QTextCursor::MoveAnchor);
    editor->movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    Q_ASSERT(editor->hasSelection());

    // remove the text + the bookmark from the document
    editor->deleteChar();

    // check whether the bookmark has gone.
    Q_ASSERT(rangeManager->textRanges().length() == 0);
}

void TestKoTextEditor::pushSectionStart(int num, KoSection *sec, KoTextEditor *editor)
{
    editor->insertText(QString("[ %1").arg(num));

    QTextBlockFormat fmt = editor->blockFormat();
    KoSectionUtils::setSectionStartings(fmt, QList<KoSection *>() << sec);
    editor->setBlockFormat(fmt);

    editor->insertText("\n");
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    editor->setBlockFormat(fmt);
}

void TestKoTextEditor::pushSectionEnd(int num, KoSectionEnd *secEnd, KoTextEditor *editor)
{
    editor->insertText(QString("%1 ]").arg(num));

    QTextBlockFormat fmt = editor->blockFormat();
    KoSectionUtils::setSectionEndings(fmt, QList<KoSectionEnd *>() << secEnd);
    editor->setBlockFormat(fmt);
    secEnd->correspondingSection()->setKeepEndBound(true);

    editor->insertText("\n");
    fmt.clearProperty(KoParagraphStyle::SectionEndings);
    fmt.clearProperty(KoParagraphStyle::SectionStartings);
    editor->setBlockFormat(fmt);
}

void TestKoTextEditor::formSectionTestDocument(TestDocument *doc)
{
    // Here we are going to create next document with nested sections:
    //      ** offset  ** block num
    // [ 0P     0           0
    // [ 1P     4           1
    // [ 2P     8           2
    // 2 ]P     12          3
    // 1 ]P     16          4
    // [ 3P     20          5
    // 3 ]P     24          6
    // [ 4P     28          7
    // 4 ]P     32          8
    // 0 ]P     36          9
    // (**empty_block**)    10
    //
    // Sections will receive names "0", "1", etc.
    // [ and ] is actual text, not a sign!

    KoTextEditor *editor = doc->textEditor();

    const int TOTAL_SECTIONS = 5;
    KoSection *sec[TOTAL_SECTIONS];
    KoSectionEnd *secEnd[TOTAL_SECTIONS];

    sec[0] = doc->sectionModel()->createSection(editor->constCursor(), 0, QString::number(0));
    pushSectionStart(0, sec[0], editor);

    sec[1] = doc->sectionModel()->createSection(editor->constCursor(), sec[0], QString::number(1));
    pushSectionStart(1, sec[1], editor);

    sec[2] = doc->sectionModel()->createSection(editor->constCursor(), sec[1], QString::number(2));
    pushSectionStart(2, sec[2], editor);

    secEnd[2] = doc->sectionModel()->createSectionEnd(sec[2]);
    pushSectionEnd(2, secEnd[2], editor);

    secEnd[1] = doc->sectionModel()->createSectionEnd(sec[1]);
    pushSectionEnd(1, secEnd[1], editor);

    sec[3] = doc->sectionModel()->createSection(editor->constCursor(), sec[0], QString::number(3));
    pushSectionStart(3, sec[3], editor);

    secEnd[3] = doc->sectionModel()->createSectionEnd(sec[3]);
    pushSectionEnd(3, secEnd[3], editor);

    sec[4] = doc->sectionModel()->createSection(editor->constCursor(), sec[0], QString::number(4));
    pushSectionStart(4, sec[4], editor);

    secEnd[4] = doc->sectionModel()->createSectionEnd(sec[4]);
    pushSectionEnd(4, secEnd[4], editor);

    secEnd[0] = doc->sectionModel()->createSectionEnd(sec[0]);
    pushSectionEnd(0, secEnd[0], editor);

    doc->sectionModel()->allowMovingEndBound();
}

bool TestKoTextEditor::checkStartings(const QVector<QString> &needStartings, KoTextEditor *editor)
{
    QList<KoSection *> lst = KoSectionUtils::sectionStartings(editor->blockFormat());

    if (lst.size() != needStartings.size()) {
        debugText << QString("Startings list size is wrong."
                    " Found %1, Expected %2.").arg(lst.size()).arg(needStartings.size());
        return false;
    }

    for (int i = 0; i < needStartings.size(); i++) {
        if (lst[i]->name() != needStartings[i]) {
            debugText << QString("Found unexpected section starting."
                        " Expected %1 section.").arg(needStartings[i]);
            return false;
        }
    }

    return true;
}

bool TestKoTextEditor::checkEndings(const QVector<QString> &needEndings, KoTextEditor *editor)
{
    QList<KoSectionEnd *> lst = KoSectionUtils::sectionEndings(editor->blockFormat());

    if (lst.size() != needEndings.size()) {
        debugText << QString("Endings list size is wrong."
                    " Found %1, expected %2.").arg(lst.size()).arg(needEndings.size());
        return false;
    }

    for (int i = 0; i < needEndings.size(); i++) {
        if (lst[i]->correspondingSection()->name() != needEndings[i]) {
            debugText << QString("Found unexpected section ending."
                        " Expected %1 section.").arg(needEndings[i]);
            return false;
        }
    }

    return true;
}

void TestKoTextEditor::checkSectionFormattingLevel(
    TestDocument *doc,
    int neededBlockCount,
    const QVector< QVector<QString> > &needStartings,
    const QVector< QVector<QString> > &needEndings)
{
    // Assuming here that we can check names of the sections
    // instead of actual pointers. This seems to be true for now.
    QCOMPARE(needStartings.size(), neededBlockCount);
    QCOMPARE(needEndings.size(), neededBlockCount);

    KoTextEditor *editor = doc->textEditor();
    editor->movePosition(QTextCursor::Start);

    QCOMPARE(doc->m_document->blockCount(), neededBlockCount);
    for (int i = 0; i < doc->m_document->blockCount(); i++) {
        if (!checkStartings(needStartings[i], editor)
                || !checkEndings(needEndings[i], editor)) {
            QFAIL("Wrong section information.");
        }
        editor->movePosition(QTextCursor::NextBlock);
    }
}

void TestKoTextEditor::checkSectionModelLevelRecursive(QModelIndex index, TestKoTextEditor::SectionHandle *handle)
{
    QCOMPARE(index.data(KoSectionModel::PointerRole).value<KoSection *>(), handle->sec);
    QCOMPARE(index.model()->rowCount(index), handle->children.size());
    QModelIndex parent = index.parent();
    QCOMPARE(parent.data(KoSectionModel::PointerRole).value<KoSection *>(), handle->parent);
    for (int i = 0; i < handle->children.size(); i++) {
        checkSectionModelLevelRecursive(index.child(i, 0), handle->children[i]);
    }
}

void TestKoTextEditor::checkSectionModelLevel(TestDocument *doc)
{
    // Assuming here that Formatting level is OK
    // Below I will rebuild sections structure from scratch
    // and compare it then with a KoSectionModel

    QVector<SectionHandle *> allSections, rootSections;
    QStack<SectionHandle *> sectionStack;

    QTextBlock curBlock = doc->m_document->firstBlock();
    // This kind of cycle should visit all blocks
    // including ones in tables and frames.
    while (curBlock.isValid()) {
        QList<KoSection *> secStartings = KoSectionUtils::sectionStartings(curBlock.blockFormat());
        QList<KoSectionEnd *> secEndings = KoSectionUtils::sectionEndings(curBlock.blockFormat());

        Q_FOREACH (KoSection *sec, secStartings) {
            SectionHandle *handle = new SectionHandle(sec);
            if (sectionStack.empty()) {
                rootSections.push_back(handle);
                handle->parent = 0;
            } else {
                sectionStack.top()->children.push_back(handle);
                handle->parent = sectionStack.top()->sec;
            }

            allSections.push_back(handle);
            sectionStack.push(handle);
        }

        Q_FOREACH (KoSectionEnd *secEnd, secEndings) {
            sectionStack.pop();
        }

        curBlock = curBlock.next();
    }

    // Now lets compare builded tree with KoSectionModel
    KoSectionModel *model = doc->sectionModel();
    QCOMPARE(model->rowCount(), rootSections.size());
    for (int i = 0; i < rootSections.size(); i++) {
        checkSectionModelLevelRecursive(model->index(i, 0), rootSections[i]);
    }

    foreach (SectionHandle *handle, allSections) {
        delete handle;
    }
}

void TestKoTextEditor::dumpSectionFormattingLevel(TestDocument *doc)
{
    QString result;
    result += QString("QTest::newRow(\"%1\") << %2\n").arg(QTest::currentDataTag()).arg(doc->m_document->blockCount());
    result += "    << (QVector< QVector<QString> >()\n";
    QTextBlock curBlock = doc->m_document->firstBlock();
    // This kind of cycle should visit all blocks
    // including ones in tables and frames.
    while (curBlock.isValid()) {
        result += "    << (QVector<QString>()";
        QList<KoSection *> l = KoSectionUtils::sectionStartings(curBlock.blockFormat());
        foreach (KoSection *s, l) {
            result += QString(" << \"%1\"").arg(s->name());
        }
        result += ")";
        curBlock = curBlock.next();
        if (curBlock.isValid()) {
            result += "\n";
        } else {
            result += ")\n";
        }
    }

    result += "    << (QVector< QVector<QString> >()\n";
    curBlock = doc->m_document->firstBlock();
    while (curBlock.isValid()) {
        result += "    << (QVector<QString>()";
        QList<KoSectionEnd *> l = KoSectionUtils::sectionEndings(curBlock.blockFormat());
        foreach (KoSectionEnd *e, l) {
            result += QString(" << \"%1\"").arg(e->correspondingSection()->name());
        }
        result += ")";
        curBlock = curBlock.next();
        if (curBlock.isValid()) {
            result += "\n";
        } else {
            result += ")\n";
        }
    }
    result += ";";

    QFile out(QString("dump_%1.txt").arg(QTest::currentDataTag()));
    if (out.open(QIODevice::ReadWrite)) {
        QTextStream(&out) << result;
    }
    out.close();
}

void TestKoTextEditor::checkSectionTestDocument(TestDocument *doc)
{
    int neededBlockCount = 11;
    QVector< QVector<QString> > needStartings =
    QVector< QVector<QString> >()
    << (QVector<QString>() << "0")
    << (QVector<QString>() << "1")
    << (QVector<QString>() << "2")
    << (QVector<QString>())
    << (QVector<QString>())
    << (QVector<QString>() << "3")
    << (QVector<QString>())
    << (QVector<QString>() << "4")
    << (QVector<QString>())
    << (QVector<QString>())
    << (QVector<QString>());
    QVector< QVector<QString> > needEndings =
    QVector< QVector<QString> >()
    << (QVector<QString>())
    << (QVector<QString>())
    << (QVector<QString>())
    << (QVector<QString>() << "2")
    << (QVector<QString>() << "1")
    << (QVector<QString>())
    << (QVector<QString>() << "3")
    << (QVector<QString>())
    << (QVector<QString>() << "4")
    << (QVector<QString>() << "0")
    << (QVector<QString>());

    checkSectionFormattingLevel(doc, neededBlockCount, needStartings, needEndings);
    checkSectionModelLevel(doc);
}

void TestKoTextEditor::testBasicSectionCreation()
{
    TestDocument doc;
    formSectionTestDocument(&doc);
    checkSectionTestDocument(&doc);
}

#include "TestInsertSectionHandling_data.cpp"

void TestKoTextEditor::testInsertSectionHandling()
{
    TestDocument doc;
    formSectionTestDocument(&doc);

    KoTextEditor *editor = doc.textEditor();

    QFETCH(int, insertPosition);
    editor->setPosition(insertPosition);
    editor->newSection();

    QFETCH(int, neededBlockCount);
    QFETCH(QVector< QVector<QString> >, needStartings);
    QFETCH(QVector< QVector<QString> >, needEndings);
    checkSectionFormattingLevel(&doc, neededBlockCount, needStartings, needEndings);
    checkSectionModelLevel(&doc);

    // undo changes and check a source document
    KoTextDocument(doc.m_document).undoStack()->undo();
    checkSectionTestDocument(&doc);
}

#include "TestDeleteSectionHandling_data.cpp"
#include <kundo2stack.h>

// This test tests delete handling only on Formatting Level
// See KoSectionModel
void TestKoTextEditor::testDeleteSectionHandling()
{
    testBasicSectionCreation();
    // create a document
    TestDocument doc;
    formSectionTestDocument(&doc);
    KoTextEditor *editor = doc.textEditor();

    QFETCH(int, selectionStart);
    QFETCH(int, selectionEnd);
    QFETCH(int, neededBlockCount);
    QFETCH(QVector< QVector<QString> >, needStartings);
    QFETCH(QVector< QVector<QString> >, needEndings);

    // placing selection
    editor->setPosition(selectionStart);
    editor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, selectionEnd - selectionStart);

    // doing deletion
    editor->deleteChar();

    checkSectionFormattingLevel(&doc, neededBlockCount, needStartings, needEndings);
    checkSectionModelLevel(&doc);

    // undo changes and check a source document
    KoTextDocument(doc.m_document).undoStack()->undo();
    checkSectionTestDocument(&doc);
}

QTEST_MAIN(TestKoTextEditor)
