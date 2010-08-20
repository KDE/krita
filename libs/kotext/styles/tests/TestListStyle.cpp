/* This file is part of the KDE project
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
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
#include "TestListStyle.h"

#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextBlockBorderData.h"
#include "KoTextDocument.h"
#include "styles/KoStyleManager.h"

#include <QTextDocument>
#include <QTextCursor>

void TestListStyle::testListStyle()
{
    KoListStyle ls;
    KoListLevelProperties llp = ls.levelProperties(2);
    QCOMPARE(llp.level(), 2);

    llp.setStyle(KoListStyle::AlphaLowerItem);
    KoListLevelProperties llp2 = ls.levelProperties(2);
    QVERIFY(llp2.style() != llp.style());

    ls.setLevelProperties(llp);
    QCOMPARE(llp.level(), 2);
    QCOMPARE(llp.style(), KoListStyle::AlphaLowerItem);

    llp = ls.levelProperties(2);
    QCOMPARE(llp.level(), 2);
    QCOMPARE(llp.style(), KoListStyle::AlphaLowerItem);

    QTextDocument doc;
    KoTextDocument kodoc(&doc);
    kodoc.setStyleManager(new KoStyleManager);
    QTextCursor cursor(&doc);
    cursor.insertText("foo\nbar\nBaz\n");
    QTextBlock block = doc.begin();
    ls.applyStyle(block, 2);
    QVERIFY(block.textList());
    QTextList *textList = block.textList();
    QTextListFormat format = textList->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int)(KoListStyle::AlphaLowerItem));

    block = block.next();
    QVERIFY(block.isValid());
    ls.applyStyle(block, 2);
    QVERIFY(block.textList());
    QCOMPARE(block.textList(), textList);

    ls.applyStyle(block, 10); // should set the properties of the only one that is set, level 1
    QVERIFY(block.textList());
    textList = block.textList();
    format = textList->format();
    QCOMPARE(format.intProperty(QTextListFormat::ListStyle), (int)(KoListStyle::AlphaLowerItem));

    // getting a properties without setting it doesn't change the list.
    KoListLevelProperties l4 = ls.levelProperties(4);
    QCOMPARE(l4.level(), 4);
    QCOMPARE(l4.displayLevel(), 0); // default
    l4.setDisplayLevel(3);
    QCOMPARE(l4.displayLevel(), 3);
    QCOMPARE(ls.hasLevelProperties(4), false);

    KoListLevelProperties anotherL4 = ls.levelProperties(4);
    QCOMPARE(anotherL4.level(), 4);
    QCOMPARE(anotherL4.displayLevel(), 0); // default
    QCOMPARE(ls.hasLevelProperties(4), false);

    QCOMPARE(ls.hasLevelProperties(5), false);
    // new levels are a copy of the existing level.
    KoListLevelProperties l5 = ls.levelProperties(5);
    QCOMPARE(l5.displayLevel(), 0);
    QCOMPARE(l5.style(), KoListStyle::AlphaLowerItem);
    QCOMPARE(l5.indent(), 0.);
}

QTEST_MAIN(TestListStyle)
#include <TestListStyle.moc>
