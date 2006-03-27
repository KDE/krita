/***************************************************************************
 * script.cpp
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

#include "script.h"
#include "object.h"
#include "list.h"
#include "interpreter.h"
#include "exception.h"
#include "../main/scriptcontainer.h"

using namespace Kross::Api;

Script::Script(Interpreter* const interpreter, ScriptContainer* const scriptcontainer)
    : m_interpreter(interpreter)
    , m_scriptcontainer(scriptcontainer)
    , m_exception(0)
{
}

Script::~Script()
{
}

bool Script::hadException()
{
    return m_exception != 0;
}

Exception::Ptr Script::getException()
{
    return m_exception;
}

void Script::setException(Exception::Ptr e)
{
    m_exception = e;
}

void Script::clearException()
{
    m_exception = 0;
}

