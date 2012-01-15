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
#include "TestKoBookmarkManager.h"

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

void TestKoBookmarkManager::testCreation()
{
    KoBookmarkManager *manager = new KoBookmarkManager();
    Q_ASSERT(manager);

    KoBookmark* bm = manager->retrieveBookmark("bla");
    Q_ASSERT(bm == 0);

    QList<QString> bmlist = manager->bookmarkNameList();
    Q_ASSERT(bmlist.isEmpty());

    delete manager;
}


void TestKoBookmarkManager::testRetrieve()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a startmark
    KoBookmark *startmark = new KoBookmark(&doc);
    startmark->setType(KoBookmark::StartBookmark);
    manager.insert("start!", startmark);
    manager.insert("another1", new KoBookmark(&doc));
    manager.insert("another2", new KoBookmark(&doc));

    KoBookmark *bm = manager.retrieveBookmark("start!");
    Q_ASSERT(bm == startmark);
}

void TestKoBookmarkManager::testRetrieveByEndmark()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);
    textDoc.setTextEditor(&editor);

    // enter some lorem ipsum
    editor.insertText("bla bla bla");

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);
    startmark->setName("start!");
    editor.insertInlineObject(startmark);

    editor.insertText("bla bla bla");

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);
    Q_ASSERT(endmark->name() == startmark->name());
    editor.insertInlineObject(endmark);
    Q_ASSERT(endmark->name() == startmark->name());

    editor.insertText("bla bla bla");

    KoBookmark *mark= inlineObjectManager.bookmarkManager()->retrieveBookmark(endmark->name());
    Q_ASSERT(mark);
    Q_ASSERT(mark == startmark);
    Q_ASSERT(mark != endmark);
}

void TestKoBookmarkManager::testInsert()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a startmark
    KoBookmark *startmark = new KoBookmark(&doc);
    startmark->setType(KoBookmark::StartBookmark);
    manager.insert("start!", startmark);
    Q_ASSERT(startmark->name() == "start!");
    Q_ASSERT(manager.bookmarkNameList().length() == 1);
    Q_ASSERT(manager.bookmarkNameList().contains("start!"));

    // Insert a single mark
    KoBookmark *singleMark = new KoBookmark(&doc);
    singleMark->setType(KoBookmark::SinglePosition);
    manager.insert("single!", singleMark);
    Q_ASSERT(singleMark->name() == "single!");
    Q_ASSERT(manager.bookmarkNameList().length() == 2);
    Q_ASSERT(manager.bookmarkNameList().contains("single!"));

    // Insert an endmark -- this shouldn't actually do anything
    KoBookmark *endmark = new KoBookmark(&doc);
    endmark->setType(KoBookmark::EndBookmark);
    manager.insert("endmark!", endmark);
    Q_ASSERT(endmark->name() != "endmark!");
    Q_ASSERT(manager.bookmarkNameList().length() == 2);
    Q_ASSERT(!manager.bookmarkNameList().contains("endmark!"));
}

void TestKoBookmarkManager::testRemove()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a startmark
    KoBookmark *startmark = new KoBookmark(&doc);
    startmark->setType(KoBookmark::StartBookmark);
    manager.insert("start!", startmark);
    manager.insert("another1", new KoBookmark(&doc));
    manager.insert("another2", new KoBookmark(&doc));

    manager.remove("start!");

    Q_ASSERT(manager.retrieveBookmark("start!") == 0);
    Q_ASSERT(manager.bookmarkNameList().length() == 2);
    Q_ASSERT(!manager.bookmarkNameList().contains("start!"));
}

void TestKoBookmarkManager::testRename()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a startmark
    KoBookmark *startmark = new KoBookmark(&doc);
    startmark->setType(KoBookmark::StartBookmark);
    manager.insert("start!", startmark);
    Q_ASSERT(startmark->name() == "start!");
    Q_ASSERT(manager.bookmarkNameList().length() == 1);
    Q_ASSERT(manager.bookmarkNameList().contains("start!"));

    // Insert an endmark -- this shouldn't actually do anything
    KoBookmark *endmark = new KoBookmark(&doc);
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);
    Q_ASSERT(endmark->name() == "start!");
    Q_ASSERT(manager.bookmarkNameList().length() == 1);

    KoBookmark *another = new KoBookmark(&doc);
    another->setType(KoBookmark::StartBookmark);
    manager.insert("another", another);

    manager.rename("start!", "renamed!");

    Q_ASSERT(startmark->name() == "renamed!");
    Q_ASSERT(endmark->name() == "renamed!");
    Q_ASSERT(another->name() == "another");
}

QTEST_MAIN(TestKoBookmarkManager)

#include "TestKoBookmarkManager.moc"
