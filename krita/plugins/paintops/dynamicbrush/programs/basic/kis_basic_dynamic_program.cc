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

#include "kis_basic_dynamic_program.h"

#include <QDomNode>
#include <QWidget>

#include <klocale.h>

// Dynamic Brush lib includes
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_program_factory_registry.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_transformation.h"
#include "kis_dynamic_transformations_factory.h"

// basic program includes
#include "kis_basic_dynamic_program_editor.h"

class Factory {
    public:
        Factory()
        {
            KisDynamicProgramFactoryRegistry::instance()->add( new KisBasicDynamicProgramFactory );
        }
};

static Factory factory;

KisBasicDynamicProgram::~KisBasicDynamicProgram()
{
}

void KisBasicDynamicProgram::apply(KisDynamicShape* shape, KisDynamicColoring* coloringsrc, const KisPaintInformation& adjustedInfo)
{

}

void KisBasicDynamicProgram::appendTransformation(KisDynamicTransformation* transfo) {
    kDebug(41006) << "Append transfo : " << transfo->name();
    m_transformations.append(transfo);
    emit(programChanged());
}

QWidget* KisBasicDynamicProgram::createEditor(QWidget* /*parent*/)
{
    return new KisBasicDynamicProgramEditor(this);
}

void KisBasicDynamicProgram::fromXML(const QDomElement& elt)
{
    KisDynamicProgram::fromXML(elt);
}

void KisBasicDynamicProgram::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    KisDynamicProgram::toXML(doc, rootElt);
}

//--- KisBasicDynamicProgramFactory ---//

KisBasicDynamicProgramFactory::KisBasicDynamicProgramFactory() :
    KisDynamicProgramFactory("basic", i18n("Basic"))
{
}

KisDynamicProgram* KisBasicDynamicProgramFactory::program(QString name)
{
    return new KisBasicDynamicProgram(name);
}

