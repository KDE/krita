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

#ifndef SPACENAVIGATOREVENT_H
#define SPACENAVIGATOREVENT_H

#include <KoInputDeviceHandlerEvent.h>

class KoPointerEvent;

class SpaceNavigatorEvent : public KoInputDeviceHandlerEvent
{
public:
    SpaceNavigatorEvent( KoInputDeviceHandlerEvent::Type type );
    virtual ~SpaceNavigatorEvent();

    void setPosition( int x, int y, int z );
    void setRotation( int rx, int ry, int rz );

    virtual KoPointerEvent * pointerEvent();

private:
    int m_x, m_y, m_z;
    int m_rx, m_ry, m_rz;
};

#endif // SPACENAVIGATOREVENT_H
