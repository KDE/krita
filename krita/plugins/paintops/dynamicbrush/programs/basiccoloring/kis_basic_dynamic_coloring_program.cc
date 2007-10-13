/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "kis_basic_dynamic_coloring_program.h"

// Dynamic Brush lib includes
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_coloring_program_factory_registry.h"
#include "kis_dynamic_sensor.h"

// basic program includes
#include "kis_basic_dynamic_coloring_program_editor.h"

class Factory {
    public:
        Factory()
        {
            KisDynamicColoringProgramFactoryRegistry::instance()->add( new KisBasicDynamicColoringProgramFactory );
        }
};

static Factory factory;

KisBasicDynamicColoringProgram::KisBasicDynamicColoringProgram(const QString& name) : KisDynamicColoringProgram(name, "basiccoloring")
{
}

KisBasicDynamicColoringProgram::~KisBasicDynamicColoringProgram()
{
}

void KisBasicDynamicColoringProgram::apply(KisDynamicColoring* coloring, const KisPaintInformation& adjustedInfo) const
{
}

QWidget* KisBasicDynamicColoringProgram::createEditor(QWidget* parent)
{
    return new KisBasicDynamicColoringProgramEditor(this);
}

void KisBasicDynamicColoringProgram::fromXML(const QDomElement& elt)
{
    KisDynamicColoringProgram::fromXML(elt);
}

void KisBasicDynamicColoringProgram::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisDynamicColoringProgram::toXML(doc, elt);
}

//--- KisBasicDynamicColoringProgramFactory ---//

KisBasicDynamicColoringProgramFactory::KisBasicDynamicColoringProgramFactory() :
    KisDynamicColoringProgramFactory("basiccoloring", i18n("Basic"))
{
}

KisDynamicColoringProgram* KisBasicDynamicColoringProgramFactory::coloringProgram(QString name) const
{
    return new KisBasicDynamicColoringProgram(name);
}

#include "kis_basic_dynamic_coloring_program.moc"
