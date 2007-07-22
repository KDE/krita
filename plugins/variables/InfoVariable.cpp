/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "InfoVariable.h"

#include <KoInlineTextObjectManager.h>
#include <KoProperties.h>
#include <kdebug.h>

InfoVariable::InfoVariable()
    : KoVariable(true),
    m_type(KoInlineObject::DocumentURL)
{
}

void InfoVariable::setProperties(const KoProperties *props) {
    m_type = (Property) props->property("property").value<int>();
    //kDebug() << "Ok, we've got the type " << m_type << endl;
    setValue(manager()->property(m_type).toString());
}

void InfoVariable::propertyChanged(Property property, const QVariant &value) {
    //kDebug() << "property is :" << manager()->stringProperty(m_type) << endl;
    //kDebug() << "Property " << property << " changed to " << value << endl;
    if (property == m_type) {
        setValue(value.toString());
    }
}
