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
//#include <QVariant>

#include <ksharedptr.h>

#include "../core/krossconfig.h"
#include "../core/childreninterface.h"

namespace Kross {

    // Forward declarations.
    class Interpreter;
    class Action;
    class ManagerPrivate;
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
    class KDE_EXPORT Manager : public KShared, public ChildrenInterface
    {
        protected:

            /**
             * Constructor. Use \a scriptManager() to access
             * the Manager singleton instance.
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
            QMap<QString, InterpreterInfo*> getInterpreterInfos();

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
            InterpreterInfo* getInterpreterInfo(const QString& interpretername);

            /**
             * \return the name of the \a Interpreter that feels responsible
             * for the defined \p file .
             *
             * \param file The filename we should try to determinate the
             *        interpretername for.
             * \return The name of the \a Interpreter which will be used
             *        to execute the file or QString::null if we failed
             *        to determinate a matching interpreter for the file.
             */
            const QString getInterpreternameForFile(const QString& file);

            /**
             * Return the existing \a Action with scriptname
             * or create a new \a Action instance and associate
             * the passed scriptname with it.
             *
             * \param scriptname The name of the script. This
             *        should be unique for each \a Script and
             *        could be something like the filename.
             * \return The \a Action instance matching to
             *         scriptname.
             */
            KSharedPtr<Action> createAction(const QString& scriptname);

            /**
             * Return the \a Interpreter instance defined by
             * the interpretername.
             *
             * \param interpretername The name of the interpreter.
             *        e.g. "python" or "kjs".
             * \return The Interpreter instance or NULL if there
             *         does not exists an interpreter with such
             *         an interpretername.
             */
            Interpreter* getInterpreter(const QString& interpretername);

            /**
             * \return a list of names of the at the backend
             * supported interpreters.
             */
            const QStringList getInterpreters();

#if 0
            /**
             * Add the an external module to the global shared list of 
             * loaded modules.
             *
             * \param modulename the name of the module
             * \param module The @a Module instace to add.
             * \return true if the module was added successfully else false.
             */
            bool addModule(const QString& modulename, Module::Ptr module);

            /**
             * Load an external module and return it. We don't keep
             * a copy of the module-instance. So, once the caller
             * doesn't hold a sharedptr any longer, the module will
             * get unloaded.
             *
             * \param modulename The name of the library we should try to 
             *        load. Those library needs to be a valid kross module.
             * \return The loaded \a Object or NULL if loading
             *        failed. The loaded Module isn't added to the global
             *        shared list of modules.
             */
            Module::Ptr loadModule(const QString& modulename);
#endif

        private:
            /// Private d-pointer class.
            ManagerPrivate* d;
    };

}

#endif

