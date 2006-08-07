/***************************************************************************
 * object.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#include "object.h"
#include "list.h"
#include "variant.h"
//#include "function.h"
#include "event.h"
#include "exception.h"

using namespace Kross::Api;

Object::Object()
    : KShared()
{
#ifdef KROSS_API_OBJECT_CTOR_DEBUG
    krossdebug( QString("Kross::Api::Object::Constructor() name='%1' refcount='%2'").arg(m_name).arg(_KShared_count()) );
#endif
}

Object::~Object()
{
#ifdef KROSS_API_OBJECT_DTOR_DEBUG
    krossdebug( QString("Kross::Api::Object::Destructor() name='%1' refcount='%2'").arg(m_name).arg(_KShared_count()) );
#endif
    //removeAllChildren(); // not needed cause we use KShared to handle ref-couting and freeing.
}

Object::Ptr Object::call(const QString& name, List::Ptr arguments)
{
    Q_UNUSED(arguments);

#ifdef KROSS_API_OBJECT_CALL_DEBUG
    krossdebug( QString("Kross::Api::Object::call(%1) name=%2").arg(name).arg(getName()) );
#endif

    if(name.isEmpty()) // return a self-reference if no functionname is defined.
        return Object::Ptr(this);

    throw Exception::Ptr( new Exception(QString("No callable object named '%2'").arg(name)) );
}

const QString Object::toString()
{
    return "Kross::Api::Object";
}
