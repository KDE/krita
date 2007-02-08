/*
 *  Copyright (c) 2006 Adrian Page <adrian@pagenet.plus.com>

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
  Boston, MA 02110-1301, USA.
*/

#ifndef KO_INPUT_DEVICE_H_
#define KO_INPUT_DEVICE_H_

#include <QList>
#include <flake_export.h>


class FLAKE_EXPORT KoInputDevice {
public:
    KoInputDevice();

    static KoInputDevice allocateInputDevice();
    static QList<KoInputDevice> inputDevices();

    friend inline bool operator==(const KoInputDevice&, const KoInputDevice&);
    friend inline bool operator!=(const KoInputDevice&, const KoInputDevice&);

    friend inline bool operator<(const KoInputDevice &, const KoInputDevice &);
    friend inline bool operator>(const KoInputDevice &, const KoInputDevice &);

    static KoInputDevice mouse();     // Standard mouse
    static KoInputDevice stylus();    // Wacom stylus via QTabletEvent
    static KoInputDevice eraser();    // Wacom eraser via QTabletEvent
    static KoInputDevice puck();      // Wacom puck via QTabletEvent
    static KoInputDevice unknown();

private:
    KoInputDevice(qint32 id) : m_id(id) {}

    qint32 id() const { return m_id; }

    static void allocateDefaultDevicesIfNeeded();
    static KoInputDevice allocateNextDevice();

private:
     qint32 m_id;

     static qint32 NextInputDeviceID;
     static QList<KoInputDevice> InputDevices;

     static KoInputDevice Mouse;
     static KoInputDevice Stylus;
     static KoInputDevice Eraser;
     static KoInputDevice Puck;
     static KoInputDevice Unknown;
};

inline bool operator==(const KoInputDevice &a, const KoInputDevice &b)
{
    return a.id() == b.id();
}

inline bool operator!=(const KoInputDevice &a, const KoInputDevice &b)
{
    return a.id() != b.id();
}

inline bool operator<(const KoInputDevice &a, const KoInputDevice &b)
{
    return a.id() < b.id();
}


inline bool operator>(const KoInputDevice &a, const KoInputDevice &b)
{
    return a.id() > b.id();
}

#endif // KIS_INPUT_DEVICE_H_

