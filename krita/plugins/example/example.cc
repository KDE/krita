/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

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
#include <stdlib.h>
#include <vector>

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
#include <kis_iterators_quantum.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kistile.h>
#include <kistilemgr.h>

#include "kis_filter_configuration_widget.h"

// #include <kmessagebox.h>

#include "example.h"

typedef KGenericFactory<KritaExample> KritaExampleFactory;
K_EXPORT_COMPONENT_FACTORY( kritaexample, KritaExampleFactory( "krita" ) )

KritaExample::KritaExample(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(KritaExampleFactory::instance());


	//kdDebug() << "Example plugin. Class: " 
	//	  << className() 
	//	  << ", Parent: " 
	//	  << parent -> className()
	//	  << "\n";

	if ( !parent->inherits("KisView") )
	{
		return;
	} else {
		m_view = (KisView*) parent;
	}

	KisFilterSP kfi = createFilter<KisFilterInvert>(m_view);
	(void) new KAction(i18n("&Invert..."), 0, 0, kfi, SLOT(slotActivated()), actionCollection(), "krita_example");
}

KritaExample::~KritaExample()
{
}

KisFilterInvert::KisFilterInvert(KisView * view) : KisFilter(name(), view)
{
}

void KisFilterInvert::process(KisPaintDeviceSP device, KisFilterConfiguration* /*config*/, const QRect& rect, KisTileCommand* ktc)
{
	KisIteratorLineQuantum lineIt = device->iteratorQuantumSelectionBegin(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() );
	KisIteratorLineQuantum lastLine = device->iteratorQuantumSelectionEnd(ktc, rect.x(), rect.x() + rect.width() - 1, rect.y() + rect.height() - 1);
	Q_INT32 depth = device->depth() - 1;
	while( lineIt <= lastLine )
	{
		KisIteratorQuantum quantumIt = *lineIt;
		KisIteratorQuantum lastQuantum = lineIt.end();
		while( quantumIt <= lastQuantum )
		{
			for( int i = 0; i < depth; i++)
			{
				quantumIt = QUANTUM_MAX - quantumIt;
				++quantumIt;
			}
			++quantumIt;
		}
		++lineIt;
	}
}
