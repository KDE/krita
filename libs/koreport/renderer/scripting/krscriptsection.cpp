/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krscriptsection.h"
#include "krscriptlabel.h"
#include "krscriptfield.h"
#include "krscripttext.h"
#include "krscriptbarcode.h"
#include "krscriptimage.h"
#include "krscriptline.h"
#include "krscriptchart.h"
#include <kdebug.h>

namespace Scripting
{
Section::Section(KRSectionData* sec)
{
    m_section = sec;
    m_scriptObject = 0;
}


Section::~Section()
{
}

QColor Section::backgroundColor()
{
    return m_section->m_backgroundColor->value().value<QColor>();
}

void   Section::setBackgroundColor(const QColor &c)
{
    kDebug() << c.name();
    m_section->m_backgroundColor->setValue(c);
}

qreal Section::height()
{
    return m_section->m_height->value().toDouble();
}

void Section::setHeight(qreal h)
{
    m_section->m_height->setValue(h);
}

QString Section::name()
{
    return m_section->m_name;
}

QObject* Section::objectByNumber(int i)
{
    switch (m_section->m_objects[i]->type()) {
    case KRObjectData::EntityLabel:
        return new Scripting::Label(m_section->m_objects[i]->toLabel());
        break;
    case KRObjectData::EntityField:
        return new Scripting::Field(m_section->m_objects[i]->toField());
        break;
    case KRObjectData::EntityText:
        return new Scripting::Field(m_section->m_objects[i]->toField());
        break;
    case KRObjectData::EntityBarcode:
        return new Scripting::Barcode(m_section->m_objects[i]->toBarcode());
        break;
    case KRObjectData::EntityLine:
        return new Scripting::Line(m_section->m_objects[i]->toLine());
        break;
    case KRObjectData::EntityChart:
        return new Scripting::Chart(m_section->m_objects[i]->toChart());
        break;
    case KRObjectData::EntityImage:
        return new Scripting::Image(m_section->m_objects[i]->toImage());
        break;
    default:
        return new QObject();
    }

}

QObject* Section::objectByName(const QString& n)
{
    for (int i = 0; i < m_section->objects().count(); ++i) {
        if (m_section->m_objects[i]->entityName() == n) {
            return objectByNumber(i);
        }
    }
    return 0;
}

void Section::initialize(Kross::Object::Ptr p)
{
    m_scriptObject = p;
}

void Section::eventOnRender()
{
    if (m_scriptObject)
        m_scriptObject->callMethod("OnRender");
}
}
