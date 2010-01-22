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
#include "SpaceNavigatorEvent.h"
#include <KoToolManager.h>
#include <KoCanvasController.h>
#include <KDebug>
#include <spnav.h>
#include <math.h>

#define SpaceNavigatorDevice_ID "SpaceNavigator"

SpaceNavigatorDevice::SpaceNavigatorDevice( QObject * parent )
: KoInputDeviceHandler( parent, SpaceNavigatorDevice_ID ), m_thread( new SpaceNavigatorPollingThread( this ) )
{
    qRegisterMetaType<Qt::MouseButtons>( "Qt::MouseButtons" );
    qRegisterMetaType<Qt::MouseButton>( "Qt::MouseButton" );
    connect( m_thread, SIGNAL(moveEvent(int,int,int,int,int,int,Qt::MouseButtons)),
            this, SLOT(slotMoveEvent(int,int,int,int,int,int,Qt::MouseButtons)));
    connect( m_thread, SIGNAL(buttonEvent(int,int,int,int,int,int,Qt::MouseButtons,Qt::MouseButton,int)),
            this, SLOT(slotButtonEvent(int,int,int,int,int,int,Qt::MouseButtons,Qt::MouseButton,int)));
}

SpaceNavigatorDevice::~SpaceNavigatorDevice()
{
}

bool SpaceNavigatorDevice::start()
{
    kDebug() << "starting spacenavigator device...";

    if( m_thread->isRunning() )
        return true;

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

void SpaceNavigatorDevice::slotMoveEvent( int x, int y, int z, int rx, int ry, int rz, Qt::MouseButtons buttons )
{
    SpaceNavigatorEvent e( KoInputDeviceHandlerEvent::PositionChanged );
    e.setPosition( x, y, z );
    e.setRotation( rx, ry, rz );
    e.setButton( Qt::NoButton );
    e.setButtons( buttons );
    KoToolManager::instance()->injectDeviceEvent( &e );

    if( ! e.isAccepted() )
    {
        // no tool wants the event, so do some standard actions
        KoCanvasController * controller = KoToolManager::instance()->activeCanvasController();
        // check if the z-movement is dominant
        if( qAbs(z) > qAbs(x) && qAbs(z) > qAbs(y) )
        {
            // zoom
            controller->zoomBy( controller->preferredCenter(), pow(1.01,-z/10) );
        }
        else
        {
            // pan
            controller->pan( QPoint( -x, -y ) );
        }
    }
}

void SpaceNavigatorDevice::slotButtonEvent( int x, int y, int z, int rx, int ry, int rz, Qt::MouseButtons buttons, Qt::MouseButton button, int type )
{
    SpaceNavigatorEvent e( static_cast<KoInputDeviceHandlerEvent::Type>( type ) );
    e.setPosition( x, y, z );
    e.setRotation( rx, ry, rz );
    e.setButton( button );
    e.setButtons( buttons );
    KoToolManager::instance()->injectDeviceEvent( &e );
}
