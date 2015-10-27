/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "ruler_assistant_tool.h"
#include "kis_ruler_assistant_tool.h"

#include <kpluginfactory.h>

#include <KoToolRegistry.h>
#include "RulerAssistant.h"
#include "EllipseAssistant.h"
#include "SplineAssistant.h"
#include "PerspectiveAssistant.h"
#include "VanishingPointAssistant.h"
#include "InfiniteRulerAssistant.h"
#include "ParallelRulerAssistant.h"
#include "ConcentricEllipseAssistant.h"
#include "FisheyePointAssistant.h"
//#include "mesh_assistant.h"

K_PLUGIN_FACTORY_WITH_JSON(RulerAssistantToolFactory, "kritarulerassistanttool.json", registerPlugin<RulerAssistantToolPlugin>();)


RulerAssistantToolPlugin::RulerAssistantToolPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisRulerAssistantToolFactory());

    KisPaintingAssistantFactoryRegistry::instance()->add(new RulerAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new EllipseAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new SplineAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new PerspectiveAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new VanishingPointAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new InfiniteRulerAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new ParallelRulerAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new ConcentricEllipseAssistantFactory);
    KisPaintingAssistantFactoryRegistry::instance()->add(new FisheyePointAssistantFactory);
//    KisPaintingAssistantFactoryRegistry::instance()->add(new MeshAssistantFactory);
}

RulerAssistantToolPlugin::~RulerAssistantToolPlugin()
{
}

#include "ruler_assistant_tool.moc"
