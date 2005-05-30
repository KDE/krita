/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers
   Copyright (C) 2005 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qhbuttongroup.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qptrvector.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <koMainWindow.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kinputdialog.h>

#include "kis_history_docker.h"

static long g_lastKey = 0;

VHistoryGroupItem::VHistoryGroupItem( VHistoryItem* item, QListView* parent, QListViewItem* after )
		: QListViewItem( parent, after )
{
	setPixmap( 0, *item->pixmap( 0 ) );
	setText( 0, item->text( 0 ) );
	parent->takeItem( item );
	insertItem( item );
	m_key = item->key( 0, true );
}

VHistoryGroupItem::~VHistoryGroupItem()
{
}

void VHistoryGroupItem::paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
{
	int e = 0;
	int n = 0;
	VHistoryItem* item = (VHistoryItem*)firstChild();
	while ( item )
	{
		if ( item->command()->success() )
			e++;
		else
			n++;
		item = (VHistoryItem*)item->nextSibling();
	}
	if ( e > 0 )
	{
		p->fillRect( 0, 0, width, height(), cg.base() );
		if ( n > 0 )
			p->fillRect( 0, 0, width, height(), QBrush( cg.base().dark( 140 ), QBrush::BDiagPattern ) );
	}
	else
		p->fillRect( 0, 0, width, height(), cg.base().dark( 140 ) );

	const QPixmap* pixmap = this->pixmap( column );
	int xstart;
	if ( pixmap )
	{
		int pw = pixmap->width();
		int ph = pixmap->height();
		p->drawPixmap( ( height() - pw ) / 2, ( height() - ph ) / 2, *pixmap );
		xstart = height();
	}
	else
		xstart = 4;
	p->setPen( cg.text() );
	p->drawText( xstart, 0, width - xstart, height(), align | Qt::AlignVCenter, text( column ) );
} 

VHistoryItem::VHistoryItem( VCommand* command, QListView* parent, QListViewItem* after )
		: QListViewItem( parent, after ), m_command( command )
{
	init();
}

VHistoryItem::VHistoryItem( VCommand* command, VHistoryGroupItem* parent, QListViewItem* after )
		: QListViewItem( parent, after ), m_command( command )
{
	init();
}

void VHistoryItem::init()
{
	kdDebug(38000) << "In VHistoryItem::init() : " << m_command->name() << endl;
	char buffer[64];
	sprintf( buffer, "%064ld", ++g_lastKey );
	m_key = buffer;
	setPixmap( 0, QPixmap( KGlobal::iconLoader()->iconPath( m_command->icon(), KIcon::Small ) ) );
	setText( 0, m_command->name() );
} 

VHistoryItem::~VHistoryItem()
{
}

void VHistoryItem::paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
{
	p->fillRect( 0, 0, width - 1, height() - 1, ( m_command->success() ? cg.base() : cg.base().dark( 140 ) ) );

	const QPixmap* pixmap = this->pixmap( column );
	int xstart;
	if ( pixmap )
	{
		int pw = pixmap->width();
		int ph = pixmap->height();
		p->drawPixmap( ( height() - pw ) / 2, ( height() - ph ) / 2, *pixmap );
		xstart = height();
	}
	else
		xstart = 4;
	p->setPen( cg.text() );
	p->drawText( xstart, 0, width - xstart, height(), align | Qt::AlignVCenter, text( column ) );
} 

