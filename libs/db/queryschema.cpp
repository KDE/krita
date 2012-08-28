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

#include "queryschema.h"
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

QueryColumnInfo::QueryColumnInfo(Field *f, const QByteArray& _alias, bool _visible,
                                 QueryColumnInfo *foreignColumn)
        : field(f), alias(_alias), visible(_visible), m_indexForVisibleLookupValue(-1)
        , m_foreignColumn(foreignColumn)
{
}

QueryColumnInfo::~QueryColumnInfo()
{
}

QString QueryColumnInfo::debugString() const
{
    QString res;
    if (field->table()) {
        res += field->table()->name() + QLatin1String(".");
    }
    res += field->debugString() +
           (alias.isEmpty() ? QString()
            : (QLatin1String(" AS ") + QString(alias)))
            + (visible ? QString() : QLatin1String(" [INVISIBLE]"));
    return res;
}

//=======================================
namespace KexiDB
{
//! @internal
class QuerySchemaPrivate
{
public:
    QuerySchemaPrivate(QuerySchema* q, QuerySchemaPrivate* copy = 0)
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
            , regenerateExprAliases(false) {
//Qt 4   columnAliases.setAutoDelete(true);
//Qt 4   tableAliases.setAutoDelete(true);
//Qt 4   asterisks.setAutoDelete(true);
//Qt 4   relations.setAutoDelete(true);
//Qt 4   tablePositionsForAliases.setAutoDelete(true);
//Qt 4   columnPositionsForAliases.setAutoDelete(true);
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
    ~QuerySchemaPrivate() {
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

    void clear() {
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

    void clearCachedData() {
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

    inline void setColumnAlias(uint position, const QByteArray& alias) {
        if (alias.isEmpty()) {
            columnAliases.remove(position);
            maxIndexWithAlias = -1;
        } else {
            setColumnAliasInternal(position, alias);
        }
    }

    inline void setTableAlias(uint position, const QByteArray& alias) {
        tableAliases.insert(position, alias.toLower());
        tablePositionsForAliases.insert(alias.toLower(), position);
    }

    inline bool hasColumnAliases() {
        tryRegenerateExprAliases();
        return !columnAliases.isEmpty();
    }

    inline QByteArray columnAlias(uint position) {
        tryRegenerateExprAliases();
        return columnAliases.value(position);
    }

    inline bool hasColumnAlias(uint position) {
        tryRegenerateExprAliases();
        return columnAliases.contains(position);
    }

    inline void removeTablePositionForAlias(const QByteArray& alias) {
        tablePositionsForAliases.remove(alias.toLower());
    }

    inline int tablePositionForAlias(const QByteArray& alias) const {
        return tablePositionsForAliases.value(alias.toLower(), -1);
    }

    inline int columnPositionForAlias(const QByteArray& alias) const {
        return columnPositionsForAliases.value(alias.toLower(), -1);
    }

    QuerySchema *query;

    /*! Master table of the query. (may be NULL)
      Any data modifications can be performed if we know master table.
      If null, query's records cannot be modified. */
    TableSchema *masterTable;

    /*! List of tables used in this query */
    TableSchema::List tables;

    Field *fakeRowIDField; //! used to mark a place for ROWID
    QueryColumnInfo *fakeRowIDCol; //! used to mark a place for ROWID

protected:
    void tryRegenerateExprAliases() {
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
                    columnAlias = (i18nc("short for 'expression' word, e.g. 'expr' (only latin letters, please)", "expr")
                                   .toLatin1() + QByteArray::number(colNum));
                } while (-1 != tablePositionForAlias(columnAlias));

                setColumnAliasInternal(p, columnAlias);
            }
        }
        regenerateExprAliases = false;
    }

    void setColumnAliasInternal(uint position, const QByteArray& alias) {
        columnAliases.insert(position, alias.toLower());
        columnPositionsForAliases.insert(alias.toLower(), position);
        maxIndexWithAlias = qMax(maxIndexWithAlias, (int)position);
    }

    /*! Used to mapping columns to its aliases for this query */
    QHash<int, QByteArray> columnAliases;

    /*! Collects table positions for aliases: used in tablePositionForAlias(). */
    QHash<QByteArray, int> tablePositionsForAliases;

    /*! Collects column positions for aliases: used in columnPositionForAlias(). */
    QHash<QByteArray, int> columnPositionsForAliases;

public:
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
    bool regenerateExprAliases : 1;
};
}

//=======================================

OrderByColumn::OrderByColumn()
        : m_column(0)
        , m_pos(-1)
        , m_field(0)
        , m_ascending(true)
{
}

OrderByColumn::OrderByColumn(QueryColumnInfo& column, bool ascending, int pos)
        : m_column(&column)
        , m_pos(pos)
        , m_field(0)
        , m_ascending(ascending)
{
}

OrderByColumn::OrderByColumn(Field& field, bool ascending)
        : m_column(0)
        , m_pos(-1)
        , m_field(&field)
        , m_ascending(ascending)
{
}

OrderByColumn* OrderByColumn::copy(QuerySchema* fromQuery, QuerySchema* toQuery) const
{
    //kDebug() << "this=" << this << debugString() << "m_column=" << m_column;
    if (m_field) {
        return new OrderByColumn(*m_field, m_ascending);
    }
    if (m_column) {
        QueryColumnInfo* columnInfo;
        if (fromQuery && toQuery) {
            int columnIndex = fromQuery->columnsOrder().value(m_column);
            if (columnIndex < 0) {
                kDebug() << "index not found for column" << m_column->debugString();
                return 0;
            }
            columnInfo = toQuery->expandedOrInternalField(columnIndex);
            if (!columnInfo) {
                kDebug() << "column info not found at index" << columnIndex << "in toQuery";
                return 0;
            }
        }
        else {
            columnInfo = m_column;
        }
        return new OrderByColumn(*columnInfo, m_ascending, m_pos);
    }
    Q_ASSERT(m_field || m_column);
    return 0;
}

OrderByColumn::~OrderByColumn()
{
    //kDebug() << this << debugString();
}

QString OrderByColumn::debugString() const
{
    QString orderString(m_ascending ? "ascending" : "descending");
    if (m_column) {
        if (m_pos > -1)
            return QString("COLUMN_AT_POSITION_%1(%2, %3)")
                   .arg(m_pos + 1).arg(m_column->debugString()).arg(orderString);
        else
            return QString("COLUMN(%1, %2)").arg(m_column->debugString()).arg(orderString);
    }
    return m_field ? QString("FIELD(%1, %2)").arg(m_field->debugString()).arg(orderString)
           : QString("NONE");
}

QString OrderByColumn::toSQLString(bool includeTableName, const Driver *drv, int identifierEscaping) const
{
    //kDebug() << this << debugString();
    const QString orderString(m_ascending ? "" : " DESC");
    QString fieldName, tableName, collationString;
    if (m_column) {
        if (m_pos > -1)
            return QString::number(m_pos + 1) + orderString;
        else {
            if (includeTableName && m_column->alias.isEmpty()) {
                tableName = KexiDB::escapeIdentifier(
                                drv, m_column->field->table()->name(), identifierEscaping) + '.';
            }
            fieldName = KexiDB::escapeIdentifier(drv, m_column->aliasOrName(), identifierEscaping);
        }
        if (m_column->field->isTextType() && drv) {
            collationString = drv->collationSQL();
        }
    } else {
        if (m_field && includeTableName) {
            tableName = KexiDB::escapeIdentifier(
                            drv, m_field->table()->name(), identifierEscaping) + '.';
        }
        fieldName = KexiDB::escapeIdentifier(drv, m_field ? m_field->name() : "??"/*error*/,
                                             identifierEscaping);
        if (m_field && m_field->isTextType() && drv) {
            collationString = drv->collationSQL();
        }
    }
    return tableName + fieldName + collationString + orderString;
}

//=======================================

OrderByColumnList::OrderByColumnList()
        : OrderByColumnListBase()
{
}

OrderByColumnList::OrderByColumnList(const OrderByColumnList& other,
                                     QuerySchema* fromQuery, QuerySchema* toQuery)
        : OrderByColumnListBase()
{
    for (QList<OrderByColumn*>::ConstIterator it(other.constBegin()); it != other.constEnd(); ++it) {
        OrderByColumn* order = (*it)->copy(fromQuery, toQuery);
        if (order) {
            append(order);
        }
    }
}

bool OrderByColumnList::appendFields(QuerySchema& querySchema,
                                     const QString& field1, bool ascending1,
                                     const QString& field2, bool ascending2,
                                     const QString& field3, bool ascending3,
                                     const QString& field4, bool ascending4,
                                     const QString& field5, bool ascending5)
{
    uint numAdded = 0;
#define ADD_COL(fieldName, ascending) \
    if (ok && !fieldName.isEmpty()) { \
        if (!appendField( querySchema, fieldName, ascending )) \
            ok = false; \
        else \
            numAdded++; \
    }
    bool ok = true;
    ADD_COL(field1, ascending1);
    ADD_COL(field2, ascending2);
    ADD_COL(field3, ascending3);
    ADD_COL(field4, ascending4);
    ADD_COL(field5, ascending5);
#undef ADD_COL
    if (ok)
        return true;
    for (uint i = 0; i < numAdded; i++)
        removeLast();
    return false;
}

OrderByColumnList::~OrderByColumnList()
{
    qDeleteAll(begin(), end());
}

void OrderByColumnList::appendColumn(QueryColumnInfo& columnInfo, bool ascending)
{
    append(new OrderByColumn(columnInfo, ascending));
}

bool OrderByColumnList::appendColumn(QuerySchema& querySchema, bool ascending, int pos)
{
    QueryColumnInfo::Vector fieldsExpanded(querySchema.fieldsExpanded());
    QueryColumnInfo* ci = (pos >= (int)fieldsExpanded.size()) ? 0 : fieldsExpanded[pos];
    if (!ci)
        return false;
    append(new OrderByColumn(*ci, ascending, pos));
    return true;
}

void OrderByColumnList::appendField(Field& field, bool ascending)
{
    append(new OrderByColumn(field, ascending));
}

bool OrderByColumnList::appendField(QuerySchema& querySchema,
                                    const QString& fieldName, bool ascending)
{
    QueryColumnInfo *columnInfo = querySchema.columnInfo(fieldName);
    if (columnInfo) {
        append(new OrderByColumn(*columnInfo, ascending));
        return true;
    }
    Field *field = querySchema.findTableField(fieldName);
    if (field) {
        append(new OrderByColumn(*field, ascending));
        return true;
    }
    KexiDBWarn << "OrderByColumnList::addColumn(QuerySchema& querySchema, "
    "const QString& column, bool ascending): no such field \"" << fieldName << "\"";
    return false;
}

QString OrderByColumnList::debugString() const
{
    if (isEmpty())
        return "NONE";
    QString dbg;
    for (QList<OrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!dbg.isEmpty())
            dbg += "\n";
        dbg += (*it)->debugString();
    }
    return dbg;
}

