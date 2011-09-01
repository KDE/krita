/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestKoBookmark.h"

#include <QtTest/QTest>
#include <QDebug>
#include <QString>
#include <QTextDocument>
#include <QList>

#include <KoTextEditor.h>
#include <KoInlineTextObjectManager.h>
#include <KoBookmarkManager.h>
#include <KoBookmark.h>
#include <KoTextDocument.h>

void TestKoBookmark::testInsertion()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);

    // enter some lorem ipsum
    editor.insertText("1bla bla bla");

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);
    startmark->setName("start!");
    editor.insertInlineObject(startmark);

    editor.insertText("2bla bla bla");

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);
    editor.insertInlineObject(endmark);

    editor.insertText("3bla bla bla");

    startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);
    startmark->setName("start 2!");
    editor.insertInlineObject(startmark);

    editor.insertText("4bla bla bla");

    endmark = new KoBookmark(editor.document());
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);
    editor.insertInlineObject(endmark);

    editor.insertText("5bla bla bla");

    QCOMPARE(inlineObjectManager.bookmarkManager()->bookmarkNameList().length(), 2);
    QCOMPARE(inlineObjectManager.inlineTextObjects().length(), 4);

    int bookmarksFound = 0;
    QTextCursor cursor = doc.find(QString(QChar::ObjectReplacementCharacter), 0);
    while (!cursor.isNull()) {
        QTextCharFormat fmt = cursor.charFormat();
        KoInlineObject *obj = inlineObjectManager.inlineTextObject(fmt);
        KoBookmark *bm = dynamic_cast<KoBookmark*>(obj);
        if (bm) {
            qDebug() << "found bookmark" << bm->name() << bm->type() << cursor.position();
            bookmarksFound++;
        }
        cursor = doc.find(QString(QChar::ObjectReplacementCharacter), cursor.position());
    }
    QCOMPARE(bookmarksFound, 4);

}


QTEST_MAIN(TestKoBookmark)

#include "TestKoBookmark.moc"
