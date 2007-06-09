/***************************************************************************
 * KoScriptingGuiClient.h
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

#ifndef KOKROSS_KOSCRIPTINGGUICLIENT_H
#define KOKROSS_KOSCRIPTINGGUICLIENT_H

//#include <QObject>
//#include <QWidget>
//#include <QPointer>
//#include <KoView.h>
//#include <KoDocument.h>

#include <kross/ui/guiclient.h>

class KDialog;

#define KOKROSS_EXPORT KDE_EXPORT

/**
* The KoScriptingGuiClient class implements the top-level guiclient
* functionality to integrate scripting using Kross into a KOffice
* application.
*/
class KOKROSS_EXPORT KoScriptingGuiClient : public Kross::GUIClient
{
        Q_OBJECT
    public:

        /**
        * Constructor.
        *
        * \param guiclient The parent KXMLGUIClient instance this
        * guiclient is child of. This will be normaly a KParts::Plugin
        * instance that provides an application plugin using the KParts
        * technology.
        * \param parent The parent QObject.
        */
        explicit KoScriptingGuiClient(KXMLGUIClient* guiclient, QObject* parent = 0);

        /**
        * Destructor.
        */
        virtual ~KoScriptingGuiClient();

        static KDialog* showScriptManager();

    protected Q_SLOTS:

        /**
        * Show the modal "Script Manager" dialog.
        */
        void slotShowScriptManager();

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};

#endif
