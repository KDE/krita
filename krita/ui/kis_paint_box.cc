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
#include <qstringlist.h>

#include <kglobalsettings.h>

#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_paint_box.h"



KisPaintBox::KisPaintBox(QWidget * parent, const char * name)
	: super(parent, name)
{
	QDockWindow::boxLayout() -> setSpacing( 0 );
	QDockWindow::boxLayout() -> setMargin ( 0 );

	setWidget( m_toolbox = new QToolBox( this ) );
	m_toolbox -> setFont( KGlobalSettings::toolBarFont() );

	addPaintOps();
}

KisPaintBox::~KisPaintBox()
{
}

void KisPaintBox::plug(QWidget *w, const QString & label)
{
	m_toolbox -> addItem( w,  label );
}

void KisPaintBox::plug(QWidget *w, const QString & label, const QIconSet & iconset)
{
	m_toolbox -> addItem( w,  iconset,  label );
}

QWidget * KisPaintBox::getWidget(const QString & label)
{
	for ( int i = 0; i < m_toolbox -> count(); ++i ) {
		if ( label.compare( m_toolbox -> itemLabel( i ) ) == 0 ) {
			return m_toolbox -> item( i );
		}
	}
	return 0;
}

void KisPaintBox::unplug(QWidget *w)
{
	m_toolbox -> removeItem( w );
}

void KisPaintBox::showPage(QWidget *w)
{
	m_toolbox -> setCurrentItem( w );
}

void KisPaintBox::addPaintOps()
{

	KisPaintOpRegistry * por = KisPaintOpRegistry::instance();
	KisPaintOpFactorySP pof = 0;
	QStringList keys = por -> listKeys();
	for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
// 		pof = por -> get(*it);
// 		if ( pof )
// 			pof -> slot( this );
	}
}


#include "kis_paint_box.moc"
