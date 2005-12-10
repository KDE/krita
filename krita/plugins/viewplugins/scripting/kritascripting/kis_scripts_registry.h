/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_SCRIPTS_REGISTRY_H
#define KIS_SCRIPTS_REGISTRY_H

#include <qobject.h>

#include <ksharedptr.h>

#include <kis_generic_registry.h>

class Scripting;
class KisScript;
typedef KSharedPtr<KisScript> KisScriptSP;
class KisView;


/**
@author Cyrille Berger
*/
class KisScriptsRegistry :  public QObject, public KisGenericRegistry<KisScriptSP>
{
    Q_OBJECT
    private:
        KisScriptsRegistry();
        ~KisScriptsRegistry();
    public:
        /**
         * Return the unique instance of the registry. If the registry doesn't exist yet,
         * the function will create it.
         */
        static KisScriptsRegistry* instance();
        inline void setRunningScript(KisScriptSP s) { m_runningScript = s; };
        inline KisScriptSP getRunningScript() { return m_runningScript; };
    private:
        static KisScriptsRegistry *m_singleton;
        KisView* m_view;
        KisScriptSP m_runningScript;
};

#endif
