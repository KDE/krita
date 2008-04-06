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

#include "SpaceNavigatorDevice.h"
#include "SpaceNavigatorPollingThread.h"
#include <KDebug>
#include <spnav.h>

#define SpaceNavigatorDevice_ID "SpaceNavigator"

SpaceNavigatorDevice::SpaceNavigatorDevice( QObject * parent )
: KoDevice( parent, SpaceNavigatorDevice_ID ), m_thread( new SpaceNavigatorPollingThread( this ) )
{
}

SpaceNavigatorDevice::~SpaceNavigatorDevice()
{
}

bool SpaceNavigatorDevice::start()
{
    kDebug() << "starting spacenavigator device...";

    if( m_thread->isRunning() )
        return true;

    if(spnav_open() == -1)
        return false;

    m_thread->start();

    return true;
}

bool SpaceNavigatorDevice::stop()
{
    kDebug() << "stopping spacenavigator device...";

    if( ! m_thread->isRunning() )
        return true;

    m_thread->stop();

    if( ! m_thread->wait( 500 ) )
        m_thread->terminate();

    spnav_close();

    return true;
}
