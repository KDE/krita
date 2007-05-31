/***************************************************************************
 * KoScriptingGuiClient.cpp
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

#include "KoScriptingGuiClient.h"
#include "KoScriptManager.h"

// qt
//#include <QApplication>
// kde
#include <kaction.h>
#include <kactioncollection.h>
//#include <kdebug.h>
//#include <klocale.h>
// koffice
//#include <KoMainWindow.h>
//#include <KoApplicationAdaptor.h>
//#include <KoDocumentAdaptor.h>
//#include <KoView.h>
//#include <kross/core/manager.h>
//#include <kross/core/model.h>
//#include <kross/core/guiclient.h>
//#include <core/actioncollection.h>

/// \internal d-pointer class.
class KoScriptingGuiClient::Private
{
    public:
};

KoScriptingGuiClient::KoScriptingGuiClient(KXMLGUIClient* guiclient, QObject* parent)
    : Kross::GUIClient(guiclient, parent), d(new Private())
{
    KAction* execaction  = new KAction(i18n("Execute Script File..."), this);
    guiclient->actionCollection()->addAction("executescriptfile", execaction);
    connect(execaction, SIGNAL(triggered(bool)), this, SLOT(slotShowExecuteScriptFile()));

    KAction* manageraction  = new KAction(i18n("Script Manager..."), this);
    guiclient->actionCollection()->addAction("scriptmanager", manageraction);
    connect(manageraction, SIGNAL(triggered(bool)), this, SLOT(slotShowScriptManager()));

    QAction* scriptmenuaction = this->action("scripts");
    if( scriptmenuaction )
        guiclient->actionCollection()->addAction("scripts", scriptmenuaction);
}

KoScriptingGuiClient::~KoScriptingGuiClient()
{
    delete d;
}

void KoScriptingGuiClient::slotShowScriptManager()
{
    KDialog* dialog = new KDialog();
    dialog->setCaption( i18n("Script Manager") );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    dialog->setMainWidget( new KoScriptManagerCollection(dialog->mainWidget()) );
    dialog->resize( QSize(520, 380).expandedTo( dialog->minimumSizeHint() ) );
    int result = dialog->exec();
#if 0
    if ( view->isModified() ) {
        if( result == QDialog::Accepted /*&& dialog->result() == KDialog::Ok*/ ) {
            // save new config
            Manager::self().writeConfig();
        }
        else {
            // restore old config
            Manager::self().readConfig();
        }
        QMetaObject::invokeMethod(&Manager::self(), "configChanged");
    }
#else
    Q_UNUSED(result);
#endif
    dialog->delayedDestruct();
}

#include "KoScriptingGuiClient.moc"
