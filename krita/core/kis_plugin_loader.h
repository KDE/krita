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
#ifndef KIS_PLUGIN_LOADER_H_
#define KIS_PLUGIN_LOADER_H_

#include <stdlib.h>

#include <qobject.h>
#include <qdict.h>
#include <qstring.h>

#include <ksharedptr.h>
#include <kdebug.h>
#include <kaction.h>
#include <ktrader.h>

#include "kis_types.h"

class KisView;

struct KisPluginInfo {
	KService::Ptr service;
	QString name;
	QString comment;
	QString type;
};

typedef QDict<KisPluginInfo> KisPluginList;


/**
 * The pluginloader loads all plugins of a certain type, but
 * carefully refrains from instantiating them until asked.
 *
 * There are two ways of instantiating a plugin: into the GUI of
 * the current view, or as a non-gui object for use by other objects.
 *
 * Generally speaking, a gui plugin will want a corresponding general
 * object as an argument.
 */
class KisPluginLoader : public QObject
{

	Q_OBJECT

public:

	KisPluginLoader(QObject * parent, const QString & pluginType);
	virtual ~KisPluginLoader();


public:

	void loadPlugins();

	QObject * createPlugin(const QString & name, KisView * view);

	QStringList * pluginList();


private:
	QString m_pluginType;

	KisPluginList * m_plugins;
};


#endif // KIS_PLUGIN_LOADER_H_
