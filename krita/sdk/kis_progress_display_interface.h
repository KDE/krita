/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_PROGRESS_DISPLAY_INTERFACE_H_
#define KIS_PROGRESS_DISPLAY_INTERFACE_H_

class KisProgressSubject;


namespace KisProgress {

    const int ProgressEventBase = QEvent::User + 42 + 42;

    class UpdateEvent : QCustomEvent {
    
    public:
    
        UpdateEvent(int percent) : QCustomEvent(ProgressEventBase + 1), m_percent(percent) {};
        int m_percent;
    };
    
    class UpdateStageEvent : QCustomEvent {
    
    public:
    
        UpdateStageEvent(const QString & stage, int percent) : QCustomEvent(ProgressEventBase + 2 ), m_stage(stage), m_percent(percent) {};
        QString m_stage;
        int m_percent;
    };
    
    class DoneEvent : QCustomEvent {
        DoneEvent() : QCustomEvent(ProgressEventBase + 3){};
    };
    
    class ErrorEvent : QCustomEvent {
        ErrorEvent() : QCustomEvent(ProgressEventBase + 4){};
    };
    
    class DestroyedEvent: QCustomEvent {
        DestroyedEvent() : QCustomEvent(ProgressEventBase + 5){};
    };


}



class KisProgressDisplayInterface {
public:
    KisProgressDisplayInterface() {}
    virtual ~KisProgressDisplayInterface() {}

    virtual void setSubject(KisProgressSubject *subject, bool modal, bool canCancel) = 0;

private:
    KisProgressDisplayInterface(const KisProgressDisplayInterface&);
    KisProgressDisplayInterface& operator=(const KisProgressDisplayInterface&);
};

#endif // KIS_PROGRESS_DISPLAY_INTERFACE_H_

