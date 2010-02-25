/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
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

#ifndef KPROPERTY_SIZEFEDIT_H
#define KPROPERTY_SIZEFEDIT_H

#include "koproperty/Factory.h"

namespace KoProperty
{

class KOPROPERTY_EXPORT SizeFComposedProperty : public ComposedPropertyInterface
{
public:
    explicit SizeFComposedProperty(Property *parent);

    virtual void setValue(Property *property, 
        const QVariant &value, bool rememberOldValue);

    virtual void childValueChanged(Property *child, 
        const QVariant &value, bool rememberOldValue);
};

class KOPROPERTY_EXPORT SizeFDelegate : public LabelCreator, 
                                       public ComposedPropertyCreator<SizeFComposedProperty>
{
public:
    SizeFDelegate() {}
    virtual QString displayText( const QVariant& value ) const;
};

}

#endif
