/*
 * kritamagick.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <stdlib.h>
#include <vector>

#include <magick/api.h>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

#include <kis_global.h>
#include <kis_types.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>
#include <kis_factory.h>
#include <kis_view.h>
#include <kis_factory.h>

#include "kritamagick.h"

namespace {
	void InitGlobalMagick()
	{
		static bool init = false;

		if (!init) {
			KApplication *app = KApplication::kApplication();

			InitializeMagick(*app -> argv());
			atexit(DestroyMagick);
			init = true;
		}
	}
}

typedef KGenericFactory<KritaMagick> KritaMagickFactory;
K_EXPORT_COMPONENT_FACTORY( kritatoolfilter, KritaMagickFactory( "krita" ) )

KritaMagick::KritaMagick(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(KritaMagickFactory::instance());


 	kdDebug() << "ImageMagick integration plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

 	if ( parent->inherits("KisView") )
 	{
		m_view = dynamic_cast<KisView*>( parent );
	}
	else if ( parent -> inherits( "KisFactory" ) ) {
		InitGlobalMagick();
 	}
}

KritaMagick::~KritaMagick()
{
}

#include "kritamagick.moc"
