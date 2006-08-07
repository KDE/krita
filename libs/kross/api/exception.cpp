/***************************************************************************
 * exception.cpp
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

#include "exception.h"

//#include <QString>
//#include <ksharedptr.h>

using namespace Kross::Api;

Exception::Exception(const QString& error, long lineno)
    : Object()
    , m_error(error)
    , m_lineno(lineno)
{
    krosswarning( QString("Kross::Api::Exception error='%1' lineno='%3'").arg(m_error).arg(m_lineno) );
}

Exception::~Exception()
{
}

const QString Exception::toString()
{
    return (m_lineno != -1)
        ? QString("Exception at line %1: %2").arg(m_lineno).arg(m_error)
        : QString("Exception: %1").arg(m_error);
}

const QString Exception::getError() const
{
    return m_error;
}

const QString Exception::getTrace() const
{
    return m_trace;
}

void Exception::setTrace(const QString& tracemessage)
{
    m_trace = tracemessage;
}

long Exception::getLineNo() const
{
    return m_lineno;
}