QString OrderByColumnList::toSQLString(bool includeTableNames, const Driver *drv, int identifierEscaping) const
{
    QString string;
    for (QList<OrderByColumn*>::ConstIterator it(constBegin()); it != constEnd(); ++it) {
        if (!string.isEmpty())
            string += ", ";
        string += (*it)->toSQLString(includeTableNames, drv, identifierEscaping);
    }
    return string;
}

void OrderByColumnList::clear()
{
    qDeleteAll(begin(), end());
    OrderByColumnListBase::clear();
}

//=======================================

QuerySchema::QuerySchema()
        : FieldList(false)//fields are not owned by QuerySchema object
        , SchemaData(KexiDB::QueryObjectType)
        , d(new QuerySchemaPrivate(this))
{
    init();
}

QuerySchema::QuerySchema(TableSchema& tableSchema)
        : FieldList(false)
        , SchemaData(KexiDB::QueryObjectType)
        , d(new QuerySchemaPrivate(this))
{
    d->masterTable = &tableSchema;
    init();
    /*if (!d->masterTable) {
      KexiDBWarn << "QuerySchema(TableSchema*): !d->masterTable";
      m_name.clear();
      return;
    }*/
    addTable(d->masterTable);
    //defaults:
    //inherit name from a table
    m_name = d->masterTable->name();
    //inherit caption from a table
    m_caption = d->masterTable->caption();

//replaced by explicit field list: //add all fields of the table as asterisk:
//replaced by explicit field list: addField( new QueryAsterisk(this) );

    // add explicit field list to avoid problems (e.g. with fields added outside of Kexi):
    foreach(Field* f, *d->masterTable->fields()) {
        addField(f);
    }
}

QuerySchema::QuerySchema(const QuerySchema& querySchema)
        : FieldList(querySchema, false /* !deepCopyFields */)
        , SchemaData(querySchema)
        , d(new QuerySchemaPrivate(this, querySchema.d))
{
    //only deep copy query asterisks
    foreach(Field* f, querySchema.m_fields) {
        Field *copiedField;
        if (dynamic_cast<QueryAsterisk*>(f)) {
            copiedField = f->copy();
            if (static_cast<const KexiDB::FieldList *>(f->m_parent) == &querySchema) {
                copiedField->m_parent = this;
            }
        }
        else {
            copiedField = f;
        }
        addField(copiedField);
    }
    // this deep copy must be after the 'd' initialization because fieldsExpanded() is used there
    d->orderByColumnList = new OrderByColumnList(*querySchema.d->orderByColumnList,
                                                 const_cast<QuerySchema*>(&querySchema), this);
}

QuerySchema::~QuerySchema()
{
    delete d;
}

void QuerySchema::init()
{
    m_type = KexiDB::QueryObjectType;
//m_fields_by_name.setAutoDelete( true ); //because we're using QueryColumnInfoEntry objects
}

void QuerySchema::clear()
{
    FieldList::clear();
    SchemaData::clear();
    d->clear();
}

FieldList& QuerySchema::insertField(uint position, Field *field, bool visible)
{
    return insertField(position, field, -1/*don't bind*/, visible);
}

/*virtual*/
FieldList& QuerySchema::insertField(uint position, Field *field)
{
    return insertField(position, field, -1/*don't bind*/, true);
}

