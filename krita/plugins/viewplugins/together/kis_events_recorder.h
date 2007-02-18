/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_EVENTS_RECORDER_H_
#define _KIS_EVENTS_RECORDER_H_

#include <QObject>
#include <QEvent>
#include <QMouseEvent>

struct EventEntry {
    QObject* receiver;
    QEvent::Type type;
    QPoint pos;
    QPoint globalPos;
    Qt::MouseButton button;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
};

class KisEventsRecorder : public QObject {
    Q_OBJECT
    public:
        KisEventsRecorder();
    public:
        void start();
        void stop();
        void replay();
    protected:
        bool eventFilter(QObject *obj, QEvent *event);
    private:
        bool m_recording;
        QList<EventEntry> m_listEvents;
};

#endif
