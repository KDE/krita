/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef KIS_VIEW_PLUGIN_H
#define KIS_VIEW_PLUGIN_H

#include <kparts/plugin.h>
#include <krita_export.h>

class KisAction;
class KisView2;

/**
 *  KisViewPlugin is the base for plugins which add actions to the view
 */
class KRITAUI_EXPORT KisViewPlugin : public KParts::Plugin
{
public:
    KisViewPlugin(QObject* parent = 0, const QString& rcFile = QString());

protected:
    /**
    *  adds an action to UI and action manager
    *  @param name name of the action in the rc file
    *  @param action the action that should be added
    */
    void addAction(const QString& name, KisAction* action);

    KisView2* m_view;
};

#endif // KIS_VIEW_PLUGIN_H