FieldList& QuerySchema::insertField(uint position, Field *field,
                                    int bindToTable, bool visible)
{
    if (!field) {
        KexiDBWarn << "QuerySchema::insertField(): !field";
        return *this;
    }

    if (position > (uint)m_fields.count()) {
        KexiDBWarn << "QuerySchema::insertField(): position (" << position << ") out of range";
        return *this;
    }
    if (!field->isQueryAsterisk() && !field->isExpression() && !field->table()) {
        KexiDBWarn << "QuerySchema::insertField(): WARNING: field '" << field->name()
        << "' must contain table information!";
        return *this;
    }
    if ((int)fieldCount() >= d->visibility.size()) {
        d->visibility.resize(d->visibility.size()*2);
        d->tablesBoundToColumns.resize(d->tablesBoundToColumns.size()*2);
    }
    d->clearCachedData();
    FieldList::insertField(position, field);
    if (field->isQueryAsterisk()) {
        //kDebug() << "d->asterisks.append:" << field;
        //field->debug();
        d->asterisks.append(field);
        //if this is single-table asterisk,
        //add a table to list if doesn't exist there:
        if (field->table() && !d->tables.contains(field->table()))
            d->tables.append(field->table());
    } else if (field->table()) {
        //add a table to list if doesn't exist there:
        if (!d->tables.contains(field->table()))
            d->tables.append(field->table());
    }
// //visible by default
// setFieldVisible(field, true);
// d->visibility.setBit(fieldCount()-1, visible);
    //update visibility
    //--move bits to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->visibility.setBit(i, d->visibility.testBit(i - 1));
    d->visibility.setBit(position, visible);

    //bind to table
    if (bindToTable < -1 && bindToTable > (int)d->tables.count()) {
        KexiDBWarn << "QuerySchema::insertField(): bindToTable (" << bindToTable
        << ") out of range";
        bindToTable = -1;
    }
    //--move items to make a place for a new one
    for (uint i = fieldCount() - 1; i > position; i--)
        d->tablesBoundToColumns[i] = d->tablesBoundToColumns[i-1];
    d->tablesBoundToColumns[ position ] = bindToTable;

    KexiDBDbg << "QuerySchema::insertField(): bound to table (" << bindToTable << "): ";
    if (bindToTable == -1)
        KexiDBDbg << " <NOT SPECIFIED>";
    else
        KexiDBDbg << " name=" << d->tables.at(bindToTable)->name()
        << " alias=" << tableAlias(bindToTable);
    QString s;
    for (uint i = 0; i < fieldCount();i++)
        s += (QString::number(d->tablesBoundToColumns[i]) + " ");
    KexiDBDbg << "tablesBoundToColumns == [" << s << "]";

    if (field->isExpression())
        d->regenerateExprAliases = true;

    return *this;
}

int QuerySchema::tableBoundToColumn(uint columnPosition) const
{
    int res = d->tablesBoundToColumns.value(columnPosition, -99);
    if (res == -99) {
        KexiDBWarn << "QuerySchema::tableBoundToColumn(): columnPosition (" << columnPosition
        << ") out of range";
        return -1;
    }
    return res;
}

KexiDB::FieldList& QuerySchema::addField(KexiDB::Field* field, bool visible)
{
    return insertField(m_fields.count(), field, visible);
}

KexiDB::FieldList& QuerySchema::addField(KexiDB::Field* field, int bindToTable,
        bool visible)
{
    return insertField(m_fields.count(), field, bindToTable, visible);
}

void QuerySchema::removeField(KexiDB::Field *field)
{
    if (!field)
        return;
    d->clearCachedData();
    if (field->isQueryAsterisk()) {
        //kDebug() << "d->asterisks.removeAt:" << field;
        //field->debug();
        d->asterisks.removeAt(d->asterisks.indexOf(field));   //this will destroy this asterisk
    }
//TODO: should we also remove table for this field or asterisk?
    FieldList::removeField(field);
}

FieldList& QuerySchema::addExpression(BaseExpr* expr, bool visible)
{
    return addField(new Field(this, expr), visible);
}

bool QuerySchema::isColumnVisible(uint position) const
{
    return (position < fieldCount()) ? d->visibility.testBit(position) : false;
}

void QuerySchema::setColumnVisible(uint position, bool v)
{
    if (position < fieldCount())
        d->visibility.setBit(position, v);
}

FieldList& QuerySchema::addAsterisk(QueryAsterisk *asterisk, bool visible)
{
    if (!asterisk)
        return *this;
    //make unique name
    asterisk->m_name = (asterisk->table() ? asterisk->table()->name() + ".*" : "*")
                       + QString::number(asterisks()->count());
    return addField(asterisk, visible);
}

Connection* QuerySchema::connection() const
{
    TableSchema *mt = masterTable();
    return mt ? mt->connection() : 0;
}

QString QuerySchema::debugString()
{
    QString dbg;
    dbg.reserve(1024);
    //fields
    TableSchema *mt = masterTable();
    dbg = QString("QUERY ") + schemaDataDebugString() + "\n"
          + "-masterTable=" + (mt ? mt->name() : "<NULL>")
          + "\n-COLUMNS:\n"
          + ((fieldCount() > 0) ? FieldList::debugString() : "<NONE>") + "\n"
          + "-FIELDS EXPANDED ";

    QString dbg1;
    uint fieldsExpandedCount = 0;
    if (fieldCount() > 0) {
        QueryColumnInfo::Vector fe(fieldsExpanded());
        fieldsExpandedCount = fe.size();
        for (uint i = 0; i < fieldsExpandedCount; i++) {
            QueryColumnInfo *ci = fe[i];
            if (!dbg1.isEmpty())
                dbg1 += ",\n";
            dbg1 += ci->debugString();
        }
        dbg1 += "\n";
    } else {
        dbg1 = "<NONE>\n";
    }
    dbg1.prepend(QString("(%1):\n").arg(fieldsExpandedCount));
    dbg += dbg1;

    //it's safer to delete fieldsExpanded for now
    // (debugString() could be called before all fields are added)
//causes a crash d->clearCachedData();

    //bindings
    QString dbg2;
    dbg2.reserve(512);
    for (uint i = 0; i < fieldCount(); i++) {
        int tablePos = tableBoundToColumn(i);
        if (tablePos >= 0) {
            QByteArray tAlias(tableAlias(tablePos));
            if (!tAlias.isEmpty()) {
                dbg2 += (QString::fromLatin1(" field \"") + FieldList::field(i)->name()
                         + "\" uses alias \"" + QString(tAlias) + "\" of table \""
                         + d->tables.at(tablePos)->name() + "\"\n");
            }
        }
    }
    if (!dbg2.isEmpty()) {
        dbg += "\n-BINDINGS:\n";
        dbg += dbg2;
    }

    //tables
    QString table_names;
    table_names.reserve(512);
    foreach(TableSchema *table, d->tables) {
        if (!table_names.isEmpty())
            table_names += ", ";
        table_names += (QString("'") + table->name() + "'");
    }
    if (d->tables.isEmpty())
        table_names = "<NONE>";
    dbg += (QString("-TABLES:\n") + table_names);
    QString aliases;
    if (!d->hasColumnAliases())
        aliases = "<NONE>\n";
    else {
        int i = -1;
        foreach(Field *f, m_fields) {
            i++;
            QByteArray alias(d->columnAlias(i));
            if (!alias.isEmpty())
                aliases += (QString("field #%1: ").arg(i)
                            + (f->name().isEmpty() ? "<noname>" : f->name())
                            + " -> " + alias + "\n");
        }
    }
    //aliases
    dbg += QString("\n-COLUMN ALIASES:\n" + aliases);
    if (d->tableAliases.isEmpty())
        aliases = "<NONE>";
    else {
        aliases = "";
        int i = -1;
        foreach(TableSchema* table, d->tables) {
            i++;
            QByteArray alias(d->tableAliases.value(i));
            if (!alias.isEmpty())
                aliases += (QString("table #%1: ").arg(i)
                            + (table->name().isEmpty() ? "<noname>" : table->name())
                            + " -> " + alias + "\n");
        }
    }
    dbg += QString("-TABLE ALIASES:\n" + aliases);
    QString where = d->whereExpr ? d->whereExpr->debugString() : QString();
    if (!where.isEmpty())
        dbg += (QString("\n-WHERE EXPRESSION:\n") + where);
    if (!orderByColumnList().isEmpty())
        dbg += (QString("\n-ORDER BY (%1):\n").arg(orderByColumnList().count())
                + orderByColumnList().debugString());
    return dbg;
}

