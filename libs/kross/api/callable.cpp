/***************************************************************************
 * callable.cpp
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

#include "callable.h"
#include "variant.h"
#include "dict.h"

#include "krossconfig.h"

using namespace Kross::Api;

Callable::Callable(const QString& name)
    : Object()
    , m_name(name)
{
}

Callable::~Callable()
{
}

const QString Callable::getName() const
{
    return m_name;
}

const QString Callable::getClassName() const
{
    return "Kross::Api::Callable";
}

bool Callable::hasChild(const QString& name) const
{
    return m_children.contains(name);
}

Object::Ptr Callable::getChild(const QString& name) const
{
    return m_children[name];
}

QMap<QString, Object::Ptr> Callable::getChildren() const
{
    return m_children;
}

bool Callable::addChild(const QString& name, Object* object)
{
#ifdef KROSS_API_OBJECT_ADDCHILD_DEBUG
    krossdebug( QString("Kross::Api::Callable::addChild() object.name='%2' object.classname='%3'").arg(name).arg(object->getClassName()) );
#endif
    return m_children.insert(name, Object::Ptr(object)) != m_children.end();
}

bool Callable::addChild(Callable* object)
{
    return addChild(object->getName(), object);
}

void Callable::removeChild(const QString& name)
{
#ifdef KROSS_API_OBJECT_REMCHILD_DEBUG
    krossdebug( QString("Kross::Api::Callable::removeChild() name='%1'").arg(name) );
#endif
    m_children.remove(name);
}

void Callable::removeAllChildren()
{
#ifdef KROSS_API_OBJECT_REMCHILD_DEBUG
    krossdebug( "Kross::Api::Callable::removeAllChildren()" );
#endif
    m_children.clear();
}

Object::Ptr Callable::call(const QString& name, List::Ptr args)
{
#ifdef KROSS_API_CALLABLE_CALL_DEBUG
    krossdebug( QString("Kross::Api::Callable::call() name=%1 getName()=%2 arguments=%3").arg(name).arg(getName()).arg(args ? args->toString() : QString("")) );
#endif

    if(name.isEmpty()) // return a self-reference if no functionname is defined.
        return Object::Ptr(this);

    // if name is defined try to get the matching child and pass the call to it.
    Object::Ptr object = getChild(name);
    if(object) {
        //TODO handle namespace, e.g. "mychild1.mychild2.myfunction"
        return object->call(name, args);
    }

    if(name == "get") {
        QString s = Variant::toString(args->item(0));
        Object::Ptr obj = getChild(s);
        if(! obj)
            throw Exception::Ptr( new Exception(QString("The object '%1' has no child object '%2'").arg(getName()).arg(s)) );
        return obj;
    }
    else if(name == "has") {
        return Object::Ptr(new Variant( hasChild( Variant::toString(args->item(0)) ) ));
    }
    else if(name == "call") {
        //TODO should we remove first args-item?
        return Object::call(Variant::toString(args->item(0)), args);
    }
    else if(name == "list") {
        QStringList list;
        QMap<QString, Object::Ptr> children = getChildren();
        QMap<QString, Object::Ptr>::Iterator it( children.begin() );
        for(; it != children.end(); ++it)
            list.append( it.key() );
        return Object::Ptr(new Variant(list));
    }
    else if(name == "dict") {
        return Object::Ptr(new Dict( getChildren() ));
    }

    // If there exists no such object return NULL.
    krossdebug( QString("Object '%1' has no callable object named '%2'.").arg(getName()).arg(name) );
    return Object::Ptr(0);
}
