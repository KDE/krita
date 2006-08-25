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

#ifndef SCRIPTINGMONITOR_H
#define SCRIPTINGMONITOR_H

#include <QObject>
#include <krita_export.h>

namespace Kross {
    class GUIClient;
    class Action;
}

/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class KRITASCRIPTING_EXPORT ScriptingMonitor : public QObject {
    Q_OBJECT
    private:
        ScriptingMonitor();
        ~ScriptingMonitor();
    public:
        static ScriptingMonitor* instance();
        void monitor(Kross::GUIClient* guiClient);
    signals:
        void executionFinished(const Kross::Action* );
        void executionStarted(const Kross::Action* );
    private:
        static ScriptingMonitor* s_instance;

};

#endif
