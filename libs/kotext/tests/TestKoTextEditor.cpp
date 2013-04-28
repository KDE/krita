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

#include <KoShapeBasedDocumentBase.h>
#include <KoShape.h>
#include <KoStyleManager.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoShapeController.h>
#include <KoDocumentResourceManager.h>
#include <KoDocumentRdfBase.h>

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

void TestKoTextEditor::testPaste()
{
    TestDocument *source = new TestDocument();
    TestDocument *destination = new TestDocument();

    Q_ASSERT(source->textEditor() != destination->textEditor());

    KoShapeController shapeController(0, destination);

    source->textEditor()->insertText("bla");

    destination->textEditor()->paste(source->textEditor(), &shapeController);

    Q_ASSERT(destination->m_document->toPlainText() == "bla");


}

QTEST_MAIN(TestKoTextEditor)

#include "TestKoTextEditor.moc"
