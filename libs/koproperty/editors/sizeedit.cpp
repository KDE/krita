/* This file is part of the KDE project
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "sizeedit.h"

#include <KLocale>
#include <QtCore/QSize>

using namespace KoProperty;

static const char *SIZEEDIT_MASK = "%1x%2";

QString SizeDelegate::displayText( const QVariant& value ) const
{
    const QSize s(value.toSize());
    return QString::fromLatin1(SIZEEDIT_MASK)
        .arg(s.width())
        .arg(s.height());
}

//------------

SizeComposedProperty::SizeComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("width", 
        QVariant(), i18n("Width"), i18n("Width"), UInt, property);
    (void)new Property("height", 
        QVariant(), i18n("Height"), i18n("Height"), UInt, property);
}

void SizeComposedProperty::setValue(Property *property, 
    const QVariant &value, bool rememberOldValue)
{
    const QSize s( value.toSize() );
    property->child("width")->setValue(s.width(), rememberOldValue, false);
    property->child("height")->setValue(s.height(), rememberOldValue, false);
}

void SizeComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    QSize s( child->parent()->value().toSize() );
    if (child->name() == "width")
        s.setWidth(value.toInt());
    else if (child->name() == "height")
        s.setHeight(value.toInt());

    child->parent()->setValue(s, true, false);
}
