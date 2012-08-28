/* This file is part of the KDE project
   Copyright (C) 2003, 2006 Jarosław Staniek <staniek@kde.org>

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

#ifndef SQLTYPES_H
#define SQLTYPES_H

#include <QVariant>
#include <QList>

extern int current;
extern QByteArray ctoken;

struct dateType {
    int year;
    int month;
    int day;
};

struct realType {
    int integer;
    int fractional;
};

//! @internal
struct OrderByColumnInternal {
    typedef QList<OrderByColumnInternal> List;
    typedef QList<OrderByColumnInternal>::ConstIterator ListConstIterator;
    OrderByColumnInternal()
            : columnNumber(-1)
            , ascending(true) {
    }

    void setColumnByNameOrNumber(const QVariant& nameOrNumber) {
        if (nameOrNumber.type() == QVariant::String) {
            aliasOrName = nameOrNumber.toString();
            columnNumber = -1;
        } else {
            columnNumber = nameOrNumber.toInt();
            aliasOrName.clear();
        }
    }

    QString aliasOrName; //!< Can include a "tablename." prefix
    int columnNumber; //!< Optional, used instead of aliasOrName to refer to column
    //!< by its number rather than name.
    bool ascending : 1;
};

//! @internal
struct SelectOptionsInternal {
    SelectOptionsInternal() : whereExpr(0), orderByColumns(0) {}
    ~SelectOptionsInternal() {
        delete orderByColumns; // delete because this is internal temp. structure
    }
    KexiDB::BaseExpr* whereExpr;
    OrderByColumnInternal::List* orderByColumns;
};

#endif
