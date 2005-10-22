/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kdebug.h"
#include <kaction.h>

#include "kis_generic_registry.h"
#include "kis_types.h"
#include "kis_tool_registry.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
#include "kis_canvas_subject.h"
#include "kis_id.h"

KisToolRegistry *KisToolRegistry::m_singleton = 0;

KisToolRegistry::KisToolRegistry()
{
}

KisToolRegistry::~KisToolRegistry()
{
}

KisToolRegistry* KisToolRegistry::instance()
{
     if(KisToolRegistry::m_singleton == 0)
     {
         KisToolRegistry::m_singleton = new KisToolRegistry();
     }
    return KisToolRegistry::m_singleton;
}



vKisTool KisToolRegistry::createTools(KActionCollection * ac, KisCanvasSubject *subject) const
{
    Q_ASSERT(subject);

    vKisTool tools;

    KisIDList factories = listKeys();

    for (KisIDList::Iterator it = factories.begin(); it != factories.end(); ++it )
    {
        KisToolFactorySP f = get(*it);

        KisTool * tool = f -> createTool(ac);
        subject -> attach(tool);
        tools.push_back(tool);
    }

    subject -> notifyObservers();

    return tools;
}

KisTool * KisToolRegistry::createTool(KActionCollection * ac, KisCanvasSubject * subject, KisID & id) const
{
    KisToolFactorySP f = get(id);
    KisTool * t = f -> createTool(ac);
    subject->attach(t);
    return t;
}
