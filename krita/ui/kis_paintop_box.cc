/*
 * This file is part of Krita
 *
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

#include <qtoolbox.h>
#include <qlistbox.h>
#include <qlayout.h>

#include <kglobalsettings.h>

#include "kis_paintop.h"

#include "kis_paintop_box.h"

KisPaintOpBox::KisPaintOpBox(QWidget * parent, const char * name)
	: super(parent, name)
{
	QDockWindow::boxLayout() -> setSpacing( 0 );
	QDockWindow::boxLayout() -> setMargin ( 0 );

	setWidget( m_toolbox = new QToolBox( this ) );
	m_toolbox -> setFont( KGlobalSettings::toolBarFont() );
	m_toolbox -> addItem( new QListBox ( this ), "Pencils" );
	m_toolbox -> addItem( new QListBox ( this ), "Pens" );
	m_toolbox -> addItem( new QListBox ( this ), "Brushes" );
	m_toolbox -> addItem( new QListBox ( this ), "Airbrushes" );
	QListBox * lb;
	m_toolbox -> addItem( lb = new QListBox ( this ), "Fills" );
	lb -> insertItem( "Solid fill" );
	lb -> insertItem( "Gradient" );
	lb -> insertItem( "Pattern" );
	m_toolbox -> addItem( new QListBox ( this ), "Chalks" );
	m_toolbox -> addItem( new QListBox ( this ), "Filters" );
	m_toolbox -> addItem( new QListBox ( this ), "Erasers and smudgers" );
}

KisPaintOpBox::~KisPaintOpBox()
{
}

void KisPaintOpBox::plug(QWidget *w, const QString & label)
{
	m_toolbox -> addItem( w,  label );
}

void KisPaintOpBox::plug(QWidget *w, const QString & label, const QIconSet & iconset)
{
	m_toolbox -> addItem( w,  iconset,  label );
}


void KisPaintOpBox::unplug(QWidget *w)
{
	m_toolbox -> removeItem( w );
}

void KisPaintOpBox::showPage(QWidget *w)
{
	m_toolbox -> setCurrentItem( w );
}


#include "kis_paintop_box.moc"
