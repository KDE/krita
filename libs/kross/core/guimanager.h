/***************************************************************************
 * guimanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KROSS_GUIMANAGER_H
#define KROSS_GUIMANAGER_H

//#include "action.h"

#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QListView>

#include <kdialog.h>
#include <kactioncollection.h>

#include <koffice_export.h>

class Scripting;

namespace Kross {

    class GUIClient;

    class KROSS_EXPORT GUIManagerModel : public QAbstractItemModel
    {
        public:
            GUIManagerModel(KActionCollection* collection, QObject* parent);
            virtual ~GUIManagerModel();

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const; 
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

        private:
            class Private;
            Private* const d;
    };

    class KROSS_EXPORT GUIManagerView : public QListView
    {
            Q_OBJECT
        public:
            GUIManagerView(GUIClient* guiclient, QWidget* parent);
            virtual ~GUIManagerView();

            KActionCollection* actionCollection() const;

        private slots:
            void slotSelectionChanged();
            void slotRun();
            void slotStop();
            void slotInstall();
            void slotUninstall();
            void slotNewScripts();

        private:
            class Private;
            Private* const d;
    };

    class KROSS_EXPORT GUIManagerDialog : public KDialog
    {
            Q_OBJECT
        public:
            GUIManagerDialog(GUIClient* guiclient, QWidget* parent = 0);
            virtual ~GUIManagerDialog();

        private:
            class Private;
            Private* const d;
    };

}

#endif
