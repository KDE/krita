/*
 *  kis_dockframedocker.cc - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qobject.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qimage.h>
#include <qdockwindow.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktabwidget.h>

#include <koColorChooser.h>
#include <koFrameButton.h>

#include "kis_dockframedocker.h"
#include "dialogs/wdgdockertab.h"

// static const unsigned char img0_wdgdockertab[] = { 
//     0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
//     0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x13,
//     0x08, 0x06, 0x00, 0x00, 0x00, 0x90, 0x8c, 0x2d, 0xb5, 0x00, 0x00, 0x00,
//     0xb2, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xcd, 0x94, 0x51, 0x0a, 0xc4,
//     0x20, 0x0c, 0x44, 0x93, 0x65, 0xbf, 0x05, 0x4b, 0xee, 0x99, 0x13, 0x94,
//     0xd2, 0xde, 0xc0, 0x63, 0x16, 0xa5, 0x82, 0x1e, 0x20, 0xfb, 0xd5, 0xa5,
//     0x54, 0xdb, 0x48, 0x71, 0x61, 0x03, 0x22, 0x04, 0xe6, 0x39, 0x43, 0x54,
//     0xcc, 0x39, 0x0b, 0x74, 0xac, 0xb7, 0x48, 0x57, 0xde, 0x0f, 0x80, 0xe7,
//     0x86, 0x31, 0x06, 0x5b, 0xc5, 0x29, 0xa5, 0xc2, 0x4d, 0xd5, 0x21, 0x33,
//     0x37, 0xd9, 0x5e, 0x96, 0xa5, 0xe8, 0x5d, 0x46, 0x76, 0xce, 0xdd, 0x3a,
//     0x65, 0x66, 0xa9, 0x69, 0x8b, 0xc8, 0x47, 0x81, 0x6a, 0xb1, 0x52, 0xb8,
//     0x6d, 0x9b, 0x2a, 0xb4, 0xd6, 0x62, 0x8c, 0xb1, 0xe9, 0x00, 0x75, 0xca,
//     0xc3, 0x30, 0xe0, 0xee, 0x76, 0x9a, 0x26, 0x15, 0xf8, 0x6a, 0x85, 0x01,
//     0x00, 0xcc, 0xf3, 0xac, 0x03, 0x45, 0x04, 0x6a, 0xeb, 0x0c, 0x3b, 0x42,
//     0xaf, 0x34, 0x22, 0x02, 0xe8, 0xbd, 0xbf, 0xcd, 0x4c, 0x44, 0x5f, 0xf0,
//     0x38, 0x8e, 0xaa, 0xc3, 0x2a, 0x90, 0x88, 0x30, 0x84, 0x20, 0xda, 0xde,
//     0x1c, 0x79, 0xbf, 0x63, 0xda, 0x5e, 0x8d, 0xbc, 0xae, 0x6b, 0xd7, 0xc7,
//     0x7c, 0x3b, 0xe5, 0x27, 0xf5, 0xff, 0xdf, 0x57, 0xf7, 0xc8, 0x1f, 0x91,
//     0xa7, 0xc0, 0x49, 0x0e, 0x5a, 0xc4, 0x98, 0x00, 0x00, 0x00, 0x00, 0x49,
//     0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
// };


KisDockFrameDocker::KisDockFrameDocker( QWidget* parent, const char* name ) 
	: KisBaseDocker( parent, name )
{
	QDockWindow::boxLayout() -> setSpacing(0);
	QDockWindow::boxLayout() -> setMargin(0);

	setWidget( m_page = new WdgDockerTab ( this ) );
// 	setWidget(m_tabWidget = new QTabWidget(this));
	m_page  -> setBaseSize( 175, 125 );
// 	m_tabWidget -> setBaseSize(175, 125);

 	if (m_page -> layout() != 0) {
 		m_page -> layout() -> setSpacing(0);
 		m_page -> layout() -> setMargin(0);
 	}

  	QObject::connect(m_page -> bnShade, SIGNAL(toggled(bool)), this, SLOT(shade(bool)));
	QObject::connect(this, SIGNAL(placeChanged(QDockWindow::Place)), this, SLOT(slotPlaceChanged(QDockWindow::Place)));

// 	QImage img;
// 	img.loadFromData( img0_wdgdockertab, sizeof( img0_wdgdockertab ), "PNG" );
// 	image0 = img;

// 	m_toggleShade = new QToolButton( this, "m_toggleShade" );
// 	m_toggleShade->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, m_toggleShade->sizePolicy().hasHeightForWidth() ) );
// 	m_toggleShade -> setMinimumSize( QSize( 18, 18 ) );
// 	m_toggleShade -> setMaximumSize( QSize( 18, 18 ) );
// 	m_toggleShade -> setBackgroundMode( QToolButton::PaletteMid );
// 	m_toggleShade -> setToggleButton( TRUE );
// 	m_toggleShade -> setOn( FALSE );
// 	m_toggleShade -> setIconSet( QIconSet( image0 ) );

//  	QObject::connect(m_toggleShade, SIGNAL(toggled(bool)), this, SLOT(shade(bool)));

// 	m_tabWidget -> setCornerWidget(m_toggleShade);
}

KisDockFrameDocker::~KisDockFrameDocker()
{
        delete m_page;
// 	delete m_toggleShade;
// 	delete m_tabWidget;
}

void KisDockFrameDocker::plug (QWidget* w)
{
        m_page-> tabWidget -> addTab( w , w -> caption());
// 	m_tabWidget -> addTab( w , w -> caption());
}

void KisDockFrameDocker::unplug(QWidget *w)
{
        m_page -> tabWidget -> removePage(w);
// 	m_tabWidget -> removePage(w);
}

void KisDockFrameDocker::showPage(QWidget *w)
{
        m_page-> tabWidget -> showPage(w);
// 	m_tabWidget -> showPage(w);
}

void KisDockFrameDocker::setCaption(const QString & caption)
{
	KisBaseDocker::setCaption(caption);
 	m_page -> lblCaption -> setText(caption);
}

void KisDockFrameDocker::shade(bool toggle)
{
	if (!toggle) {
 		m_page -> tabWidget -> show();
// 		for (int i = 0; i < m_tabWidget -> count(); i++) {
// 			m_tabWidget -> page(i) -> hide();
// 		}
	}
	else {
 		m_page -> tabWidget -> hide();
		if (!m_docked) {
			resize(minimumSize());
		}
// 		for (int i = 0; i < m_tabWidget -> count(); i++) {
// 			m_tabWidget -> page(i) -> show();
// 		}

	}
}

void KisDockFrameDocker::slotPlaceChanged(QDockWindow::Place p)
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
		m_page -> tabWidget -> show();
		m_page -> lblCaption -> setText("");
		resize(sizeHint());
	}

}
#include "kis_dockframedocker.moc"
