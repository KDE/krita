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

#include "kis_dynamic_coloring_program.h"

#include <QDomElement>

#include <kdebug.h>

struct KisDynamicColoringProgram::Private {
};

KisDynamicColoringProgram::KisDynamicColoringProgram(const QString& name, const QString& type) : KisDynamicProgram(name, type), d(new Private) {
}

KisDynamicColoringProgram::~KisDynamicColoringProgram() { delete d; }


class KisDynamicDummyColoringProgram : public KisDynamicProgram {
    public:
        KisDynamicDummyColoringProgram(const QString& name) : KisDynamicProgram(name, "dummy") { }
        virtual void apply( KisDynamicColoring* , const KisPaintInformation& ) const { }
        virtual QWidget* createEditor(QWidget* ) { return 0; }
};


//----------- KisDynamicProgramFactory -----------//

struct KisDynamicColoringProgramFactory::Private {
};

KisDynamicColoringProgramFactory::KisDynamicColoringProgramFactory(QString id, QString name) : KisDynamicProgramFactory(id, name), d(0)
{
}

KisDynamicColoringProgramFactory::~KisDynamicColoringProgramFactory()
{
    delete d;
}

KisDynamicProgram* KisDynamicColoringProgramFactory::program(QString name) const
{
    return coloringProgram(name);
}

//----------- KisDynamicProgramsFactory -----------//

#include "kis_dynamic_coloring_program_factory_registry.h"

KisDynamicColoringProgramsFactory::~KisDynamicColoringProgramsFactory()
{
}

KisSerializableConfiguration* KisDynamicColoringProgramsFactory::createDefault()
{
    return new KisDynamicDummyColoringProgram("");
}


KisSerializableConfiguration* KisDynamicColoringProgramsFactory::create(const QDomElement& e)
{
    QString type = e.attribute("type", "");
    QString name = e.attribute("name", "");
    KisDynamicColoringProgramFactory* factory = KisDynamicColoringProgramFactoryRegistry::instance()->value( type );
    kDebug() << "Type is : " << type;
    Q_ASSERT(factory);
    KisDynamicColoringProgram* program = factory->coloringProgram( name );
    Q_ASSERT(program);
    program->fromXML(e);
    return program;
}

#include "kis_dynamic_coloring_program.moc"
