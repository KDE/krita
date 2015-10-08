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

#include <QTest>
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

    KoBookmark* bm = manager->bookmark("bla");
    Q_ASSERT(bm == 0); Q_UNUSED(bm);

    QList<QString> bmlist = manager->bookmarkNameList();
    Q_ASSERT(bmlist.isEmpty());

    delete manager;
}


void TestKoBookmarkManager::testInsertAndRetrieve()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a startmark
    QTextCursor cursor(doc.firstBlock());
    KoBookmark *mark = new KoBookmark(cursor);
    manager.insert("start!", mark);
    manager.insert("another1", new KoBookmark(cursor));
    manager.insert("another2", new KoBookmark(cursor));

    KoBookmark *bm = manager.bookmark("start!");
    Q_ASSERT(bm == mark); Q_UNUSED(bm);
}

void TestKoBookmarkManager::testRemove()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a mark
    QTextCursor cursor(doc.firstBlock());
    KoBookmark *mark = new KoBookmark(cursor);
    manager.insert("start!", mark);
    manager.insert("another1", new KoBookmark(cursor));
    manager.insert("another2", new KoBookmark(cursor));

    manager.remove("start!");

    Q_ASSERT(manager.bookmark("start!") == 0);
    Q_ASSERT(manager.bookmarkNameList().length() == 2);
    Q_ASSERT(!manager.bookmarkNameList().contains("start!"));
}

void TestKoBookmarkManager::testRename()
{
    KoBookmarkManager manager;
    QTextDocument doc;

    // Insert a mark
    QTextCursor cursor(doc.firstBlock());
    KoBookmark *mark = new KoBookmark(cursor);
    manager.insert("start!", mark);
    Q_ASSERT(mark->name() == "start!");
    Q_ASSERT(manager.bookmarkNameList().length() == 1);
    Q_ASSERT(manager.bookmarkNameList().contains("start!"));

    KoBookmark *another = new KoBookmark(cursor);
    manager.insert("another", another);

    manager.rename("start!", "renamed!");

    Q_ASSERT(mark->name() == "renamed!");
    Q_ASSERT(another->name() == "another");
}

QTEST_MAIN(TestKoBookmarkManager)
