/*
 *  kis_factory.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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

#include <kinstance.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include "kis_factory.h"
#include "kis_aboutdata.h"
#include "kis_resourceserver.h"
#include "kis_doc.h"

extern "C" 
{
	void* init_libkritapart()
	{
		return new KisFactory;
	}
}

KAboutData* KisFactory::s_aboutData = 0;
KInstance* KisFactory::s_global = 0;
KisResourceServer* KisFactory::s_rserver = 0;

KisFactory::KisFactory( QObject* parent, const char* name )
	: KoFactory( parent, name )
{
	s_aboutData = newKritaAboutData();

	(void)global();
	s_rserver = new KisResourceServer;
}

KisFactory::~KisFactory()
{
	delete s_rserver;
	s_rserver = 0L;
	delete s_aboutData;
	s_aboutData = 0L;
	delete s_global;
	s_global = 0L;
}

/**
 * Create the document
 */
KParts::Part* KisFactory::createPartObject( QWidget *parentWidget,
					    const char *widgetName, QObject* parent,
					    const char* name, const char* classname, const QStringList & )
{
	bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

	KisDoc *doc = new KisDoc( parentWidget,
				  widgetName, parent, name, !bWantKoDocument );

	if ( !bWantKoDocument )
		doc->setReadWrite( false );

	return doc;
}

KInstance* KisFactory::global()
{
	if ( !s_global )
	{
		s_global = new KInstance(s_aboutData);

		s_global->dirs()->addResourceType("kis",
						  KStandardDirs::kde_default("data") + "krita/");

		s_global->dirs()->addResourceType("kis_images",
						  KStandardDirs::kde_default("data") + "krita/images/");

		s_global->dirs()->addResourceType("kis_brushes",
						  KStandardDirs::kde_default("data") + "krita/brushes/");

		s_global->dirs()->addResourceType("kis_patterns",
						  KStandardDirs::kde_default("data") + "krita/patterns/");

		s_global->dirs()->addResourceType("kis_gradients",
						  KStandardDirs::kde_default("data") + "krita/gradients/");

		s_global->dirs()->addResourceType("kis_pics",
						  KStandardDirs::kde_default("data") + "krita/pics/");

		s_global->dirs()->addResourceType("toolbars",
						  KStandardDirs::kde_default("data") + "koffice/toolbar/");

		// Tell the iconloader about share/apps/koffice/icons
		s_global->iconLoader()->addAppDir("koffice");
	}

	return s_global;
}

KAboutData* KisFactory::aboutData()
{
	return s_aboutData;
}

KisResourceServer* KisFactory::rServer()
{
	return s_rserver;
}

#include "kis_factory.moc"
