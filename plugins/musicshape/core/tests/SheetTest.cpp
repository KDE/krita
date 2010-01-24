/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include <QtTest/QtTest>

#include "Sheet.h"
#include "Part.h"
#include "PartGroup.h"
#include "Bar.h"
#include "Voice.h"

using namespace MusicCore;

class SheetTest : public QObject
{
    Q_OBJECT
private:
    Sheet* sheet;
private slots:
    void init()
    {
            sheet = new Sheet();
    }

    void cleanup()
    {
        delete sheet;
    }

    void testConstruction()
    {
        QCOMPARE(sheet->partCount(), 0);
        QCOMPARE(sheet->partGroupCount(), 0);
        QCOMPARE(sheet->barCount(), 0);
    }

    void testAddPart()
    {
        Part* p1 = sheet->addPart("part1");
        Part* p2 = sheet->addPart("part2");
        
        QCOMPARE(p1->name(), QString("part1"));
        QCOMPARE(p1->sheet(), sheet);
        QCOMPARE(sheet->partCount(), 2);
        QCOMPARE(p1, sheet->part(0));
        QCOMPARE(p2, sheet->part(1));
    }

    void testInsertPart()
    {
        Part* p1 = sheet->insertPart(0, "part1");
        Part* p2 = sheet->insertPart(0, "part2");
        Part* p3 = sheet->insertPart(1, "part3");
        
        QCOMPARE(p1->name(), QString("part1"));
        QCOMPARE(p1->sheet(), sheet);
        QCOMPARE(sheet->partCount(), 3);
        QCOMPARE(p2, sheet->part(0));
        QCOMPARE(p3, sheet->part(1));
        QCOMPARE(p1, sheet->part(2));
    }

    void testRemovePart_index()
    {
        sheet->addPart("part1");
        Part* p2 = sheet->addPart("part2");
        sheet->removePart(0);
        
        QCOMPARE(sheet->partCount(), 1);
        QCOMPARE(sheet->part(0), p2);
    }

    void testRemovePart_part()
    {
        Part* p1 = sheet->addPart("part1");
        Part* p2 = sheet->addPart("part2");
        sheet->removePart(p1);
        
        QCOMPARE(sheet->partCount(), 1);
        QCOMPARE(sheet->part(0), p2);
    }

    void testAddPartGroup()
    {
        sheet->addPart("part 1");
        sheet->addPart("part 2");
        
        PartGroup *pg1 = sheet->addPartGroup(0, 1);
        PartGroup *pg2 = sheet->addPartGroup(0, 1);
        QCOMPARE(pg1->sheet(), sheet);
        QCOMPARE(sheet->partGroupCount(), 2);
        QCOMPARE(sheet->partGroup(0), pg1);
        QCOMPARE(sheet->partGroup(1), pg2);
    }

    void testRemovePartGroup()
    {
        sheet->addPart("part 1");
        sheet->addPart("part 2");
        
        PartGroup *pg1 = sheet->addPartGroup(0, 1);
        PartGroup *pg2 = sheet->addPartGroup(0, 1);
        sheet->removePartGroup(pg1);
        QCOMPARE(sheet->partGroupCount(), 1);
        QCOMPARE(sheet->partGroup(0), pg2);
    }

    void testAddBar()
    {
        Bar* bar = sheet->addBar();
        
        QCOMPARE(bar->sheet(), sheet);
        QCOMPARE(sheet->barCount(), 1);
        QCOMPARE(sheet->bar(0), bar);
    }

    void testAddBars()
    {
        sheet->addBars(3);
        
        QCOMPARE(sheet->barCount(), 3);
        QCOMPARE(sheet->bar(0)->sheet(), sheet);
    }

    void testInsertBar()
    {
        Bar* b1 = sheet->insertBar(0);
        Bar* b2 = sheet->insertBar(0);
        Bar* b3 = sheet->insertBar(1);
        
        QCOMPARE(sheet->barCount(), 3);
        QCOMPARE(sheet->bar(0)->sheet(), sheet);
        QCOMPARE(sheet->bar(0), b2);
        QCOMPARE(sheet->bar(1), b3);
        QCOMPARE(sheet->bar(2), b1);
    }

    void testRemoveBar()
    {
        sheet->addBars(3);
        Bar* b = sheet->bar(2);
        sheet->removeBar(1);
        
        QCOMPARE(sheet->barCount(), 2);
        QCOMPARE(sheet->bar(1), b);
    }

    void testRemoveBars()
    {
        sheet->addBars(4);
        Bar* b = sheet->bar(3);
        sheet->removeBars(1, 2);
        
        QCOMPARE(sheet->barCount(), 2);
        QCOMPARE(sheet->bar(1), b);
    }
};


QTEST_MAIN(SheetTest)

#include <SheetTest.moc>
