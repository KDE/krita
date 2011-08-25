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

#include <QtTest/QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCharFormat>

#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );


void TestKoTextEditor::testRemoveSelectedText()
{
    QObject parent;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);

    // enter some lorem ipsum
    editor.insertText(lorem);

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);
    startmark->setName("start!");
    editor.insertInlineObject(startmark);
    Q_ASSERT(startmark->position() == 444);

    editor.insertText(lorem);

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);
    editor.insertInlineObject(endmark);
    Q_ASSERT(endmark->position() == 888);

    // select all text
    editor.setPosition(0, QTextCursor::MoveAnchor);
    editor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    Q_ASSERT(editor.hasSelection());

    // remove the table + the markers from the document
    editor.removeSelectedText();

    // check whether the bookmarks have gone.
    Q_ASSERT(inlineObjectManager.inlineTextObjects().length() == 0);
}

QTEST_MAIN(TestKoTextEditor)

#include "TestKoTextEditor.moc"
