/* This file is part of the KDE libraries
    Copyright (C) 2001 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPartSelectAction.h"
#include "KoPartSelectDia.h"

#include <kdebug.h>
//Added by qt3to4:
#include <Q3ValueList>

#warning "KDE4: TODO change argument as kactionmenu (when all will compile)"

KoPartSelectAction::KoPartSelectAction( const QString& text, KActionCollection* parent, const char* name )
    : KActionMenu( text, parent, name )
{
    init();
}

KoPartSelectAction::KoPartSelectAction( const QString& text, const QString& icon,
                                        KActionCollection* parent, const char* name )
    : KActionMenu( KIcon(icon),text, parent, name )
{
    init();
}

KoPartSelectAction::KoPartSelectAction( const QString& text, const QString& icon,
                                        QObject* receiver, const char* slot,
                                        KActionCollection* parent, const char* name )
    : KActionMenu( KIcon(icon),text, parent, name )
{
    if (receiver)
        connect( this, SIGNAL( activated() ), receiver, slot );
    init();
}

void KoPartSelectAction::init()
{
    // Query for documents
    m_lstEntries = KoDocumentEntry::query();
    Q3ValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
    for( ; it != m_lstEntries.end(); ++it ) {
        KService::Ptr serv = (*it).service();
        if (!serv->genericName().isEmpty()) {
            KAction *action = new KAction(KIcon(serv->icon()), serv->genericName().replace('&',"&&"), parentCollection(), serv->name().toLatin1());
            connect(action, SIGNAL(triggered()), this, SLOT(slotActionActivated()));
            addAction( action );
        }
    }

}

// Called when selecting a part
void KoPartSelectAction::slotActionActivated()
{
    QString servName = sender()->objectName();
    KService::Ptr serv = KService::serviceByName( servName );
    m_documentEntry = KoDocumentEntry( serv );
    trigger();
}

// Called when activating the toolbar button
void KoPartSelectAction::slotActivated()
{
    m_documentEntry = KoPartSelectDia::selectPart( 0L );
    trigger();
}

#include "KoPartSelectAction.moc"
