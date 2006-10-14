/***************************************************************************
 * model.h
 * This file is part of the KDE project
 * copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_MODEL_H
#define KROSS_MODEL_H

#include "krossconfig.h"
//#include "../core/action.h"

#include <QAbstractItemModel>
#include <kactioncollection.h>
#include <kactionmenu.h>

namespace Kross {

    /**
     *
     */
    class KDE_EXPORT ActionCollectionModel : public QAbstractItemModel
    {
        public:
            ActionCollectionModel(QObject* parent, KActionCollection* actioncollection);
            virtual ~ActionCollectionModel();

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual Qt::ItemFlags flags(const QModelIndex &index) const;
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
            virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

    /**
     *
     */
    class KDE_EXPORT ActionMenuModel : public QAbstractItemModel
    {
        public:
            ActionMenuModel(QObject* parent, KMenu* menu);
            virtual ~ActionMenuModel();

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

