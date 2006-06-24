/***************************************************************************
 * list.cpp
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

#include "list.h"
#include "exception.h"
//Added by qt3to4:
#include <Q3ValueList>

using namespace Kross::Api;

List::List(Q3ValueList<Object::Ptr> value)
    : Value< List, Q3ValueList<Object::Ptr> >(value)
{
}

List::~List()
{
}

const QString List::getClassName() const
{
    return "Kross::Api::List";
}

const QString List::toString()
{
    QString s = "[";
    Q3ValueList<Object::Ptr> list = getValue();
    for(Q3ValueList<Object::Ptr>::Iterator it = list.begin(); it != list.end(); ++it)
        s += "'" + (*it)->toString() + "', ";
    return (s.endsWith(", ") ? s.left(s.length() - 2) : s) + "]";
}

Object* List::item(int idx, Object* defaultobject)
{
    Q3ValueList<Object::Ptr>& list = getValue();
    if(idx >= list.count()) {
        if(defaultobject)
            return defaultobject;
        krossdebug( QString("List::item index=%1 is out of bounds. Raising TypeException.").arg(idx) );
        throw Exception::Ptr( new Exception(QString("List-index %1 out of bounds.").arg(idx)) );
    }
    return list[idx].data();
}

uint List::count()
{
    return getValue().count();
}

void List::append(Object::Ptr object)
{
    getValue().append(object);
}

