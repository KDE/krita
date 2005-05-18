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
#include <qtoolbutton.h>
#include <qlabel.h>

#include <kglobalsettings.h>

#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_id.h"
#include "kis_paint_box.h"

#include "wdgdockertoolbox.h"

KisPaintBox::KisPaintBox(QWidget * parent, const char * name)
	: super(parent, name)
{
	QDockWindow::boxLayout() -> setSpacing( 0 );
	QDockWindow::boxLayout() -> setMargin ( 0 );

	setWidget( m_page = new WdgDockerToolBox(this) );

        // Compute a small fontsize for the dockers; not everyone has their
        // toolbar fontsize at a sensible 6 points.
	m_font = KGlobalSettings::toolBarFont();
	QFont f2 = KGlobalSettings::generalFont();
	if (m_font.pointSize() >= f2.pointSize() ) {
		float ps = f2.pointSize() * 0.8;
		m_font.setPointSize((int)ps);
	}	
	m_page -> setFont(m_font);
	m_page -> lblCaption -> setFont(m_font);
	m_page  -> setBaseSize( 175, 125 );
 	if (m_page -> layout() != 0) {
 		m_page -> layout() -> setSpacing(0);
 		m_page -> layout() -> setMargin(0);
 	}

  	QObject::connect(m_page -> bnShade, SIGNAL(toggled(bool)), this, SLOT(shade(bool)));
	QObject::connect(this, SIGNAL(placeChanged(QDockWindow::Place)), this, SLOT(slotPlaceChanged(QDockWindow::Place)));

	addPaintOps();
}

KisPaintBox::~KisPaintBox()
{
}

void KisPaintBox::plug( QWidget *w )
{
	plug( w,  w -> caption() );
}
void KisPaintBox::plug(QWidget *w, const QString & label)
{
	w -> setFont(m_font);
	m_page -> toolBox -> addItem( w,  label );
}

void KisPaintBox::plug(QWidget *w, const QString & label, const QIconSet & iconset)
{
	w -> setFont(m_font);
	m_page -> toolBox -> addItem( w,  iconset,  label );
}

QWidget * KisPaintBox::getWidget(const QString & label)
{
	for ( int i = 0; i < m_page -> toolBox -> count(); ++i ) {
		if ( label.compare( m_page -> toolBox -> itemLabel( i ) ) == 0 ) {
			return m_page -> toolBox -> item( i );
		}
	}
	return 0;
}

void KisPaintBox::unplug(QWidget *w)
{
	m_page -> toolBox -> removeItem( w );
}

void KisPaintBox::showPage(QWidget *w)
{
	m_page -> toolBox -> setCurrentItem( w );
}

void KisPaintBox::addPaintOps()
{

	KisPaintOpRegistry * por = KisPaintOpRegistry::instance();
	KisPaintOpFactorySP pof = 0;
	KisIDList keys = por -> listKeys();
	for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
// 		pof = por -> get(*it);
// 		if ( pof )
// 			pof -> slot( this );
	}
}

void KisPaintBox::setCaption(const QString & caption)
{
	KisBaseDocker::setCaption(caption);
 	m_page -> lblCaption -> setText(caption);
}

void KisPaintBox::shade(bool toggle)
{
	m_shaded = toggle;
	if (!toggle) {
 		m_page -> toolBox -> show();
	}
	else {
 		m_page -> toolBox -> hide();
		if (!m_docked) {
			resize(minimumSize());
		}

	}
}

void KisPaintBox::slotPlaceChanged(QDockWindow::Place p)
{

	if (p == QDockWindow::InDock) {
		m_docked = true;
		m_page -> lblCaption -> show();
		m_page -> bnShade -> show();
		m_page -> lblCaption -> setText(caption());
		resize(sizeHint());
	}
	else {
		m_docked = false;
		m_page -> lblCaption -> hide();
		m_page -> bnShade -> hide();
		m_page -> toolBox -> show();
		m_page -> lblCaption -> setText("");
		resize(sizeHint());
	}

}

#include "kis_paint_box.moc"
