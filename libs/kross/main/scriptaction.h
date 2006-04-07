/***************************************************************************
 * scriptaction.h
 * This file is part of the KDE project
 * copyright (C) 2005 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_API_SCRIPTACTION_H
#define KROSS_API_SCRIPTACTION_H

#include <qdom.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>
#include <kaction.h>

#include "scriptcontainer.h"

namespace Kross { namespace Api {

    // Forward declarations.
    class ScriptContainer;
    class ScriptActionCollection;
    class ScriptActionPrivate;

    /**
     * A ScriptAction extends a KAction by providing a wrapper around
     * a \a ScriptContainer to execute scripting code on activation.
     */
    class ScriptAction
        : public KAction
        , public Kross::Api::ScriptContainer
    {
            Q_OBJECT

            /// The name of the interpreter used to execute the scripting code.
            //Q_PROPERTY(QString interpretername READ getInterpreterName WRITE setInterpreterName)

            /// The scripting code which should be executed.
            //Q_PROPERTY(QString code READ getCode WRITE setCode)

            /// The scriptfile which should be executed.
            //Q_PROPERTY(QString file READ getFile WRITE setFile)

            /// The description for this \a ScriptAction .
            Q_PROPERTY(QString description READ getDescription WRITE setDescription)

        public:

            /// Shared pointer to implement reference-counting.
            typedef KSharedPtr<ScriptAction> Ptr;

            /// A list of \a ScriptAction instances.
            //typedef QValueList<ScriptAction::Ptr> List;

            /**
             * Constructor.
             *
             * \param file The KUrl scriptfile this \a ScriptAction
             *        points to.
             */
            explicit ScriptAction(const QString& file);

            /**
             * Constructor.
             *
             * \param scriptconfigfile The XML-configurationfile
             *        the DOM-element was readed from.
             * \param element The QDomElement which will be used
             *        to setup the \a ScriptAction attributes.
             */
            explicit ScriptAction(const QString& scriptconfigfile, const QDomElement& element);

            /**
             * Destructor.
             */
            virtual ~ScriptAction();

            /**
             * \return the version this script has. Versions are used
             * to be able to manage different versions of the same
             * script. The version is 0 by default if not defined to
             * something else in the rc-file.
             */
            int version() const;

            /**
             * \return the unique name this action will be accessible as.
             */
            const QString getName() const;

           /**
             * Set the unique name this action will be accessible as.
             */
            void setName(const QString& name);

            /**
             * \return the description for this \a ScriptAction has.
             */
            const QString getDescription() const;

            /**
             * Set the description \p description for this \a ScriptAction .
             */
            void setDescription(const QString& description);

            /**
             * Set the name of the interpreter which will be used
             * on activation to execute the scripting code.
             *
             * \param name The name of the \a Interpreter . This
             *        could be e.g. "python".
             */
            void setInterpreterName(const QString& name);

            /**
             * \return the path of the package this \a ScriptAction
             * belongs to or QString::null if it doesn't belong to
             * any package.
             */
            const QString getPackagePath() const;

            /**
             * \return a list of all kind of logs this \a ScriptAction
             * does remember.
             */
            const QStringList& getLogs() const;

            /**
             * Attach this \a ScriptAction to the \a ScriptActionCollection
             * \p collection .
             */
            void attach(ScriptActionCollection* collection);

            /**
             * Detach this \a ScriptAction from the \a ScriptActionCollection
             * \p collection .
             */
            void detach(ScriptActionCollection* collection);

            /**
             * Detach this \a ScriptAction from all \a ScriptActionCollection
             * instance his \a ScriptAction is attached to.
             */
            void detachAll();

        public slots:

            /**
             * If the \a ScriptAction got activated the \a ScriptContainer
             * got executed. Once this slot got executed it will emit a
             * \a success() or \a failed() signal.
             */
            virtual void activate();

            /**
             * This slot finalizes the \a ScriptContainer and tries to clean
             * any still running script.
             */
            void finalize();

        signals:

            /**
             * This signal got emitted when this action is emitted before execution.
             */
            void activated(const Kross::Api::ScriptAction*);
            
            /**
            * This signal got emitted after this \a ScriptAction got
            * executed successfully.
            */
            void success();

            /**
            * This signal got emitted after the try to execute this
            * \a ScriptAction failed. The \p errormessage contains
            * the error message.
            */
            void failed(const QString& errormessage, const QString& tracedetails);

        private:
            /// Internaly used private d-pointer.
            ScriptActionPrivate* d;
    };

    /**
     * A collection to store \a ScriptAction shared pointers.
     *
     * A \a ScriptAction instance could be stored within
     * multiple \a ScriptActionCollection instances.
     */
    class ScriptActionCollection
    {
        private:

            /**
             * The list of \a ScriptAction shared pointers.
             */
            Q3ValueList<ScriptAction::Ptr> m_list;

            /**
             * A map of \a ScriptAction shared pointers used to access
             * the actions with there name.
             */
            QMap<Q3CString, ScriptAction::Ptr> m_actions;

            /**
             * A KActionMenu which could be used to display the
             * content of this \a ScriptActionCollection instance.
             */
            KActionMenu* m_actionmenu;

            /**
             * Boolean value used to represent the modified-state. Will
             * be true if this \a ScriptActionCollection is modified
             * aka dirty and e.g. the \a m_actionmenu needs to be
             * updated else its false.
             */
            bool m_dirty;

            /**
             * Copy-constructor. The cctor is private cause instances
             * of this class shouldn't be copied. If that changes one
             * day, don't forgot that it's needed to copy the private
             * member variables as well or we may end in dirty
             * crashes :)
             */
            ScriptActionCollection(const ScriptActionCollection&) {}

        public:

            /**
             * Constructor.
             *
             * \param text The text used to display some describing caption.
             * \param ac The KActionCollection which should be used to as
             *        initial content for the KActionMenu \a m_actionmenu .
             * \param name The internal name.
             */
            ScriptActionCollection(const QString& text, KActionCollection* ac, const char* name)
                : m_actionmenu( new KActionMenu(text, ac, name) )
                , m_dirty(true) {}


            /**
             * Destructor.
             */
            ~ScriptActionCollection() {
                for(Q3ValueList<ScriptAction::Ptr>::Iterator it = m_list.begin(); it != m_list.end(); ++it)
                    (*it)->detach(this);
            }

            /**
             * \return the \a ScriptAction instance which has the name \p name
             * or NULL if there exists no such action.
             */
            ScriptAction::Ptr action(const Q3CString& name) { return m_actions[name]; }

            /**
             * \return a list of actions.
             */
            Q3ValueList<ScriptAction::Ptr> actions() { return m_list; }

            /**
             * \return the KActionMenu \a m_actionmenu .
             */
            KActionMenu* actionMenu() { return m_actionmenu; }

            /**
             * Attach a \a ScriptAction instance to this \a ScriptActionCollection .
             */
            void attach(ScriptAction::Ptr action) {
                m_dirty = true;
                m_actions[ action->name() ] = action;
                m_list.append(action);
                m_actionmenu->insert(action.data());
                action->attach(this);
            }

            /**
             * Detach a \a ScriptAction instance from this \a ScriptActionCollection .
             */
            void detach(ScriptAction::Ptr action) {
                m_dirty = true;
                m_actions.remove(action->name());
                m_list.remove(action);
                m_actionmenu->remove(action.data());
                action->detach(this);
            }

            /**
             * Clear this \a ScriptActionCollection . The collection
             * will be empty and there are no actions attach any longer.
             */
            void clear() {
                for(Q3ValueList<ScriptAction::Ptr>::Iterator it = m_list.begin(); it != m_list.end(); ++it) {
                    m_actionmenu->remove( (*it).data() );
                    (*it)->detach(this);
                }
                m_list.clear();
                m_actions.clear();
            }

    };

}}

#endif

