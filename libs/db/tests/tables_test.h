/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef TABLETEST_H
#define TABLETEST_H

int tablesTest()
{
    if (dbCreationTest() != 0)
        return 1;

/*    if (!conn->useDatabase(db_name)) {
        conn->debugError();
        return 1;
    }*/

    conn->setAutoCommit(false);
    KexiDB::Transaction t = conn->beginTransaction();
    if (conn->error()) {
        conn->debugError();
        return 1;
    }

    //now: lets create tables:
    KexiDB::Field *f;
    KexiDB::TableSchema *t_persons = new KexiDB::TableSchema("persons");
    t_persons->setCaption("Persons in our factory");
    t_persons->addField(f = new KexiDB::Field("id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned));
    f->setCaption("ID");
    t_persons->addField(f = new KexiDB::Field("age", KexiDB::Field::Integer, 0, KexiDB::Field::Unsigned));
    f->setCaption("Age");
    t_persons->addField(f = new KexiDB::Field("name", KexiDB::Field::Text));
    f->setCaption("Name");
    t_persons->addField(f = new KexiDB::Field("surname", KexiDB::Field::Text));
    f->setCaption("Surname");
    if (!conn->createTable(t_persons)) {
        conn->debugError();
        return 1;
    }
    kDebug() << "-- PERSONS created --";
    t_persons->debug();

    if (!conn->insertRecord(*t_persons, QVariant(1), QVariant(27), QVariant("Jaroslaw"), QVariant("Staniek"))
            || !conn->insertRecord(*t_persons, QVariant(2), QVariant(60), QVariant("Lech"), QVariant("Walesa"))
            || !conn->insertRecord(*t_persons, QVariant(3), QVariant(45), QVariant("Bill"), QVariant("Gates"))
            || !conn->insertRecord(*t_persons, QVariant(4), QVariant(35), QVariant("John"), QVariant("Smith"))
       ) {
        kDebug() << "-- PERSONS data err. --";
        return 1;
    }
    kDebug() << "-- PERSONS data created --";


    KexiDB::TableSchema *t_cars = new KexiDB::TableSchema("cars");
    t_cars->setCaption("Cars owned by persons");
    t_cars->addField(f = new KexiDB::Field("id", KexiDB::Field::Integer, KexiDB::Field::PrimaryKey | KexiDB::Field::AutoInc, KexiDB::Field::Unsigned));
    f->setCaption("ID");
    t_cars->addField(f = new KexiDB::Field("owner", KexiDB::Field::Integer, 0, KexiDB::Field::Unsigned));
    f->setCaption("Car owner");
    t_cars->addField(f = new KexiDB::Field("model", KexiDB::Field::Text));
    f->setCaption("Car model");
    if (!conn->createTable(t_cars)) {
        conn->debugError();
        return 1;
    }
    kDebug() << "-- CARS created --";
    if (!conn->insertRecord(*t_cars, QVariant(1), QVariant(1), QVariant("Fiat"))
            || !conn->insertRecord(*t_cars, QVariant(2), QVariant(2), QVariant("Syrena"))
            || !conn->insertRecord(*t_cars, QVariant(3), QVariant(3), QVariant("Chrysler"))
            || !conn->insertRecord(*t_cars, QVariant(4), QVariant(3), QVariant("BMW"))
            || !conn->insertRecord(*t_cars, QVariant(5), QVariant(4), QVariant("Volvo"))
       ) {
        kDebug() << "-- CARS data err. --";
        return 1;
    }
    kDebug() << "-- CARS data created --";

    if (!conn->commitTransaction(t)) {
        conn->debugError();
        return 1;
    }

    kDebug() << "NOW, TABLE LIST: ";
    QStringList tnames = conn->tableNames();
    for (QStringList::iterator it = tnames.begin(); it != tnames.end(); ++it) {
        kDebug() << " - " << (*it);
    }


    if (!conn->closeDatabase()) {
        conn->debugError();
        return 1;
    }

    return 0;
}

#endif

