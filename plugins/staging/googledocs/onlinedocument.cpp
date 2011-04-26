/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <klocale.h>
#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kurl.h>
#include <KoView.h>
#include <KoDocument.h>
#include <kpluginfactory.h>
#include <kparts/partmanager.h>
#include <KoMainWindow.h>
#include <onlinedocument.moc>
#include "loginwindow.h"
#include "googledocumentservice.h"

K_PLUGIN_FACTORY(OnlineDocumentFactory, registerPlugin<OnlineDocument>();)
K_EXPORT_PLUGIN(OnlineDocumentFactory("googledocs_plugin"))

OnlineDocument::OnlineDocument(QObject *parent, const QVariantList &)
    : KParts::Plugin(parent)
    , m_login(0)
{
    setComponentData(OnlineDocumentFactory::componentData());

    KAction *action  = new KAction(i18n("&Google Online Document..."), this);
    actionCollection()->addAction("google_docs", action );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotOnlineDocument()));
}

OnlineDocument::~OnlineDocument()
{
    delete m_login;
}

void OnlineDocument::slotOnlineDocument()
{
//    if (0 = m_login) {
        m_login = new LoginWindow();
        if (QDialog::Accepted == m_login->exec()) {
            connect(m_login->googleService(), SIGNAL(receivedDocument(QString)), this,
                    SLOT(receivedOnlineDocument(QString )));
        }
//    } else {
//        m_login->
//    }
}

void OnlineDocument::receivedOnlineDocument(QString  path)
{
    KoView *view = dynamic_cast<KoView *>(parent());
    if (!view) {
        return;
    }

    KUrl url;
    url.setPath(path);
    view->shell()->openDocument(url);
}
