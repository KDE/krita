/***************************************************************************
 * action.h
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

#ifndef KROSS_ACTION_H
#define KROSS_ACTION_H

#include <QString>
#include <QVariant>
#include <QObject>

#include <kaction.h>
#include <ksharedptr.h>
#include <koffice_export.h>

#include "errorinterface.h"
#include "childreninterface.h"

namespace Kross {

    // Forward declarations.
    class ActionPrivate;

    /**
     * The Action class is something like a single standalone scriptfile.
     *
     * Once you've such a Action instance you're able to perform actions
     * with it like to execute scripting code. The \a Manager takes care of
     * handling the Action instances application width.
     */
    class KROSS_EXPORT Action : public KAction, public KShared, public ChildrenInterface, public ErrorInterface
    {
            Q_OBJECT

            // We protected the constructor cause Action instances should
            // be created only within the Manager::getAction() method.
            friend class Manager;

        protected:

            /**
             * Constructor.
             *
             * The constructor is protected cause only with the \a ScriptManager
             * it's possible to access \a Action instances.
             *
             * \param name The unique name this Action has. It's used
             * e.g. at the \a Manager to identify the Action. The
             * name is accessible via \a QObject::objectName .
             */
            explicit Action(const QString& name = QString::null);

        public:

            /// Shared pointer to implement reference-counting.
            typedef KSharedPtr<Action> Ptr;

            /**
             * Destructor.
             */
            virtual ~Action();

            /**
             * Return the scriptcode this Action holds.
             */
            QString getCode() const;

            /**
             * Set the scriptcode this Action holds.
             */
            void setCode(const QString& code);

            /**
             * \return the name of the interpreter.
             */
            QString getInterpreterName() const;

            /**
             * Set the name of the interpreter.
             */
            void setInterpreterName(const QString& interpretername);

            /**
             * \return the filename which will be executed.
             */
            QString getFile() const;

            /**
             * Set the filename which will be executed if the triggered signal
             * got emitted. The \p scriptfile needs to be a valid local file or
             * QString::null if you don't like to use a file rather then the
             * with \a setCode() defined scripting code.
             */
            void setFile(const QString& scriptfile);

            /**
             * \return a map of options this \a Action defines.
             * The options are returned call-by-ref, so you are able to
             * manipulate them.
             */
            QMap<QString, QVariant>& getOptions();

            /**
             * \return the value of the option defined with \p name .
             * If there doesn't exists an option with such a name,
             * the \p defaultvalue is returned. If \p recursive is
             * true then first the \a Action options are
             * seeked for the matching \p name and if not found
             * the \a Manager options are seeked for the \p name and
             * if not found either the \p defaultvalue is returned.
             */
            QVariant getOption(const QString name, QVariant defaultvalue = QVariant(), bool recursive = false);

            /**
             * Set the \a Interpreter::Option value.
             */
            bool setOption(const QString name, const QVariant& value);

            /**
             * \return the list of functionnames.
             */
            QStringList functionNames();

            /**
             * Call a function in the script.
             *
             * \param name The name of the function which should be called.
             * \param args The optional list of arguments.
             */
            QVariant callFunction(const QString& name, const QVariantList& args = QVariantList());

#if 0
            /**
             * Execute the script container.
             */
            Object::Ptr execute();

            /**
             * Return a list of functionnames the with
             * \a setCode defined scriptcode spends.
             */
            const QStringList getFunctionNames();

            /**
             * Call a function in the script container.
             *
             * \param functionname The name of the function to call.
             * \param arguments Optional list of arguments passed to the function.
             * \return \a Object instance representing the functioncall returnvalue.
             */
            KSharedPtr<Object> callFunction(const QString& functionname, KSharedPtr<List> arguments = KSharedPtr<List>());

            /**
             * Return a list of classes.
             */
            QStringList getClassNames();

            /**
             * Create and return a new class instance.
             */
            KSharedPtr<Object> classInstance(const QString& classname);
#endif

        private slots:

            /**
             * This private slot is connected with the \a QAction::triggered
             * signal. To execute the script just emit that signal and this
             * slot tries to execute the script.
             */
            void slotTriggered();

        private:
            /// Internaly used private d-pointer.
            ActionPrivate* d;

            /**
             * Initialize the \a Script instance.
             *
             * Normaly it's not needed to call this function direct cause
             * if will be internaly called if needed (e.g. on \a execute ).
             *
             * \return true if the initialization was successfully else
             * false is returned.
             */
            bool initialize();

            /**
             * Finalize the \a Script instance and free's any cached or still
             * running executions. Normaly it's not needed to call this
             * function direct cause the \a Action will take care
             * of calling it if needed.
             */
            void finalize();
    };

}

#endif

