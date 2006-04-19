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
//Added by qt3to4:
#include <Q3ValueList>

using namespace Kross::Api;

Callable::Callable(const QString& name, Object* parent, const ArgumentList& arglist)
    : Object(name, parent)
    , m_arglist(arglist)
{
}

Callable::~Callable()
{
}

const QString Callable::getClassName() const
{
    return "Kross::Api::Callable";
}

Object::Ptr Callable::call(const QString& name, List::Ptr arguments)
{
#ifdef KROSS_API_CALLABLE_CALL_DEBUG
    krossdebug( QString("Kross::Api::Callable::call() name=%1 getName()=%2 arguments=%3").arg(name).arg(getName()).arg(arguments ? arguments->toString() : QString("")) );
#endif

    if(name == "get") {
        //checkArguments( ArgumentList() << Argument("Kross::Api::Variant::String") );
        return getChild(arguments);
    }
    else if(name == "has") {
        //checkArguments( ArgumentList() << Argument("Kross::Api::Variant::String") );
        return hasChild(arguments);
    }
    else if(name == "call") {
        //checkArguments( ArgumentList() << Argument("Kross::Api::Variant::String") << Argument("Kross::Api::List", new List( QValueList<Object::Ptr>() )) );
        return callChild(arguments);
    }
    else if(name == "list") {
        //checkArguments( ArgumentList() << Argument("Kross::Api::Variant::String") << Argument("Kross::Api::List", new List( QValueList<Object::Ptr>() )) );
        return getChildrenList(arguments);
    }
    else if(name == "dict") {
        //checkArguments( ArgumentList() << Argument("Kross::Api::Variant::String") << Argument("Kross::Api::List", new List( QValueList<Object::Ptr>() )) );
        return getChildrenDict(arguments);
    }

    return Object::call(name, arguments);
}

#if 0
void Callable::checkArguments(List::Ptr arguments)
{
#ifdef KROSS_API_CALLABLE_CHECKARG_DEBUG
    krossdebug( QString("Kross::Api::Callable::checkArguments() getName()=%1 arguments=%2")
                 .arg(getName()).arg(arguments ? arguments->toString() : QString::null) );
#endif

    Q3ValueList<Object::Ptr>& arglist = arguments->getValue();

    // check the number of parameters passed.
    if(arglist.size() < m_arglist.getMinParams())
        throw Exception::Ptr( new Exception(QString("Too few parameters for callable object '%1'.").arg(getName())) );
    // Don't check if the user passed more arguments as allowed cause it's cheaper
    // to just ignore the additional arguments.
    //if(arglist.size() > m_arglist.getMaxParams())
    //    throw Exception::Ptr( new Exception(QString("Too many parameters for callable object '%1'.").arg(getName())) );

    // check type of passed parameters.
    Q3ValueList<Argument>& farglist = m_arglist;
    Q3ValueList<Argument>::Iterator it = farglist.begin();
    Q3ValueList<Object::Ptr>::Iterator argit = arglist.begin();
    bool argend = ( argit == arglist.end() );
    for(; it != farglist.end(); ++it) {
        //if(! argend)
/*

        if( ! (*it).isVisible() ) {
            // if the argument isn't visibled, we always use the default argument.
            if(argend)
                arglist.append( (*it).getObject() );
            else
                arglist.insert(argit, (*it).getObject());
        }
        else {
            // the argument is visibled and therefore the passed arguments may
            // define the value.

            if(argend) {
*/
            if(argend) {
                // no argument defined, use the default value.
                arglist.append( (*it).getObject() );
            }
            else {

                // Check if the type of the passed argument matches to what we 
                // expect. The given argument could have just the same type like 
                // the expected argument or could be a specialization of it.
                QString fcn = (*it).getClassName(); // expected argument
                QString ocn = (*argit)->getClassName(); // given argument
                if(! ocn.startsWith(fcn)) {
                    if(! (ocn.startsWith("Kross::Api::Variant") && fcn.startsWith("Kross::Api::Variant")))
                        throw Exception::Ptr( new Exception(QString("Callable object '%1' expected parameter of type '%2', but got '%3'").arg(getName()).arg(fcn).arg(ocn)) );
                }
++argit;
argend = ( argit == arglist.end() );
            }
        //}

        //if(! argend)

    }
}
#endif

Object::Ptr Callable::hasChild(List::Ptr args)
{
    return Object::Ptr( new Variant( Object::hasChild(Variant::toString(args->item(0))) ) );
}

Object::Ptr Callable::getChild(List::Ptr args)
{
    QString s = Variant::toString(args->item(0));
    Object::Ptr obj = Object::getChild(s);
    if(! obj)
        throw Exception::Ptr( new Exception(QString("The object '%1' has no child object '%2'").arg(getName()).arg(s)) );
    return obj;
}

Object::Ptr Callable::getChildrenList(List::Ptr)
{
    QStringList list;
    QMap<QString, Object::Ptr> children = getChildren();
    QMap<QString, Object::Ptr>::Iterator it( children.begin() );
    for(; it != children.end(); ++it)
        list.append( it.key() );
    return Object::Ptr( new Variant(list) );
}

Object::Ptr Callable::getChildrenDict(List::Ptr)
{
    return Object::Ptr( new Dict(Object::getChildren()) );
}

Object::Ptr Callable::callChild(List::Ptr args)
{
    return Object::call(Variant::toString(args->item(0)), args);
}
