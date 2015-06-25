/* This file is part of the KDE project
   Copyright (C) 2003-2012 Jarosław Staniek <staniek@kde.org>

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

#include "queryschema_p.h"
#include "driver.h"
#include "connection.h"
#include "expression.h"
#include "parser/sqlparser.h"
#include "utils.h"
#include "lookupfieldschema.h"

#include <assert.h>

#include <QBitArray>

#include <kdebug.h>
#include <klocale.h>

using namespace KexiDB;

QuerySchemaPrivate::QuerySchemaPrivate(QuerySchema* q, QuerySchemaPrivate* copy)
        : query(q)
        , masterTable(0)
        , fakeRowIDField(0)
        , fakeRowIDCol(0)
        , maxIndexWithAlias(-1)
        , visibility(64)
        , fieldsExpanded(0)
        , internalFields(0)
        , fieldsExpandedWithInternalAndRowID(0)
        , fieldsExpandedWithInternal(0)
        , orderByColumnList(0)
        , autoincFields(0)
        , columnsOrder(0)
        , columnsOrderWithoutAsterisks(0)
        , columnsOrderExpanded(0)
        , pkeyFieldsOrder(0)
        , pkeyFieldsCount(0)
        , tablesBoundToColumns(64, -1) // will be resized if needed
        , whereExpr(0)
        , ownedVisibleColumns(0)
        , regenerateExprAliases(false)
{
    visibility.fill(false);
    if (copy) {
        // deep copy
        *this = *copy;
        // <clear, so computeFieldsExpanded() will re-create it>
        fieldsExpanded = 0;
        internalFields = 0;
        columnsOrder = 0;
        columnsOrderWithoutAsterisks = 0;
        columnsOrderExpanded = 0;
        orderByColumnList = 0;
        autoincFields = 0;
        autoIncrementSQLFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        ownedVisibleColumns = 0;
        fieldsExpandedWithInternalAndRowID = 0;
        fieldsExpandedWithInternal = 0;
        pkeyFieldsOrder = 0;
        fakeRowIDCol = 0;
        fakeRowIDField = 0;
        ownedVisibleColumns = 0;
        // </clear, so computeFieldsExpanded() will re-create it>
        if (copy->whereExpr) {
            whereExpr = copy->whereExpr->copy();
        }
        // "*this = *copy" causes copying pointers; pull of them without destroying,
        // will be deep-copied in the QuerySchema ctor.
        asterisks.setAutoDelete(false);
        asterisks.clear();
        asterisks.setAutoDelete(true);
    }
    else {
        orderByColumnList = new OrderByColumnList;
    }
}

QuerySchemaPrivate::~QuerySchemaPrivate()
{
    delete orderByColumnList;
    delete autoincFields;
    delete columnsOrder;
    delete columnsOrderWithoutAsterisks;
    delete columnsOrderExpanded;
    delete pkeyFieldsOrder;
    delete whereExpr;
    delete fakeRowIDCol;
    delete fakeRowIDField;
    delete ownedVisibleColumns;
    if (fieldsExpanded)
        qDeleteAll(*fieldsExpanded);
    delete fieldsExpanded;
    if (internalFields) {
        qDeleteAll(*internalFields);
        delete internalFields;
    }
    delete fieldsExpandedWithInternalAndRowID;
    delete fieldsExpandedWithInternal;
}

//static
QuerySchema* QuerySchemaPrivate::createQuery(Connection *conn)
{
    return new QuerySchema(conn);
}

void QuerySchemaPrivate::clear()
{
    columnAliases.clear();
    tableAliases.clear();
    asterisks.clear();
    relations.clear();
    masterTable = 0;
    tables.clear();
    clearCachedData();
    delete pkeyFieldsOrder;
    pkeyFieldsOrder = 0;
    visibility.fill(false);
    tablesBoundToColumns = QVector<int>(64, -1); // will be resized if needed
    tablePositionsForAliases.clear();
    columnPositionsForAliases.clear();
}

void QuerySchemaPrivate::clearCachedData()
{
    if (orderByColumnList) {
        orderByColumnList->clear();
    }
    if (fieldsExpanded) {
        delete columnsOrder;
        columnsOrder = 0;
        delete columnsOrderWithoutAsterisks;
        columnsOrderWithoutAsterisks = 0;
        delete columnsOrderExpanded;
        columnsOrderExpanded = 0;
        delete autoincFields;
        autoincFields = 0;
        autoIncrementSQLFieldsList.clear();
        columnInfosByNameExpanded.clear();
        columnInfosByName.clear();
        delete ownedVisibleColumns;
        ownedVisibleColumns = 0;
        qDeleteAll(*fieldsExpanded);
        delete fieldsExpanded;
        fieldsExpanded = 0;
        if (internalFields) {
            qDeleteAll(*internalFields);
            delete internalFields;
            internalFields = 0;
        }
    }
}

void QuerySchemaPrivate::setColumnAlias(uint position, const QByteArray& alias)
{
    if (alias.isEmpty()) {
        columnAliases.remove(position);
        maxIndexWithAlias = -1;
    } else {
        setColumnAliasInternal(position, alias);
    }
}

void QuerySchemaPrivate::setTableAlias(uint position, const QByteArray& alias)
{
    tableAliases.insert(position, alias.toLower());
    tablePositionsForAliases.insert(alias.toLower(), position);
}

bool QuerySchemaPrivate::hasColumnAliases()
{
    tryRegenerateExprAliases();
    return !columnAliases.isEmpty();
}

QByteArray QuerySchemaPrivate::columnAlias(uint position)
{
    tryRegenerateExprAliases();
    return columnAliases.value(position);
}

bool QuerySchemaPrivate::hasColumnAlias(uint position)
{
    tryRegenerateExprAliases();
    return columnAliases.contains(position);
}

void QuerySchemaPrivate::removeTablePositionForAlias(const QByteArray& alias)
{
    tablePositionsForAliases.remove(alias.toLower());
}

int QuerySchemaPrivate::tablePositionForAlias(const QByteArray& alias) const
{
    return tablePositionsForAliases.value(alias.toLower(), -1);
}

int QuerySchemaPrivate::columnPositionForAlias(const QByteArray& alias) const
{
    return columnPositionsForAliases.value(alias.toLower(), -1);
}

void QuerySchemaPrivate::tryRegenerateExprAliases()
{
    if (!regenerateExprAliases)
        return;
    //regenerate missing aliases for experessions
    uint colNum = 0; //used to generate a name
    QByteArray columnAlias;
    uint p = -1;
    foreach(Field* f, *query->fields()) {
        p++;
        if (f->isExpression() && columnAliases.value(p).isEmpty()) {
            //missing
            do { //find 1st unused
                colNum++;
                columnAlias = (i18nc("short for 'expression' word, e.g. 'expr' (only latin letters, please, no '.')", "expr")
                               .toLatin1() + QByteArray::number(colNum));
            } while (-1 != tablePositionForAlias(columnAlias));

            setColumnAliasInternal(p, columnAlias);
        }
    }
    regenerateExprAliases = false;
}

void QuerySchemaPrivate::setColumnAliasInternal(uint position, const QByteArray& alias)
{
    columnAliases.insert(position, alias.toLower());
    columnPositionsForAliases.insert(alias.toLower(), position);
    maxIndexWithAlias = qMax(maxIndexWithAlias, (int)position);
}
