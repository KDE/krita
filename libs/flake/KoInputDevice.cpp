/*
 *  Copyright (c) 2006 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoInputDevice.h"

#define UNKNOWN_INPUT_DEVICE_ID -1
#define FIRST_INPUT_DEVICE_ID 0

qint32 KoInputDevice::NextInputDeviceID = FIRST_INPUT_DEVICE_ID;

KoInputDevice KoInputDevice::Mouse;
KoInputDevice KoInputDevice::Stylus;
KoInputDevice KoInputDevice::Eraser;
KoInputDevice KoInputDevice::Puck;
KoInputDevice KoInputDevice::Unknown(UNKNOWN_INPUT_DEVICE_ID);

QList<KoInputDevice> KoInputDevice::InputDevices;

KoInputDevice::KoInputDevice()
{
    m_id = UNKNOWN_INPUT_DEVICE_ID;
}

KoInputDevice KoInputDevice::allocateNextDevice()
{
    KoInputDevice inputDevice(NextInputDeviceID);
    NextInputDeviceID++;
    InputDevices.append(inputDevice);

    return inputDevice;
}

KoInputDevice KoInputDevice::allocateInputDevice()
{
    allocateDefaultDevicesIfNeeded();

    return allocateNextDevice();
}

void KoInputDevice::allocateDefaultDevicesIfNeeded()
{
    if (NextInputDeviceID == FIRST_INPUT_DEVICE_ID) {
        Mouse = allocateNextDevice();
        Stylus = allocateNextDevice();
        Eraser = allocateNextDevice();
        Puck = allocateNextDevice();
    }
}

QList<KoInputDevice> KoInputDevice::inputDevices()
{
    allocateDefaultDevicesIfNeeded();

    return InputDevices;
}

KoInputDevice KoInputDevice::mouse()
{
    allocateDefaultDevicesIfNeeded();
    return Mouse;
}

KoInputDevice KoInputDevice::stylus()
{
    allocateDefaultDevicesIfNeeded();
    return Stylus;
}

KoInputDevice KoInputDevice::eraser()
{
    allocateDefaultDevicesIfNeeded();
    return Eraser;
}

KoInputDevice KoInputDevice::puck()
{
    allocateDefaultDevicesIfNeeded();
    return Puck;
}

KoInputDevice KoInputDevice::unknown()
{
    allocateDefaultDevicesIfNeeded();
    return Unknown;
}

