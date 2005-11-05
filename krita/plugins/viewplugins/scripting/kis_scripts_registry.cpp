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

#include "kis_scripts_registry.h"

#include <kfiledialog.h>

#include <klocale.h>
#include <kis_view.h>
#include "kis_script.h"

KisScriptsRegistry *KisScriptsRegistry::m_singleton = 0;

KisScriptsRegistry::KisScriptsRegistry()
 : KisGenericRegistry<KisScriptSP>()
{
}


KisScriptsRegistry::~KisScriptsRegistry()
{
}

KisScriptsRegistry* KisScriptsRegistry::instance()
{
    if(KisScriptsRegistry::m_singleton == 0)
    {
        KisScriptsRegistry::m_singleton = new KisScriptsRegistry();
    }
    return KisScriptsRegistry::m_singleton;
}

#include "kis_scripts_registry.moc"
