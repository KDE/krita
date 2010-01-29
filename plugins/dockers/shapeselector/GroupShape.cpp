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

#include "GroupShape.h"

#include <KoShapeFactoryBase.h>
#include <KoCreateShapesTool.h>

#include <QDomElement>

GroupShape::GroupShape(KoShapeFactoryBase *shapeFactory)
    : IconShape(shapeFactory->icon())
{
    m_shapeFactory = shapeFactory;
}

void GroupShape::visit(KoCreateShapesTool *tool)
{
    tool->setShapeId(m_shapeFactory->id());
    tool->setShapeProperties(0);
}

QString GroupShape::toolTip()
{
    return m_shapeFactory->toolTip();
}

QString GroupShape::groupId() const
{
    return m_shapeFactory->id();
}

void GroupShape::save(QDomElement &root)
{
    Q_UNUSED(root);
}

