/***************************************************************************
 * eventscript.cpp
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

#include "eventscript.h"
//#include "object.h"
//#include "variant.h"
//#include "../main/scriptcontainer.h"

using namespace Kross::Api;

EventScript::EventScript(const QString& name, Object::Ptr parent)
    : Event<EventScript>(name, parent)
{
}

EventScript::~EventScript()
{
}

const QString EventScript::getClassName() const
{
    return "Kross::Api::EventScript";
}

Object::Ptr EventScript::call(const QString& name, KSharedPtr<List> arguments)
{
    kDebug() << QString("EventScript::call() name=%1 arguments=%2").arg(name).arg(arguments->toString()) << endl;
    //TODO
    return Object::Ptr();
}