TableSchema* QuerySchema::masterTable() const
{
    if (d->masterTable)
        return d->masterTable;
    if (d->tables.isEmpty())
        return 0;

    //try to find master table if there's only one table (with possible aliasses)
    QString tableNameLower;
    int num = -1;
    foreach(TableSchema *table, d->tables) {
        num++;
        if (!tableNameLower.isEmpty() && table->name().toLower() != tableNameLower) {
            //two or more different tables
            return 0;
        }
        tableNameLower = tableAlias(num);
    }
    return d->tables.first();
}

void QuerySchema::setMasterTable(TableSchema *table)
{
    if (table)
        d->masterTable = table;
}

TableSchema::List* QuerySchema::tables() const
{
    return &d->tables;
}

void QuerySchema::addTable(TableSchema *table, const QByteArray& alias)
{
    KexiDBDbg << "QuerySchema::addTable() " << (void *)table
    << " alias=" << alias;
    if (!table)
        return;

    //only append table if:
    //-it has alias
    //-it has no alias but there is no such table on the list
    if (alias.isEmpty() && d->tables.contains(table)) {
        const QString tableNameLower(table->name().toLower());
        const QString aliasLower(alias.toLower());
        int num = -1;
        foreach(TableSchema *table, d->tables) {
            num++;
            if (table->name().toLower() == tableNameLower) {
                const QString& tAlias = tableAlias(num);
                if (tAlias == aliasLower) {
                    KexiDBWarn << "QuerySchema::addTable(): table with \""
                    << tAlias << "\" alias already added!";
                    return;
                }
            }
        }
    }

    d->tables.append(table);

    if (!alias.isEmpty())
        setTableAlias(d->tables.count() - 1, alias);
}

void QuerySchema::removeTable(TableSchema *table)
{
    if (!table)
        return;
    if (d->masterTable == table)
        d->masterTable = 0;
    d->tables.removeAt(d->tables.indexOf(table));
//! @todo remove fields!
}

TableSchema* QuerySchema::table(const QString& tableName) const
{
//! @todo maybe use tables_byname?
    foreach(TableSchema *table, d->tables) {
        if (table->name().toLower() == tableName.toLower())
            return table;
    }
    return 0;
}

bool QuerySchema::contains(TableSchema *table) const
{
    return d->tables.contains(table);
}

Field* QuerySchema::findTableField(const QString &tableOrTableAndFieldName) const
{
    QString tableName, fieldName;
    if (!KexiDB::splitToTableAndFieldParts(tableOrTableAndFieldName,
                                           tableName, fieldName, KexiDB::SetFieldNameIfNoTableName)) {
        return 0;
    }
    if (tableName.isEmpty()) {
        foreach(TableSchema *table, d->tables) {
            if (table->field(fieldName))
                return table->field(fieldName);
        }
        return 0;
    }
    TableSchema *tableSchema = table(tableName);
    if (!tableSchema)
        return 0;
    return tableSchema->field(fieldName);
}

QByteArray QuerySchema::columnAlias(uint position) const
{
    return d->columnAlias(position);
}

bool QuerySchema::hasColumnAlias(uint position) const
{
    return d->hasColumnAlias(position);
}

void QuerySchema::setColumnAlias(uint position, const QByteArray& alias)
{
    if (position >= (uint)m_fields.count()) {
        KexiDBWarn << "QuerySchema::setColumnAlias(): position ("  << position
        << ") out of range!";
        return;
    }
    QByteArray fixedAlias(alias.trimmed());
    Field *f = FieldList::field(position);
    if (f->captionOrName().isEmpty() && fixedAlias.isEmpty()) {
        KexiDBWarn << "QuerySchema::setColumnAlias(): position ("  << position
        << ") could not remove alias when no name is specified for expression column!";
        return;
    }
    d->setColumnAlias(position, fixedAlias);
}

QByteArray QuerySchema::tableAlias(uint position) const
{
    return d->tableAliases.value(position);
}

int QuerySchema::tablePositionForAlias(const QByteArray& name) const
{
    return d->tablePositionForAlias(name);
}

int QuerySchema::tablePosition(const QString& tableName) const
{
    int num = -1;
    QString tableNameLower(tableName.toLower());
    foreach(TableSchema* table, d->tables) {
        num++;
        if (table->name().toLower() == tableNameLower)
            return num;
    }
    return -1;
}

QList<int> QuerySchema::tablePositions(const QString& tableName) const
{
    QList<int> result;
    QString tableNameLower(tableName.toLower());
    int num = -1;
    foreach(TableSchema* table, d->tables) {
        num++;
        if (table->name().toLower() == tableNameLower) {
            result += num;
        }
    }
    return result;
}

bool QuerySchema::hasTableAlias(uint position) const
{
    return d->tableAliases.contains(position);
}

int QuerySchema::columnPositionForAlias(const QByteArray& name) const
{
    return d->columnPositionForAlias(name);
}

void QuerySchema::setTableAlias(uint position, const QByteArray& alias)
{
    if (position >= (uint)d->tables.count()) {
        KexiDBWarn << "QuerySchema::setTableAlias(): position ("  << position
        << ") out of range!";
        return;
    }
    QByteArray fixedAlias(alias.trimmed());
    if (fixedAlias.isEmpty()) {
        QByteArray oldAlias(d->tableAliases.take(position));
        if (!oldAlias.isEmpty()) {
            d->removeTablePositionForAlias(oldAlias);
        }
//   d->maxIndexWithTableAlias = -1;
    } else {
        d->setTableAlias(position, fixedAlias);
//  d->maxIndexWithTableAlias = qMax( d->maxIndexWithTableAlias, (int)index );
    }
}

Relationship::List* QuerySchema::relationships() const
{
    return &d->relations;
}

Field::List* QuerySchema::asterisks() const
{
    return &d->asterisks;
}

QString QuerySchema::statement() const
{
    return d->statement;
}

void QuerySchema::setStatement(const QString &s)
{
    d->statement = s;
}

Field* QuerySchema::field(const QString& identifier, bool expanded)
{
    QueryColumnInfo *ci = columnInfo(identifier, expanded);
    return ci ? ci->field : 0;
}

QueryColumnInfo* QuerySchema::columnInfo(const QString& identifier, bool expanded)
{
    computeFieldsExpanded();
    return expanded ? d->columnInfosByNameExpanded[identifier] : d->columnInfosByName[identifier];
}

