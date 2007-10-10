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

#include "kis_dynamic_program.h"

#include <QDomElement>

#include <kdebug.h>

#include "kis_filters_list_dynamic_program.h"

struct KisDynamicProgram::Private {
    QString name;
    QString type;
};

KisDynamicProgram::KisDynamicProgram(const QString& name, const QString& type) : d(new Private) {
    d->name = name;
    d->type = type;
}

KisDynamicProgram::~KisDynamicProgram() { delete d; }

QString KisDynamicProgram::name() const { return d->name; }
QString KisDynamicProgram::id() const { return d->name; }
QString KisDynamicProgram::type() const { return d->type; }

void KisDynamicProgram::fromXML(const QDomElement& e)
{
    Q_ASSERT(e.attribute("type","") == type());
    d->name = e.attribute("name", "");
}

void KisDynamicProgram::toXML(QDomDocument& /*doc*/, QDomElement& e) const
{
    e.setAttribute("type", type());
    e.setAttribute("name", name());
}

//----------- KisDynamicProgramFactory -----------//

struct KisDynamicProgramFactory::Private {
    QString id;
    QString name;
};

KisDynamicProgramFactory::KisDynamicProgramFactory(QString id, QString name) : d(new Private)
{
    d->id = id;
    d->name = name;
}

KisDynamicProgramFactory::~KisDynamicProgramFactory()
{
}

QString KisDynamicProgramFactory::id() const
{
    return d->id;
}

QString KisDynamicProgramFactory::name() const
{
    return d->name;
}

//----------- KisDynamicProgramsFactory -----------//

#include "kis_dynamic_program_factory_registry.h"

KisDynamicProgramsFactory::~KisDynamicProgramsFactory()
{
}

KisSerializableConfiguration* KisDynamicProgramsFactory::createDefault()
{
    return new KisDynamicDummyProgram("");
}


KisSerializableConfiguration* KisDynamicProgramsFactory::create(const QDomElement& e)
{
    QString type = e.attribute("type", "");
    QString name = e.attribute("name", "");
    KisDynamicProgramFactory* factory = KisDynamicProgramFactoryRegistry::instance()->value( type );
    kDebug() << "Type is : " << type;
    Q_ASSERT(factory);
    KisDynamicProgram* program = factory->program( name );
    Q_ASSERT(program);
    program->fromXML(e);
    return program;
}

#include "kis_dynamic_program.moc"
