/* This file is part of the KDE libraries
    Copyright (C) 1998 Torben Weis <weis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoPartSelectDia.h>

#include <kiconloader.h>
#include <klocale.h>
#include <q3listview.h>
#include <QPixmap>
//Added by qt3to4:
#include <Q3ValueList>

/****************************************************
 *
 * KoPartSelectDia
 *
 ****************************************************/

KoPartSelectDia::KoPartSelectDia( QWidget* parent, const char* name ) :
    KDialog( parent )
{
    setButtons( KDialog::Ok | KDialog::Cancel );
    setCaption( i18n("Insert Object") );
    setModal( true );
    setObjectName( name );

    listview = new Q3ListView( this );
    listview->addColumn( i18n( "Object" ) );
    listview->addColumn( i18n( "Comment" ) );
    listview->setAllColumnsShowFocus( TRUE );
    listview->setShowSortIndicator( TRUE );
    setMainWidget( listview );
    connect( listview, SIGNAL( doubleClicked( Q3ListViewItem * ) ),
	     this, SLOT( slotOk() ) );
    connect( listview, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
	     this, SLOT( selectionChanged( Q3ListViewItem * ) ) );

    // Query for documents
    m_lstEntries = KoDocumentEntry::query();
    Q3ValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
    for( ; it != m_lstEntries.end(); ++it ) {
        KService::Ptr serv = (*it).service();
	if (!serv->genericName().isEmpty()) {
    	    Q3ListViewItem *item = new Q3ListViewItem( listview, serv->name(), serv->genericName() );
	    item->setPixmap( 0, SmallIcon( serv->icon() ) );
	}
    }

    selectionChanged( 0 );
    setFocus();
    resize( listview->sizeHint().width() + 20, 300 );
}

void KoPartSelectDia::selectionChanged( Q3ListViewItem *item )
{
    enableButtonOk( item != 0 );
}

KoDocumentEntry KoPartSelectDia::entry()
{
    if ( listview->currentItem() ) {
	Q3ValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
	for ( ; it != m_lstEntries.end(); ++it ) {
	    if ( ( *it ).service()->name() == listview->currentItem()->text( 0 ) )
		return *it;
	}
    }
    return KoDocumentEntry();
}

KoDocumentEntry KoPartSelectDia::selectPart( QWidget *parent )
{
    KoDocumentEntry e;

    KoPartSelectDia *dlg = new KoPartSelectDia( parent, "PartSelect" );
    dlg->setFocus();
    if (dlg->exec() == QDialog::Accepted)
	e = dlg->entry();

    delete dlg;

    return e;
}

#include <KoPartSelectDia.moc>
