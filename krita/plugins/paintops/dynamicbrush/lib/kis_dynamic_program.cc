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

// #include "kis_filters_list_dynamic_program.h"

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

void KisDynamicProgram::toXML(QDomDocument& doc, QDomElement& e) const
{
    e.setAttribute("type", type());
    e.setAttribute("name", name());
}

//----------- KisDynamicProgramsFactory -----------//

KisDynamicProgramsFactory::~KisDynamicProgramsFactory()
{
}

KisSerializableConfiguration* KisDynamicProgramsFactory::create(const QDomElement& e)
{
    KisDynamicProgram* program = 0;
    QString type = e.attribute("type","");
    if(type == "filterslist") {
//         program = new KisFiltersListDynamicProgram("");
        program = 0;
    } else {
        return new KisDynamicDummyProgram("");
    }
    Q_ASSERT(program);
    program->fromXML(e);
    return program;
}
