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

#include <qobject.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qstringlist.h>
#include <qguardedptr.h>
#include <qmap.h>

#include <kparts/componentfactory.h>

#include <ksharedptr.h>
#include <kdebug.h>
#include <kaction.h>
#include <ktrader.h>
#include <kservice.h>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_plugin_loader.h"



KisPluginLoader::KisPluginLoader(QObject * parent, const QString & pluginType)
	: QObject( parent ),
	  m_pluginType( pluginType )
{
	m_plugins = new KisPluginList();
}

KisPluginLoader::~KisPluginLoader()
{
}


void KisPluginLoader::loadPlugins()
{
	QValueList<KService::Ptr> traderList= KTrader::self()->query(m_pluginType);

	KTrader::OfferList::Iterator it(traderList.begin());
	for( ; it != traderList.end(); ++it) {
		KService::Ptr ptr = (*it);

		QString pVersion = ptr -> property("X-Krita-Version").toString();

		if ((pVersion >= "0") && (pVersion <= KRITA_VERSION))
		{
			KisPluginInfo * info = new KisPluginInfo;

			info -> service = ptr;
			// Get name, description from the plugin data
			// and put in info object.
			info -> name = ptr -> property( "Name" ).toString();
			QString name = ptr -> property("Name").toString();
			info -> comment = ptr -> property( "Comment" ).toString();
			info -> type = ptr -> property("ServiceTypes").toString();

			m_plugins -> insert(name.ascii(), info);
		}
	}
}

QObject * KisPluginLoader::createPlugin(const QString & /*name*/,
					KisView * /*view*/)
{
// 	KisPluginInfo * info = m_plugins -> find(name);

// 	return KParts::ComponentFactory::createInstanceFromLibrary<Plugin>( QFile::encodeName(info->service->library()),
// 									    project, 0, QStringList());
	return 0;
}

