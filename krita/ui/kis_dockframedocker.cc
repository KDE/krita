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

#include <kdebug.h>
#include <klocale.h>

#include <koColorChooser.h>
#include <koFrameButton.h>

#include "kis_dockframedocker.h"


DockFrameDocker::DockFrameDocker( QWidget* parent, const char* name ) : BaseDocker( parent, name )
{
	setWidget( m_tabwidget = new QTabWidget( this ) );
	m_tabwidget -> setBaseSize( 200, 175 );
       
}

DockFrameDocker::~DockFrameDocker()
{
        delete m_tabwidget;
}

void DockFrameDocker::plug (QWidget* w)
{
        m_tabwidget-> addTab( w , w -> caption());
}

void DockFrameDocker::unplug(QWidget *w)
{
        m_tabwidget -> removePage(w);
}

void DockFrameDocker::showPage(QWidget *w)
{
        m_tabwidget-> showPage(w);
}

#include "kis_dockframedocker.moc"

