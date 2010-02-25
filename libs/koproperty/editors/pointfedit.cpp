/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2010 Adam Pigg <adam@piggz.co.uk>

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

#include "pointfedit.h"

#include <KLocale>
#include <QtCore/QPointF>

using namespace KoProperty;

static const char *POINTFEDIT_MASK = "%1, %2";

QString PointFDelegate::displayText( const QVariant& value ) const
{
    const QPointF p(value.toPointF());
    return QString::fromLatin1(POINTFEDIT_MASK)
        .arg(p.x())
        .arg(p.y());
}

//------------

PointFComposedProperty::PointFComposedProperty(Property *property)
        : ComposedPropertyInterface(property)
{
    (void)new Property("x",
        QVariant(), i18n("X"), i18n("X"), Double, property);
    (void)new Property("y",
        QVariant(), i18n("Y"), i18n("Y"), Double, property);
}

void PointFComposedProperty::setValue(Property *property, 
    const QVariant &value, bool rememberOldValue)
{
    const QPointF p( value.toPointF() );
    property->child("x")->setValue(p.x(), rememberOldValue, false);
    property->child("y")->setValue(p.y(), rememberOldValue, false);
}

void PointFComposedProperty::childValueChanged(Property *child,
    const QVariant &value, bool rememberOldValue)
{
    QPointF p( child->parent()->value().toPointF() );

    if (child->name() == "x")
        p.setX(value.toDouble());
    else if (child->name() == "y")
        p.setY(value.toDouble());

    child->parent()->setValue(p, true, false);
}
