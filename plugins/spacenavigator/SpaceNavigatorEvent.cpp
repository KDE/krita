/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SpaceNavigatorEvent.h"
#include <KoPointerEvent.h>

SpaceNavigatorEvent::SpaceNavigatorEvent( KoInputDeviceHandlerEvent::Type type )
: KoInputDeviceHandlerEvent( type )
{
}

SpaceNavigatorEvent::~SpaceNavigatorEvent()
{
}

void SpaceNavigatorEvent::setPosition( int x, int y, int z )
{
    m_x = x;
    m_y = y;
    m_z = z;
}

void SpaceNavigatorEvent::setRotation( int rx, int ry, int rz )
{
    m_rx = rx;
    m_ry = ry;
    m_rz = rz;
}

KoPointerEvent * SpaceNavigatorEvent::pointerEvent()
{
    if( ! m_event )
        m_event = new KoPointerEvent( this, m_x, m_y, m_z, m_rx, m_ry, m_rz );
    return m_event;
}
