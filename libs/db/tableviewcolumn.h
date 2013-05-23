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

#ifndef KEXIDB_TABLEVIEWCOLUMN_H
#define KEXIDB_TABLEVIEWCOLUMN_H

#include "queryschema.h"

namespace KexiDB
{
class TableViewData;
class Validator;

/*! Single column definition. */
class CALLIGRADB_EXPORT TableViewColumn
{
public:
    typedef QList<TableViewColumn*> List;
    typedef QList<TableViewColumn*>::ConstIterator ListIterator;

    /*! Not db-aware ctor. if \a owner is true, the field \a will be owned by this column,
     so you shouldn't care about destroying this field. */
    TableViewColumn(Field& f, bool owner = false);

    /*! Not db-aware, convenience ctor, like above. The field is created using specified parameters that are
     equal to these accepted by KexiDB::Field ctor. The column will be the owner
     of this automatically generated field.
     */
    TableViewColumn(const QString& name, Field::Type ctype,
                        uint cconst = Field::NoConstraints,
                        uint options = Field::NoOptions,
                        uint length = 0, uint precision = 0,
                        QVariant defaultValue = QVariant(),
                        const QString& caption = QString(),
                        const QString& description = QString());

    /*! Not db-aware, convenience ctor, simplified version of the above. */
    TableViewColumn(const QString& name, Field::Type ctype, const QString& caption,
                        const QString& description = QString());

    //! Db-aware version.
    TableViewColumn(const QuerySchema &query, QueryColumnInfo& aColumnInfo,
                        QueryColumnInfo* aVisibleLookupColumnInfo = 0);

    virtual ~TableViewColumn();

    virtual bool acceptsFirstChar(const QChar& ch) const;

    /*! \return true if the column is read-only
     For db-aware column this can depend on whether the column
     is in parent table of this query. \sa setReadOnly() */
    bool isReadOnly() const;

    //! forces readOnly flag to be set to \a ro
    //! @todo synchronize this with table view:
    void setReadOnly(bool ro);

    //! Column visibility. By default column is visible.
    bool isVisible() const;

    //! Changes column visibility.
    void setVisible(bool v);

    /*! Sets icon for displaying in the caption area (header). */
    void setIcon(const QIcon& icon);

    /*! \return bame of icon displayed in the caption area (header). */
    QIcon icon() const;

    /*! If \a visible is true, caption has to be displayed in the column's header,
     (or field's name if caption is empty. True by default. */
    void setHeaderTextVisible(bool visible);

    /*! \return true if caption has to be displayed in the column's header,
     (or field's name if caption is empty. */
    bool isHeaderTextVisible() const;

    /*! \return whatever is available:
     - field's caption
     - or field's alias (from query)
     - or finally - field's name */
    QString captionAliasOrName() const;

    /*! Assigns validator \a v for this column.
     If the validator has no parent object, it will be owned by the column,
     so you shouldn't KexiDB about destroying it. */
    void setValidator(KexiDB::Validator* v);

    //! \return validator assigned for this column of 0 if there is no validator assigned.
    KexiDB::Validator* validator() const;

    /*! For not-db-aware data only:
     Sets related data \a data for this column, what defines simple one-field,
     one-to-many relationship between this column and the primary key in \a data.
     The relationship will be used to generate a popup editor instead of just regular editor.
     This assignment has no result if \a data has no primary key defined.
     \a data is owned, so is will be destroyed when needed. It is also destroyed
     when another data (or NULL) is set for the same column. */
    void setRelatedData(TableViewData *data);

    /*! For not-db-aware data only:
     Related data \a data for this column, what defines simple one-field.
     NULL by default. \sa setRelatedData() */
    TableViewData *relatedData() const;

    /*! \return field for this column.
     For db-aware information is taken from columnInfo(). */
    Field* field() const;

    /*! Only usable if related data is set (ie. this is for combo boxes).
     Sets 'editable' flag for this column, what means a new value can be entered
     by hand. This is similar to QComboBox::setEditable(). */
    void setRelatedDataEditable(bool set);

    /*! Only usable if related data is set (ie. this is for combo boxes).
     \return 'editable' flag for this column.
     False by default. @see setRelatedDataEditable(bool). */
    bool isRelatedDataEditable() const;

    /*! A rich field information for db-aware data.
     For not-db-aware data it is always 0 (use field() instead). */
    QueryColumnInfo* columnInfo() const;

    /*! A rich field information for db-aware data. Specifies information for a column
     that should be visible instead of columnInfo. For example case see
     @ref KexiDB::QueryColumnInfo::Vector KexiDB::QuerySchema::fieldsExpanded(KexiDB::QuerySchema::FieldsExpandedOptions options = Default)

     For not-db-aware data it is always 0. */
    QueryColumnInfo* visibleLookupColumnInfo() const;

    //! \return true if data is stored in DB, not only in memeory.
    bool isDBAware() const;

    /*! Sets visible width for this column to \a w (usually in pixels or points).
        0 means there is no hint for the width. */
    void setWidth(uint w);

    /*! \return width of this field (usually in pixels or points).
         0 (the default) means there is no hint for the width. */
    uint width() const;

protected:
    //! special ctor that does not allocate d member;
    TableViewColumn(bool);

    void init();

    //! used by TableViewData::addColumn()
    void setData(TableViewData* data);

private:
    class Private;
    Private * const d;

    friend class TableViewData;
};
}

#endif
