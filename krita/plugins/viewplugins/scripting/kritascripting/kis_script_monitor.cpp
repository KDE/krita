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

#include "kis_script_monitor.h"

#include <main/scriptaction.h>
#include <main/scriptguiclient.h>


KisScriptMonitor::KisScriptMonitor()
{
}


KisScriptMonitor::~KisScriptMonitor()
{
    s_instance = 0;
}

void KisScriptMonitor::monitor(Kross::Api::ScriptGUIClient* guiClient)
{
    connect(guiClient, SIGNAL(executionFinished( const Kross::Api::ScriptAction* )), SIGNAL(executionFinished( const Kross::Api::ScriptAction* )));
    connect(guiClient, SIGNAL(executionStarted( const Kross::Api::ScriptAction* )), SIGNAL(executionStarted( const Kross::Api::ScriptAction* )));
}

KisScriptMonitor* KisScriptMonitor::s_instance = 0;

KisScriptMonitor* KisScriptMonitor::instance()
{
    if(s_instance == 0)
        s_instance = new KisScriptMonitor();
    return s_instance;
}

#include "kis_script_monitor.moc"

