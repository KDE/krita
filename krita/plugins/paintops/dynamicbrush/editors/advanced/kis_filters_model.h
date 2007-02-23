/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTERS_MODEL_H_
#define _KIS_FILTERS_MODEL_H_

#include <QAbstractListModel>

class KisDynamicBrush;

class KisFiltersModel : public QAbstractListModel {
    Q_OBJECT

    public:
        KisFiltersModel(KisDynamicBrush* db, QObject *parent = 0);
        ~KisFiltersModel();
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
#if 0
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;
        QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
        QModelIndex parent(const QModelIndex &index) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
#endif
    private slots:
        void addNewFilter();
        void deleteCurrentFilter();
        void setCurrentFilterType(int filterType);
        void setCurrentFilter(const QModelIndex&);
    private:
        KisDynamicBrush* m_dynamicBrush;
        int m_currentFilterType;
        int m_currentTransformation;
};

#endif
