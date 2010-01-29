/*
 *  Copyright (c) 2006 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>

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

#include "flake_export.h"

#include <QHash>
#include <QTabletEvent>
#include <QDebug>

/**
 * This class represents an input device.
 * A user can manipulate flake-shapes using a large variety of input devices. This ranges from
 * a mouse to a paintbrush-like tool connected to a tablet.  All of those need to be handled
 * separately and be given their own tool instance to do their work.
 * @see KoToolFactoryBase::inputDeviceAgnostic()
 */
class FLAKE_EXPORT KoInputDevice
{
public:
    /**
     * Copy constructor.
     */
    KoInputDevice(const KoInputDevice &other);

    /**
     * Constructor for a tablet.
     * Create a new input device with one of the many types that the tablet can have.
     * @param device the device as found on a QTabletEvent
     * @param pointer the pointer as found on a QTabletEvent
     * @param uniqueTabletId the uniqueId as found on a QTabletEvent
     */
    explicit KoInputDevice(QTabletEvent::TabletDevice device, QTabletEvent::PointerType pointer, qint64 uniqueTabletId = -1);

    /**
     * Constructor for the mouse as input device.
     */
    KoInputDevice();

    ~KoInputDevice();

    /**
     * Return the tablet device used
     */
    QTabletEvent::TabletDevice device() const;

    /**
     * Return the pointer used
     */
    QTabletEvent::PointerType pointer() const;

    /**
     * Return the unique tablet id as registered by QTabletEvents.
     */
    qint64 uniqueTabletId() const;

    /**
     * Return if this is a mouse device.
     */
    bool isMouse() const;

    /// equal
    bool operator==(const KoInputDevice&) const;
    /// not equal
    bool operator!=(const KoInputDevice&) const;
    /// assignment
    KoInputDevice & operator=(const KoInputDevice &);

    static KoInputDevice mouse();     ///< Standard mouse
    static KoInputDevice stylus();    ///< Wacom style/pen
    static KoInputDevice eraser();    ///< Wacom eraser


private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoInputDevice)

FLAKE_EXPORT QDebug operator<<(QDebug debug, const KoInputDevice &device);

inline uint qHash(const KoInputDevice &key)
{
    return qHash(QString(":%1:%2:%3:%4")
                 .arg(key.device())
                 .arg(key.pointer())
                 .arg(key.uniqueTabletId())
                 .arg(key.isMouse()));
}

#endif

