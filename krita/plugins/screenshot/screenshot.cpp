/* This file is part of the KDE project
   Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <koView.h>
#include <kgenericfactory.h>

#include "ksnapshot.h"
#include <kimageio.h>

#include "screenshot.moc"

typedef KGenericFactory<Screenshot> ScreenshotFactory;
K_EXPORT_COMPONENT_FACTORY( kritascreenshot, ScreenshotFactory( "kscreenshot_plugin" ) )

	Screenshot::Screenshot(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(ScreenshotFactory::instance());

	KImageIO::registerFormats(); // ???

	snapshot = new KSnapshot();
	connect( snapshot, SIGNAL( screenGrabbed() ), SLOT( slotScreenGrabbed() ) );

	(void) new KAction(i18n("&Screenshot..."), SmallIcon("digikam"), 0, this, SLOT(slotScreenshot()), actionCollection(), "screenshot");
}

Screenshot::~Screenshot()
{
	delete snapshot;
}

void Screenshot::slotScreenshot()
{
	snapshot -> show();
}     

void Screenshot::slotScreenGrabbed()
{
	KTempFile temp(locateLocal("tmp", "screenshot"), ".png");
	snapshot -> save(temp.name());
	     
	KoView *view = dynamic_cast<KoView *>(parent());
	if(view)
		emit view->embeddImage(temp.name());
}
