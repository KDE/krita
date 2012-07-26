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
#include "rdf_test.h"

#include <QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCharFormat>

#include <KoRdfSemanticItem.h>
#include <KoDocumentRdf.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextInlineRdf.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

#include "TestSemanticItem.h"

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );


QString RdfTest::insertSemItem(KoTextEditor &editor,
                               KoDocumentRdf &rdfDoc,
                               QObject &parent,
                               const QString name)
{
    editor.insertTable(5,10);
    const QTextTable *table = editor.currentTable();

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);

    KoTextInlineRdf *inlineRdf(new KoTextInlineRdf(editor.document(), startmark));
    QString newId = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newId);

    startmark->setName(newId);
    startmark->setInlineRdf(inlineRdf);

    editor.setPosition(table->firstPosition());
    editor.movePosition(QTextCursor::PreviousCharacter);
    editor.insertInlineObject(startmark);

    hTestSemanticItem testItem(new TestSemanticItem(0, &rdfDoc));
    testItem->setName(name);
    Soprano::Statement st(
                testItem->linkingSubject(), // subject
                Soprano::Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")), // predicate
                Soprano::Node::createLiteralNode(newId), // object
                rdfDoc.manifestRdfNode()); // manifest datastore
    rdfDoc.model()->addStatement(st);
    rdfDoc.rememberNewInlineRdfObject(inlineRdf);

    Q_ASSERT(rdfDoc.model()->statementCount() > 0);

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setName(newId);
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);

    editor.setPosition(table->lastPosition());
    editor.movePosition(QTextCursor::NextCharacter);
    editor.insertInlineObject(endmark);

    return newId;

}

void RdfTest::testCreateMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");

    // enter some more lorem
    editor.insertText(lorem);

    // verify that the markers are there
    QPair<int,int> position = rdfDoc.findExtent(newId);
    Q_ASSERT(position.first == 444);
    Q_ASSERT(position.second == 497);

    editor.setPosition(position.first + 1);

    QPair<int,int> position2 = rdfDoc.findExtent(&editor);
    Q_ASSERT(position == position2);
    Q_UNUSED(position2);

    // verify that we don't find markers where there aren't any
    editor.setPosition(10);
    QPair<int,int> position4 = rdfDoc.findExtent(&editor);
    Q_ASSERT(position4.first == 0);
    Q_ASSERT(position4.second == 0);
    Q_UNUSED(position4);


    // go back to the semitem
    editor.setPosition(position.first + 1);
    QCOMPARE(rdfDoc.findXmlId(&editor), newId);
}

void RdfTest::testFindMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QStringList idList;
    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");
    idList << newId;

    editor.setPosition(0);

    // now use soprano to find the tables
    QList<hTestSemanticItem> semItems = TestSemanticItem::allObjects(&rdfDoc);
    Q_ASSERT(semItems.length() == 1);

    foreach(hTestSemanticItem semItem, semItems) {
        QStringList xmlidlist = semItem->xmlIdList();

        Q_ASSERT(xmlidlist.length() == 1);
        foreach(const QString xmlid, xmlidlist) {
            Q_ASSERT(idList.contains(xmlid));
            QPair<int, int> position = rdfDoc.findExtent(xmlid);
            Q_ASSERT(position.first == 444);
            Q_ASSERT(position.second == 497);
            editor.setPosition(position.first + 2);
            const QTextTable *table = editor.currentTable();
            Q_ASSERT(table);
	    Q_UNUSED(table);
        }
    }

    // add an extra semitem
    editor.insertText(lorem);
    editor.movePosition(QTextCursor::End);
    QString newId2 = insertSemItem(editor, rdfDoc, parent, "test item2");
    idList << newId2;

    editor.insertText(lorem);

    // get all semantic items and verify they are correct
    semItems = TestSemanticItem::allObjects(&rdfDoc);
    QCOMPARE(semItems.length(), 2);

    // find the extents from the xml-id's in the semantic items
    QCOMPARE(semItems[0]->xmlIdList().length(), 1);
    QPair<int, int> position = rdfDoc.findExtent(semItems[0]->xmlIdList()[0]);
    QCOMPARE(position.first, 942);
    QCOMPARE(position.second, 995);
    editor.setPosition(position.first + 2);
    const QTextTable *table = editor.currentTable();
    Q_ASSERT(table);
    Q_UNUSED(table);

    QCOMPARE(semItems[1]->xmlIdList().length(), 1);
    position = rdfDoc.findExtent(semItems[1]->xmlIdList()[0]);
    QCOMPARE(position.first, 444);
    QCOMPARE(position.second, 497);
    editor.setPosition(position.first + 2);
    table = editor.currentTable();
    Q_ASSERT(table);

    // check there's two ranges in the document, so only four bookmarks
    QCOMPARE(inlineObjectManager.bookmarkManager()->bookmarkNameList().length(), 2);
    QCOMPARE(inlineObjectManager.inlineTextObjects().length(), 4);
    int bookmarksFound = 0;
    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    while (!cursor.isNull()) {
        QTextCharFormat fmt = cursor.charFormat();
        KoInlineObject *obj = inlineObjectManager.inlineTextObject(fmt);
        KoBookmark *bm = dynamic_cast<KoBookmark*>(obj);
        if (bm) {
            qDebug() << "found bookmark" << bm->name() << cursor.position();
            bookmarksFound++;
        }
        cursor = doc.find(QString(QChar::ObjectReplacementCharacter), cursor.position());
    }
    QCOMPARE(bookmarksFound, 4);
}

