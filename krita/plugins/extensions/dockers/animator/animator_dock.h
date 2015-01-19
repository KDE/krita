/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _ANIMATOR_DOCK_H_
#define _ANIMATOR_DOCK_H_

#include <QDockWidget>

#include <KoCanvasObserverBase.h>

class QLabel;
class KisCanvas2;
class KisAnimationModel;
class KisTimelineWidget;

/**
 * The animator docker class
 */
class AnimatorDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    AnimatorDock();
    QString observerName() { return "AnimatorDock"; }
    virtual void setCanvas(KoCanvasBase *canvasBase);
    virtual void unsetCanvas();

private:
    KisTimelineWidget *m_timeLine;
};


#endif
