/*
 *  kis_basedocker.cc - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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
#include <kglobalsettings.h>


#include <qlayout.h>

#include "kis_basedocker.h"

KisBaseDocker::KisBaseDocker( QWidget* parent, const char* name) 
	: QDockWindow( QDockWindow::OutsideDock, parent ,name )
{
	setCloseMode( QDockWindow::Always );
	setResizeEnabled(true);
	setOpaqueMoving(false);

// 	setVerticallyStretchable(false);
// 	setHorizontallyStretchable(true);
	setNewLine(true);
	layout() -> setSpacing(0);
	layout() -> setMargin(0);

	setFont(KGlobalSettings::toolBarFont());
}

#include "kis_basedocker.moc"

