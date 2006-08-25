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

#include <core/action.h>
#include <core/guiclient.h>


ScriptingMonitor::ScriptingMonitor()
{
}


ScriptingMonitor::~ScriptingMonitor()
{
    s_instance = 0;
}

void ScriptingMonitor::monitor(Kross::GUIClient* guiClient)
{
    connect(guiClient, SIGNAL(executionFinished( const Kross::Action* )), SIGNAL(executionFinished( const Kross::Action* )));
    connect(guiClient, SIGNAL(executionStarted( const Kross::Action* )), SIGNAL(executionStarted( const Kross::Action* )));
}

ScriptingMonitor* ScriptingMonitor::s_instance = 0;

ScriptingMonitor* ScriptingMonitor::instance()
{
    if(s_instance == 0)
        s_instance = new ScriptingMonitor();
    return s_instance;
}

#include "scriptingmonitor.moc"

