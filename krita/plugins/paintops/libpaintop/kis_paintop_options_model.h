/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_PAINTOP_OPTIONS_MODEL_H_
#define _KIS_PAINTOP_OPTIONS_MODEL_H_

#include <QAbstractListModel>

class KisPaintOpOption;

/**
 * This model can be use to show a list of visible composite op in a list view.
 */
class KisPaintOpOptionsModel : public QAbstractListModel
{
public:
    enum CustomRoles {
        WidgetIndexRole = 0x3201,
        SortingRole = 0x3202
    };
public:
    KisPaintOpOptionsModel();
    ~KisPaintOpOptionsModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void addPaintOpOption(KisPaintOpOption * option, int widgetIndex);
    Qt::ItemFlags flags(const QModelIndex & index) const;
    using QAbstractListModel::reset;
private:
    QList< KisPaintOpOption* > m_list;
    QList< int > m_widgetIndex;
};

#endif
