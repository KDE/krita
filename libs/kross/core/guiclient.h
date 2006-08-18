/***************************************************************************
 * guiclient.h
 * This file is part of the KDE project
 * copyright (C) 2005-2006 by Sebastian Sauer (mail@dipe.org)
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

#ifndef KROSS_GUICLIENT_H
#define KROSS_GUICLIENT_H

#include "../core/krossconfig.h"
#include "../core/action.h"

#include <QObject>
//#include <QList>
#include <qdom.h>
#include <kurl.h>
#include <kxmlguiclient.h>
//class QWdiget;

namespace Kross {

    /**
     * The GUIClient class provides abstract access to
     * scripting code used to extend an applications functionality.
     */
    class KDE_EXPORT GUIClient
        : public QObject
        , public KXMLGUIClient
    {
            Q_OBJECT

        public:

            /**
             * Constructor.
             *
             * \param guiclient The KXMLGUIClient this \a GUIClient
             *        is a child of.
             * \param parent The parent QWidget. If defined Qt will handle
             *        freeing this \a GUIClient instance else the
             *        caller has to take care of freeing this instance
             *        if not needed any longer.
             */
            explicit GUIClient(KXMLGUIClient* guiclient, QWidget* parent = 0);

            /**
             * Destructor.
             */
            virtual ~GUIClient();

            /**
             * KXMLGUIClient overloaded method to set the XML file.
             */
            virtual void setXMLFile(const QString& file, bool merge = false, bool setXMLDoc = true);

            /**
             * KXMLGUIClient overloaded method to set the XML DOM-document.
             */
            virtual void setDOMDocument(const QDomDocument &document, bool merge = false);

            /**
             * \return the KActionCollection which holds the list of \a Action instances.
             */
            KActionCollection* scriptsActionCollection() const;

            /**
             * Read the configurations like e.g. the installed script-packages
             * from the KConfig \p config configuration-backend.
             */
            void readConfig(KConfig* config);

            /**
             * Write the configurations to the \p config configuration-backend.
             */
            void writeConfig(KConfig* config);

#if 0
            /**
             * Reload the list of installed scripts.
             */
            void loadScriptConfig();

            /**
             * Install the packagefile \p scriptpackagefile . Those
             * packagefile should be a tar.gz-archive which will be
             * extracted and to the users script-directory.
             */
            bool installScriptPackage(const QString& scriptpackagefile);

            /**
             * Uninstall the scriptpackage located in the path
             * \p scriptpackagepath . This just deletes the whole
             * directory.
             */
            bool uninstallScriptPackage(const QString& scriptpackagepath);

            /**
             * Load the scriptpackage's configurationfile
             * \p scriptconfigfile and add the defined \a ScriptAction
             * instances to the list of installed scripts.
             */
            bool loadScriptConfigFile(const QString& scriptconfigfile);

            /**
             * Load the \p document DOM-document from the scriptpackage's
             * XML-configfile \p scriptconfigfile and add the defined
             * \a ScriptAction instances to the list of installed scripts.
             */
            bool loadScriptConfigDocument(const QString& scriptconfigfile, const QDomDocument &document);
#endif

        protected slots:

           /**
            * A KFileDialog will be displayed to let the user choose
            * a scriptfile. The choosen file will be returned as KUrl.
            */
            KUrl openFile(const QString& caption);

        public slots: // to execute a scriptfile

            /**
            * A KFileDialog will be displayed to let the user choose the scriptfile
            * that should be executed.
            */
            bool executeFile();

            /**
            * Execute the scriptfile \p file . Internaly we try to use
            * the defined filename to auto-detect the \a Interpreter which
            * should be used for the execution.
            */
            bool executeFile(const KUrl& file);

            /**
             * This method executes the \a Action \p action . Internaly we just
             * call \a Action::trigger and redirect the success/failed signals
             * to our internal slots.
             */
            bool executeAction(Action::Ptr action);

#if 0
           /**
            * A KFileDialog will be displayed to let the user choose
            * a scriptfile that should be loaded.
            * Those loaded \a ScriptAction will be added to the
            * \a ScriptActionCollection of loaded scripts.
            */
            bool loadScriptFile();
#endif

            /**
            * The \a ScriptManagerGUI dialog will be displayed to
            * let the user manage the scriptfiles.
            */
            void showManager();

        private slots:

            /**
             * Called if execution failed and displays an errormessage-dialog.
             */
            void executionFailed(const QString& errormessage, const QString& tracedetails);

            /**
             * Called if execution was successfully.
             */
            void executionSuccessfull();

        signals:
#if 0
            /// Emitted if a \a ScriptActionCollection instances changed.
            void collectionChanged(ScriptActionCollection*);
#endif

            /**
             * This signal is emited when the execution of a script is started.
             */
            void executionStarted(const Action*);

            /**
             * This signal is emited when the execution of a script is finished.
             */
            void executionFinished(const Action*);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

