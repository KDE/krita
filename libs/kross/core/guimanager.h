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

#include <QObject>
#include <QWidget>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QTreeView>

#include <kdialog.h>
#include <kactioncollection.h>

#include <koffice_export.h>

namespace Kross {

    class Action;
    class GUIClient;
    class GUIManagerModule;

#if 0
    /**
    * This class implements a abstract model to display the \a Action
    * instances provided by a \a GUIClient .
    */
    class KROSS_EXPORT GUIManagerModel : public QAbstractItemModel
    {
        public:
            GUIManagerModel(KActionCollection* collection, QObject* parent, bool editable);
            virtual ~GUIManagerModel();

            virtual int columnCount(const QModelIndex& parent = QModelIndex()) const; 
            virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual Qt::ItemFlags flags(const QModelIndex &index) const;
            virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
            virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        private:
            class Private;
            Private* const d;
    };
#endif

    /**
    * The listview that displays the items provided by the \a GUIManagerModel
    * model and offers a collection of actions to run, stop, install, uninstall
    * and to get new scripts.
    */
    class KROSS_EXPORT GUIManagerView : public QTreeView
    {
            Q_OBJECT
        public:
            GUIManagerView(GUIManagerModule* module, QWidget* parent);
            virtual ~GUIManagerView();

            /**
            * \return true if the user changed some data else if the data
            * was not changed at all return false.
            */
            bool isModified() const;

            /**
            * Install the scriptpackage \p file . The scriptpackage should be a
            * tar.gz or tar.bzip archivefile.
            *
            * \param scriptpackagefile The local tar.gz or tar.bzip archivfile
            * which contains the files that should be installed.
            * \return true if installing was successfully else false.
            */
            bool installPackage(const QString& scriptpackagefile);

            /**
            * Uninstalls the scriptpackage \p action and removes all to the package
            * belonging files.
            *
            * \param action The \a Action that should be removed.
            * \return true if the uninstall was successfully else false.
            */
            bool uninstallPackage(Action* action);

        public slots:
            void slotRun();
            void slotStop();
            bool slotInstall();
            void slotUninstall();
            void slotNewScripts();

        private slots:
            void slotSelectionChanged();
            void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
            void slotNewScriptsInstallFinished();

        private:
            class Private;
            Private* const d;
    };

    /**
    * The GUIManagerModule class provides access to the GUIManager
    * functionality.
    */
    class KROSS_EXPORT GUIManagerModule : public QObject
    {
            Q_OBJECT
        public:
            GUIManagerModule();
            virtual ~GUIManagerModule();

        public slots:

            /**
            * Display the "Script Manager" KDialog.
            */
            void showManagerDialog();

        private:
            class Private;
            Private* const d;
    };
}

#endif
