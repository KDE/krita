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

#include "kis_types.h"
#include "kis_plugin_registry.h"
 
#include "kis_colorspace_factory.h"
#include "kis_strategy_colorspace.h"
#include "kis_global.h"
#include "kis_types.h"

namespace {
	KisPluginRegistry moveMe; // XXX Where to create singletons in Krita?
}

KisPluginRegistry *KisPluginRegistry::m_singleton = 0;

KisPluginRegistry::KisPluginRegistry()
{
	kdDebug() << "Creating plugin registry\n";
	KisPluginRegistry::m_singleton = this;
}

KisPluginRegistry *KisPluginRegistry::singleton()
{
	return KisPluginRegistry::m_singleton;
}

KisPluginRegistry::~KisPluginRegistry()
{
}

void KisPluginRegistry::registerColorStrategy(const QString & name, enumImgType imgType, KisStrategyColorSpaceSP colorspace)
{
	kdDebug() << "Adding color strategy\n";
	KisColorSpaceFactory *factory = KisColorSpaceFactory::singleton();
        Q_ASSERT(factory);
	factory -> add(imgType, colorspace);
}

void KisPluginRegistry::registerTool(const QString & /*name*/, KisToolSP /*tool*/)
{
	// XXX: Currently, tools are created whenever a view is
	// created. That is to say, tools are not singletons, and the
	// list of tools is not a singleton either; and every input
	// device has its own list of tools.

	// This needs to be reviewed -- I don't know how to create new
	// instances of tools when a new view is created because C++
	// doesn't have the concept of a Class class.

	// This makes it hard to make a tool plugin...

	// Perhaps have the toolplugin register itself once in a
	// registry and provide a factory function. The
	// KisToolFactory::create method could then use the factory
	// function to add the plugin tools to the hard-coded tools.
}
