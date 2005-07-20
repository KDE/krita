/*
 *  Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef DEFAULT_PAINTOPS_PLUGIN_H_
#define DEFAULT_PAINTOPS_PLUGIN_H_

#include <kparts/plugin.h>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class DefaultPaintOpsPlugin : public KParts::Plugin
{
	Q_OBJECT
public:
	DefaultPaintOpsPlugin(QObject *parent, const char *name, const QStringList &);
	virtual ~DefaultPaintOpsPlugin();
};

#endif // DEFAULT_PAINTOPSGRAY_PLUGIN_H_
