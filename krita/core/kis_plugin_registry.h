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
#ifndef KIS_PLUGIN_REGISTRY_H_
#define KIS_PLUGIN_REGISTRY_H_

#include <stdlib.h>

#include <qwidget.h>

#include <kdebug.h>
#include <ktrader.h>

#include "kis_types.h"

/**
 * The plugin registry keeps track of the various plugins Krita
 * supports. Plugins call the appropriate register function, and
 * from that moment Krita's internals can use that plugin.
 *
 * Plugins that don't register themselves cannot be used from within
 * Krita, even though they can use Krita's internals themselves.
 *
 * The plugin registry simply adds the various plugins to the 
 * relevant factory, since there are still internally defined 
 * items, too.
 */

class KisPluginRegistry : public QObject, public KShared
{

	typedef QObject super;
	Q_OBJECT

	
public:

	KisPluginRegistry();
	virtual ~KisPluginRegistry();
	
	void registerColorStrategy(const QString & name, KisStrategyColorSpaceSP colorspace);
	void registerTool(const QString & name, KisToolSP tool);

	static KisPluginRegistry * singleton();

private:
	QStringList availablePlugins();

	static KisPluginRegistry * m_singleton;

};


#endif // KIS_PLUGIN_REGISTRY_H_
