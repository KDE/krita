/***************************************************************************
 * script.cpp
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

#include "script.h"
#include "interpreter.h"
#include "action.h"
#include "krossconfig.h"

using namespace Kross;

Script::Script(Interpreter* const interpreter, Action* const action)
    : QObject()
    , ErrorInterface()
    , m_interpreter(interpreter)
    , m_action(action)
    //, m_exception(0)
{
}

Script::~Script()
{
}

#if 0
bool Script::hadException()
{
    return m_exception.data() != 0;
}

Exception* Script::getException()
{
    return m_exception.data();
}

void Script::setException(Exception* e)
{
    m_exception = Exception::Ptr(e);
}

void Script::clearException()
{
    m_exception = Exception::Ptr(0);
}
#endif
