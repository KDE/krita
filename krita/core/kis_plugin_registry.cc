/*
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>

#include <kdebug.h>
#include <kservice.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kparts/plugin.h>

#include "kis_plugin_registry.h"
 
#include "kis_colorspace_registry.h"
#include "kis_strategy_colorspace.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_tool.h"

// XXX: is this a candidate for KStaticDeleter?
namespace {
	KisPluginRegistry moveMe; // XXX Where to create singletons in Krita?
}

KisPluginRegistry *KisPluginRegistry::m_singleton = 0;



KisPluginRegistry::KisPluginRegistry()
	: super()
{
// 	kdDebug() << "Creating plugin registry\n";

	KisPluginRegistry::m_singleton = this;


	KTrader::OfferList offers = KTrader::self() -> query(QString::fromLatin1("Krita/CoreModule"), 
							     QString::fromLatin1("Type == 'Service'"));
    
	KTrader::OfferList::ConstIterator iter;
	
	for(iter = offers.begin(); iter != offers.end(); ++iter) 
	{
		KService::Ptr service = *iter;
		int errCode = 0;
		KParts::Plugin* plugin =
			KParts::ComponentFactory::createInstanceFromService<KParts::Plugin>
			( service, this, 0, QStringList(), &errCode);
		// here we ought to check the error code.

		if (plugin) {
			// guiFactory()->addClient(plugin);

// 			kdDebug() << "KisPluginRegistry: Loaded plugin "
// 				  << (*iter) -> name() << endl;
		}
	}

}



KisPluginRegistry *KisPluginRegistry::instance()
{
	return KisPluginRegistry::m_singleton;
}

KisPluginRegistry::~KisPluginRegistry()
{
}

void KisPluginRegistry::registerColorStrategy(const QString & /*name*/, KisStrategyColorSpaceSP colorspace)
{
// 	kdDebug() << "Adding color strategy: " << name << "\n";
	KisColorSpaceRegistry::instance() -> add(colorspace);
}

#include "kis_plugin_registry.moc"
