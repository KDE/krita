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

#ifndef FILTERSCATEGORYMODEL_H
#define FILTERSCATEGORYMODEL_H

#include <QModelIndex>
#include <kis_types.h>

class FiltersModel;

class FiltersCategoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* view READ view WRITE setView NOTIFY viewChanged)
    Q_PROPERTY(QObject* filterModel READ filterModel NOTIFY filterModelChanged);
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged);
public:
    enum FiltersCategoryModelRoles {
        TextRole = Qt::UserRole + 1
    };
    explicit FiltersCategoryModel(QObject* parent = 0);
    virtual ~FiltersCategoryModel();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QObject* filterModel() const;
    Q_INVOKABLE void activateItem(int index);

    QObject* view() const;
    void setView(QObject* newView);

    bool previewEnabled() const;
    void setPreviewEnabled(bool enabled);
    Q_INVOKABLE void filterSelected(int index);

    Q_INVOKABLE int categoryIndexForConfig(QObject* config);
    Q_INVOKABLE int filterIndexForConfig(int categoryIndex, QObject* filterConfig);

Q_SIGNALS:
    void viewChanged();
    void filterModelChanged();
    void previewEnabledChanged();

private Q_SLOTS:
    void activeLayerChanged(KisLayerSP layer);
    void activeSelectionChanged();
    void filterConfigurationChanged(int index, FiltersModel* model = 0);
    void filterActivated(int index);
    void updatePreview();

private:
    class Private;
    Private* d;
};

#endif // FILTERSCATEGORYMODEL_H
