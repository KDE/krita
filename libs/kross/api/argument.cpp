/***************************************************************************
 * argument.cpp
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

#include "argument.h"

using namespace Kross::Api;

Argument::Argument(const QString& classname /*, Object::Ptr object, bool visible*/)
    : m_classname(classname)
    //, m_object(object)
    //, m_visible(visible)
{
}

Argument::~Argument()
{
}

const QString& Argument::getClassName()
{
    return m_classname;
}

/*
Object::Ptr Argument::getObject() const
{
    return m_object;
}

bool Argument::isVisible() const
{
    return m_visible;
}
*/

ArgumentList& Argument::operator << (ArgumentList& arglist)
{
    arglist << *this;
    return arglist;
}

ArgumentList::ArgumentList()
    //: m_minparams(0)
    //, m_maxparams(0)
{
}

ArgumentList::~ArgumentList()
{
}

ArgumentList& ArgumentList::operator << (const Argument& arg)
{
/*
    if(arg.isVisible()) {
        if(! arg.getObject())
            m_minparams++;
        m_maxparams++;
    }
*/
    m_arguments.append(arg);
    return *this;
}

/*
uint ArgumentList::getMinParams() const
{
    return m_minparams;
}

uint ArgumentList::getMaxParams() const
{
    return m_maxparams;
}

QString ArgumentList::toString()
{
    QString s;
    for(QValueList<Argument>::iterator it = m_arguments.begin(); it != m_arguments.end(); ++it)
        s += (*it).getClassName() + ", ";
    return s;
}
*/
