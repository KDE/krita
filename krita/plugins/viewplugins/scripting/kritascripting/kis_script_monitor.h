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

#ifndef KIS_SCRIPT_MONITOR_H
#define KIS_SCRIPT_MONITOR_H

#include <qobject.h>
#include <krita_export.h>
namespace Kross {
    namespace Api {
        class ScriptGUIClient;
        class ScriptAction;
    }
}

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class KRITASCRIPTING_EXPORT KisScriptMonitor : public QObject {
    Q_OBJECT
    private:
        KisScriptMonitor();
        ~KisScriptMonitor();
    public:
        static KisScriptMonitor* instance();
        void monitor(Kross::Api::ScriptGUIClient* guiClient);
    signals:
        void executionFinished(const Kross::Api::ScriptAction* );
        void executionStarted(const Kross::Api::ScriptAction* );
    private:
        static KisScriptMonitor* s_instance;

};

#endif
