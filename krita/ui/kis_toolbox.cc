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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include <kaction.h>
#include <kactioncollection.h>
#include <kactionclasses.h>

#include <koMainWindow.h>

#include "kis_factory.h"

#include "kis_toolbox.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KisToolBox::KisToolBox( KMainWindow *mainWin, const char* name )
	: KToolBar( mainWin, Qt::DockLeft, false, name, true, true)
{
	setLabel("Krita");
	setName("krita");
	setFullSize( false );
	setMargin(5);
	

	m_buttonGroup = new QButtonGroup( 0L );
	
	m_buttonGroup->setExclusive( true );
	connect( m_buttonGroup, SIGNAL( pressed( int ) ), this, SLOT( slotButtonPressed( int ) ) );
		
	// Create separate lists for the various sorts of tools
	for (int i = 0; i < NUMBER_OF_TOOLTYPES ; ++i) {

		ToolList * tl = new ToolList();
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
	m_idToActionMap.at(id)->activate();

}

void KisToolBox::registerTool( KAction *tool, enumToolType toolType, Q_UINT32 priority )
{
	uint prio = priority;
	ToolList * tl = m_tools.at(toolType);
	
	if (prio == 0) {
		tl->append(tool);
	}
	else {
		if (prio - 1 <= tl->count())
			tl->insert(prio - 1, tool);
		else
			tl->append(tool);
	}
}

void KisToolBox::setupTools()
{
	setBarPos(Left);
	int id = 0;
	QWidget * w = 0;
	// Loop through tooltypes
	for (uint i = 0; i < m_tools.count(); ++i) {
		ToolList * tl = m_tools.at(i);

		if (!tl) continue;
		if (tl->isEmpty()) continue;

		w = new QWidget(this);
		m_buttonParents.append(w);
		
		QGridLayout * gl = new QGridLayout(w, 1, 2, -1);
		gl->setMargin(2);
		gl->setSpacing(1);
		m_layouts.append(gl);

		// Loop through the tools for this tooltype
		int col = 0;
		int row = 0;
		for (uint j = 0; j < tl->count(); ++j) {
			KAction *tool = tl->at(j);
			if (tool) {
				QToolButton * bn = addButton(w, tool->icon().latin1(), tool->name(), id++);
				gl->addWidget(bn, row, col);
				bn->show();
				m_idToActionMap.append( tool );
				++col;
				if (col == gl->numCols()) {
					col = 0;
					++row;
				}
			}
		}
		
		addSeparator();
	}
	// select first (select tool)
	m_buttonGroup->setButton( 0 );
	m_numberOfButtons = id;

	// Color button (and perhaps later other control information
	m_colorButton = new KDualColorButton(this);

}

QToolButton * KisToolBox::addButton(QWidget * parent,  const char* iconName, QString tooltip, int id )
{
	QToolButton *button = new QToolButton(parent);
	
	if ( iconName != "" ) {
		QPixmap pixmap = BarIcon( iconName, KisFactory::global() );
		button->setPixmap( pixmap );
		button->setToggleButton( true );
	}
	
	if ( !tooltip.isEmpty() ) {
		QToolTip::add( button, tooltip );
	}
   
	m_buttonGroup->insert( button, id );

	return button;

}


void KisToolBox::setOrientation ( Qt::Orientation o )
{
	if( barPos() == Floating ) { // when floating, make it a standing toolbox.
                o = o == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
        }
        
	QDockWindow::setOrientation( o );

}

void KisToolBox::enableTools(bool enable)
{
	if (m_tools.isEmpty()) return;
	if (!m_buttonGroup) return;
	if (m_numberOfButtons <= 0) return;

	for (uint i = 0; i < m_tools.count(); ++i) {
		ToolList * tl = m_tools.at(i);
		
		if (!tl) continue;

		for (uint j = 0; j < tl->count(); ++j) {
			KAction *tool = tl->at(j);
			tool->setEnabled(enable);
		}
	}
	m_buttonGroup->setEnabled(enable);
	for (Q_UINT32 i = 0; i < m_numberOfButtons; ++i) {
		m_buttonGroup->find( i )->setEnabled( enable );
	}
}

#include "kis_toolbox.moc"
