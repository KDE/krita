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

#ifndef KCHART_TABLESOURCE_H
#define KCHART_TABLESOURCE_H

// Qt
#include <QObject>
#include <QAbstractItemModel>

// KChart
#include "ChartShape.h"


class QString;


namespace KChart {

class Table
{
    friend class TableSource;
public:
    QAbstractItemModel *model() const { return m_model; }
    QString name() const { return m_name; }

private:
    Table(const QString &name, QAbstractItemModel *model);

    QString m_name;
    QAbstractItemModel *m_model;
};

typedef QMap<QString, Table*> TableMap;

class CHARTSHAPELIB_EXPORT TableSource : public QObject
{
    Q_OBJECT

public:
    TableSource();
    ~TableSource();

    /**
     * Returns the table (model/name pair) associated with @a tableName.
     *
     * Note: The table name will be updated automatically when changed
     * by some "table source".
     */
    Table *get(const QString &tableName) const;

    /**
     * Returns the table (model/name pair) associated with @a model.
     *
     * Note: The table name will be updated automatically when changed
     * by some "table source".
     */
    Table *get(const QAbstractItemModel *model) const;

    /**
     * Returns a map of all name/table pairs in this source.
     * Mostly for debugging purposes.
     */
    TableMap tableMap() const;

    /**
     * Sets the KSpread::SheetAccessModel instance to use to get notified
     * about added/removed/renamed sheets in KSpread.
     *
     * This method is only relevant if the chart is embedded in KSpread or
     * somehow needs access to KSpread's sheets.
     */
    void setSheetAccessModel(QAbstractItemModel *model);

    /**
     * Adds a named model to this source.
     *
     * @return Pointer to new table (name/model pair) instance
     */
    Table *add(const QString &name, QAbstractItemModel *model);

    /**
     * Makes sure that the name of the specified table always stays unique.
     *
     * Use this for programatically added tables (like internal chart table).
     * Whenever another table with the same name is added/renamed, the
     * table specified will be renamed (to a sane similar name) to not
     * collide with the new name.
     */
    // TODO
    // void setRenameOnNameClash(const QString &tableName);
    // or
    // void setRenameOnNameClash(Table *table);

    /**
     * Removes a table from this source.
     */
    void remove(const QString &name);

    /**
     * Renames a table that has previously been added.
     */
    void rename(const QString &from, const QString &to);

    /**
     * Removes all tables and the sheetAccessModel.
     *
     * Note that all Table* pointers from this source are invalid after
     * calling this method!
     */
    void clear();

signals:
    /**
     * Emitted whenever a table is added to this source.
     */
    void tableAdded(Table *table);

    /**
     * Emitted whenever a table is removed from this source
     *
     * Note that right after this signal is emitted, the Table* instance
     * is deleted, thus you can't use it anymore afterwards.
     */
    void tableRemoved(Table *table);

private slots:
    /**
     * Methods that react on changes in the SheetAccessModel ("sam")
     */
    void samColumnsInserted(QModelIndex, int, int);
    void samColumnsRemoved(QModelIndex, int, int);
    void samDataChanged(const QModelIndex &first, const QModelIndex &last);
    void samHeaderDataChanged(Qt::Orientation, int, int );

private:
    class Private;
    Private *const d;
};

} // namespace KChart

#endif // KCHART_TABLESOURCE_H
