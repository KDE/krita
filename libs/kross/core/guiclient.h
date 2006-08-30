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
            void readConfig();

            /**
             * Write the configurations to the \p config configuration-backend.
             */
            void writeConfig();

            /**
             * This method tries to determinate all available packages and adds them
             * to the list of scripts. Normaly only those packages defined in the
             * KConfig are enabled and therefore it's not needed to read all the
             * packages what is somewhat slow. So, this method is only needed for
             * purposes, where you like to have all packages in the list, even those
             * ones that are disabled. As example the \a GUIManagerModel uses this
             * to be able to offer the user even those packages he didn't enabled yet.
             */
            void readAllConfigs();

        public slots:

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

            /**
            * A KFileDialog will be displayed to let the user choose a scriptpackage
            * that should be installed.
            */
            bool installPackage();

            /**
            * Install the scriptpackage \p file . The scriptpackage should be a
            * tar.gz or tar.bzip archivefile.
            *
            * \param file The tar.gz or tar.bzip archivfile which contains
            * the files that should be installed.
            * \return true if installing was successfully else false.
            */
            bool installPackage(const KUrl& file);

            /**
            * Uninstalls the scriptpackage \p action and removes all to the package
            * belonging files.
            *
            * \param action The \a Action that should be removed.
            * \return true if the uninstall was successfully else false.
            */
            bool uninstallPackage(Action* action);

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
            void executionStarted(Kross::Action*);

            /**
             * This signal is emited when the execution of a script is finished.
             */
            void executionFinished(Kross::Action*);

        private:
            /// \internal d-pointer class.
            class Private;
            /// \internal d-pointer instance.
            Private* const d;
    };

}

#endif

