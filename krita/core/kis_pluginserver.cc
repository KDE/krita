/*
 *  kis_pluginserver.cc - part of Krayon
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

// don't remove, --enable-final breaks otherwise because of some
// stupid X headers (Werner)
#ifdef Unsorted
#undef Unsorted
#endif
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qpopupmenu.h>

#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>

#include "kis_factory.h"
#include "kis_pluginserver.h"

KisPluginServer::KisPluginServer()
{
	m_count = 0;
	m_plugins.setAutoDelete(true);

	/*
	 *   Find plugin dirs. For example ~/.kde/share/apps/krayon/plugins or
	 *   $KDEDIR/share/apps/krayon/plugins
	 */

	QStringList pluginDirs
		= KisFactory::global()->dirs()->resourceDirs("kis_plugins");

	if (!pluginDirs.isEmpty())
	{
		QStringList::Iterator it;

		for ( it = pluginDirs.begin(); it != pluginDirs.end(); ++it )
		{
			kdDebug()<<"Searching plugins in: "<<(*it).latin1()<<endl;
			findPlugins(*it);
		}
	}
	else
	{
		kdDebug()<<"Warning: No plugin directories found.\n";
	}
}

KisPluginServer::~KisPluginServer()
{
	m_plugins.clear();
}


void KisPluginServer::findPlugins( const QString &directory )
{
	QString pname, pcomment, pdir, plib, ptype, pcategory;
	PluginType type = PLUGIN_FILTER;

	QDir dir(directory, "*.kisplugin");
	if (!dir.exists()) return;

	const QFileInfoList *list = dir.entryInfoList();
	QFileInfoListIterator it(*list);
	QFileInfo *fi;

	while ((fi = it.current()))
	{
		KSimpleConfig config(fi->absFilePath(), true);

		config.setGroup("General");
		pname = config.readEntry("Name", fi->baseName());
		pcomment = config.readEntry("Comment",
					    i18n("No description available."));
		pdir = directory + config.readEntry("Subdir", fi->baseName());
		plib = config.readEntry("Library",
					QString("libkray_") + fi->baseName());
		pcategory = config.readEntry("Category", "General");
		ptype = config.readEntry("Type", "Filter");
		kdDebug()<<"Plugin category is "<< pcategory.latin1()<<endl;

		if ( ptype == "Filter" )
			type = PLUGIN_FILTER;
		else if (ptype == "Tool" )
			type = PLUGIN_TOOL;
		else
			kdDebug()<<"Warning: "<<ptype.latin1()<<" is not a valid Krayon plugin type.\n";

		PluginInfo *pi = new PluginInfo(pname, pcomment,
						pdir, plib, pcategory, type);
		m_plugins.append(pi);

		++it;
	}
}


void KisPluginServer::buildFilterMenu( QPopupMenu *menu )
{
	if (!menu) return;
	menu->clear();

	QStringList categories;

	// build a list of filter categories.
	for (PluginInfo *pi = m_plugins.first(); pi != 0; pi = m_plugins.next())
	{
		if (!pi->type() == PLUGIN_FILTER)
			continue;

		if (!categories.contains(pi->category()))
			categories.append(pi->category());
	}

	// sort it alphabetically
	categories.sort();

	// loop through the categories and build submenus
	for (QStringList::Iterator it = categories.begin(); it != categories.end(); it++)
	{
		if ((*it) == "General")
			continue;

		kdDebug()<<"hallo\n";
		QPopupMenu *submenu = new QPopupMenu( menu );

		for (PluginInfo *pi = m_plugins.first(); pi != 0;
		     pi = m_plugins.next())
		{
			if (!pi->type() == PLUGIN_FILTER)
				continue;

			if (pi->category() == (*it))
			{
				kdDebug()<<"hallo2\n";
				int id = ++m_count;
				id = submenu->insertItem(pi->name(), id);
				pi->setId(id);
			}
		}

		kdDebug()<<"hallo3\n";
		menu->insertItem((*it), submenu);
	}

	// build general (== toplevel) category
	for (PluginInfo *pi = m_plugins.first(); pi != 0; pi = m_plugins.next())
	{
		if (!pi->type() == PLUGIN_FILTER)
			continue;

		if (pi->category() == "General")
		{
			int id = ++m_count;
			menu->insertItem(pi->name(), id);
			pi->setId(id);
		}
	}
}

void KisPluginServer::activatePlugin( int )
{
}
