/*
 * SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
