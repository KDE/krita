/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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
#include "assistant_tool.h"
#include "kis_assistant_tool.h"

#include <kpluginfactory.h>
#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>
#include <kis_algebra_2d.h>
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

K_PLUGIN_FACTORY_WITH_JSON(AssistantToolFactory, "kritaassistanttool.json", registerPlugin<AssistantToolPlugin>();)


AssistantToolPlugin::AssistantToolPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KoToolRegistry::instance()->add(new KisAssistantToolFactory());

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

AssistantToolPlugin::~AssistantToolPlugin()
{
}

#include "assistant_tool.moc"
