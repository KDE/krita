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

#include "rectedit.h"

#include <KLocale>
#include <QtCore/QRect>

using namespace KoProperty;

static const char *RECTEDIT_MASK = "%1, %2, %3x%4";

QString RectDelegate::displayText( const QVariant& value ) const
{
    const QRect r(value.toRect());
    return QString::fromLatin1(RECTEDIT_MASK)
        .arg(r.x())
        .arg(r.y())
        .arg(r.width())
        .arg(r.height());
}

//------------

RectComposedProperty::RectComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("x",
        QVariant(), i18n("X"), i18n("X"), Int, property);
    (void)new Property("y",
        QVariant(), i18n("Y"), i18n("Y"), Int, property);
    (void)new Property("width",
        QVariant(), i18n("Width"), i18n("Width"), UInt, property);
    (void)new Property("height",
        QVariant(), i18n("Height"), i18n("Height"), UInt, property);
}

void RectComposedProperty::setValue(Property *property,
    const QVariant &value, bool rememberOldValue)
{
    const QRect r( value.toRect() );
    property->child("x")->setValue(r.x(), rememberOldValue, false);
    property->child("y")->setValue(r.y(), rememberOldValue, false);
    property->child("width")->setValue(r.width(), rememberOldValue, false);
    property->child("height")->setValue(r.height(), rememberOldValue, false);
}

void RectComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    QRect r( child->parent()->value().toRect() );

    if (child->name() == "x")
        r.moveLeft(value.toInt());
    else if (child->name() == "y")
        r.moveTop(value.toInt());
    else if (child->name() == "width")
        r.setWidth(value.toInt());
    else if (child->name() == "height")
        r.setHeight(value.toInt());

    child->parent()->setValue(r, true, false);
}
