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

#ifndef SPACENAVIGATORDEVICE_H
#define SPACENAVIGATORDEVICE_H

#include <KoInputDeviceHandler.h>

class SpaceNavigatorPollingThread;

class SpaceNavigatorDevice : public KoInputDeviceHandler
{
    Q_OBJECT
public:
    SpaceNavigatorDevice( QObject * parent );
    virtual ~SpaceNavigatorDevice();

    /// reimplemented from KoInputDeviceHandler
    virtual bool start();
    /// reimplemented from KoInputDeviceHandler
    virtual bool stop();
private slots:
    void slotMoveEvent( int x, int y, int z, int rx, int ry, int rz, Qt::MouseButtons );
    void slotButtonEvent( int x, int y, int z, int rx, int ry, int rz, Qt::MouseButtons, Qt::MouseButton, int type );

private:
    SpaceNavigatorPollingThread * m_thread;
};

#endif // SPACENAVIGATORDEVICE_H
