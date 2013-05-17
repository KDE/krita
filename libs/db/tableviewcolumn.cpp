/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2003   Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003-2007 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.

   Original Author:  Till Busch <till@bux.at>
   Original Project: buX (www.bux.at)
*/

#include "tableviewcolumn.h"

#include <db/validator.h>

#include "field.h"
#include "queryschema.h"
#include "roweditbuffer.h"
#include "cursor.h"
#include "utils.h"
#include "tableviewdata.h"
//#include <kexi.h>

namespace KexiDB {
class TableViewColumn::Private
{
public:
  Private()
      : data(0)
      , field(0)
      , columnInfo(0)
      , visibleLookupColumnInfo(0)
    {
    }

    //! Data that this column is assigned to. Set by TableViewColumn::setData()
    KexiDB::TableViewData* data;

    QString captionAliasOrName;

    QIcon icon;

    KexiDB::Validator* validator;

    KexiDB::TableViewData* relatedData;
    uint relatedDataPKeyID;

    Field* field;

    //! @see columnInfo()
    QueryColumnInfo* columnInfo;

    //! @see visibleLookupColumnInfo()
    QueryColumnInfo* visibleLookupColumnInfo;

    uint width;
    bool isDBAware; //!< true if data is stored in DB, not only in memeory
    bool readOnly;
    bool fieldOwned;
    bool visible;
    bool relatedDataEditable;
    bool headerTextVisible;
};
}

//------------------------

using namespace KexiDB;

TableViewColumn::TableViewColumn(Field& f, bool owner)
        : d(new Private)
{
    d->field = &f;
    d->isDBAware = false;
    d->fieldOwned = owner;
    d->captionAliasOrName = d->field->captionOrName();
    init();
}

TableViewColumn::TableViewColumn(const QString& name, Field::Type ctype,
        uint cconst,
        uint options,
        uint length, uint precision,
        QVariant defaultValue,
        const QString& caption, const QString& description)
        : d(new Private)
{
    d->field = new Field(
        name, ctype,
        cconst,
        options,
        length, precision,
        defaultValue,
        caption, description);

    d->isDBAware = false;
    d->fieldOwned = true;
    d->captionAliasOrName = d->field->captionOrName();
    init();
}

TableViewColumn::TableViewColumn(const QString& name, Field::Type ctype,
        const QString& caption, const QString& description)
        : d(new Private)
{
    d->field = new Field(
        name, ctype,
        Field::NoConstraints,
        Field::NoOptions,
        0, 0,
        QVariant(),
        caption, description);

    d->isDBAware = false;
    d->fieldOwned = true;
    d->captionAliasOrName = d->field->captionOrName();
    init();
}

// db-aware
TableViewColumn::TableViewColumn(
    const QuerySchema &query, QueryColumnInfo& aColumnInfo,
    QueryColumnInfo* aVisibleLookupColumnInfo)
        : d(new Private)
{
    d->field = aColumnInfo.field;
    d->columnInfo = &aColumnInfo;
    d->visibleLookupColumnInfo = aVisibleLookupColumnInfo;
    d->isDBAware = true;
    d->fieldOwned = false;

    //setup column's caption:
    if (!d->columnInfo->field->caption().isEmpty()) {
        d->captionAliasOrName = d->columnInfo->field->caption();
    } else {
        //reuse alias if available:
        d->captionAliasOrName = d->columnInfo->alias;
        //last hance: use field name
        if (d->captionAliasOrName.isEmpty())
            d->captionAliasOrName = d->columnInfo->field->name();
        //todo: compute other auto-name?
    }
    init();
    //setup column's readonly flag: true, if
    // - it's not from parent table's field, or
    // - if the query itself is coming from read-only connection, or
    // - if the query itself is stored (i.e. has connection) and lookup column is defined
    const bool columnFromMasterTable = query.masterTable() == d->columnInfo->field->table();
    d->readOnly = !columnFromMasterTable
                 || (query.connection() && query.connection()->isReadOnly());
//  || (query.connection() && (query.connection()->isReadOnly() || visibleLookupColumnInfo));
//! @todo 2.0: remove this when queries become editable            ^^^^^^^^^^^^^^
// kDebug() << "TableViewColumn: query.masterTable()=="
//  << (query.masterTable() ? query.masterTable()->name() : "notable") << ", columnInfo->field->table()=="
//  << (columnInfo->field->table() ? columnInfo->field->table()->name()  : "notable");

// d->visible = query.isFieldVisible(&f);
}

