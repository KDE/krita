/*
 * variation.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>
#include <kis_iterators.h>
#include <kis_selection.h>

#include "screenmode.h"

typedef KGenericFactory<ScreenMode> ScreenModeFactory;
K_EXPORT_COMPONENT_FACTORY( screenmode, ScreenModeFactory( "krita" ) )

ScreenMode::ScreenMode(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(ScreenModeFactory::instance());
	kdDebug() << "ScreenMode\n";

	KToggleAction * a1;
	KToggleAction * a2;
	KToggleAction * a3;

	a1 = new KToggleAction(i18n("&Full Screen"), "window_fullscreen", 0, this, SLOT(slotFullScreen()), actionCollection(), "fullscreen");
	a1 -> setExclusiveGroup("screenmode");

	a2 = new KToggleAction(i18n("&Full Screen with Menubar"), "view_remove", 0, this, SLOT(slotFullScreenMenu()), actionCollection(), "fullscreenmenubar");
	a2 -> setExclusiveGroup("screenmode");
	
	a3 = new KToggleAction(i18n("&Normal"), "window_nofullscreen", 0, this, SLOT(slotNormal()), actionCollection(), "normal");
	a3 -> setExclusiveGroup("screenmode");

	if ( !parent->inherits("KisView") )
	{
		m_view = 0;
	} else {
		m_view = (KisView*) parent;
	}
}

ScreenMode::~ScreenMode()
{
}

void ScreenMode::slotFullScreen()
{
	m_view -> setWindowState( m_view -> windowState() ^ WindowFullScreen);
}

void ScreenMode::slotFullScreenMenu()
{
}

void ScreenMode::slotNormal()
{
}


#include "screenmode.moc"