QueryColumnInfo::Vector QuerySchema::fieldsExpanded(FieldsExpandedOptions options)
{
    computeFieldsExpanded();
    if (options == WithInternalFields || options == WithInternalFieldsAndRowID) {
        //a ref to a proper pointer (as we cache the vector for two cases)
        QueryColumnInfo::Vector*& tmpFieldsExpandedWithInternal =
            (options == WithInternalFields) ? d->fieldsExpandedWithInternal : d->fieldsExpandedWithInternalAndRowID;
        //special case
        if (!tmpFieldsExpandedWithInternal) {
            //glue expanded and internal fields and cache it
            const uint size = d->fieldsExpanded->count()
                              + (d->internalFields ? d->internalFields->count() : 0)
                              + ((options == WithInternalFieldsAndRowID) ? 1 : 0) /*ROWID*/;
            tmpFieldsExpandedWithInternal = new QueryColumnInfo::Vector(size);
            const uint fieldsExpandedVectorSize = d->fieldsExpanded->size();
            for (uint i = 0; i < fieldsExpandedVectorSize; i++)
                (*tmpFieldsExpandedWithInternal)[i] = d->fieldsExpanded->at(i);
            const uint internalFieldsCount = d->internalFields ? d->internalFields->size() : 0;
            if (internalFieldsCount > 0) {
                for (uint i = 0; i < internalFieldsCount; i++)
                    (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + i] = d->internalFields->at(i);
            }
            if (options == WithInternalFieldsAndRowID) {
                if (!d->fakeRowIDField) {
                    d->fakeRowIDField = new Field("rowID", Field::BigInteger);
                    d->fakeRowIDCol = new QueryColumnInfo(d->fakeRowIDField, QByteArray(), true);
                }
                (*tmpFieldsExpandedWithInternal)[fieldsExpandedVectorSize + internalFieldsCount] = d->fakeRowIDCol;
            }
        }
        return *tmpFieldsExpandedWithInternal;
    }

    if (options == Default)
        return *d->fieldsExpanded;

    //options == Unique:
    QSet<QByteArray> columnsAlreadyFound;
    const uint fieldsExpandedCount(d->fieldsExpanded->count());
    QueryColumnInfo::Vector result(fieldsExpandedCount);   //initial size is set
// QMapConstIterator<QueryColumnInfo*, bool> columnsAlreadyFoundIt;
    //compute unique list
    uint uniqueListCount = 0;
    for (uint i = 0; i < fieldsExpandedCount; i++) {
        QueryColumnInfo *ci = d->fieldsExpanded->at(i);
//  columnsAlreadyFoundIt = columnsAlreadyFound.find(ci);
//  uint foundColumnIndex = -1;
        if (!columnsAlreadyFound.contains(ci->aliasOrName())) {// columnsAlreadyFoundIt==columnsAlreadyFound.constEnd())
            columnsAlreadyFound.insert(ci->aliasOrName());
            result[uniqueListCount++] = ci;
        }
    }
    result.resize(uniqueListCount); //update result size
    return result;
}

QueryColumnInfo::Vector QuerySchema::internalFields()
{
    computeFieldsExpanded();
    return d->internalFields ? *d->internalFields : QueryColumnInfo::Vector();
}

QueryColumnInfo* QuerySchema::expandedOrInternalField(uint index)
{
    return fieldsExpanded(WithInternalFields).value(index);
}

inline QString lookupColumnKey(Field *foreignField, Field* field)
{
    QString res;
    if (field->table()) // can be 0 for anonymous fields built as joined multiple visible columns
        res = field->table()->name() + ".";
    return res + field->name() + "_" + foreignField->table()->name() + "." + foreignField->name();
}

