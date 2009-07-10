/*
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TemplateShape.h"

#include <KoCreateShapesTool.h>
#include <KoProperties.h>

#include <QDomElement>

TemplateShape::TemplateShape(const KoShapeTemplate &shapeTemplate)
    : IconShape(shapeTemplate.icon)
{
    m_shapeTemplate = shapeTemplate;
}

void TemplateShape::visit(KoCreateShapesTool *tool)
{
    tool->setShapeId(m_shapeTemplate.id);
    tool->setShapeProperties(m_shapeTemplate.properties);
}

QString TemplateShape::toolTip()
{
    return m_shapeTemplate.toolTip;
}

void TemplateShape::save(QDomElement &root)
{
    QDomElement t = root.ownerDocument().createElement("template");
    root.appendChild(t);
    t.setAttribute("name", m_shapeTemplate.name);
    t.setAttribute("id", m_shapeTemplate.id);
    t.setAttribute("toolTip", m_shapeTemplate.toolTip);
    t.setAttribute("icon", m_shapeTemplate.icon);
    if (m_shapeTemplate.properties)
        m_shapeTemplate.properties->save(t);
}

// static
TemplateShape *TemplateShape::createShape(const QDomElement &element)
{
    KoShapeTemplate t;
    t.name = element.attribute("name");
    t.id = element.attribute("id");
    t.toolTip = element.attribute("toolTip");
    t.icon = element.attribute("icon");
    if (!element.firstChildElement("property").isNull()) { // has properties
        t.properties = new KoProperties();
        t.properties->load(element);
    }
    return new TemplateShape(t);
}

