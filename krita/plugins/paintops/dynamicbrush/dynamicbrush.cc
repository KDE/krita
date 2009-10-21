/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dynamicbrush.h"


#include <kactioncollection.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kis_paintop_registry.h>
#include <kis_view2.h>

#include "kis_dynamicop_factory.h"
#include "kis_dynamic_shape_program.h"
#include "kis_dynamic_coloring_program.h"
#include "kis_dynamic_programs_editor.h"
#include "kis_dynamic_coloring_program_factory_registry.h"
#include "kis_dynamic_shape_program_factory_registry.h"

#include <kis_bookmarked_configuration_manager.h>

typedef KGenericFactory<DynamicBrush> DynamicBrushFactory;
K_EXPORT_COMPONENT_FACTORY(kritadynamicbrushpaintop, DynamicBrushFactory("krita"))

DynamicBrush::DynamicBrush(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
        , m_shapeBookmarksManager(new KisBookmarkedConfigurationManager("dynamicopshape", new KisDynamicShapeProgramsFactory()))
        , m_shapeBookmarksModel(new KisBookmarkedConfigurationsModel(m_shapeBookmarksManager))
        , m_coloringBookmarksManager(new KisBookmarkedConfigurationManager("dynamicopcoloring", new KisDynamicColoringProgramsFactory()))
        , m_coloringBookmarksModel(new KisBookmarkedConfigurationsModel(m_coloringBookmarksManager))

{
    setComponentData(DynamicBrushFactory::componentData());

    // This is not a gui plugin; only load it when the doc is created.
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    r->add(new KisDynamicOpFactory(m_shapeBookmarksModel, m_coloringBookmarksModel));
#if 0
    {
        // TODO: remove this, temp stuff for testing only
        {
            KisFiltersListDynamicProgram* programSpeed = new KisFiltersListDynamicProgram("speed");
            programSpeed->appendTransformation(new KisSizeTransformation(KisDynamicSensor::id2Sensor("speed"), KisDynamicSensor::id2Sensor("speed")));
            KisDynamicProgramRegistry::instance()->add(programSpeed);
        }
        {
            KisFiltersListDynamicProgram* programPressure = new KisFiltersListDynamicProgram("pressure");
            programPressure->appendTransformation(new KisSizeTransformation(KisDynamicSensor::id2Sensor("pressure"), KisDynamicSensor::id2Sensor("pressure")));
            KisDynamicProgramRegistry::instance()->add(programPressure);
        }
        {
            KisFiltersListDynamicProgram* programRotation = new KisFiltersListDynamicProgram("rotation");
            programRotation->appendTransformation(new KisRotationTransformation(KisDynamicSensor::id2Sensor("drawingangle")));
            KisDynamicProgramRegistry::instance()->add(programRotation);
        }
        {
            KisFiltersListDynamicProgram* programTime = new KisFiltersListDynamicProgram("time");
            programTime->appendTransformation(new KisRotationTransformation(KisDynamicSensor::id2Sensor("time")));
            KisDynamicProgramRegistry::instance()->add(programTime);
        }
    }
#endif
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/dynamicbrush.rc"), true);

        KAction *action  = new KAction(i18n("Edit dynamic shape programs"), this);
        actionCollection()->addAction("EditDynamicShapePrograms", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotEditDynamicShapePrograms()));
        action  = new KAction(i18n("Edit dynamic coloring programs"), this);
        actionCollection()->addAction("EditDynamicColoringPrograms", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotEditDynamicColoringPrograms()));
    }
}

DynamicBrush::~DynamicBrush()
{
}

void DynamicBrush::slotEditDynamicShapePrograms()
{
    KisDynamicProgramsEditor editor(m_view, m_shapeBookmarksModel, KisDynamicShapeProgramFactoryRegistry::instance());
    editor.exec();
}

void DynamicBrush::slotEditDynamicColoringPrograms()
{
    KisDynamicProgramsEditor editor(m_view, m_coloringBookmarksModel, KisDynamicColoringProgramFactoryRegistry::instance());
    editor.exec();
}

#include "dynamicbrush.moc"
