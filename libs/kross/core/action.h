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
//#include <qdom.h>

#include <kaction.h>
#include <ksharedptr.h>
#include <kurl.h>
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

        public:

            /// Shared pointer to implement reference-counting.
            typedef KSharedPtr<Action> Ptr;

            /**
             * Constructor.
             *
             * \param name The unique name this Action has. It's used
             * e.g. at the \a Manager to identify the Action. The
             * name is accessible via \a QObject::objectName .
             */
            Action(const QString& name = QString::null);

            /**
             * Constructor.
             *
             * \param file The in the KUrl defined path() should point
             * to a valid scriptingfile. This \a Action will be filled
             * with the content of the file (e.g. the file is readed
             * and \a getCode should return it's content and it's also
             * tried to determinate the \a getInterpreterName ).
             *
             * The \p file needs to be a valid local file and can't be
             * changed later cause the file.path() will be used as
             * name for this KAction.
             */
            Action(const KUrl& file);

           /**
             * Constructor.
             *
             * \param collection The KActionCollection this Action
             * is child of.
             * \param name The unique name this Action has. It's used
             * e.g. at the \a Manager to identify the Action. The
             * name is accessible via \a QObject::objectName .
             * \param file The in the KUrl defined path() should point
             * to a valid scriptingfile. This \a Action will be filled
             * with the content of the file (e.g. the file is readed
             * and \a getCode should return it's content and it's also
             * tried to determinate the \a getInterpreterName ).
             */
            Action(KActionCollection* collection, const QString& name, const KUrl& file);

            /**
             * Destructor.
             */
            virtual ~Action();

            /**
             * \return the filename which will be executed.
             */
            KUrl getFile() const;

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

        public slots:

            /**
             * \return the optional description for this Action.
             */
            QString description() const;

            /**
             * Set the optional description for this Action.
             */
            void setDescription(const QString& description);

            /**
             * \return the scriptcode this Action holds.
             */
            QString code() const;

            /**
             * Set the scriptcode \p code this Action should execute.
             */
            void setCode(const QString& code);

            /**
             * \return the name of the interpreter. Could be for
             * example "python" or "ruby".
             */
            QString interpreter() const;

            /**
             * Set the name of the interpreter.
             */
            void setInterpreter(const QString& interpretername);

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
    };

}

#endif

