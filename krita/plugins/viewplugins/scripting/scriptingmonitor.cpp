/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "scriptingmonitor.h"

#include <kross/core/manager.h>
#include <kross/core/action.h>
#include <kross/core/guiclient.h>


ScriptingMonitor::ScriptingMonitor()
{
}


ScriptingMonitor::~ScriptingMonitor()
{
    s_instance = 0;
}

void ScriptingMonitor::monitor(Kross::GUIClient* guiClient)
{
    Q_UNUSED(guiClient);
    connect(&Kross::Manager::self(), SIGNAL(started(Kross::Action*)), SIGNAL(started(Kross::Action*)));
    connect(&Kross::Manager::self(), SIGNAL(finished(Kross::Action*)), SIGNAL(finished(Kross::Action*)));
}

ScriptingMonitor* ScriptingMonitor::s_instance = 0;

ScriptingMonitor* ScriptingMonitor::instance()
{
    if(s_instance == 0)
        s_instance = new ScriptingMonitor();
    return s_instance;
}

#include "scriptingmonitor.moc"

