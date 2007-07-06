/***************************************************************************
 * KoScriptingPart.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef KOKROSS_KOSCRIPTINGPART_H
#define KOKROSS_KOSCRIPTINGPART_H

#include <QObject>
#include <QStringList>

#include <kparts/plugin.h>

class KDialog;
class KoScriptingModule;
namespace Kross {
    class Action;
}

#define KOKROSS_EXPORT KDE_EXPORT

/**
* The KoScriptingPart class implements the top-level guiclient
* functionality to integrate scripting using Kross into a KOffice
* application.
*/
class KOKROSS_EXPORT KoScriptingPart : public KParts::Plugin
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        *
        * \param parent The parent QObject.
        * \param args the optional list of arguments.
        */
        KoScriptingPart(KoScriptingModule* const module, const QStringList& args);

        /**
        * Destructor.
        */
        virtual ~KoScriptingPart();

        /**
        * \return the \a KoScriptingModule instance that is the top-level module for
        * the scripting backends.
        */
        KoScriptingModule* module() const;

        /**
        * Show the "Execute Script File" filedialog that allows the user to pick
        * a scripting file and execute it.
        * \return true if the user did choose a file and the file got executed successful.
        */
        static bool showExecuteScriptFile();

    protected Q_SLOTS:

        /**
        * Show the modal "Execute Script File" dialog.
        */
        virtual void slotShowExecuteScriptFile();

        /**
        * The scripts-menu is about to show, update the content.
        */
        virtual void slotMenuAboutToShow();

        /**
        * Show the modal "Script Manager" dialog.
        */
        virtual void slotShowScriptManager();

        /**
        * Called if a script got executed.
        */
        virtual void slotStarted(Kross::Action*);

        /**
        * Called if execution of a script finished.
        */
        virtual void slotFinished(Kross::Action*);

        /**
        * Called if the script finalized.
        */
        virtual void slotFinalized(Kross::Action*);

    protected:
        virtual void myStarted(Kross::Action*) {}
        virtual void myFinished(Kross::Action*) {}
        virtual void myFinalized(Kross::Action*) {}

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

#endif