TableViewColumn::TableViewColumn(bool)
        : d(new Private)
{
    d->isDBAware = false;
    init();
}

TableViewColumn::~TableViewColumn()
{
    if (d->fieldOwned)
        delete d->field;
    setValidator(0);
    delete d->relatedData;
    delete d;
}

void TableViewColumn::init()
{
    d->relatedData = 0;
    d->readOnly = false;
    d->visible = true;
    d->validator = 0;
    d->relatedDataEditable = false;
    d->headerTextVisible = true;
    d->width = 0;
}

void TableViewColumn::setValidator(KexiDB::Validator* v)
{
    if (d->validator) {//remove old one
        if (!d->validator->parent()) //destroy if has no parent
            delete d->validator;
    }
    d->validator = v;
}

void TableViewColumn::setData(KexiDB::TableViewData* data)
{
    d->data = data;
}

void TableViewColumn::setRelatedData(KexiDB::TableViewData *data)
{
    if (d->isDBAware)
        return;
    if (d->relatedData)
        delete d->relatedData;
    d->relatedData = 0;
    if (!data)
        return;
    //find a primary key
    const TableViewColumn::List *columns = data->columns();
    int id = -1;
    foreach(TableViewColumn* col, *columns) {
        id++;
        if (col->field()->isPrimaryKey()) {
            //found, remember
            d->relatedDataPKeyID = id;
            d->relatedData = data;
            return;
        }
    }
}

bool TableViewColumn::isReadOnly() const
{
    return d->readOnly || (d->data && d->data->isReadOnly());
}

void TableViewColumn::setReadOnly(bool ro)
{
    d->readOnly = ro;
}

bool TableViewColumn::isVisible() const
{
    return d->columnInfo ? d->columnInfo->visible : d->visible;
}

void TableViewColumn::setVisible(bool v)
{
    if (d->columnInfo)
        d->columnInfo->visible = v;
    d->visible = v;
}

void TableViewColumn::setIcon(const QIcon& icon)
{
    d->icon = icon;
}

QIcon TableViewColumn::icon() const
{
    return d->icon;
}

void TableViewColumn::setHeaderTextVisible(bool visible)
{
    d->headerTextVisible = visible;
}

bool TableViewColumn::isHeaderTextVisible() const
{
    return d->headerTextVisible;
}

QString TableViewColumn::captionAliasOrName() const
{
    return d->captionAliasOrName;
}

KexiDB::Validator* TableViewColumn::validator() const
{
    return d->validator;
}

KexiDB::TableViewData *TableViewColumn::relatedData() const
{
    return d->relatedData;
}

Field* TableViewColumn::field() const
{
    return d->field;
}

void TableViewColumn::setRelatedDataEditable(bool set)
{
    d->relatedDataEditable = set;
}

bool TableViewColumn::isRelatedDataEditable() const
{
    return d->relatedDataEditable;
}

QueryColumnInfo* TableViewColumn::columnInfo() const
{
    return d->columnInfo;
}

QueryColumnInfo* TableViewColumn::visibleLookupColumnInfo() const
{
    return d->visibleLookupColumnInfo;
}

//! \return true if data is stored in DB, not only in memeory.
bool TableViewColumn::isDBAware() const
{
    return d->isDBAware;
}


bool TableViewColumn::acceptsFirstChar(const QChar& ch) const
{
    // the field we're looking at can be related to "visible lookup column"
    // if lookup column is present
    Field *visibleField = d->visibleLookupColumnInfo
                                  ? d->visibleLookupColumnInfo->field : d->field;
    if (visibleField->isNumericType()) {
        if (ch == '.' || ch == ',')
            return visibleField->isFPNumericType();
        if (ch == '-')
            return !visibleField->isUnsigned();
        if (ch == '+' || (ch >= '0' && ch <= '9'))
            return true;
        return false;
    }

    switch (visibleField->type()) {
    case Field::Boolean:
        return false;
    case Field::Date:
    case Field::DateTime:
    case Field::Time:
        return ch >= '0' && ch <= '9';
    default:;
    }
    return true;
}

void TableViewColumn::setWidth(uint w)
{
    d->width = w;
}

uint TableViewColumn::width() const
{
    return d->width;
}
