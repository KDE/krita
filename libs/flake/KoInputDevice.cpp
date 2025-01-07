/*
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInputDevice.h"

class Q_DECL_HIDDEN KoInputDevice::Private
{
public:
    Private(KoInputDevice::InputDevice d, KoInputDevice::Pointer p, qint64 id, bool m)
        : device(d),
        pointer(p),
        uniqueTabletId(id),
        mouse(m) {
    }
    KoInputDevice::InputDevice device;
    KoInputDevice::Pointer pointer;
    qint64 uniqueTabletId;
    bool mouse;
};

KoInputDevice::KoInputDevice(KoInputDevice::InputDevice device, KoInputDevice::Pointer pointer, qint64 uniqueTabletId)
    : d(new Private(device, pointer, uniqueTabletId, false))
{
}

KoInputDevice::KoInputDevice()
    : d(new Private(KoInputDevice::InputDevice::Unknown, KoInputDevice::Pointer::Unknown, -1, true))
{
}

KoInputDevice::KoInputDevice(const KoInputDevice &other)
    : d(new Private(other.d->device, other.d->pointer, other.d->uniqueTabletId, other.d->mouse))
{
}


KoInputDevice::~KoInputDevice()
{
    delete d;
}

KoInputDevice::InputDevice KoInputDevice::device() const
{
    return d->device;
}

KoInputDevice::Pointer KoInputDevice::pointer() const
{
    return d->pointer;
}

qint64 KoInputDevice::uniqueTabletId() const
{
    return d->uniqueTabletId;
}

bool KoInputDevice::isMouse() const
{
    // sometimes, the system gives us tablet events with NoDevice or UnknownPointer. This is
    // likely an XInput2 bug. However, assuming that if cannot identify the tablet device we've
    // actually got a mouse is reasonable. See https://bugs.kde.org/show_bug.cgi?id=283130.
    return d->mouse || d->device == KoInputDevice::InputDevice::Unknown
            || d->pointer == KoInputDevice::Pointer::Unknown;
}


bool KoInputDevice::operator==(const KoInputDevice &other) const
{
    return d->device == other.d->device && d->pointer == other.d->pointer &&
        d->uniqueTabletId == other.d->uniqueTabletId && d->mouse == other.d->mouse;
}

KoInputDevice & KoInputDevice::operator=(const KoInputDevice & other)
{
    d->device = other.d->device;
    d->pointer = other.d->pointer;
    d->uniqueTabletId = other.d->uniqueTabletId;
    d->mouse = other.d->mouse;
    return *this;
}

// static
KoInputDevice KoInputDevice::invalid()
{
    KoInputDevice id(KoInputDevice::InputDevice::Unknown, KoInputDevice::Pointer::Unknown);
    return id;
}



KoInputDevice KoInputDevice::mouse()
{
    KoInputDevice id;
    return id;
}

// static
KoInputDevice KoInputDevice::stylus()
{
    KoInputDevice id(KoInputDevice::InputDevice::Stylus, KoInputDevice::Pointer::Pen);
    return id;
}

// static
KoInputDevice KoInputDevice::eraser()
{
    KoInputDevice id(KoInputDevice::InputDevice::Stylus, KoInputDevice::Pointer::Eraser);
    return id;
}

QDebug operator<<(QDebug dbg, const KoInputDevice &device)
{
    if (device.isMouse())
        dbg.nospace() << "mouse";
    else {
        switch (device.pointer()) {
        case KoInputDevice::Pointer::Unknown:
            dbg.nospace() << "unknown pointer";
            break;
        case KoInputDevice::Pointer::Pen:
            dbg.nospace() << "pen";
            break;
        case KoInputDevice::Pointer::Cursor:
            dbg.nospace() << "cursor";
            break;
        case KoInputDevice::Pointer::Eraser:
            dbg.nospace() << "eraser";
            break;
        default:
            dbg.space() << "Unsupported pointer type";
        }
        switch(device.device()) {
        case KoInputDevice::InputDevice::Unknown:
            dbg.space() << "no device";
            break;
        case KoInputDevice::InputDevice::Puck:
            dbg.space() << "puck";
            break;
        case KoInputDevice::InputDevice::Stylus:
            dbg.space() << "stylus";
            break;
        case KoInputDevice::InputDevice::Airbrush:
            dbg.space() << "airbrush";
            break;
        default:
            dbg.space() << "Unsupported device type";
        }
        dbg.space() << "(id: " << device.uniqueTabletId() << ")";
    }
    return dbg.space();
}
