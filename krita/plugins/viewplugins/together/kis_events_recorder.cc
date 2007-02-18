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

#include "kis_events_recorder.h"

#include <QApplication>
#include <QEvent>

#include <kdebug.h>

KisEventsRecorder::KisEventsRecorder() : m_recording(false)
{
    
}

bool KisEventsRecorder::eventFilter(QObject *obj, QEvent *event)
{
    if(m_recording)
    {
        if( event->type() == QEvent::MouseButtonDblClick
            or event->type() == QEvent::MouseButtonPress
            or event->type() == QEvent::MouseButtonRelease
            or event->type() == QEvent::MouseMove
            or event->type() ==  QEvent::MouseTrackingChange )
        {
            kDebug() << "event from " << obj << " event " << event << " of type " << event->type() << endl;
            QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent)
            {
                EventEntry ee;
                ee.receiver = obj;
                ee.type = mouseEvent->type();
                ee.pos = mouseEvent->pos();
                ee.globalPos = mouseEvent->globalPos();
                ee.button = mouseEvent->button();
                ee.buttons = mouseEvent->buttons();
                ee.modifiers = mouseEvent->modifiers();
                m_listEvents.append(ee);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void KisEventsRecorder::start()
{
    m_recording = true;
}
void KisEventsRecorder::stop()
{
    m_recording = false;
}
void KisEventsRecorder::replay()
{
    kDebug() << "Replaying" << endl;
    for(QList<EventEntry>::iterator it = m_listEvents.begin(); it != m_listEvents.end(); it++)
    {
        QApplication::instance()->postEvent( it->receiver, new QMouseEvent( it->type, it->pos, it->globalPos, it->button, it->buttons, it->modifiers));
    }
}

#include "kis_events_recorder.moc"
