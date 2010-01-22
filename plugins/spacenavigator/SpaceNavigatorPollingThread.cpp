/* This file is part of the KDE project
 * Copyright (c) 2008 Hans Bakker <hansmbakker@gmail.com>
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

#include "SpaceNavigatorPollingThread.h"
#include <KoInputDeviceHandlerEvent.h>
#include <KDebug>
#include <spnav.h>

SpaceNavigatorPollingThread::SpaceNavigatorPollingThread( QObject * parent )
: QThread( parent ), m_stopped( false )
{
}

SpaceNavigatorPollingThread::~SpaceNavigatorPollingThread()
{
}

void SpaceNavigatorPollingThread::run()
{
    m_stopped = false;
    if(spnav_open() == -1)
        return;

    qreal posfactor = 0.1;
    int currX = 0, currY = 0, currZ = 0;
    int currRX = 0, currRY = 0, currRZ = 0;
    Qt::MouseButtons buttons = Qt::NoButton;

    kDebug() << "started spacenavigator polling thread";
    while( ! m_stopped )
    {
        spnav_event event;

        if( spnav_poll_event( &event ) )
        {
            if( event.type == SPNAV_EVENT_MOTION )
            {
                /*
                 * The coordinate system of the space navigator is like the following:
                 * x-axis : from left to right
                 * y-axis : from down to up
                 * z-axis : from front to back
                 * We probably want to make sure that the x- and y-axis match Qt widget
                 * coordinate system:
                 * x-axis : from left to right
                 * y-axis : from back to front
                 * The z-axis would then go from up to down in a right handed coordinate system.
                 * z-axis : from up to down
                 */
                //kDebug() << "got motion event: t("<< event.motion.x << event.motion.y << event.motion.z << ") r(" << event.motion.rx << event.motion.ry << event.motion.rz << ")";
                currX = static_cast<int>( posfactor * event.motion.x );
                currY = -static_cast<int>( posfactor * event.motion.z );
                currZ = -static_cast<int>( posfactor * event.motion.y );
                currRX = static_cast<int>( posfactor * event.motion.rx );
                currRY = static_cast<int>( -posfactor * event.motion.rz );
                currRZ = static_cast<int>( -posfactor * event.motion.ry );
                emit moveEvent( currX, currY, currZ, currRX, currRY, currRZ, buttons );
            }
            else
            {
                /* SPNAV_EVENT_BUTTON */
                Qt::MouseButton button = event.button.bnum == 0 ? Qt::LeftButton : Qt::RightButton;
                KoInputDeviceHandlerEvent::Type type;
                if( event.button.press )
                {
                    //kDebug() << "got button press event b(" << event.button.bnum << ")";
                    buttons |= button;
                    type = KoInputDeviceHandlerEvent::ButtonPressed;
                }
                else
                {
                    //kDebug() << "got button release event b(" << event.button.bnum << ")";
                    buttons &= ~button;
                    type = KoInputDeviceHandlerEvent::ButtonReleased;
                }
                emit buttonEvent( currX, currY, currZ, currRX, currRY, currRZ, buttons, button, type );
            }
            spnav_remove_events( event.type );
        }
        msleep(10);
    }

    kDebug() << "finished spacenavigator polling thread";
}

void SpaceNavigatorPollingThread::stop()
{
    m_stopped = true;
}
