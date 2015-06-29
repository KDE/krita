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

#ifndef KEXIDB_QUERYSCHEMA_P_H
#define KEXIDB_QUERYSCHEMA_P_H

#include <QVector>
#include <QString>
#include <QHash>
#include <QList>
#include <QByteArray>
#include <QBitArray>

#include "fieldlist.h"
#include "schemadata.h"
#include "queryschema.h"
#include "tableschema.h"
#include "relationship.h"

namespace KexiDB
{

class Connection;

//! @internal Internals of QuerySchema
class QuerySchemaPrivate
{
public:
    explicit QuerySchemaPrivate(QuerySchema* q, QuerySchemaPrivate* copy = 0);

    ~QuerySchemaPrivate();

    //! @return a new query that's associated with @a conn. Used internally, e.g. by the parser.
    //! Uses an internal QuerySchema ctor.
    static QuerySchema* createQuery(Connection *conn);

    void clear();

    void clearCachedData();

    void setColumnAlias(uint position, const QByteArray& alias);

    void setTableAlias(uint position, const QByteArray& alias);

    bool hasColumnAliases();

    QByteArray columnAlias(uint position);

    bool hasColumnAlias(uint position);

    void removeTablePositionForAlias(const QByteArray& alias);

    int tablePositionForAlias(const QByteArray& alias) const;

    int columnPositionForAlias(const QByteArray& alias) const;

    QuerySchema *query;

    /*! Master table of the query. (may be NULL)
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    TableSchema *masterTable;

    /*! List of tables used in this query */
    TableSchema::List tables;

    Field *fakeRowIDField; //! used to mark a place for ROWID
    QueryColumnInfo *fakeRowIDCol; //! used to mark a place for ROWID

    /*! Connection on which this query operates */
    QPointer<Connection> conn;

    /*! Used to mapping tables to its aliases for this query */
    QHash<int, QByteArray> tableAliases;

    /*! Helper used with aliases */
    int maxIndexWithAlias;

    /*! Helper used with tableAliases */
    int maxIndexWithTableAlias;

    /*! Used to store visibility flag for every field */
    QBitArray visibility;

    /*! List of asterisks defined for this query  */
    Field::List asterisks;

    /*! Temporary field vector for using in fieldsExpanded() */
    QueryColumnInfo::Vector *fieldsExpanded;

    /*! Temporary field vector containing internal fields used for lookup columns. */
    QueryColumnInfo::Vector *internalFields;

    /*! Temporary, used to cache sum of expanded fields and internal fields (+rowid) used for lookup columns.
     Contains not auto-deleted items.*/
    QueryColumnInfo::Vector *fieldsExpandedWithInternalAndRowID;

    /*! Temporary, used to cache sum of expanded fields and internal fields used for lookup columns.
     Contains not auto-deleted items.*/
    QueryColumnInfo::Vector *fieldsExpandedWithInternal;

    /*! A list of fields for ORDER BY section. @see QuerySchema::orderByColumnList(). */
    OrderByColumnList* orderByColumnList;

    /*! A cache for autoIncrementFields(). */
    QueryColumnInfo::List *autoincFields;

    /*! A cache for autoIncrementSQLFieldsList(). */
    QString autoIncrementSQLFieldsList;
    QPointer<Driver> lastUsedDriverForAutoIncrementSQLFieldsList;

    /*! A hash for fast lookup of query columns' order (unexpanded version). */
    QHash<QueryColumnInfo*, int> *columnsOrder;

    /*! A hash for fast lookup of query columns' order (unexpanded version without asterisks). */
    QHash<QueryColumnInfo*, int> *columnsOrderWithoutAsterisks;

    /*! A hash for fast lookup of query columns' order.
     This is exactly opposite information compared to vector returned
     by fieldsExpanded() */
    QHash<QueryColumnInfo*, int> *columnsOrderExpanded;

//  QValueList<bool> detailedVisibility;

    /*! order of PKEY fields (e.g. for updateRow() ) */
    QVector<int> *pkeyFieldsOrder;

    /*! number of PKEY fields within the query */
    uint pkeyFieldsCount;

    /*! forced (predefined) statement */
    QString statement;

    /*! Relationships defined for this query. */
    Relationship::List relations;

    /*! Information about columns bound to tables.
     Used if table is used in FROM section more than once
     (using table aliases).

     This list is updated by insertField(uint position, Field *field,
     int bindToTable, bool visible), using bindToTable parameter.

     Example: for this statement:
     SELECT t1.a, othertable.x, t2.b FROM table t1, table t2, othertable;
     tablesBoundToColumns list looks like this:
     [ 0, -1, 1 ]
     - first column is bound to table 0 "t1"
     - second coulmn is not specially bound (othertable.x isn't ambiguous)
     - third column is bound to table 1 "t2"
    */
    QVector<int> tablesBoundToColumns;

    /*! WHERE expression */
    BaseExpr *whereExpr;

    QHash<QString, QueryColumnInfo*> columnInfosByNameExpanded;

    QHash<QString, QueryColumnInfo*> columnInfosByName; //!< Same as columnInfosByNameExpanded but asterisks are skipped

    //! field schemas created for multiple joined columns like a||' '||b||' '||c
    Field::List *ownedVisibleColumns;

    /*! Set by insertField(): true, if aliases for expression columns should
     be generated on next columnAlias() call. */
    bool regenerateExprAliases;

protected:
    void tryRegenerateExprAliases();

    void setColumnAliasInternal(uint position, const QByteArray& alias);

    /*! Used to mapping columns to its aliases for this query */
    QHash<int, QByteArray> columnAliases;

    /*! Collects table positions for aliases: used in tablePositionForAlias(). */
    QHash<QByteArray, int> tablePositionsForAliases;

    /*! Collects column positions for aliases: used in columnPositionForAlias(). */
    QHash<QByteArray, int> columnPositionsForAliases;
};

} //namespace KexiDB

#endif
