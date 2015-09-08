/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Own
#include "TestTableSource.h"

// Qt
#include <QList>
#include <QString>
#include <QTest>

TestTableSource::TestTableSource()
    : QObject()
{
}

void TestTableSource::init()
{
    m_source.clear();
    m_source.add("Table1", &m_table1);
    m_source.add("Table2", &m_table2);

    QStandardItem *table3 = new QStandardItem;
    QStandardItem *table4 = new QStandardItem;
    table3->setData(qVariantFromValue(QPointer<QAbstractItemModel>(&m_table3)),
                    Qt::DisplayRole);
    table4->setData(qVariantFromValue(QPointer<QAbstractItemModel>(&m_table4)),
                    Qt::DisplayRole);

    m_sheetAccessModel.setItem(0, 0, table3);
    m_sheetAccessModel.setHeaderData(0, Qt::Horizontal, "Table3");

    // Setting the sheetAccessModel now is done on purpose to test
    // if already existent data is automatically used by table source
    m_source.setSheetAccessModel(&m_sheetAccessModel);

    m_sheetAccessModel.setItem(0, 1, table4);
    m_sheetAccessModel.setHeaderData(1, Qt::Horizontal, "Table4");
}

void TestTableSource::testAdding()
{
    QVERIFY(m_source.get("Table1"));
    QCOMPARE(m_source.get("Table1")->model(), &m_table1);
    QVERIFY(m_source.get("Table2"));
    QCOMPARE(m_source.get("Table2")->model(), &m_table2);
}

void TestTableSource::testRenaming()
{
    m_source.rename("Table2", "BlueBerryCake");
    QVERIFY(m_source.get("Table1"));
    QCOMPARE(m_source.get("Table1")->model(), &m_table1);
    QVERIFY(m_source.get("BlueBerryCake"));
    QCOMPARE(m_source.get("BlueBerryCake")->model(), &m_table2);
}

void TestTableSource::testRemoval()
{
    m_source.remove("Table1");
    QCOMPARE(m_source.get("Table1"), (Table*)0);
    QVERIFY(m_source.get("Table2"));
    QCOMPARE(m_source.get("Table2")->model(), &m_table2);

    m_source.remove("Table2");
    QCOMPARE(m_source.get("Table1"), (Table*)0);
    QCOMPARE(m_source.get("Table2"), (Table*)0);
}

void TestTableSource::testAdding_SAM()
{
    QVERIFY(m_source.get("Table3"));
    QCOMPARE(m_source.get("Table3")->model(), &m_table3);
    QVERIFY(m_source.get("Table4"));
    QCOMPARE(m_source.get("Table4")->model(), &m_table4);
}

void TestTableSource::testRenaming_SAM()
{
    m_sheetAccessModel.setHeaderData(0, Qt::Horizontal, "RedCarpet");
    QVERIFY(m_source.get("RedCarpet"));
    QCOMPARE(m_source.get("RedCarpet")->model(), &m_table3);
    QVERIFY(m_source.get("Table4"));
    QCOMPARE(m_source.get("Table4")->model(), &m_table4);

    m_sheetAccessModel.setHeaderData(1, Qt::Horizontal, "Bartholomew");
    QVERIFY(m_source.get("RedCarpet"));
    QCOMPARE(m_source.get("RedCarpet")->model(), &m_table3);
    QVERIFY(m_source.get("Bartholomew"));
    QCOMPARE(m_source.get("Bartholomew")->model(), &m_table4);
}

void TestTableSource::testRemoval_SAM()
{
    m_sheetAccessModel.removeColumns(0, 1);
    QCOMPARE(m_source.get("Table3"), (Table*)0);
    QVERIFY(m_source.get("Table4"));
    QCOMPARE(m_source.get("Table4")->model(), &m_table4);

    m_sheetAccessModel.removeColumns(0, 1);
    QCOMPARE(m_source.get("Table3"), (Table*)0);
    QCOMPARE(m_source.get("Table4"), (Table*)0);
}

QTEST_MAIN(TestTableSource)

Q_DECLARE_METATYPE(QPointer<QAbstractItemModel>)
