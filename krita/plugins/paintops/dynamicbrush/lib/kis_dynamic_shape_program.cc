/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "kis_dynamic_shape_program.h"

#include <QDomElement>

#include <kdebug.h>

struct KisDynamicShapeProgram::Private {
};

KisDynamicShapeProgram::KisDynamicShapeProgram(const QString& name, const QString& type) : KisDynamicProgram(name, type), d(new Private) {
}

KisDynamicShapeProgram::~KisDynamicShapeProgram() { delete d; }

class KisDynamicDummyShapeProgram : public KisDynamicShapeProgram {
    public:
        KisDynamicDummyShapeProgram(const QString& name) : KisDynamicShapeProgram(name, "dummy") { }
        virtual void apply( KisDynamicShape* , const KisPaintInformation& ) const { }
        virtual QWidget* createEditor(QWidget* ) { return 0; }
};


//----------- KisDynamicProgramFactory -----------//

struct KisDynamicShapeProgramFactory::Private {
};

KisDynamicShapeProgramFactory::KisDynamicShapeProgramFactory(QString id, QString name) : KisDynamicProgramFactory(id, name), d(0)
{
}

KisDynamicShapeProgramFactory::~KisDynamicShapeProgramFactory()
{
    delete d;
}

KisDynamicProgram* KisDynamicShapeProgramFactory::program(QString name) const
{
    return shapeProgram(name);
}

//----------- KisDynamicProgramsFactory -----------//

#include "kis_dynamic_shape_program_factory_registry.h"

KisDynamicShapeProgramsFactory::~KisDynamicShapeProgramsFactory()
{
}

KisSerializableConfiguration* KisDynamicShapeProgramsFactory::createDefault()
{
    return new KisDynamicDummyShapeProgram("");
}


KisSerializableConfiguration* KisDynamicShapeProgramsFactory::create(const QDomElement& e)
{
    QString type = e.attribute("type", "");
    QString name = e.attribute("name", "");
    KisDynamicShapeProgramFactory* factory = KisDynamicShapeProgramFactoryRegistry::instance()->value( type );
    kDebug() << "Type is : " << type;
    Q_ASSERT(factory);
    KisDynamicShapeProgram* program = factory->shapeProgram( name );
    Q_ASSERT(program);
    program->fromXML(e);
    return program;

}

#include "kis_dynamic_shape_program.moc"
