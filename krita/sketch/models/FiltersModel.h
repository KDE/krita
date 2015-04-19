/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef FILTERSMODEL_H
#define FILTERSMODEL_H

#include <QModelIndex>
#include <kis_types.h>

class FiltersModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
public:
    enum FiltersModelRoles {
        TextRole = Qt::UserRole + 1
    };
    explicit FiltersModel(QObject* parent = 0);
    virtual ~FiltersModel();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    Q_INVOKABLE bool filterRequiresConfiguration(int index);
    Q_INVOKABLE QString filterID(int index);
    Q_INVOKABLE void activateFilter(int index);
    KisFilter* filter(int index);

    QString categoryId;
    QString categoryName;

    void addFilter(KisFilterSP filter);

    QObject* view() const;
    void setView(QObject* newView);

    Q_INVOKABLE QObject* configuration(int index);
    Q_INVOKABLE void setConfiguration(int index, QObject* configuration);

Q_SIGNALS:
    void viewChanged();
    void configurationChanged(int index);
    void filterActivated(int index);

private:
    class Private;
    Private* d;
};

#endif // FILTERSMODEL_H
