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
#include <qfontmetrics.h>
#include <qfont.h>

#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktabwidget.h>

#include "kis_dockframedocker.h"
#include "wdgdockertab.h"


KisDockFrameDocker::KisDockFrameDocker( QWidget* parent, const char* name )
	: KisBaseDocker( parent, name )
{
	QDockWindow::boxLayout() -> setSpacing(0);
	QDockWindow::boxLayout() -> setMargin(0);

	setWidget( m_page = new WdgDockerTab ( this ) );

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

}

KisDockFrameDocker::~KisDockFrameDocker()
{
        delete m_page;
}

void KisDockFrameDocker::plug (QWidget* w)
{
	w -> setFont(m_font);
        m_page -> tabWidget -> addTab( w , w -> caption());
	if (w -> layout() != 0) {
 		w -> layout() -> setSpacing(0);
 		w -> layout() -> setMargin(0);
	}
}

void KisDockFrameDocker::unplug(QWidget *w)
{
        m_page -> tabWidget -> removePage(w);
}

void KisDockFrameDocker::showPage(QWidget *w)
{
        m_page-> tabWidget -> showPage(w);
}

void KisDockFrameDocker::setCaption(const QString & caption)
{
	KisBaseDocker::setCaption(caption);
 	m_page -> lblCaption -> setText(caption);
}

void KisDockFrameDocker::shade(bool toggle)
{
	m_shaded = toggle;
	if (!toggle) {
 		m_page -> tabWidget -> show();
	}
	else {
 		m_page -> tabWidget -> hide();
		if (!m_docked) {
			resize(minimumSize());
		}

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
