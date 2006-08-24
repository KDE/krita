/***************************************************************************
 * guimanager.h
 * This file is part of the KDE project
 * copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 * copyright (C) 2006 Sebastian Sauer <mail@dipe.org>
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

    /**
    * This class implements a abstract model to display the \a Action
    * instances provided by a \a GUIClient .
    */
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

    /**
    * The listview that displays the items provided by the \a GUIManagerModel
    * model and offers a collection of actions to run, stop, install, uninstall
    * and to get new scripts.
    */
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
            void slotNewScriptsInstallFinished();

        private:
            class Private;
            Private* const d;
    };

    /**
    * The "Scripts Manager" dialog that displays the \a GUIManagerView
    * and buttons to run, stop, install, uninstall and to get new scripts.
    */
    class KROSS_EXPORT GUIManagerDialog : public KDialog
    {
            Q_OBJECT
        public:
            GUIManagerDialog(GUIClient* guiclient, QWidget* parent);
            virtual ~GUIManagerDialog();

        private:
            class Private;
            Private* const d;
    };

}

#endif
