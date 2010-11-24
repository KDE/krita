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

#include "krscriptline.h"
#include "KoReportItemBase.h"
#include "KoReportPluginManager.h"
#include "KoReportItemLine.h"

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
    return m_section->objectName();
}

QObject* Section::objectByNumber(int i)
{
    if (m_section->m_objects[i]->typeName() == "report:line") {
                return new Scripting::Line(dynamic_cast<KoReportItemLine*>(m_section->m_objects[i]));
    }
    else {
        KoReportPluginManager* manager = KoReportPluginManager::self();
        KoReportPluginInterface *plugin = manager->plugin(m_section->m_objects[i]->typeName());
        if (plugin) {
            QObject *obj = plugin->createScriptInstance(m_section->m_objects[i]);
            if (obj) {
                return obj;
            }
        }
        else {
            kDebug() << "Encountered unknown node while parsing section: " << m_section->m_objects[i]->typeName();
        }
    }

    return new QObject();
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
