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
#include <koMainWindow.h>
#include <kseparator.h>

#include "kis_view.h"
#include "kis_doc.h"
#include "kis_tool.h"
#include "kis_tool_registry.h"

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
	rightLayout = new QBoxLayout( m_right, d );
	
	m_columnsLayouter->addWidget( m_right );

	m_insertLeft = true;

	// setup tool collections
	m_manipulationtools.resize( 10 );
	m_shapetools.resize( 20 );
	m_misctools.resize( 30 );
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

	int freehandstart = m_freehandtools.count();
	int shapestart = freehandstart + m_manipulationtools.count();
	int transformstart = transform
	int miscstart = m_manipulationtools.count() + m_shapetools.count();
	int miscend = miscstart + m_misctools.count();
	if( id < shapestart )
		emit activeToolChanged( m_manipulationtools.at( id ) );
	else if( id < miscstart )
		emit activeToolChanged( m_shapetools.at( id - shapestart ) );
	else if( id < miscend )
		emit activeToolChanged( m_misctools.at( id - miscstart ) );
}

void
KisToolBox::registerTool( VTool *tool )
{
	kdDebug(38000) << "KisToolBox::registerTool : " << tool->name() << endl;
	uint prio = tool->priority();
	if( tool->category() == "shapecreation" )
		m_shapetools.insert( ( prio == 0 ) ? m_shapetools.count() : prio - 1, tool );
	else if( tool->category() == "manipulation" )
		m_manipulationtools.insert( ( prio == 0 ) ? m_manipulationtools.count() : prio - 1, tool );
	else
		m_misctools.insert( ( prio == 0 ) ? m_misctools.count() : prio - 1, tool );
}

void
KisToolBox::setupTools()
{
	QDictIterator<VTool> itr( m_part->toolController()->tools() );
	kdDebug(38000) << "count : " << m_part->toolController()->tools().count() << endl;
	for( ; itr.current() ; ++itr )
		registerTool( itr.current() );

	int id = 0;
	for( uint i = 0; i < m_manipulationtools.count() ; i++ )
	{
		VTool *tool = m_manipulationtools.at( i );
		if( tool )
			addButton( tool->icon().latin1(), tool->name(), id++ );
	}

	for( uint i = 0; i < m_shapetools.count() ; i++ )
	{
		VTool *tool = m_shapetools.at( i );
		if( tool )
			addButton( tool->icon().latin1(), tool->name(), id++ );
	}

	for( uint i = 0; i < m_misctools.count() ; i++ )
	{
		VTool *tool = m_misctools.at( i );
		if( tool )
			addButton( tool->icon().latin1(), tool->name(), id++ );
	}
	if( !insertLeft ) // uneven count, make dummy button
		addButton( "karbon", "", id );
	// select first (select tool)
	buttonGroup->setButton( 0 );
}

QToolButton *
KisToolBox::addButton( const char* iconName, QString tooltip, int id )
{
	kdDebug(38000) << "Adding : " << iconName << endl;
	QToolButton *button = new QToolButton( insertLeft ? left : right );
	if( iconName != "" )
	{
		QPixmap pixmap = BarIcon( iconName, KarbonFactory::instance() );
		button->setPixmap( pixmap );
		button->setToggleButton( true );
	}
	if( !tooltip.isEmpty() )
	QToolTip::add( button, tooltip );
	if( insertLeft )
		leftLayout->addWidget( button );
	else
		rightLayout->addWidget( button );

	buttonGroup->insert( button, id );
	insertLeft =! insertLeft;

	return button;
}


void
KisToolBox::setOrientation ( Qt::Orientation o )
{
	if( barPos() == Floating ) { // when floating, make it a standing toolbox.
		o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
	}
	QBoxLayout::Direction d = o == Qt::Vertical ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	columnsLayouter->setDirection( d );
	d = o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
	leftLayout->setDirection( d );
	rightLayout->setDirection( d );
	QDockWindow::setOrientation( o );
}


#include "kis_toolbox.moc"

