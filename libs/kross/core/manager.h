/***************************************************************************
 * manager.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_MANAGER_H
#define KROSS_MANAGER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QObject>

#include "krossconfig.h"
#include "childreninterface.h"

class QAbstractItemModel;
class KActionCollection;
class KMenu;

namespace Kross {

    // Forward declarations.
    class Interpreter;
    class Action;
    class InterpreterInfo;

    /**
     * The Manager class is the main entry point to work with
     * Kross. It spends an abstraction layer between what is
     * under the hood of Kross and the functionality you need
     * to access.
     * Use \a Interpreter to just work with some implementated
     * interpreter like python. While \a Script spends a more
     * flexible container.
     */
    class KDE_EXPORT Manager : public QObject, public ChildrenInterface
    {
            Q_OBJECT

        protected:

            /**
             * Protected constructor. Use \a self() to access the Manager
             * singleton instance.
             */
            Manager();

        public:

            /**
             * Destructor.
             */
            ~Manager();

            /**
             * Return the Manager instance. Always use this
             * function to access the Manager singleton.
             */
            static Manager& self();

            /**
             * \return a map with \a InterpreterInfo* instances
             * used to describe interpreters.
             */
            QMap<QString, InterpreterInfo*> interpreterInfos();

            /**
             * \return true if there exists an interpreter with the
             * name \p interpretername else false.
             */
            bool hasInterpreterInfo(const QString& interpretername) const;

            /**
             * \return the \a InterpreterInfo* matching to the defined
             * \p interpretername or NULL if there does not exists such
             * a interpreter.
             */
            InterpreterInfo* interpreterInfo(const QString& interpretername);

            /**
             * Return the name of the \a Interpreter that feels responsible
             * for the defined \p file .
             *
             * \param file The filename we should try to determinate the
             * interpretername for.
             * \return The name of the \a Interpreter which will be used
             * to execute the file or QString::null if we failed to determinate
             * a matching interpreter for the file.
             */
            const QString interpreternameForFile(const QString& file);

            /**
             * Return the \a Interpreter instance defined by
             * the interpretername.
             *
             * \param interpretername The name of the interpreter.
             * e.g. "python" or "kjs".
             * \return The Interpreter instance or NULL if there does not exists
             * an interpreter with such an interpretername.
             */
            Interpreter* interpreter(const QString& interpretername);

            /**
             * Read the configurations like e.g. the installed script-packages
             * from the KConfig configuration-backend.
             */
            bool readConfig();

            /**
             * Write the configurations to the KConfig configuration-backend.
             */
            bool writeConfig();

            /**
             * \return a collection of all \a Action instances.
             */
            KActionCollection* actionCollection() const;

        public slots:

            /**
             * \return a list of names of all supported scripting interpreters.
             * The list may contain for example "python", "ruby" and "kjs" depending
             * on what interpreter-plugins are installed.
             */
            QStringList interpreters();

            /**
            * \return true if there exists a \a Action QObject instance
            * which is child of this \a Manager instance and is defined as \p name
            * else false is returned.
            */
            bool hasAction(const QString& name);

            /**
            * \return the \a Action QObject instance defined with \p name which is
            * child of this \a Manager instance. If there exists no such \a Action
            * yet, create one.
            */
            QObject* action(const QString& name);

            /**
             * Load and return an external module. Modules are dynamic loadable
             * plugins which could be loaded on demand to provide additional
             * functionality.
             *
             * \param modulename The name of the module we should try to load.
             * \return The QObject instance that repesents the module or NULL
             * if loading failed.
             */
            QObject* module(const QString& modulename);

        signals:

            /**
             * This signal is emitted when the execution of a script is started.
             */
            void started(Kross::Action*);

            /**
             * This signal is emitted when the execution of a script is finished.
             */
            void finished(Kross::Action*);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

