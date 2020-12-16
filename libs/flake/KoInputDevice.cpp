/*
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoInputDevice.h"

class Q_DECL_HIDDEN KoInputDevice::Private
{
public:
    Private(QTabletEvent::TabletDevice d, QTabletEvent::PointerType p, qint64 id, bool m)
            : device(d),
            pointer(p),
            uniqueTabletId(id),
            mouse(m) {
    }
    QTabletEvent::TabletDevice device;
    QTabletEvent::PointerType pointer;
    qint64 uniqueTabletId;
    bool mouse;
};

KoInputDevice::KoInputDevice(QTabletEvent::TabletDevice device, QTabletEvent::PointerType pointer, qint64 uniqueTabletId)
        : d(new Private(device, pointer, uniqueTabletId, false))
{
}

KoInputDevice::KoInputDevice()
        : d(new Private(QTabletEvent::NoDevice, QTabletEvent::UnknownPointer, -1, true))
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

QTabletEvent::TabletDevice KoInputDevice::device() const
{
    return d->device;
}

QTabletEvent::PointerType KoInputDevice::pointer() const
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
    return d->mouse || d->device == QTabletEvent::NoDevice || d->pointer == QTabletEvent::UnknownPointer;
}


bool KoInputDevice::operator==(const KoInputDevice &other) const
{
    return d->device == other.d->device && d->pointer == other.d->pointer &&
           d->uniqueTabletId == other.d->uniqueTabletId && d->mouse == other.d->mouse;
}

bool KoInputDevice::operator!=(const KoInputDevice &other) const
{
    return !(operator==(other));
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
    KoInputDevice id(QTabletEvent::NoDevice, QTabletEvent::UnknownPointer);
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
    KoInputDevice id(QTabletEvent::Stylus, QTabletEvent::Pen);
    return id;
}

// static
KoInputDevice KoInputDevice::eraser()
{
    KoInputDevice id(QTabletEvent::Stylus, QTabletEvent::Eraser);
    return id;
}

QDebug operator<<(QDebug dbg, const KoInputDevice &device)
{
    if (device.isMouse())
        dbg.nospace() << "mouse";
    else {
        switch (device.pointer()) {
        case QTabletEvent::UnknownPointer:
            dbg.nospace() << "unknown pointer";
            break;
        case QTabletEvent::Pen:
            dbg.nospace() << "pen";
            break;
        case QTabletEvent::Cursor:
            dbg.nospace() << "cursor";
            break;
        case QTabletEvent::Eraser:
            dbg.nospace() << "eraser";
            break;
        }
        switch(device.device()) {
        case QTabletEvent::NoDevice:
            dbg.space() << "no device";
            break;
        case QTabletEvent::Puck:
            dbg.space() << "puck";
            break;
        case QTabletEvent::Stylus:
            dbg.space() << "stylus";
            break;
        case QTabletEvent::Airbrush:
            dbg.space() << "airbrush";
            break;
        case QTabletEvent::FourDMouse:
            dbg.space() << "four2mouse";
            break;
        case QTabletEvent::RotationStylus:
            dbg.space() << "rotationstylus";
            break;
        case QTabletEvent::XFreeEraser:
            dbg.space() << "XFreeEraser";
            break;
        }
        dbg.space() << "(id: " << device.uniqueTabletId() << ")";
    }
    return dbg.space();
}
