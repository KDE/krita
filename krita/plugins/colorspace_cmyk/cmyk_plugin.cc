/* 
 * cmyk_plugin.cc -- Part of Krita
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

#include "cmyk_plugin.h"

#include "kis_strategy_colorspace_cmyk.h"

typedef KGenericFactory<CMYKPlugin> CMYKPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritacmykplugin, CMYKPluginFactory( "krita" ) )


CMYKPlugin::CMYKPlugin(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(CMYKPluginFactory::instance());

	kdDebug() << "CMYK Color model plugin. Class: " 
		  << className() 
		  << ", Parent: " 
		  << parent -> className()
		  << "\n";

	m_StrategyColorSpaceCMYK = new KisStrategyColorSpaceCMYK();
// 	KisPluginRegistry::singleton() -> registerColorStrategy("CMYK", IMAGE_TYPE_CMYK, m_StrategyColorSpaceCMYK);
// 	KisPluginRegistry::singleton() -> registerColorStrategy("CMYK + Alpha", IMAGE_TYPE_CMYKA, m_StrategyColorSpaceCMYK);
	
}

CMYKPlugin::~CMYKPlugin()
{
}

#include "cmyk_plugin.moc"
