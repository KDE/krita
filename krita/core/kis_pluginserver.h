/*
 *  kis_pluginserver.h - part of KImageShop
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

#ifndef __kis_pluginserver_h__
#define __kis_pluginserver_h__

#include <qptrlist.h>
#include <qstring.h>

class QPopupMenu;

enum PluginType { PLUGIN_FILTER, PLUGIN_TOOL };

class PluginInfo
{
 public:
	PluginInfo(const QString& name, const QString& comment, 
		   const QString& dir, const QString& lib, const QString& category, 
		   PluginType type)
		{ m_name = name; m_comment = comment; m_dir = dir; 
		m_library = lib; m_category = category; m_type = type; }

	QString name()     { return m_name; }
	QString comment()  { return m_comment; }
	QString dir()      { return m_dir; }
	QString library()  { return m_library; }
	PluginType type () { return m_type; }
	QString category() { return m_category; }
	int id()           { return m_id; }
	void setId(int id) { m_id = id; }

 private:
	QString    m_name;
	QString    m_comment;
	QString    m_dir;
	QString    m_library;
	QString    m_category;
	int        m_id;
	PluginType m_type;
};

typedef QPtrList<PluginInfo> PluginInfoList ;

class KisPluginServer
{

 public:

	KisPluginServer();
	~KisPluginServer();

	/*
	 * Build plugin filter menu.
	 */
	void buildFilterMenu( QPopupMenu *menu );
	void activatePlugin( int id );

 protected:
	/*
	 * Find plugins in 'directory' and add them to the database.
	 */
	void findPlugins( const QString &directory );

 private:
	PluginInfoList m_plugins;
	int m_count;
};

#endif // __kis_pluginserver_h__
