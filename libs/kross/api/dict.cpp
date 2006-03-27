/***************************************************************************
 * dict.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "dict.h"
//#include "exception.h"

using namespace Kross::Api;

Dict::Dict(const QMap<QString, Object::Ptr> value, const QString& name)
    : Value< List, QMap<QString, Object::Ptr> >(value, name)
{
}

Dict::~Dict()
{
}

const QString Dict::getClassName() const
{
    return "Kross::Api::Dict";
}

const QString Dict::toString()
{
    QString s = "[";
    QMap<QString, Object::Ptr> list = getValue();
    for(QMap<QString, Object::Ptr>::Iterator it = list.begin(); it != list.end(); ++it)
        s += "'" + it.key() + "' = '" + it.data()->toString() + "', ";
    return (s.endsWith(", ") ? s.left(s.length() - 2) : s) + "]";
}

