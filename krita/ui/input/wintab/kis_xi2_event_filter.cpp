/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_xi2_event_filter.h"

#include "kis_debug.h"
#include <QGlobalStatic>

//#include <X11/extensions/XInput2.h>
//#include <X11/extensions/XI2proto.h>
#include <xcb/xcb.h>

#include "qxcbconnection_xi2.h"

struct KisXi2EventFilter::Private
{
    QScopedPointer<QXcbConnection> connection;
};


Q_GLOBAL_STATIC(KisXi2EventFilter, s_instance)

KisXi2EventFilter::KisXi2EventFilter()
: m_d(new Private)
{
    m_d->connection.reset(new QXcbConnection(true, ":0"));
}

KisXi2EventFilter::~KisXi2EventFilter()
{
}

bool KisXi2EventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(message);

    uint response_type = event->response_type & ~0x80;

    switch (response_type) {
    case XCB_GE_GENERIC: {
        xcb_ge_event_t *geEvent = reinterpret_cast<xcb_ge_event_t *>(event);

        const int eventSize = sizeof(xcb_ge_event_t) + 4 * geEvent->length;

        // Qt's code *modifies* (!) the supplied event internally!!!
        // And since we reuse Qt's code, we should make a copy of it.

        xcb_ge_event_t *copiedEvent = (xcb_ge_event_t*) malloc(eventSize);
        memcpy(copiedEvent, geEvent, eventSize);
        bool result = m_d->connection->xi2HandleEvent(copiedEvent);
        free(copiedEvent);

        return result;
    }
    case XCB_ENTER_NOTIFY: {
        xcb_enter_notify_event_t *enter = (xcb_enter_notify_event_t *)event;
        m_d->connection->notifyEnterEvent(enter);
    }
    default:
        break;
    }

    return false;
}

KisXi2EventFilter* KisXi2EventFilter::instance()
{
    return s_instance;
}
