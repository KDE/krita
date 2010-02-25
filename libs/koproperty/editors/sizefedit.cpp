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

#include "sizefedit.h"

#include <KLocale>
#include <QtCore/QSize>

using namespace KoProperty;

static const char *SIZEFEDIT_MASK = "%1x%2";

QString SizeFDelegate::displayText( const QVariant& value ) const
{
    const QSizeF s(value.toSizeF());
    return QString::fromLatin1(SIZEFEDIT_MASK)
        .arg(s.width())
        .arg(s.height());
}

//------------

SizeFComposedProperty::SizeFComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("width", 
        QVariant(), i18n("Width"), i18n("Width"), Double, property);
    (void)new Property("height", 
        QVariant(), i18n("Height"), i18n("Height"), Double, property);
}

void SizeFComposedProperty::setValue(Property *property, 
    const QVariant &value, bool rememberOldValue)
{
    const QSizeF s( value.toSizeF() );
    property->child("width")->setValue(s.width(), rememberOldValue, false);
    property->child("height")->setValue(s.height(), rememberOldValue, false);
}

void SizeFComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    QSizeF s( child->parent()->value().toSizeF() );
    if (child->name() == "width")
        s.setWidth(value.toDouble());
    else if (child->name() == "height")
        s.setHeight(value.toDouble());

    child->parent()->setValue(s, true, false);
}
