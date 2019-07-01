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

#include "kritaimage_export.h"

#include <QDockWidget>
#include <kis_mainwindow_observer.h>

#include <QScopedPointer>

class KisCanvas2;
class KisAction;

class TimelineDocker : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    TimelineDocker();
    ~TimelineDocker() override;

    QString observerName() override { return "TimelineDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;

public Q_SLOTS:
    void slotUpdateIcons();
    void slotUpdateFrameCache();

    /// This helps update other views that might need to know about timeline scrubbing
    /// removing this slot will make scrubbing not update opacity keyframes when frame cache
    /// is on
    void slotUIUpdate();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
