/* 
 * gray_plugin.cc -- Part of Krita
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
#include <kis_iterators.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_tile_command.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_plugin_registry.h>
#include <kistile.h>
#include <kistilemgr.h>

#include "gray_plugin.h"

#include "kis_strategy_colorspace_grayscale.h"

typedef KGenericFactory<GrayPlugin> GrayPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritagrayplugin, GrayPluginFactory( "kritacore" ) )


GrayPlugin::GrayPlugin(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(GrayPluginFactory::instance());

// 	kdDebug() << "GRAY Color model plugin. Class: " 
// 		  << className() 
// 		  << ", Parent: " 
// 		  << parent -> className()
// 		  << "\n";

	// This is not a gui plugin; only load it when the doc is created.
	if ( parent->inherits("KisPluginRegistry") )
	{
		m_StrategyColorSpaceGray = new KisStrategyColorSpaceGrayscale();
		KisPluginRegistry::singleton() -> registerColorStrategy("Grayscale", m_StrategyColorSpaceGray);
		KisPluginRegistry::singleton() -> registerColorStrategy("Grayscale/Alpha", m_StrategyColorSpaceGray);
	}
	
}

GrayPlugin::~GrayPlugin()
{
}

#include "gray_plugin.moc"
