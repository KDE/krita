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
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include "kis_generic_registry.h"
#include "kis_types.h"
#include "kis_tool_registry.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
#include "kis_canvas_subject.h"
#include "kis_id.h"
#include "kis_debug_areas.h"

KisToolRegistry *KisToolRegistry::m_singleton = 0;

KisToolRegistry::KisToolRegistry()
{
    // Load all modules: color models, paintops, filters
	KService::List offers = KServiceTypeTrader::self()->query(QString::fromLatin1("Krita/Tool"),
                                                         QString::fromLatin1("(Type == 'Service') and "
                                                                             "([X-Krita-Version] == 2)"));

	KService::List::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        int errCode = 0;
        KParts::Plugin* plugin =
             KParts::ComponentFactory::createInstanceFromService<KParts::Plugin> ( service, this, QStringList(), &errCode);
        if ( plugin )
            kDebug(DBG_AREA_PLUGINS) << "found plugin " << service->property("Name").toString() << "\n";
        else {
            kDebug(41006) << "found plugin " << service->property("Name").toString() << ", " << errCode << "\n";
            if( errCode == KParts::ComponentFactory::ErrNoLibrary)
            {
                kWarning(41006) << " Error loading plugin was : ErrNoLibrary " << KLibLoader::self()->lastErrorMessage() << endl;
            }
        }

    }

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

        KisTool * tool = f->createTool(ac);
        subject->attach(tool);
        tools.push_back(KisToolSP(tool));
    }

    subject->notifyObservers();

    return tools;
}

KisTool * KisToolRegistry::createTool(KActionCollection * ac, KisCanvasSubject * subject, KisID & id) const
{
    KisToolFactorySP f = get(id);
    KisTool * t = f->createTool(ac);
    subject->attach(t);
    return t;
}

#include "kis_tool_registry.moc"
