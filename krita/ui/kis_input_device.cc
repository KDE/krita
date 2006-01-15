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

#include "kis_input_device.h"

#define UNKNOWN_INPUT_DEVICE_ID -1
#define FIRST_INPUT_DEVICE_ID 0

Q_INT32 KisInputDevice::NextInputDeviceID = FIRST_INPUT_DEVICE_ID;

KisInputDevice KisInputDevice::Mouse;
KisInputDevice KisInputDevice::Stylus;
KisInputDevice KisInputDevice::Eraser;
KisInputDevice KisInputDevice::Puck;
KisInputDevice KisInputDevice::Unknown(UNKNOWN_INPUT_DEVICE_ID);

QValueVector<KisInputDevice> KisInputDevice::InputDevices;

KisInputDevice::KisInputDevice()
{
    m_id = UNKNOWN_INPUT_DEVICE_ID;
}

KisInputDevice KisInputDevice::allocateNextDevice()
{
    KisInputDevice inputDevice(NextInputDeviceID);
    NextInputDeviceID++;
    InputDevices.append(inputDevice);

    return inputDevice;
}

KisInputDevice KisInputDevice::allocateInputDevice()
{
    allocateDefaultDevicesIfNeeded();

    return allocateNextDevice();
}

void KisInputDevice::allocateDefaultDevicesIfNeeded()
{
    if (NextInputDeviceID == FIRST_INPUT_DEVICE_ID) {
        Mouse = allocateNextDevice();
        Stylus = allocateNextDevice();
        Eraser = allocateNextDevice();
        Puck = allocateNextDevice();
    }
}

QValueVector<KisInputDevice> KisInputDevice::inputDevices()
{
    allocateDefaultDevicesIfNeeded();

    return InputDevices;
}

KisInputDevice KisInputDevice::mouse()
{
    allocateDefaultDevicesIfNeeded();
    return Mouse;
}

KisInputDevice KisInputDevice::stylus()
{
    allocateDefaultDevicesIfNeeded();
    return Stylus;
}

KisInputDevice KisInputDevice::eraser()
{
    allocateDefaultDevicesIfNeeded();
    return Eraser;
}

KisInputDevice KisInputDevice::puck()
{
    allocateDefaultDevicesIfNeeded();
    return Puck;
}

KisInputDevice KisInputDevice::unknown()   
{
    allocateDefaultDevicesIfNeeded();
    return Unknown;
}

