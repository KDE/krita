/*
 *  SPDX-FileCopyrightText: 2006 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KO_INPUT_DEVICE_H_
#define KO_INPUT_DEVICE_H_

#include "kritaflake_export.h"

#include <QTabletEvent>
#include <QDebug>
#include <QFlags>
#include <boost/operators.hpp>

/**
 * This class represents an input device.
 * A user can manipulate flake-shapes using a large variety of input devices. This ranges from
 * a mouse to a paintbrush-like tool connected to a tablet. */
class KRITAFLAKE_EXPORT KoInputDevice : public boost::equality_comparable<KoInputDevice>
{
public:

    enum class Pointer {
        Unknown = 0,
        Generic = 0x0001,   // mouse or similar
        Finger = 0x0002,    // touchscreen or pad
        Pen = 0x0004,       // stylus on a tablet
        Eraser = 0x0008,    // eraser end of a stylus
        Cursor = 0x0010,    // digitizer with crosshairs
        AllPointerTypes = 0x7FFF
    };

    enum class InputDevice {
        Unknown = 0x0000,
        Mouse = 0x0001,
        TouchScreen = 0x0002,
        TouchPad = 0x0004,
        Puck = 0x0008,
        Stylus = 0x0010,
        Airbrush = 0x0020,
        Keyboard = 0x1000,
        AllDevices = 0x7FFFFFFF
    };

    static Pointer convertPointerType(QTabletEvent *event)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        switch (event->pointerType()) {
        case QTabletEvent::UnknownPointer: {
            return Pointer::Unknown;
        }
            break;
        case QTabletEvent::Pen: {
            return Pointer::Pen;
        }
            break;
        case QTabletEvent::Cursor: {
            return Pointer::Cursor;
        }
            break;
        case QTabletEvent::Eraser: {
            return Pointer::Eraser;
        }
        default:
            return Pointer::Unknown;
        }
#else
        QPointingDevice::PointerType pointer = event->pointerType();
        switch (pointer) {
        case (QPointingDevice::PointerType::Unknown): {
            return Pointer::Unknown;
        } break;
        case QPointingDevice::PointerType::Generic:{
            return Pointer::Generic;
        } break;
        case QPointingDevice::PointerType::Finger:{
            return Pointer::Finger;
        } break;
        case QPointingDevice::PointerType::Pen:{
            return Pointer::Pen;
        } break;
        case QPointingDevice::PointerType::Eraser:{
            return Pointer::Eraser;
        } break;

        case QPointingDevice::PointerType::Cursor:{
            return Pointer::Cursor;
        } break;
        case QPointingDevice::PointerType::AllPointerTypes:{
            return Pointer::AllPointerTypes;
        } break;
        default:
            return Pointer::Unknown;
        }
#endif
    }

    static InputDevice convertDeviceType(QTabletEvent *event)
    {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        switch (event->deviceType()) {
        case QTabletEvent::NoDevice: {
            return InputDevice::Unknown;
        }
            break;
        case QTabletEvent::Puck: {
            return InputDevice::Puck;
        }
            break;
        case QTabletEvent::Stylus: {
            return InputDevice::Stylus;
        }
            break;
        case QTabletEvent::Airbrush: {
            return InputDevice::Airbrush;
        }
        case QTabletEvent::RotationStylus: {
            // Note: Qt6 no longer has the RotationStylus device type, so we will
            // have to stop supporting it.
            return InputDevice::Stylus;
        }
        default:
            return InputDevice::Unknown;
        }

#else
        QInputDevice::DeviceType deviceType = event->deviceType();
        switch(deviceType) {
        case QInputDevice::DeviceType::Unknown: {
            return InputDevice::Unknown;
        }
        break;
    case QInputDevice::DeviceType::Mouse:{
            return InputDevice::Mouse;
        }
        break;
    case QInputDevice::DeviceType::TouchScreen:
        {
            return InputDevice::TouchScreen;
        }
        break;
    case QInputDevice::DeviceType::TouchPad:
        {
            return InputDevice::TouchPad;
        }
        break;
    case QInputDevice::DeviceType::Puck:
        {
            return InputDevice::Puck;
        }
        break;
    case QInputDevice::DeviceType::Stylus:
        {
            return InputDevice::Stylus;
        }
        break;
    case QInputDevice::DeviceType::Airbrush:
        {
            return InputDevice::Airbrush;
        }
        break;
    case QInputDevice::DeviceType::Keyboard:
        {
            return InputDevice::Keyboard;
        }
        break;
    case QInputDevice::DeviceType::AllDevices:
        {
            return InputDevice::AllDevices;
        }
    default:
            return InputDevice::Unknown;
    }
#endif
    }

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
    explicit KoInputDevice(InputDevice device, Pointer pointer, qint64 uniqueTabletId = -1);

    /**
     * Constructor for the mouse as input device.
     */
    KoInputDevice();

    ~KoInputDevice();

    /**
     * Return the tablet device used
     */
    InputDevice device() const;

    /**
     * Return the pointer used
     */
    Pointer pointer() const;

    /**
     * Return the unique tablet id as registered by QTabletEvents. Note that this
     * id can change randomly, so it's not dependable.
     *
     * See https://bugs.kde.org/show_bug.cgi?id=407659
     */
    qint64 uniqueTabletId() const;

    /**
     * Return if this is a mouse device.
     */
    bool isMouse() const;

    /// equal
    bool operator==(const KoInputDevice&) const;
    /// assignment
    KoInputDevice & operator=(const KoInputDevice &);

    static KoInputDevice invalid();   ///< invalid input device
    static KoInputDevice mouse();     ///< Standard mouse
    static KoInputDevice stylus();    ///< Wacom style/pen
    static KoInputDevice eraser();    ///< Wacom eraser


private:
    class Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KoInputDevice)

KRITAFLAKE_EXPORT QDebug operator<<(QDebug debug, const KoInputDevice &device);

inline uint qHash(const KoInputDevice &key)
{
    return qHash(QString(":%1:%2:%3:%4")
                     .arg(int(key.device()))
                     .arg(int(key.pointer()))
                     .arg(int(key.uniqueTabletId()))
                     .arg(int(key.isMouse())));
}

#endif

