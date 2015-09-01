/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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



#include "testfindmatch.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTest>

#include "KoFindMatch.h"

Q_DECLARE_METATYPE(QTextDocument*)
Q_DECLARE_METATYPE(QTextCursor)

void TestFindMatch::testBasic()
{
    KoFindMatch match;

    QTextDocument *doc = new QTextDocument("Test Document", this);
    QVariant docVariant = QVariant::fromValue(doc);
    QVERIFY(docVariant.isValid());
    
    match.setContainer(docVariant);
    QCOMPARE(match.container(), docVariant);
    QCOMPARE(match.container().value<QTextDocument*>(), doc);

    QTextCursor cursor = doc->find("Document");
    QVariant curVariant = QVariant::fromValue(cursor);
    QVERIFY(curVariant.isValid());
    
    match.setLocation(curVariant);
    QCOMPARE(match.location(), curVariant);
    QCOMPARE(match.location().value<QTextCursor>(), cursor);

    QVERIFY(match.isValid());

    KoFindMatch other(docVariant, curVariant);
    QVERIFY(other.isValid());
    QCOMPARE(other.container(), docVariant);
    QCOMPARE(other.location(), curVariant);
}

void TestFindMatch::testCopyAssign()
{
    KoFindMatch match;

    QTextDocument *doc = new QTextDocument("Test Document", this);
    QVariant docVariant = QVariant::fromValue(doc);
    QTextCursor cursor = doc->find("Document");
    QVariant curVariant = QVariant::fromValue(cursor);

    QVERIFY(docVariant.isValid());
    QVERIFY(curVariant.isValid());
    
    match.setContainer(docVariant);
    match.setLocation(curVariant);

    KoFindMatch other(match);
    QVERIFY(other.isValid());
    QCOMPARE(other.container(), match.container());
    QCOMPARE(other.location(), match.location());

    KoFindMatch third = match;
    QVERIFY(third.isValid());
    QCOMPARE(third.container(), match.container());
    QCOMPARE(third.location(), match.location());

    QVERIFY(other == match);
}

QTEST_MAIN(TestFindMatch)
