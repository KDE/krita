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
#include "krshapedata.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <KoShapeRegistry.h>

KRShapeData::KRShapeData(QDomNode & element)
{
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;
    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();
        if (n == "name") {
            m_name->setValue(node.firstChild().nodeValue());
        } else {
            kDebug() << "while parsing label element encountered unknow element: " << n;
        }
    }
}

void KRShapeData::createProperties()
{
    m_set = new KoProperty::Set(0, "Shape");

    QStringList keys;

    keys << KoShapeRegistry::instance()->keys();
    m_shapeType = new KoProperty::Property("ShapeType", keys, keys, "StarShape", "Shape Type");

    m_set->addProperty(m_name);
    m_set->addProperty(m_shapeType);
}

QRectF KRShapeData::_rect()
{
    QRectF r;
    r.setRect(m_pos.toScene().x(), m_pos.toScene().y(), m_size.toScene().width(), m_size.toScene().height());
    return r;
}

// RTTI
int KRShapeData::type() const
{
    return RTTI;
}
int KRShapeData::RTTI = KRObjectData::EntityShape;
KRShapeData * KRShapeData::toShape()
{
    return this;
}

