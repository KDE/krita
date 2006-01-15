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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_INPUT_DEVICE_H_
#define KIS_INPUT_DEVICE_H_

#include <qvaluevector.h>

class KisInputDevice {
public:
    KisInputDevice();

    static KisInputDevice allocateInputDevice();
    static QValueVector<KisInputDevice> inputDevices();

    friend inline bool operator==(const KisInputDevice&, const KisInputDevice&);
    friend inline bool operator!=(const KisInputDevice&, const KisInputDevice&);

    friend inline bool operator<(const KisInputDevice &, const KisInputDevice &);
    friend inline bool operator>(const KisInputDevice &, const KisInputDevice &);

    static KisInputDevice mouse();     // Standard mouse                
    static KisInputDevice stylus();    // Wacom stylus via QTabletEvent 
    static KisInputDevice eraser();    // Wacom eraser via QTabletEvent 
    static KisInputDevice puck();      // Wacom puck via QTabletEvent   
    static KisInputDevice unknown();   

private:
    KisInputDevice(Q_INT32 id) : m_id(id) {}

    Q_INT32 id() const { return m_id; }

    static void allocateDefaultDevicesIfNeeded();
    static KisInputDevice allocateNextDevice();

private:
     Q_INT32 m_id;

     static Q_INT32 NextInputDeviceID;
     static QValueVector<KisInputDevice> InputDevices;

     static KisInputDevice Mouse;
     static KisInputDevice Stylus;
     static KisInputDevice Eraser;
     static KisInputDevice Puck;
     static KisInputDevice Unknown;
};

inline bool operator==(const KisInputDevice &a, const KisInputDevice &b)
{ 
    return a.id() == b.id(); 
}

inline bool operator!=(const KisInputDevice &a, const KisInputDevice &b)
{ 
    return a.id() != b.id();
}

inline bool operator<(const KisInputDevice &a, const KisInputDevice &b)
{
    return a.id() < b.id();
}


inline bool operator>(const KisInputDevice &a, const KisInputDevice &b)
{
    return a.id() > b.id();
}

#endif // KIS_INPUT_DEVICE_H_