void QuerySchema::computeFieldsExpanded()
{
    if (d->fieldsExpanded)
        return;

    if (!d->columnsOrder) {
        d->columnsOrder = new QHash<QueryColumnInfo*, int>();
        d->columnsOrderWithoutAsterisks = new QHash<QueryColumnInfo*, int>();
    } else {
        d->columnsOrder->clear();
        d->columnsOrderWithoutAsterisks->clear();
    }
    if (d->ownedVisibleColumns)
        d->ownedVisibleColumns->clear();

    //collect all fields in a list (not a vector yet, because we do not know its size)
    QueryColumnInfo::List list; //temporary
    QueryColumnInfo::List lookup_list; //temporary, for collecting additional fields related to lookup fields
    QHash<QueryColumnInfo*, bool> columnInfosOutsideAsterisks; //helper for filling d->columnInfosByName
    int i = 0;
    uint numberOfColumnsWithMultipleVisibleFields = 0; //used to find an unique name for anonymous field
    int fieldPosition = -1;
    foreach(Field *f, m_fields) {
        fieldPosition++;
        if (f->isQueryAsterisk()) {
            if (static_cast<QueryAsterisk*>(f)->isSingleTableAsterisk()) {
                const Field::List *ast_fields = static_cast<QueryAsterisk*>(f)->table()->fields();
                foreach(Field *ast_f, *ast_fields) {
//     d->detailedVisibility += isFieldVisible(fieldPosition);
                    QueryColumnInfo *ci = new QueryColumnInfo(ast_f, QByteArray()/*no field for asterisk!*/,
                            isColumnVisible(fieldPosition));
                    list.append(ci);
                    KexiDBDbg << "QuerySchema::computeFieldsExpanded(): caching (unexpanded) columns order: "
                    << ci->debugString() << " at position " << fieldPosition;
                    d->columnsOrder->insert(ci, fieldPosition);
//     list.append(ast_f);
                }
            } else {//all-tables asterisk: iterate through table list
                foreach(TableSchema *table, d->tables) {
                    //add all fields from this table
                    const Field::List *tab_fields = table->fields();
                    foreach(Field *tab_f, *tab_fields) {
//! \todo (js): perhaps not all fields should be appended here
//      d->detailedVisibility += isFieldVisible(fieldPosition);
//      list.append(tab_f);
                        QueryColumnInfo *ci = new QueryColumnInfo(tab_f, QByteArray()/*no field for asterisk!*/,
                                isColumnVisible(fieldPosition));
                        list.append(ci);
                        KexiDBDbg << "QuerySchema::computeFieldsExpanded(): caching (unexpanded) columns order: "
                        << ci->debugString() << " at position " << fieldPosition;
                        d->columnsOrder->insert(ci, fieldPosition);
                    }
                }
            }
        } else {
            //a single field
//   d->detailedVisibility += isFieldVisible(fieldPosition);
            QueryColumnInfo *ci = new QueryColumnInfo(f, columnAlias(fieldPosition), isColumnVisible(fieldPosition));
            list.append(ci);
            columnInfosOutsideAsterisks.insert(ci, true);
            KexiDBDbg << "QuerySchema::computeFieldsExpanded(): caching (unexpanded) column's order: "
            << ci->debugString() << " at position " << fieldPosition;
            d->columnsOrder->insert(ci, fieldPosition);
            d->columnsOrderWithoutAsterisks->insert(ci, fieldPosition);

            //handle lookup field schema
            LookupFieldSchema *lookupFieldSchema = f->table() ? f->table()->lookupFieldSchema(*f) : 0;
            if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
                continue;
            // Lookup field schema found:
            // Now we also need to fetch "visible" value from the lookup table, not only the value of binding.
            // -> build LEFT OUTER JOIN clause for this purpose (LEFT, not INNER because the binding can be broken)
            // "LEFT OUTER JOIN lookupTable ON thisTable.thisField=lookupTable.boundField"
            LookupFieldSchema::RowSource& rowSource = lookupFieldSchema->rowSource();
            if (rowSource.type() == LookupFieldSchema::RowSource::Table) {
                TableSchema *lookupTable = connection()->tableSchema(rowSource.name());
                FieldList* visibleColumns = 0;
                Field *boundField = 0;
                if (lookupTable
                        && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                        && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))
                        && (boundField = lookupTable->field(lookupFieldSchema->boundColumn()))) {
                    Field *visibleColumn = 0;
                    // for single visible column, just add it as-is
                    if (visibleColumns->fieldCount() == 1) {
                        visibleColumn = visibleColumns->fields()->first();
                    } else {
                        // for multiple visible columns, build an expression column
                        // (the expression object will be owned by column info)
                        visibleColumn = new Field();
                        visibleColumn->setName(
                            QString::fromLatin1("[multiple_visible_fields_%1]")
                            .arg(++numberOfColumnsWithMultipleVisibleFields));
                        visibleColumn->setExpression(
                            new ConstExpr(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                        if (!d->ownedVisibleColumns) {
                            d->ownedVisibleColumns = new Field::List();
//Qt 4       d->ownedVisibleColumns->setAutoDelete(true);
                        }
                        d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                    }

                    lookup_list.append(
                        new QueryColumnInfo(visibleColumn, QByteArray(), true/*visible*/, ci/*foreign*/));
                    /*
                              //add visibleField to the list of SELECTed fields if it is not yes present there
                              if (!findTableField( visibleField->table()->name()+"."+visibleField->name() )) {
                                if (!table( visibleField->table()->name() )) {
                                }
                                if (!sql.isEmpty())
                                  sql += QString::fromLatin1(", ");
                                sql += (escapeIdentifier(visibleField->table()->name(), drvEscaping) + "."
                                  + escapeIdentifier(visibleField->name(), drvEscaping));
                              }*/
                }
                delete visibleColumns;
            } else if (rowSource.type() == LookupFieldSchema::RowSource::Query) {
                QuerySchema *lookupQuery = connection()->querySchema(rowSource.name());
                if (!lookupQuery)
                    continue;
                const QueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
                if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                    continue;
                QueryColumnInfo *boundColumnInfo = 0;
                if (!(boundColumnInfo = lookupQueryFieldsExpanded[ lookupFieldSchema->boundColumn()]))
                    continue;
                Field *boundField = boundColumnInfo->field;
                if (!boundField)
                    continue;
                const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
                bool ok = true;
                // all indices in visibleColumns should be in [0..lookupQueryFieldsExpanded.size()-1]
                foreach(uint visibleColumn, visibleColumns) {
                    if (visibleColumn >= (uint)lookupQueryFieldsExpanded.count()) {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;
                Field *visibleColumn = 0;
                // for single visible column, just add it as-is
                if (visibleColumns.count() == 1) {
                    visibleColumn = lookupQueryFieldsExpanded[ visibleColumns.first()]->field;
                } else {
                    // for multiple visible columns, build an expression column
                    // (the expression object will be owned by column info)
                    visibleColumn = new Field();
                    visibleColumn->setName(
                        QString::fromLatin1("[multiple_visible_fields_%1]")
                        .arg(++numberOfColumnsWithMultipleVisibleFields));
                    visibleColumn->setExpression(
                        new ConstExpr(CHARACTER_STRING_LITERAL, QVariant()/*not important*/));
                    if (!d->ownedVisibleColumns) {
                        d->ownedVisibleColumns = new Field::List();
//Qt 4      d->ownedVisibleColumns->setAutoDelete(true);
                    }
                    d->ownedVisibleColumns->append(visibleColumn);   // remember to delete later
                }

                lookup_list.append(
                    new QueryColumnInfo(visibleColumn, QByteArray(), true/*visible*/, ci/*foreign*/));
                /*
                        //add visibleField to the list of SELECTed fields if it is not yes present there
                        if (!findTableField( visibleField->table()->name()+"."+visibleField->name() )) {
                          if (!table( visibleField->table()->name() )) {
                          }
                          if (!sql.isEmpty())
                            sql += QString::fromLatin1(", ");
                          sql += (escapeIdentifier(visibleField->table()->name(), drvEscaping) + "."
                            + escapeIdentifier(visibleField->name(), drvEscaping));
                        }*/
            }
        }
    }
    //prepare clean vector for expanded list, and a map for order information
    if (!d->fieldsExpanded) {
        d->fieldsExpanded = new QueryColumnInfo::Vector(list.count());  // Field::Vector( list.count() );
//Qt 4  d->fieldsExpanded->setAutoDelete(true);
        d->columnsOrderExpanded = new QHash<QueryColumnInfo*, int>();
    } else {//for future:
        qDeleteAll(*d->fieldsExpanded);
        d->fieldsExpanded->clear();
        d->fieldsExpanded->resize(list.count());
        d->columnsOrderExpanded->clear();
    }

    /*fill (based on prepared 'list' and 'lookup_list'):
     -the vector
     -the map
     -"fields by name" dictionary
    */
    d->columnInfosByName.clear();
    d->columnInfosByNameExpanded.clear();
    i = -1;
    foreach(QueryColumnInfo* ci, list) {
        i++;
        (*d->fieldsExpanded)[i] = ci;
        d->columnsOrderExpanded->insert(ci, i);
        //remember field by name/alias/table.name if there's no such string yet in d->columnInfosByNameExpanded
        if (!ci->alias.isEmpty()) {
            //store alias and table.alias
            if (!d->columnInfosByNameExpanded[ ci->alias ])
                d->columnInfosByNameExpanded.insert(ci->alias, ci);
            QString tableAndAlias(ci->alias);
            if (ci->field->table())
                tableAndAlias.prepend(ci->field->table()->name() + ".");
            if (!d->columnInfosByNameExpanded[ tableAndAlias ])
                d->columnInfosByNameExpanded.insert(tableAndAlias, ci);
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!d->columnInfosByName[ ci->alias ])
                    d->columnInfosByName.insert(ci->alias, ci);
                if (!d->columnInfosByName[ tableAndAlias ])
                    d->columnInfosByName.insert(tableAndAlias, ci);
            }
        } else {
            //no alias: store name and table.name
            if (!d->columnInfosByNameExpanded[ ci->field->name()])
                d->columnInfosByNameExpanded.insert(ci->field->name(), ci);
            QString tableAndName(ci->field->name());
            if (ci->field->table())
                tableAndName.prepend(ci->field->table()->name() + ".");
            if (!d->columnInfosByNameExpanded[ tableAndName ])
                d->columnInfosByNameExpanded.insert(tableAndName, ci);
            //the same for "unexpanded" list
            if (columnInfosOutsideAsterisks.contains(ci)) {
                if (!d->columnInfosByName[ ci->field->name()])
                    d->columnInfosByName.insert(ci->field->name(), ci);
                if (!d->columnInfosByName[ tableAndName ])
                    d->columnInfosByName.insert(tableAndName, ci);
            }
        }
    }

    //remove duplicates for lookup fields
    QHash<QString, uint> lookup_dict; //used to fight duplicates and to update QueryColumnInfo::indexForVisibleLookupValue()
    // (a mapping from table.name string to uint* lookupFieldIndex
    i = 0;
    for (QMutableListIterator<QueryColumnInfo*> it(lookup_list); it.hasNext();) {
        QueryColumnInfo* ci = it.next();
        const QString key(lookupColumnKey(ci->foreignColumn()->field, ci->field));
        if ( /* not needed   columnInfo( tableAndFieldName ) || */
            lookup_dict.contains(key)) {
            // this table.field is already fetched by this query
            it.remove();
            delete ci;
        } else {
            lookup_dict.insert(key, i);
            i++;
        }
    }

    //create internal expanded list with lookup fields
    if (d->internalFields) {
        qDeleteAll(*d->internalFields);
        d->internalFields->clear();
        d->internalFields->resize(lookup_list.count());
    }
    delete d->fieldsExpandedWithInternal; //clear cache
    delete d->fieldsExpandedWithInternalAndRowID; //clear cache
    d->fieldsExpandedWithInternal = 0;
    d->fieldsExpandedWithInternalAndRowID = 0;
    if (!lookup_list.isEmpty() && !d->internalFields) {//create on demand
        d->internalFields = new QueryColumnInfo::Vector(lookup_list.count());
//Qt 4  d->internalFields->setAutoDelete(true);
    }
    i = -1;
    foreach(QueryColumnInfo *ci, lookup_list) {
        i++;
        //add it to the internal list
        (*d->internalFields)[i] = ci;
        d->columnsOrderExpanded->insert(ci, list.count() + i);
    }

    //update QueryColumnInfo::indexForVisibleLookupValue() cache for columns
    numberOfColumnsWithMultipleVisibleFields = 0;
    for (i = 0; i < (int)d->fieldsExpanded->size(); ++i) {
        QueryColumnInfo* ci = d->fieldsExpanded->at(i);
//! @todo QuerySchema itself will also support lookup fields...
        LookupFieldSchema *lookupFieldSchema
        = ci->field->table() ? ci->field->table()->lookupFieldSchema(*ci->field) : 0;
        if (!lookupFieldSchema || lookupFieldSchema->boundColumn() < 0)
            continue;
        LookupFieldSchema::RowSource& rowSource = lookupFieldSchema->rowSource();
        if (rowSource.type() == LookupFieldSchema::RowSource::Table) {
            TableSchema *lookupTable = connection()->tableSchema(rowSource.name());
            FieldList* visibleColumns = 0;
            if (lookupTable
                    && (uint)lookupFieldSchema->boundColumn() < lookupTable->fieldCount()
                    && (visibleColumns = lookupTable->subList(lookupFieldSchema->visibleColumns()))) {
                Field *visibleColumn = 0;
                // for single visible column, just add it as-is
                if (visibleColumns->fieldCount() == 1) {
                    visibleColumn = visibleColumns->fields()->first();
                    const QString key(lookupColumnKey(ci->field, visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                } else {
                    const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                      .arg(++numberOfColumnsWithMultipleVisibleFields)
                                      .arg(ci->field->table()->name()).arg(ci->field->name()));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                }
            }
            delete visibleColumns;
        } else if (rowSource.type() == LookupFieldSchema::RowSource::Query) {
            QuerySchema *lookupQuery = connection()->querySchema(rowSource.name());
            if (!lookupQuery)
                continue;
            const QueryColumnInfo::Vector lookupQueryFieldsExpanded(lookupQuery->fieldsExpanded());
            if (lookupFieldSchema->boundColumn() >= lookupQueryFieldsExpanded.count())
                continue;
            QueryColumnInfo *boundColumnInfo = 0;
            if (!(boundColumnInfo = lookupQueryFieldsExpanded[ lookupFieldSchema->boundColumn()]))
                continue;
            Field *boundField = boundColumnInfo->field;
            if (!boundField)
                continue;
            const QList<uint> visibleColumns(lookupFieldSchema->visibleColumns());
            // for single visible column, just add it as-is
            if (visibleColumns.count() == 1) {
                if (lookupQueryFieldsExpanded.count() > (int)visibleColumns.first()) { // sanity check
                    Field *visibleColumn = lookupQueryFieldsExpanded.at(visibleColumns.first())->field;
                    const QString key(lookupColumnKey(ci->field, visibleColumn));
                    int index = lookup_dict.value(key, -99);
                    if (index != -99)
                        ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
                }
            } else {
                const QString key(QString::fromLatin1("[multiple_visible_fields_%1]_%2.%3")
                                  .arg(++numberOfColumnsWithMultipleVisibleFields)
                                  .arg(ci->field->table()->name()).arg(ci->field->name()));
                int index = lookup_dict.value(key, -99);
                if (index != -99)
                    ci->setIndexForVisibleLookupValue(d->fieldsExpanded->size() + index);
            }
        } else {
            KexiDBWarn << "QuerySchema::computeFieldsExpanded(): unsupported record source type "
            << rowSource.typeName();
        }
    }
}

QHash<QueryColumnInfo*, int> QuerySchema::columnsOrder(ColumnsOrderOptions options)
{
    if (!d->columnsOrder)
        computeFieldsExpanded();
    if (options == UnexpandedList)
        return *d->columnsOrder;
    else if (options == UnexpandedListWithoutAsterisks)
        return *d->columnsOrderWithoutAsterisks;
    return *d->columnsOrderExpanded;
}

QVector<int> QuerySchema::pkeyFieldsOrder()
{
    if (d->pkeyFieldsOrder)
        return *d->pkeyFieldsOrder;

    TableSchema *tbl = masterTable();
    if (!tbl || !tbl->primaryKey())
        return QVector<int>();

    //get order of PKEY fields (e.g. for rows updating or inserting )
    IndexSchema *pkey = tbl->primaryKey();
    pkey->debug();
    //debug(); //20080107, sebsauer; this seems to crash in kexi on query SQL text view
    d->pkeyFieldsOrder = new QVector<int>(pkey->fieldCount(), -1);

    const uint fCount = fieldsExpanded().count();
    d->pkeyFieldsCount = 0;
    for (uint i = 0; i < fCount; i++) {
        QueryColumnInfo *fi = d->fieldsExpanded->at(i);
        const int fieldIndex = fi->field->table() == tbl ? pkey->indexOf(fi->field) : -1;
        if (fieldIndex != -1/* field found in PK */
                && d->pkeyFieldsOrder->at(fieldIndex) == -1 /* first time */) {
            KexiDBDbg << "QuerySchema::pkeyFieldsOrder(): FIELD " << fi->field->name()
            << " IS IN PKEY AT POSITION #" << fieldIndex;
//   (*d->pkeyFieldsOrder)[j]=i;
            (*d->pkeyFieldsOrder)[fieldIndex] = i;
            d->pkeyFieldsCount++;
//   j++;
        }
    }
    KexiDBDbg << "QuerySchema::pkeyFieldsOrder(): " << d->pkeyFieldsCount
    << " OUT OF " << pkey->fieldCount() << " PKEY'S FIELDS FOUND IN QUERY " << name();
    return *d->pkeyFieldsOrder;
}

uint QuerySchema::pkeyFieldsCount()
{
    (void)pkeyFieldsOrder(); /* rebuild information */
    return d->pkeyFieldsCount;
}

Relationship* QuerySchema::addRelationship(Field *field1, Field *field2)
{
//@todo: find existing global db relationships
    Relationship *r = new Relationship(this, field1, field2);
    if (r->isEmpty()) {
        delete r;
        return 0;
    }

    d->relations.append(r);
    return r;
}

QueryColumnInfo::List* QuerySchema::autoIncrementFields()
{
    if (!d->autoincFields) {
        d->autoincFields = new QueryColumnInfo::List();
    }
    TableSchema *mt = masterTable();
    if (!mt) {
        KexiDBWarn << "QuerySchema::autoIncrementFields(): no master table!";
        return d->autoincFields;
    }
    if (d->autoincFields->isEmpty()) {//no cache
        QueryColumnInfo::Vector fexp = fieldsExpanded();
        for (int i = 0; i < (int)fexp.count(); i++) {
            QueryColumnInfo *fi = fexp[i];
            if (fi->field->table() == mt && fi->field->isAutoIncrement()) {
                d->autoincFields->append(fi);
            }
        }
    }
    return d->autoincFields;
}

QString QuerySchema::sqlColumnsList(QueryColumnInfo::List* infolist, const Driver *driver)
{
    if (!infolist)
        return QString();
    QString result;
    result.reserve(256);
    bool start = true;
    foreach(QueryColumnInfo* ci, *infolist) {
        if (!start)
            result += ",";
        else
            start = false;
        result += KexiDB::escapeIdentifier(driver, ci->field->name());
    }
    return result;
}

QString QuerySchema::autoIncrementSQLFieldsList(const Driver *driver)
{
    if ((const Driver *)d->lastUsedDriverForAutoIncrementSQLFieldsList != driver
            || d->autoIncrementSQLFieldsList.isEmpty()) {
        d->autoIncrementSQLFieldsList = QuerySchema::sqlColumnsList(autoIncrementFields(), driver);
        d->lastUsedDriverForAutoIncrementSQLFieldsList = const_cast<Driver*>(driver);
    }
    return d->autoIncrementSQLFieldsList;
}

void QuerySchema::setWhereExpression(BaseExpr *expr)
{
    delete d->whereExpr;
    d->whereExpr = expr;
}

void QuerySchema::addToWhereExpression(KexiDB::Field *field, const QVariant& value, int relation)
{
    int token;
    if (value.isNull())
        token = SQL_NULL;
    else if (field->isIntegerType()) {
        token = INTEGER_CONST;
    } else if (field->isFPNumericType()) {
        token = REAL_CONST;
    } else {
        token = CHARACTER_STRING_LITERAL;
//! @todo date, time
    }

    BinaryExpr * newExpr = new BinaryExpr(
        KexiDBExpr_Relational,
        new ConstExpr(token, value),
        relation,
        new VariableExpr((field->table() ? (field->table()->name() + ".") : QString()) + field->name())
    );
    if (d->whereExpr) {
        d->whereExpr = new BinaryExpr(
            KexiDBExpr_Logical,
            d->whereExpr,
            AND,
            newExpr
        );
    } else {
        d->whereExpr = newExpr;
    }
}

/*
void QuerySchema::addToWhereExpression(KexiDB::Field *field, const QVariant& value)
    switch (value.type()) {
    case Int: case UInt: case Bool: case LongLong: case ULongLong:
      token = INTEGER_CONST;
      break;
    case Double:
      token = REAL_CONST;
      break;
    default:
      token = CHARACTER_STRING_LITERAL;
    }
//! @todo date, time

*/

BaseExpr *QuerySchema::whereExpression() const
{
    return d->whereExpr;
}

void QuerySchema::setOrderByColumnList(const OrderByColumnList& list)
{
    delete d->orderByColumnList;
    d->orderByColumnList = new OrderByColumnList(list, 0, 0);
// all field names should be found, exit otherwise ..........?
}

OrderByColumnList& QuerySchema::orderByColumnList() const
{
    return *d->orderByColumnList;
}

QuerySchemaParameterList QuerySchema::parameters()
{
    if (!whereExpression())
        return QuerySchemaParameterList();
    QuerySchemaParameterList params;
    whereExpression()->getQueryParameters(params);
    return params;
}

/*
  new field1, Field *field2
  if (!field1 || !field2) {
    KexiDBWarn << "QuerySchema::addRelationship(): !masterField || !detailsField";
    return;
  }
  if (field1->isQueryAsterisk() || field2->isQueryAsterisk()) {
    KexiDBWarn << "QuerySchema::addRelationship(): relationship's fields cannot be asterisks";
    return;
  }
  if (!hasField(field1) && !hasField(field2)) {
    KexiDBWarn << "QuerySchema::addRelationship(): fields do not belong to this query";
    return;
  }
  if (field1->table() == field2->table()) {
    KexiDBWarn << "QuerySchema::addRelationship(): fields cannot belong to the same table";
    return;
  }
//@todo: check more things: -types
//@todo: find existing global db relationships

  Field *masterField = 0, *detailsField = 0;
  IndexSchema *masterIndex = 0, *detailsIndex = 0;
  if (field1->isPrimaryKey() && field2->isPrimaryKey()) {
    //2 primary keys
    masterField = field1;
    masterIndex = masterField->table()->primaryKey();
    detailsField = field2;
    detailsIndex = masterField->table()->primaryKey();
  }
  else if (field1->isPrimaryKey()) {
    masterField = field1;
    masterIndex = masterField->table()->primaryKey();
    detailsField = field2;
//@todo: check if it already exists
    detailsIndex = new IndexSchema(detailsField->table());
    detailsIndex->addField(detailsField);
    detailsIndex->setForeigKey(true);
  //  detailsField->setForeignKey(true);
  }
  else if (field2->isPrimaryKey()) {
    detailsField = field1;
    masterField = field2;
    masterIndex = masterField->table()->primaryKey();
//@todo
  }

  if (!masterIndex || !detailsIndex)
    return; //failed

  Relationship *rel = new Relationship(masterIndex, detailsIndex);

  d->relations.append( rel );
}*/

//---------------------------------------------------

QueryAsterisk::QueryAsterisk(QuerySchema *query, TableSchema *table)
        : Field()
        , m_table(table)
{
    assert(query);
    m_parent = query;
    setType(Field::Asterisk);
}

QueryAsterisk::QueryAsterisk(const QueryAsterisk& asterisk)
        : Field(asterisk)
        , m_table(asterisk.table())
{
}

QueryAsterisk::~QueryAsterisk()
{
    //kDebug() << this << debugString();
}

Field* QueryAsterisk::copy() const
{
    return new QueryAsterisk(*this);
}

void QueryAsterisk::setTable(TableSchema *table)
{
    KexiDBDbg << "QueryAsterisk::setTable()";
    m_table = table;
}

QString QueryAsterisk::debugString() const
{
    QString dbg;
    if (isAllTableAsterisk()) {
        dbg += "ALL-TABLES ASTERISK (*) ON TABLES(";
        QString table_names;
        foreach(TableSchema *table, *query()->tables()) {
            if (!table_names.isEmpty())
                table_names += ", ";
            table_names += table->name();
        }
        dbg += (table_names + ")");
    } else {
        dbg += ("SINGLE-TABLE ASTERISK (" + table()->name() + ".*)");
    }
    return dbg;
}

