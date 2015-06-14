/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _TIMELINE_DOCKER_H_
#define _TIMELINE_DOCKER_H_

#include "krita_export.h"

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

#include "kis_timeline_model.h"
#include "timeline_widget.h"

class KisCanvas2;
class Ui_WdgTimeline;

class TimelineDocker : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    TimelineDocker();
    QString observerName() { return "TimelineDocker"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

private slots:

private:

    KisCanvas2 *m_canvas;

    KisTimelineModel *m_model;
    TimelineWidget *m_timelineWidget;
};


#endif
