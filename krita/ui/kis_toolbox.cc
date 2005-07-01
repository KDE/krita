/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <qbuttongroup.h>
#include <qnamespace.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qpixmap.h>

#include <klocale.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kdualcolorbutton.h>
#include <kseparator.h>

#include <koMainWindow.h>

#include "kis_view.h"
#include "kis_doc.h"
#include "kis_tool.h"
#include "kis_tool_registry.h"
#include "kis_factory.h"

#include "kis_toolbox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KisToolBox::KisToolBox( KisView * view, KMainWindow *mainWin, const char* name )
	: KToolBar( mainWin, name, false, false ),
	  m_view( view )
{
	setFullSize( false );
	m_buttonGroup = new QButtonGroup( 0L );
	m_buttonGroup->setExclusive( true );
	connect( m_buttonGroup, SIGNAL( pressed( int ) ), this, SLOT( slotButtonPressed( int ) ) );

	QBoxLayout::Direction d = orientation() == Qt::Vertical ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	QWidget * base = new QWidget( this );
	m_columnsLayouter = new QBoxLayout( base, d );


	d = orientation() == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	m_left = new QWidget( base );
	m_leftLayout = new QBoxLayout( m_left, d );

	m_columnsLayouter->addWidget( m_left );

	m_right = new QWidget( base );
	m_rightLayout = new QBoxLayout( m_right, d );
	
	m_columnsLayouter->addWidget( m_right );

	m_insertLeft = true;

	// Color button (and perhaps later other control information
	m_colorButton = new KDualColorButton(this);

	// Create separate lists for the various sorts of tools
	// XXX: magic number 5: the number entries in the relevant enum. Make this param.
	for (int i = 0; i < 5; ++i) {
		ToolList * tl = new ToolList();
		tl->resize(i * 10); // See Karbon vtoolbox.cc for this
		m_tools.append(tl);
	}
}

KisToolBox::~KisToolBox()
{
	// Delete the lists owned; the owned lists do not delete their tools,
	// since those are owned by the tool controller
	m_tools.setAutoDelete(true);
}


void KisToolBox::slotPressButton( int id )
{
	m_buttonGroup->setButton( id );
	slotButtonPressed( id );
}

void KisToolBox::slotButtonPressed( int id )
{
	if( id != m_buttonGroup->selectedId() && m_buttonGroup->selected() ) {
		m_buttonGroup->selected()->setDown( false );
	}

	int start = 0;
	
	for (uint i = 0; i < m_tools.count(); ++i) {
		ToolList * tl = m_tools.at(i);
		if (!tl) continue;

		start += tl->count();
		if (id < start)
			emit activeToolChanged( tl->at( id ) );
			return;
	}
}

void KisToolBox::registerTool( KisTool *tool )
{
	uint prio = tool->priority();
	int toolType= (int) tool->toolType();
	ToolList * tl = m_tools.at(toolType);
	if (prio == 0) {
		tl->insert(tl->count(), tool);
	}
	else {
		tl->insert(prio -1, tool);
	}
}

void KisToolBox::setupTools()
{
	int id = 0;
	
	for (uint i = 0; i < m_tools.count(); ++i) {
		ToolList * tl = m_tools.at(i);
		if (!tl) continue;

		for (uint j = 0; j < tl->count(); ++j) {
			KisTool *tool = tl->at(j);
			if (tool)
				addButton(tool->icon().latin1(), tool->name(), id++);
		}
			
	}

	if( !m_insertLeft ) // uneven count, make dummy button
		addButton( "krita", "", id );

	// select first (select tool)
	m_buttonGroup->setButton( 0 );
}

QToolButton * KisToolBox::addButton( const char* iconName, QString tooltip, int id )
{
	QToolButton *button = new QToolButton( m_insertLeft ? m_left : m_right );
	
	if ( iconName != "" ) {
		QPixmap pixmap = BarIcon( iconName, KisFactory::global() );
		button->setPixmap( pixmap );
		button->setToggleButton( true );
	}
	
	if ( !tooltip.isEmpty() ) {
		QToolTip::add( button, tooltip );
	}
	
	if ( m_insertLeft ) {
		m_leftLayout->addWidget( button );
	}
	else {
		m_rightLayout->addWidget( button );
	}

	m_buttonGroup->insert( button, id );
	m_insertLeft = !m_insertLeft;

	return button;

}


void KisToolBox::setOrientation ( Qt::Orientation o )
{
	if( barPos() == Floating ) { // when floating, make it a standing toolbox.
		o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
	}
	QBoxLayout::Direction d = o == Qt::Vertical ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	m_columnsLayouter->setDirection( d );
	d = o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	m_leftLayout->setDirection( d );
	m_rightLayout->setDirection( d );
	QDockWindow::setOrientation( o );
}


#include "kis_toolbox.moc"
