/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#include "KoEventActionData.h"

class KoEventActionData::Private
{
public:
    Private( KoShape * shape, KoEventAction * eventAction )
    : shape( shape )
    , eventAction( eventAction )
    {}

    KoShape * shape;
    KoEventAction * eventAction;
};

KoEventActionData::KoEventActionData( KoShape * shape, KoEventAction * eventAction )
: d( new Private( shape, eventAction ) )
{
}

KoEventActionData::~KoEventActionData()
{
    delete d;
}

KoShape * KoEventActionData::shape()
{
    return d->shape;
}

KoEventAction * KoEventActionData::eventAction()
{
    return d->eventAction;
}