VHistoryTab::VHistoryTab( KarbonPart* part, QWidget* parent )
		: QWidget( parent ), m_part( part )
{
	QVBoxLayout* layout = new QVBoxLayout( this );
	Q_CHECK_PTR(layout);
	layout->setMargin( 3 );
	layout->setSpacing( 2 );
	layout->add( m_history = new QListView( this ) );
	m_history->setVScrollBarMode( QListView::AlwaysOn );
	m_history->setSelectionMode( QListView::NoSelection );
	m_history->addColumn( i18n( "Commands" ) );
	m_history->setResizeMode( QListView::AllColumns );
	m_history->setRootIsDecorated( true );
	layout->add( m_groupCommands = new QCheckBox( i18n( "Group commands" ), this ) );

	m_history->setSorting( 0, true );
	VHistoryGroupItem* group = 0;
	VHistoryItem* last = 0;
	QPtrVector<VCommand> cmds;
	part->commandHistory()->commands()->toVector( &cmds );
	int c = cmds.count();
	for ( int i = 0; i < c; i++ )
	{
		if ( ( i > 0 ) && ( cmds[ i ]->name() == cmds[ i - 1 ]->name() ) )
			if ( group )
			{
				QListViewItem* prev = group->firstChild();
				while ( prev && prev->nextSibling() )
					prev = prev->nextSibling();
				new VHistoryItem( cmds[ i ], group, prev );
			}
			else
			{
				group = new VHistoryGroupItem( last, m_history, last );
				new VHistoryItem( cmds[ i ], group, last );
			}
		else
		{
			last = new VHistoryItem( cmds[ i ], m_history, last );
			group = 0;
		}
	}
	m_history->sort();

	connect( m_history, SIGNAL( mouseButtonClicked( int, QListViewItem*, const QPoint&, int ) ), this, SLOT( commandClicked( int, QListViewItem*, const QPoint&, int ) ) );
	connect( m_groupCommands, SIGNAL( stateChanged( int ) ), this, SLOT( groupingChanged( int ) ) );
	connect( part->commandHistory(), SIGNAL( historyCleared() ), this, SLOT( historyCleared() ) );
	connect( part->commandHistory(), SIGNAL( commandAdded( VCommand* ) ), this, SLOT( slotCommandAdded( VCommand* ) ) );
	connect( part->commandHistory(), SIGNAL( commandExecuted( VCommand* ) ), this, SLOT( commandExecuted( VCommand* ) ) );
	connect( part->commandHistory(), SIGNAL( firstCommandRemoved() ), this, SLOT( removeFirstCommand() ) );
	connect( part->commandHistory(), SIGNAL( lastCommandRemoved() ), this, SLOT( removeLastCommand() ) );
	connect( this, SIGNAL( undoCommand( VCommand* ) ), part->commandHistory(), SLOT( undo( VCommand* ) ) );
	connect( this, SIGNAL( redoCommand( VCommand* ) ), part->commandHistory(), SLOT( redo( VCommand* ) ) );
	connect( this, SIGNAL( undoCommandsTo( VCommand* ) ), part->commandHistory(), SLOT( undoAllTo( VCommand* ) ) );
	connect( this, SIGNAL( redoCommandsTo( VCommand* ) ), part->commandHistory(), SLOT( redoAllTo( VCommand* ) ) );
}

VHistoryTab::~VHistoryTab()
{
}

bool VHistoryTab::groupingEnabled()
{
	return m_groupCommands->isChecked();
}

void VHistoryTab::historyCleared()
{
	m_history->clear();
} 

void VHistoryTab::commandExecuted( VCommand* command )
{
	QListViewItem* item = m_history->firstChild();
	bool found = false;
	while ( !found && item )
	{
		if ( item->rtti() == 1001 )
		{
			QListViewItem* child = item->firstChild();
			while ( !found && child )
			{
				found = ( ( (VHistoryItem*)child )->command() == command );
				if ( !found )
					child = child->nextSibling();
				else
					item = child;
			}
		}
		found = ( item && ( (VHistoryItem*)item )->command() == command );
		if ( !found )
			item = item->nextSibling();
	}
	if ( found )
	{
		m_history->repaintItem( item );
		if ( item->parent() )
			m_history->repaintItem( item->parent() );
		m_history->ensureItemVisible( item );
	}
} 

