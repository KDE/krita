/***************************************************************************
 * scriptmanager.h
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

#ifndef KROSS_SCRIPTMANAGER_H
#define KROSS_SCRIPTMANAGER_H

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
    class ScriptManagerModule;

    /**
    * The ScriptManagerCollection class shows a QListView where the content of a
    * \a ActionCollection is displayed and some buttons to run, stop, install,
    * uninstall and to get new scripts.
    */
    class KROSS_EXPORT ScriptManagerCollection : public QWidget
    {
            Q_OBJECT
        public:
            ScriptManagerCollection(ScriptManagerModule* module, QWidget* parent);
            virtual ~ScriptManagerCollection();
            ScriptManagerModule* module() const;
            bool isModified() const;

        public slots:
            void slotRun();
            void slotStop();
            bool slotInstall();
            //void slotUninstall();
            //void slotNewScripts();

        private slots:
            void slotSelectionChanged();
            void slotDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
            //void slotNewScriptsInstallFinished();

        private:
            class Private;
            Private* const d;
    };

    /**
    * The ScriptManagerModule provides access to the Script Manager
    * functionality like the "Script Manager" KDialog.
    */
    class KROSS_EXPORT ScriptManagerModule : public QObject
    {
            Q_OBJECT
        public:
            ScriptManagerModule();
            virtual ~ScriptManagerModule();

        public slots:

            /**
            * Install the scriptpackage \p file . The scriptpackage should be a
            * tar.gz or tar.bzip archivefile.
            *
            * \param scriptpackagefile The local tar.gz or tar.bzip archivfile
            * which contains the files that should be installed.
            * \return true if installing was successfully else false.
            */
            bool installPackage(const QString& scriptpackagefile);

#if 0
            /**
            * Uninstalls the scriptpackage \p action and removes all to the package
            * belonging files.
            *
            * \param action The \a Action that should be removed.
            * \return true if the uninstall was successfully else false.
            */
            bool uninstallPackage(Action* action);
#endif
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