void RdfTest::testFindByName()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QStringList idList;
    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");
    idList << newId;
    idList << insertSemItem(editor, rdfDoc, parent, "test item2")
           << insertSemItem(editor, rdfDoc, parent, "test item3")
           << insertSemItem(editor, rdfDoc, parent, "test item4");


    QList<hTestSemanticItem> results = TestSemanticItem::findItemsByName("test item1",
                                                                         &rdfDoc);
    Q_ASSERT(results.size() == 1);
    QStringList xmlids = results[0]->xmlIdList();
    Q_ASSERT(xmlids.size() == 1);
    Q_ASSERT(xmlids[0] == newId);

}

void RdfTest::testEditAndFindMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QStringList idList;
    QString newId = insertSemItem(editor, rdfDoc, parent, "test item1");
    idList << newId;


    // XXX: finish test!
}

void RdfTest::testRemoveMarkers()
{
    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    QStringList idList;
    idList << insertSemItem(editor, rdfDoc, parent, "test item1");
    editor.insertText(lorem);
    idList << insertSemItem(editor, rdfDoc, parent, "test item2");
    editor.insertText(lorem);
    QString newId = insertSemItem(editor, rdfDoc, parent, "test item3");
    idList << newId;
    editor.insertText(lorem);
    idList << insertSemItem(editor, rdfDoc, parent, "test item4");
    editor.insertText(lorem);

    QList<hTestSemanticItem> results = TestSemanticItem::findItemsByName("test item3", &rdfDoc);
    Q_ASSERT(results.size() == 1);
    QStringList xmlids = results[0]->xmlIdList();
    Q_ASSERT(xmlids.size() == 1);
    Q_ASSERT(xmlids[0] == newId);

    QPair<int, int> pos = rdfDoc.findExtent(newId);

    // move around the markers for this item.
    editor.setPosition(pos.first - 10, QTextCursor::MoveAnchor);
    editor.setPosition(pos.second + 10, QTextCursor::KeepAnchor);

    Q_ASSERT(editor.hasSelection());

    // remove the table + the markers from the document
    editor.deleteChar();

    results = TestSemanticItem::allObjects(&rdfDoc);

    // we haven't rebuild the rdf database yet -- and we cannot do that from KoTextEditor,
    // so there still are 4 items
    Q_ASSERT(results.count() == 4);

    // now redo the database
    rdfDoc.updateInlineRdfStatements(&doc);

    // and now there should be three
    results = TestSemanticItem::allObjects(&rdfDoc);

    qDebug() << "we have" << results.count() << "items";
    foreach(hTestSemanticItem item, results) {
        Q_ASSERT(item->xmlIdList().length() == 1);
        QPair<int,int> pos = rdfDoc.findExtent(item->xmlIdList().first());
        qDebug() << item->name() << "pos:" << pos;
        editor.setPosition(pos.first + 1);
        qDebug() << "points to table" << editor.currentTable();
    }
}

QTEST_MAIN(RdfTest)

#include "rdf_test.moc"