void VHistoryTab::slotCommandAdded( VCommand* command )
{
	if ( !command )
		return;

	QListViewItem* last = m_history->firstChild();
	while ( last && last->nextSibling() )
		last = last->nextSibling();

	if( groupingEnabled() )
	{
		if( ( last ) && last->text( 0 ) == command->name() )
		{
			if( last->rtti() == 1002 )
			{
				QListViewItem* prevSibling;
				if( m_history->childCount() > 1 )
				{
					prevSibling = m_history->firstChild();
					while ( prevSibling->nextSibling() != last )
						prevSibling = prevSibling->nextSibling();
				}
				else
					prevSibling = m_history->firstChild();
				last = new VHistoryGroupItem( (VHistoryItem*)last, m_history, prevSibling );
			}
			QListViewItem* prev = last->firstChild();
			while ( prev && prev->nextSibling() )
				prev = prev->nextSibling();
			m_history->setCurrentItem( new VHistoryItem( command, (VHistoryGroupItem*)last, prev ) );
		}
		else
			m_history->setCurrentItem( new VHistoryItem( command, m_history, last ) );
	}
	else
		m_history->setCurrentItem( new VHistoryItem( command, m_history, last ) );

	m_history->sort();
	m_history->ensureItemVisible( m_history->currentItem() );
	m_history->update();
} 

void VHistoryTab::removeFirstCommand()
{
	if ( m_history->childCount() > 0 )
		if ( m_history->firstChild()->rtti() == 1002 )
			delete m_history->firstChild();
		else
		{
			VHistoryGroupItem* group = (VHistoryGroupItem*)m_history->firstChild();
			delete group->firstChild();
			if ( group->childCount() == 1 )
			{
				new VHistoryItem( ( (VHistoryItem*)group->firstChild() )->command(), m_history, 0 );
				delete group;
			}
		}
} 

void VHistoryTab::removeLastCommand()
{
	if ( m_history->childCount() > 0 )
	{
		QListViewItem* last = m_history->firstChild();
		while ( last && last->nextSibling() )
			last = last->nextSibling();
		if ( last->rtti() == 1002 )
			delete last;
		else
		{
			VHistoryGroupItem* group = (VHistoryGroupItem*)last;
			last = group->firstChild();
			while ( last && last->nextSibling() )
				last = last->nextSibling();
			delete last;
			if ( group->childCount() == 1 )
			{
				new VHistoryItem( ( (VHistoryItem*)group->firstChild() )->command(), m_history, group );
				delete group;
			}
		}
	}
}

void VHistoryTab::commandClicked( int button, QListViewItem* item, const QPoint&, int )
{
	if ( !item || item->rtti() == 1001 )
		return;

	VCommand* cmd = ( (VHistoryItem*)item )->command();
	if ( cmd->success() )
		if ( button == 1 )
			emit undoCommandsTo( ( (VHistoryItem*)item )->command() );
		else
			emit undoCommand( ( (VHistoryItem*)item )->command() );
	else
		if ( button == 1 )
			emit redoCommandsTo( ( (VHistoryItem*)item )->command() );
		else
			emit redoCommand( ( (VHistoryItem*)item )->command() );
} 

void VHistoryTab::groupingChanged( int )
{
	if ( m_groupCommands->isChecked() && m_history->childCount() > 1 )
	{
		QListViewItem* s2last = 0;
		QListViewItem* last = m_history->firstChild();
		QListViewItem* item = last->nextSibling();
		while ( item )
			if ( last->text( 0 ) == item->text( 0 ) )
			{
				if ( last->rtti() == 1002 )
					last = new VHistoryGroupItem( (VHistoryItem*)last, m_history, s2last );
				m_history->takeItem( item );
				last->insertItem( item );
				item = last->nextSibling();
			}
			else
			{
				s2last = last;
				last = item;
				item = last->nextSibling();
			}
	}
	else
	{
		QListViewItem* item = m_history->firstChild();
		while ( item )
			if ( item->rtti() == 1001 )
			{
				QListViewItem* child;
				while ( ( child = item->firstChild() ) )
				{
					item->takeItem( child );
					m_history->insertItem( child );
				}
				child = item;
				item = item->nextSibling();
				delete child;
			}
			else
				item = item->nextSibling();
	}
	m_history->sort();
	m_history->update();
} 

#include "kis_history_docker.moc"
